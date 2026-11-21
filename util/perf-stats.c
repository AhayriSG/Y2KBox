/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "qemu/osdep.h"
#include "qemu/atomic.h"
#include "qemu/perf-stats.h"

#define QEMU_PERF_COMMAND_MAX 4096

typedef struct QemuPerfStatCounter {
    uint64_t count;
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
    [QEMU_PERF_VMVGA_DISPLAY_UPDATE] = "vmvga.display_update",
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
                " total_us=%" PRIu64 " max_us=%" PRIu64
                " over_1ms=%" PRIu64 " over_10ms=%" PRIu64
                " over_100ms=%" PRIu64 "\n",
                perf_stat_names[i], count,
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
