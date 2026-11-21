/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef QEMU_PERF_STATS_H
#define QEMU_PERF_STATS_H

#include <stdint.h>

/*
 * Opt-in, low-volume timing for the Y2KBox scene-stall investigation.
 * The counters are inert unless the VMVGA diagnostic capture is active.
 * Timings use the host monotonic clock and are aggregated across threads.
 */
typedef enum QemuPerfStat {
    QEMU_PERF_CPU_EXEC,
    QEMU_PERF_CPU_EXEC_GAP,
    QEMU_PERF_TB_FLUSH,
    QEMU_PERF_TB_INVALIDATE_RANGE,
    QEMU_PERF_TLB_RESET_DIRTY,
    QEMU_PERF_VMVGA_FIFO,
    QEMU_PERF_VMVGA_COMMAND_DISPATCH,
    QEMU_PERF_VMVGA_DISPLAY_UPDATE,
    QEMU_PERF_DXVK_SHADER_CREATE,
    QEMU_PERF_AUDIO_MIX_OUT,
    QEMU_PERF_AUDIO_MIX_OUT_GAP,
    QEMU_PERF_AUDIO_SDL_CALLBACK_OUT,
    QEMU_PERF_AUDIO_SDL_CALLBACK_GAP,
    QEMU_PERF_STAT__MAX
} QemuPerfStat;

void qemu_perf_stats_start(void);
void qemu_perf_stats_stop(void);
uint64_t qemu_perf_stats_begin(QemuPerfStat stat);
void qemu_perf_stats_end(QemuPerfStat stat, uint64_t start_ns);
void qemu_perf_stats_mark(QemuPerfStat stat);
void qemu_perf_stats_command_end(uint32_t command_id,
                                 const char *command_name,
                                 uint64_t start_ns);

#endif
