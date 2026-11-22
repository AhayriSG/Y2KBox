/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "qemu/osdep.h"
#include "qemu/atomic.h"
#include "qemu/perf-stats.h"

#define QEMU_PERF_COMMAND_MAX 4096

typedef struct QemuPerfStatCounter {
    uint64_t count;
    uint64_t work_units;
    uint64_t max_work_units;
    uint64_t total_ns;
    uint64_t max_ns;
    uint64_t over_1ms;
    uint64_t over_10ms;
    uint64_t over_100ms;
} QemuPerfStatCounter;

static QemuPerfStatCounter perf_stats[QEMU_PERF_STAT__MAX];
static QemuPerfStatCounter perf_command_stats[QEMU_PERF_COMMAND_MAX];
static const char *perf_command_names[QEMU_PERF_COMMAND_MAX];
static bool perf_stats_enabled;
static uint64_t perf_stats_start_ns;
static uint64_t perf_stats_last_mark_ns[QEMU_PERF_STAT__MAX];

static const char *const perf_stat_names[QEMU_PERF_STAT__MAX] = {
    [QEMU_PERF_CPU_EXEC] = "cpu.exec",
    [QEMU_PERF_CPU_EXEC_GAP] = "cpu.exec_gap",
    [QEMU_PERF_TB_FLUSH] = "tcg.tb_flush",
    [QEMU_PERF_TB_INVALIDATE_RANGE] = "tcg.tb_invalidate_range",
    [QEMU_PERF_TLB_RESET_DIRTY] = "tcg.tlb_reset_dirty",
    [QEMU_PERF_VMVGA_FIFO] = "vmvga.fifo_run",
    [QEMU_PERF_VMVGA_COMMAND_DISPATCH] = "vmvga.command_dispatch",
    [QEMU_PERF_GART_MAP_RESOLVE] = "vmvga.gart_map_resolve",
    [QEMU_PERF_GART_MAP_ALIAS] = "vmvga.gart_map_alias",
    [QEMU_PERF_GART_MAP_COMMIT] = "vmvga.gart_map_commit",
    [QEMU_PERF_GART_UNMAP_ALIAS] = "vmvga.gart_unmap_alias",
    [QEMU_PERF_GART_UNMAP_COMMIT] = "vmvga.gart_unmap_commit",
    [QEMU_PERF_MEMORY_TXN_COALESCED_FLUSH] = "memory.txn_coalesced_flush",
    [QEMU_PERF_MEMORY_TXN_FLATVIEWS_RESET] = "memory.txn_flatviews_reset",
    [QEMU_PERF_MEMORY_TXN_LISTENER_BEGIN] = "memory.txn_listener_begin",
    [QEMU_PERF_MEMORY_TXN_ADDRESS_SPACE_FLATVIEW] = "memory.txn_address_space_flatview",
    [QEMU_PERF_MEMORY_TXN_IOEVENTFD_UPDATE] = "memory.txn_ioeventfd_update",
    [QEMU_PERF_MEMORY_TXN_LISTENER_COMMIT] = "memory.txn_listener_commit",
    [QEMU_PERF_TOPOLOGY_TOTAL] = "memory.topology_total",
    [QEMU_PERF_TOPOLOGY_RENDER] = "memory.topology_render",
    [QEMU_PERF_TOPOLOGY_SIMPLIFY] = "memory.topology_simplify",
    [QEMU_PERF_TOPOLOGY_DISPATCH_BUILD] = "memory.topology_dispatch_build",
    [QEMU_PERF_TOPOLOGY_DISPATCH_COMPACT] = "memory.topology_dispatch_compact",
    [QEMU_PERF_TOPOLOGY_FLAT_RANGES] = "memory.topology_flat_ranges",
    [QEMU_PERF_TOPOLOGY_SIMPLIFIED_RANGES] = "memory.topology_simplified_ranges",
    [QEMU_PERF_TOPOLOGY_ROOTS] = "memory.topology_roots",
    [QEMU_PERF_FLATVIEW_INSERT] = "memory.flatview_insert",
    [QEMU_PERF_FLATVIEW_RANGE_REALLOC] = "memory.flatview_range_realloc",
    [QEMU_PERF_FLATVIEW_SCAN] = "memory.flatview_scan",
    [QEMU_PERF_FLATVIEW_MEMMOVE] = "memory.flatview_memmove",
    [QEMU_PERF_FLATVIEW_REF] = "memory.flatview_ref",
    [QEMU_PERF_FLATVIEW_SIZE] = "memory.flatview_size",
    [QEMU_PERF_FLATVIEW_SIMPLIFIED_SIZE] = "memory.flatview_simplified_size",
    [QEMU_PERF_FLATVIEW_SEARCH] = "memory.flatview_search",
    [QEMU_PERF_FLATVIEW_MERGE] = "memory.flatview_merge",
    [QEMU_PERF_FLATVIEW_BATCH_RANGES] = "memory.flatview_batch_ranges",
    [QEMU_PERF_FLATVIEW_TREE_INSERT] = "memory.flatview_tree_insert",
    [QEMU_PERF_FLATVIEW_TREE_FLATTEN] = "memory.flatview_tree_flatten",
    [QEMU_PERF_VMVGA_DISPLAY_UPDATE] = "vmvga.display_update",
    [QEMU_PERF_VMVGA_DISPLAY_FIFO] = "vmvga.display_fifo",
    [QEMU_PERF_VMVGA_DISPLAY_QUERIES] = "vmvga.display_queries",
    [QEMU_PERF_VMVGA_DISPLAY_DEFERRED_FLUSH] = "vmvga.display_deferred_flush",
    [QEMU_PERF_VMVGA_DISPLAY_SCAN_PREP] = "vmvga.display_scan_prep",
    [QEMU_PERF_VMVGA_DISPLAY_DIRTY_SCAN] = "vmvga.display_dirty_scan",
    [QEMU_PERF_VMVGA_DISPLAY_FRONTEND] = "vmvga.display_frontend",
    [QEMU_PERF_VMVGA_DISPLAY_LEGACY_PRESENT] = "vmvga.display_legacy_present",
    [QEMU_PERF_VMVGA_DISPLAY_SCREEN_TARGET] = "vmvga.display_screen_target",
    [QEMU_PERF_VMVGA_CLEAR_ACCEL] = "vmvga.clear_accel",
    [QEMU_PERF_VMVGA_CLEAR_FALLBACK] = "vmvga.clear_fallback",
    [QEMU_PERF_VMVGA_CLEAR_NOTE] = "vmvga.clear_note",
    [QEMU_PERF_VMVGA_SCREEN_TARGET_RETIRE] = "vmvga.screen_target_retire",
    [QEMU_PERF_VMVGA_SCREEN_TARGET_FLUSH_MODE] = "vmvga.screen_target_flush_mode",
    [QEMU_PERF_VMVGA_SCREEN_TARGET_QUIESCE] = "vmvga.screen_target_quiesce",
    [QEMU_PERF_VMVGA_SCREEN_TARGET_ASYNC_POLL] = "vmvga.screen_target_async_poll",
    [QEMU_PERF_VMVGA_CLEAR_MATERIALIZE] = "vmvga.clear_materialize",
    [QEMU_PERF_VMVGA_CLEAR_SHADOW] = "vmvga.clear_shadow",
    [QEMU_PERF_VMVGA_CLEAR_BACKEND] = "vmvga.clear_backend",
    [QEMU_PERF_VMVGA_CLEAR_CPU_SYNC] = "vmvga.clear_cpu_sync",
    [QEMU_PERF_VMVGA_SCREEN_TARGET_ASYNC_SUBMIT] = "vmvga.screen_target_async_submit",
    [QEMU_PERF_VMVGA_SCREEN_TARGET_SYNC_READBACK] = "vmvga.screen_target_sync_readback",
    [QEMU_PERF_VMVGA_SCREEN_TARGET_PRESENT] = "vmvga.screen_target_present",
    [QEMU_PERF_VMVGA_CLEAR_SHADOW_COLOR] = "vmvga.clear_shadow_color",
    [QEMU_PERF_VMVGA_CLEAR_SHADOW_DEPTH] = "vmvga.clear_shadow_depth",
    [QEMU_PERF_VMVGA_D3D9_DRAW_HANDLER] = "vmvga.d3d9_draw_handler",
    [QEMU_PERF_VMVGA_D3D9_DRAW_VALIDATE] = "vmvga.d3d9_draw_validate",
    [QEMU_PERF_VMVGA_D3D9_DRAW_PREPARE] = "vmvga.d3d9_draw_prepare",
    [QEMU_PERF_VMVGA_D3D9_DRAW_NOTE] = "vmvga.d3d9_draw_note",
    [QEMU_PERF_VMVGA_D3D9_DRAW_TRACE] = "vmvga.d3d9_draw_trace",
    [QEMU_PERF_VMVGA_D3D9_DRAW_TAIL_TOTAL] = "vmvga.d3d9_draw_tail_total",
    [QEMU_PERF_VMVGA_D3D9_DRAW_TAIL_END_SCENE] = "vmvga.d3d9_draw_tail_end_scene",
    [QEMU_PERF_VMVGA_D3D9_DRAW_TAIL_RESET_STATE] = "vmvga.d3d9_draw_tail_reset_state",
    [QEMU_PERF_VMVGA_D3D9_DRAW_TAIL_STATE] = "vmvga.d3d9_draw_tail_state",
    [QEMU_PERF_VMVGA_D3D9_DRAW_TAIL_TRACE] = "vmvga.d3d9_draw_tail_trace",
    [QEMU_PERF_VMVGA_D3D9_DRAW_TOTAL] = "vmvga.d3d9_draw_total",
    [QEMU_PERF_VMVGA_D3D9_DRAW_LAYOUT] = "vmvga.d3d9_draw_layout",
    [QEMU_PERF_VMVGA_D3D9_DRAW_STATE] = "vmvga.d3d9_draw_state",
    [QEMU_PERF_VMVGA_D3D9_DRAW_RESOURCE] = "vmvga.d3d9_draw_resource",
    [QEMU_PERF_VMVGA_D3D9_DRAW_SUBMIT] = "vmvga.d3d9_draw_submit",
    [QEMU_PERF_VMVGA_GB_UPDATE_PRESERVE] = "vmvga.gb_update_preserve",
    [QEMU_PERF_VMVGA_GB_UPDATE_MOB_READ] = "vmvga.gb_update_mob_read",
    [QEMU_PERF_VMVGA_GB_UPDATE_NATIVE_UPLOAD] = "vmvga.gb_update_native_upload",
    [QEMU_PERF_VMVGA_GB_UPDATE_CB_REFRESH] = "vmvga.gb_update_cb_refresh",
    [QEMU_PERF_VMVGA_GB_UPDATE_TOTAL] = "vmvga.gb_update_total",
    [QEMU_PERF_VMVGA_GB_UPDATE_LOOKUP] = "vmvga.gb_update_lookup",
    [QEMU_PERF_VMVGA_GB_UPDATE_LAYOUT] = "vmvga.gb_update_layout",
    [QEMU_PERF_VMVGA_GB_UPDATE_BACKEND] = "vmvga.gb_update_backend",
    [QEMU_PERF_VMVGA_GB_UPDATE_SURFACE_CHANGED] = "vmvga.gb_update_surface_changed",
    [QEMU_PERF_DXVK_SHADER_CREATE] = "dxvk.d3d9_shader_create",
    [QEMU_PERF_AUDIO_MIX_OUT] = "audio.mix_out",
    [QEMU_PERF_AUDIO_MIX_OUT_GAP] = "audio.mix_out_gap",
    [QEMU_PERF_AUDIO_SDL_CALLBACK_OUT] = "audio.sdl_callback_out",
    [QEMU_PERF_AUDIO_SDL_CALLBACK_GAP] = "audio.sdl_callback_gap",
};

void qemu_perf_stats_start(void)
{
    int i;

    for (i = 0; i < QEMU_PERF_STAT__MAX; i++) {
        qatomic_set(&perf_stats[i].count, 0);
        qatomic_set(&perf_stats[i].work_units, 0);
        qatomic_set(&perf_stats[i].max_work_units, 0);
        qatomic_set(&perf_stats[i].total_ns, 0);
        qatomic_set(&perf_stats[i].max_ns, 0);
        qatomic_set(&perf_stats[i].over_1ms, 0);
        qatomic_set(&perf_stats[i].over_10ms, 0);
        qatomic_set(&perf_stats[i].over_100ms, 0);
        qatomic_set(&perf_stats_last_mark_ns[i], 0);
    }
    for (i = 0; i < QEMU_PERF_COMMAND_MAX; i++) {
        qatomic_set(&perf_command_stats[i].count, 0);
        qatomic_set(&perf_command_stats[i].total_ns, 0);
        qatomic_set(&perf_command_stats[i].max_ns, 0);
        qatomic_set(&perf_command_stats[i].over_1ms, 0);
        qatomic_set(&perf_command_stats[i].over_10ms, 0);
        qatomic_set(&perf_command_stats[i].over_100ms, 0);
        qatomic_set(&perf_command_names[i], NULL);
    }
    perf_stats_start_ns = g_get_monotonic_time() * 1000;
    qatomic_set(&perf_stats_enabled, true);
}

uint64_t qemu_perf_stats_begin(QemuPerfStat stat)
{
    if (stat >= QEMU_PERF_STAT__MAX || !qatomic_read(&perf_stats_enabled)) {
        return 0;
    }

    return g_get_monotonic_time() * 1000;
}

static void qemu_perf_counter_record(QemuPerfStatCounter *counter,
                                       uint64_t elapsed_ns)
{
    uint64_t previous;

    qatomic_inc(&counter->count);
    qatomic_add(&counter->total_ns, elapsed_ns);
    if (elapsed_ns >= 1000000) {
        qatomic_inc(&counter->over_1ms);
    }
    if (elapsed_ns >= 10000000) {
        qatomic_inc(&counter->over_10ms);
    }
    if (elapsed_ns >= 100000000) {
        qatomic_inc(&counter->over_100ms);
    }

    previous = qatomic_read(&counter->max_ns);
    while (elapsed_ns > previous) {
        uint64_t observed = qatomic_cmpxchg(&counter->max_ns,
                                             previous, elapsed_ns);
        if (observed == previous) {
            break;
        }
        previous = observed;
    }
}

void qemu_perf_stats_add_work(QemuPerfStat stat, uint64_t amount)
{
    if (stat >= QEMU_PERF_STAT__MAX || !qatomic_read(&perf_stats_enabled)) {
        return;
    }

    qatomic_add(&perf_stats[stat].work_units, amount);
}

static void qemu_perf_stats_record(QemuPerfStat stat, uint64_t elapsed_ns)
{
    qemu_perf_counter_record(&perf_stats[stat], elapsed_ns);
}

void qemu_perf_stats_end(QemuPerfStat stat, uint64_t start_ns)
{
    if (!start_ns || stat >= QEMU_PERF_STAT__MAX) {
        return;
    }

    qemu_perf_stats_record(stat, g_get_monotonic_time() * 1000 - start_ns);
}

void qemu_perf_stats_command_end(uint32_t command_id,
                                 const char *command_name,
                                 uint64_t start_ns)
{
    if (!start_ns || command_id >= QEMU_PERF_COMMAND_MAX) {
        return;
    }

    if (command_name != NULL &&
        qatomic_read(&perf_command_names[command_id]) == NULL) {
        qatomic_cmpxchg(&perf_command_names[command_id], NULL, command_name);
    }

    qemu_perf_counter_record(&perf_command_stats[command_id],
                             g_get_monotonic_time() * 1000 - start_ns);
}

void qemu_perf_stats_record_work(QemuPerfStat stat, uint64_t amount)
{
    uint64_t previous;

    if (stat >= QEMU_PERF_STAT__MAX || !qatomic_read(&perf_stats_enabled)) {
        return;
    }

    qatomic_inc(&perf_stats[stat].count);
    qatomic_add(&perf_stats[stat].work_units, amount);
    previous = qatomic_read(&perf_stats[stat].max_work_units);
    while (amount > previous) {
        uint64_t observed = qatomic_cmpxchg(&perf_stats[stat].max_work_units,
                                             previous, amount);
        if (observed == previous) {
            break;
        }
        previous = observed;
    }
}

void qemu_perf_stats_mark(QemuPerfStat stat)
{
    uint64_t now_ns, previous;

    if (stat >= QEMU_PERF_STAT__MAX || !qatomic_read(&perf_stats_enabled)) {
        return;
    }

    now_ns = g_get_monotonic_time() * 1000;
    previous = qatomic_read(&perf_stats_last_mark_ns[stat]);
    for (;;) {
        uint64_t observed = qatomic_cmpxchg(&perf_stats_last_mark_ns[stat],
                                             previous, now_ns);
        if (observed == previous) {
            break;
        }
        previous = observed;
    }

    if (previous != 0) {
        qemu_perf_stats_record(stat, now_ns - previous);
    }
}

void qemu_perf_stats_stop(void)
{
    uint64_t elapsed_ns;
    int i;

    if (!qatomic_read(&perf_stats_enabled)) {
        return;
    }
    qatomic_set(&perf_stats_enabled, false);
    elapsed_ns = g_get_monotonic_time() * 1000 - perf_stats_start_ns;

    fprintf(stderr, "Y2KBOX-PERF capture_ms=%" PRIu64 "\n",
            elapsed_ns / 1000000);
    for (i = 0; i < QEMU_PERF_STAT__MAX; i++) {
        QemuPerfStatCounter *counter = &perf_stats[i];
        uint64_t count = qatomic_read(&counter->count);

        fprintf(stderr,
                "Y2KBOX-PERF stat=%s count=%" PRIu64
                " work_units=%" PRIu64 " max_work_units=%" PRIu64
                " total_us=%" PRIu64 " max_us=%" PRIu64
                " over_1ms=%" PRIu64 " over_10ms=%" PRIu64
                " over_100ms=%" PRIu64 "\n",
                perf_stat_names[i], count,
                qatomic_read(&counter->work_units),
                qatomic_read(&counter->max_work_units),
                qatomic_read(&counter->total_ns) / 1000,
                qatomic_read(&counter->max_ns) / 1000,
                qatomic_read(&counter->over_1ms),
                qatomic_read(&counter->over_10ms),
                qatomic_read(&counter->over_100ms));
    }
    for (i = 0; i < QEMU_PERF_COMMAND_MAX; i++) {
        QemuPerfStatCounter *counter = &perf_command_stats[i];
        uint64_t count = qatomic_read(&counter->count);

        if (count == 0) {
            continue;
        }
        fprintf(stderr,
                "Y2KBOX-PERF command_id=%d command=%s count=%" PRIu64
                " total_us=%" PRIu64 " max_us=%" PRIu64
                " over_1ms=%" PRIu64 " over_10ms=%" PRIu64
                " over_100ms=%" PRIu64 "\n",
                i,
                qatomic_read(&perf_command_names[i]) != NULL
                    ? qatomic_read(&perf_command_names[i]) : "UNKNOWN",
                count,
                qatomic_read(&counter->total_ns) / 1000,
                qatomic_read(&counter->max_ns) / 1000,
                qatomic_read(&counter->over_1ms),
                qatomic_read(&counter->over_10ms),
                qatomic_read(&counter->over_100ms));
    }
    fprintf(stderr, "Y2KBOX-PERF note=category_times_overlap_do_not_sum\n");
}
