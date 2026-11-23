/*
 * The per-CPU TranslationBlock jump cache.
 *
 *  Copyright (c) 2003 Fabrice Bellard
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ACCEL_TCG_TB_JMP_CACHE_H
#define ACCEL_TCG_TB_JMP_CACHE_H

#include "qemu/rcu.h"
#include "exec/cpu-common.h"

#define TB_JMP_CACHE_BITS 14
#define TB_JMP_CACHE_SIZE (1 << TB_JMP_CACHE_BITS)

/*
 * Invalidated in parallel; all accesses to 'tb' must be atomic.
 * A valid entry is read/written by a single CPU, therefore there is
 * no need for qatomic_rcu_read() and the cached TB metadata fields
 * (tc_ptr, pc, cs_base, flags, cflags) are always consistent with a
 * non-NULL value of 'tb'. Caching them inline avoids chasing a cold
 * TranslationBlock pointer on every jump-cache hit.
 */
typedef struct CPUJumpCacheEntry {
    TranslationBlock *tb;
    const void *tc_ptr;
    uint64_t gen;
    vaddr pc;
    uint64_t cs_base;
    uint32_t flags;
    uint32_t cflags;
} CPUJumpCacheEntry;

typedef struct CPUJumpCache {
    struct rcu_head rcu;
    uint64_t gen;
    CPUJumpCacheEntry array[TB_JMP_CACHE_SIZE];
} CPUJumpCache;

#endif /* ACCEL_TCG_TB_JMP_CACHE_H */
