/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "qemu/osdep.h"
#include "qemu/atomic.h"
#include "qemu/perf-stats.h"

#define QEMU_PERF_COMMAND_MAX 4096
#define QEMU_PERF_PIO_SLOTS 64
#define QEMU_PERF_PIO_TOP 16

typedef struct QemuPerfPioCounter {
    uint32_t key;       /* bit 31 = occupied, bit 0 = is_write, bits 1..16 = port */
    uint64_t count;
    uint64_t total_ns;
    uint64_t max_ns;
    uint64_t over_1ms;
} QemuPerfPioCounter;

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
static QemuPerfPioCounter perf_pio_stats[QEMU_PERF_PIO_SLOTS];
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
    [QEMU_PERF_VMVGA_CLEAR_DEPTH_ONLY_PIXELS] = "vmvga.clear_depth_only_pixels",
    [QEMU_PERF_VMVGA_CLEAR_STENCIL_ONLY_PIXELS] = "vmvga.clear_stencil_only_pixels",
    [QEMU_PERF_VMVGA_CLEAR_DEPTH_STENCIL_PIXELS] = "vmvga.clear_depth_stencil_pixels",
    [QEMU_PERF_DXVK_READBACK_FULL_SURFACE] = "dxvk.readback_full_surface",
    [QEMU_PERF_DXVK_READBACK_STRETCH_RECT] = "dxvk.readback_stretch_rect",
    [QEMU_PERF_DXVK_READBACK_GET_RENDER_TARGET_DATA] = "dxvk.readback_get_render_target_data",
    [QEMU_PERF_DXVK_READBACK_LOCK_COPY] = "dxvk.readback_lock_copy",
    [QEMU_PERF_DXVK_FULL_LOCK] = "dxvk.readback_full_lock",
    [QEMU_PERF_DXVK_FULL_COPY] = "dxvk.readback_full_copy",
    [QEMU_PERF_DXVK_FULL_UNLOCK] = "dxvk.readback_full_unlock",
    [QEMU_PERF_DXVK_TRACE_LOCK] = "dxvk.readback_trace_lock",
    [QEMU_PERF_DXVK_TRACE_COPY] = "dxvk.readback_trace_copy",
    [QEMU_PERF_DXVK_TRACE_UNLOCK] = "dxvk.readback_trace_unlock",
    [QEMU_PERF_DXVK_TRACE_CREATE_RENDER_TARGET] = "dxvk.readback_trace_create_rt",
    [QEMU_PERF_DXVK_TRACE_CREATE_STAGING] = "dxvk.readback_trace_create_staging",
    [QEMU_PERF_DXVK_TRACE_STRETCH_RECT] = "dxvk.readback_trace_stretch",
    [QEMU_PERF_DXVK_TRACE_GET_RENDER_TARGET_DATA] = "dxvk.readback_trace_get_data",
    [QEMU_PERF_DXVK_TRACE_HASH] = "dxvk.readback_trace_hash",
    [QEMU_PERF_DXVK_TRACE_RELEASE] = "dxvk.readback_trace_release",
    [QEMU_PERF_VMVGA_PRESENT_COPY] = "vmvga.present_copy",
    [QEMU_PERF_VMVGA_PRESENT_CONVERT] = "vmvga.present_convert",
    [QEMU_PERF_VMVGA_PRESENT_SOURCE_FORMAT] = "vmvga.present_source_format",
    [QEMU_PERF_VMVGA_CLEAR_DEPTH_FORMAT] = "vmvga.clear_depth_format",
    [QEMU_PERF_DXVK_READBACK_TRACE_BAND] = "dxvk.readback_trace_band",
    [QEMU_PERF_DXVK_READBACK_SECONDARY_COPY] = "dxvk.readback_secondary_copy",
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
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK] = "dxvk.upload_buffer_lock",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_VERTEX] = "dxvk.upload_buffer_lock_vertex",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_INDEX] = "dxvk.upload_buffer_lock_index",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_FULL_RANGE] = "dxvk.upload_buffer_lock_full_range",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_PARTIAL_RANGE] = "dxvk.upload_buffer_lock_partial_range",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_DISCARD] = "dxvk.upload_buffer_lock_discard",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_DISCARD_INVALIDATE] = "dxvk.upload_buffer_lock_discard_invalidate",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_NOOVERWRITE] = "dxvk.upload_buffer_lock_nooverwrite",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_FLAGS_ZERO] = "dxvk.upload_buffer_lock_flags_zero",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_OVERLAP] = "dxvk.upload_buffer_lock_overlap",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_INVALIDATE] = "dxvk.upload_buffer_invalidate",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_SIZE_LE_4K] = "dxvk.upload_buffer_lock_size_le_4k",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_SIZE_LE_64K] = "dxvk.upload_buffer_lock_size_le_64k",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_SIZE_LE_256K] = "dxvk.upload_buffer_lock_size_le_256k",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_LOCK_SIZE_GT_256K] = "dxvk.upload_buffer_lock_size_gt_256k",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_COPY] = "dxvk.upload_buffer_copy",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_UNLOCK] = "dxvk.upload_buffer_unlock",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_DEFERRED] = "dxvk.upload_buffer_deferred",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_FLUSH] = "dxvk.upload_buffer_flush",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_DEFERRED_MERGE] = "dxvk.upload_buffer_deferred_merge",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_FLUSH_DRAW] = "dxvk.upload_buffer_flush_draw",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_FLUSH_EVICT] = "dxvk.upload_buffer_flush_evict",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_FLUSH_REPLACE] = "dxvk.upload_buffer_flush_replace",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_FLUSH_UPDATES] = "dxvk.upload_buffer_flush_updates",
    [QEMU_PERF_DXVK_UPLOAD_BUFFER_DISCARD_SUPERSEDE] = "dxvk.upload_buffer_discard_supersede",
    [QEMU_PERF_DXVK_UPLOAD_TEXTURE_LOCK] = "dxvk.upload_texture_lock",
    [QEMU_PERF_DXVK_UPLOAD_TEXTURE_COPY] = "dxvk.upload_texture_copy",
    [QEMU_PERF_DXVK_UPLOAD_TEXTURE_UNLOCK] = "dxvk.upload_texture_unlock",
    [QEMU_PERF_DXVK_UPLOAD_TEXTURE_UPDATE] = "dxvk.upload_texture_update",
    [QEMU_PERF_TCG_TB_GEN_CODE] = "tcg.tb_gen_code",
    [QEMU_PERF_TCG_TB_EXEC_LOOPS] = "tcg.tb_exec_loops",
    [QEMU_PERF_TCG_CPU_EXCEPTION] = "tcg.cpu_exception",
    [QEMU_PERF_TCG_CPU_INTERRUPT] = "tcg.cpu_interrupt",
    [QEMU_PERF_TCG_TLB_FILL] = "tcg.tlb_fill",
    [QEMU_PERF_TCG_MMIO_BQL_WAIT] = "tcg.mmio_bql_wait",
    [QEMU_PERF_TCG_PIO_IN] = "tcg.pio_in",
    [QEMU_PERF_TCG_PIO_OUT] = "tcg.pio_out",
};

static void (*perf_vcpu_profile_start_hook)(void);
static void (*perf_vcpu_profile_stop_hook)(void);

void qemu_perf_stats_set_vcpu_profile_hooks(void (*on_start)(void),
                                            void (*on_stop)(void))
{
    perf_vcpu_profile_start_hook = on_start;
    perf_vcpu_profile_stop_hook = on_stop;
}

uint64_t qemu_perf_stats_get_count(QemuPerfStat stat)
{
    if (stat >= QEMU_PERF_STAT__MAX) {
        return 0;
    }
    return qatomic_read(&perf_stats[stat].count);
}

uint64_t qemu_perf_stats_get_work_units(QemuPerfStat stat)
{
    if (stat >= QEMU_PERF_STAT__MAX) {
        return 0;
    }
    return qatomic_read(&perf_stats[stat].work_units);
}

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
    for (i = 0; i < QEMU_PERF_PIO_SLOTS; i++) {
        qatomic_set(&perf_pio_stats[i].key, 0);
        qatomic_set(&perf_pio_stats[i].count, 0);
        qatomic_set(&perf_pio_stats[i].total_ns, 0);
        qatomic_set(&perf_pio_stats[i].max_ns, 0);
        qatomic_set(&perf_pio_stats[i].over_1ms, 0);
    }
    perf_stats_start_ns = g_get_monotonic_time() * 1000;
    qatomic_set(&perf_stats_enabled, true);
    if (perf_vcpu_profile_start_hook) {
        perf_vcpu_profile_start_hook();
    }
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

void qemu_perf_stats_pio_end(uint32_t port, bool is_write, uint64_t start_ns)
{
    uint64_t elapsed_ns;
    uint32_t key;
    unsigned idx, step;

    if (!start_ns) {
        return;
    }

    elapsed_ns = g_get_monotonic_time() * 1000 - start_ns;
    qemu_perf_stats_record(is_write ? QEMU_PERF_TCG_PIO_OUT
                                    : QEMU_PERF_TCG_PIO_IN,
                           elapsed_ns);

    key = 0x80000000u | ((port & 0xffffu) << 1) | (is_write ? 1u : 0u);
    idx = ((port * 2654435761u) ^ (is_write ? 0x5au : 0u)) &
          (QEMU_PERF_PIO_SLOTS - 1);
    for (step = 0; step < QEMU_PERF_PIO_SLOTS; step++) {
        QemuPerfPioCounter *slot = &perf_pio_stats[idx];
        uint32_t cur = qatomic_read(&slot->key);

        if (cur == 0) {
            cur = qatomic_cmpxchg(&slot->key, 0, key);
            if (cur == 0) {
                cur = key;
            }
        }
        if (cur == key) {
            uint64_t prev;

            qatomic_inc(&slot->count);
            qatomic_add(&slot->total_ns, elapsed_ns);
            if (elapsed_ns >= 1000000) {
                qatomic_inc(&slot->over_1ms);
            }
            prev = qatomic_read(&slot->max_ns);
            while (elapsed_ns > prev) {
                uint64_t obs = qatomic_cmpxchg(&slot->max_ns, prev, elapsed_ns);
                if (obs == prev) {
                    break;
                }
                prev = obs;
            }
            return;
        }
        idx = (idx + 1) & (QEMU_PERF_PIO_SLOTS - 1);
    }
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
    {
        QemuPerfPioCounter top[QEMU_PERF_PIO_SLOTS];
        int n = 0, j;

        for (i = 0; i < QEMU_PERF_PIO_SLOTS; i++) {
            uint32_t k = qatomic_read(&perf_pio_stats[i].key);
            uint64_t c = qatomic_read(&perf_pio_stats[i].count);
            if (k != 0 && c != 0) {
                top[n].key = k;
                top[n].count = c;
                top[n].total_ns = qatomic_read(&perf_pio_stats[i].total_ns);
                top[n].max_ns = qatomic_read(&perf_pio_stats[i].max_ns);
                top[n].over_1ms = qatomic_read(&perf_pio_stats[i].over_1ms);
                n++;
            }
        }
        for (i = 0; i < n; i++) {
            int best = i;
            for (j = i + 1; j < n; j++) {
                if (top[j].total_ns > top[best].total_ns) {
                    best = j;
                }
            }
            if (best != i) {
                QemuPerfPioCounter tmp = top[i];
                top[i] = top[best];
                top[best] = tmp;
            }
        }
        if (n > QEMU_PERF_PIO_TOP) {
            n = QEMU_PERF_PIO_TOP;
        }
        for (i = 0; i < n; i++) {
            uint32_t port = (top[i].key >> 1) & 0xffffu;
            bool is_write = (top[i].key & 1u) != 0;
            fprintf(stderr,
                    "Y2KBOX-PERF pio_port=0x%04x dir=%s count=%" PRIu64
                    " total_us=%" PRIu64 " max_us=%" PRIu64
                    " over_1ms=%" PRIu64 "\n",
                    port, is_write ? "out" : "in",
                    top[i].count,
                    top[i].total_ns / 1000,
                    top[i].max_ns / 1000,
                    top[i].over_1ms);
        }
    }

    fprintf(stderr, "Y2KBOX-PERF note=category_times_overlap_do_not_sum\n");
    if (perf_vcpu_profile_stop_hook) {
        perf_vcpu_profile_stop_hook();
    }
}
