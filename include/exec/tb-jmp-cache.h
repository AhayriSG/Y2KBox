/*
 * The per-CPU TranslationBlock jump cache.
 *
 *  Copyright (c) 2003 Fabrice Bellard
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef EXEC_TB_JMP_CACHE_H
#define EXEC_TB_JMP_CACHE_H

#include "qemu/rcu.h"
#include "exec/cpu-common.h"

#define TB_JMP_CACHE_BITS 14
#define TB_JMP_CACHE_SIZE (1 << TB_JMP_CACHE_BITS)

#ifdef CONFIG_SOFTMMU
/*
 * Split the 14-bit jump cache index into 8 page-bucket bits (256 buckets,
 * 1 MiB page stride on 4 KiB targets) and 6 intra-page address bits
 * (64 slots per page bucket).  Only the bottom TB_JMP_ADDR_BITS vary for
 * addresses on the same guest page so TLB page invalidations clear one
 * contiguous 64-entry (2 KiB) slice.
 */
#define TB_JMP_ADDR_BITS 6
#define TB_JMP_PAGE_BITS (TB_JMP_CACHE_BITS - TB_JMP_ADDR_BITS)
#define TB_JMP_PAGE_SIZE (1 << TB_JMP_ADDR_BITS)
#define TB_JMP_ADDR_MASK (TB_JMP_PAGE_SIZE - 1)
#define TB_JMP_PAGE_MASK (TB_JMP_CACHE_SIZE - TB_JMP_PAGE_SIZE)
#endif /* CONFIG_SOFTMMU */

/*
 * Invalidated in parallel; all accesses to 'tc_ptr' must be atomic.
 * A valid entry is written only by the owning CPU while concurrent
 * invalidators only clear 'tc_ptr' to NULL, therefore the cached TB
 * metadata fields (pc, gen, cs_base, flags, cflags) and the cold side
 * table CPUJumpCache::tb[h] are always consistent whenever tc_ptr != NULL
 * and gen matches.
 *
 * Kept to exactly 32 bytes (32-byte aligned, two entries per 64-byte cache
 * line, zero cache-line straddling) so both inline TCG probes and C lookups
 * touch a single cache line on hit and index with a single shift (<< 5).
 */
typedef struct CPUJumpCacheEntry {
    const void *tc_ptr;
    vaddr pc;
    uint32_t gen;
    uint32_t cs_base;
    uint32_t flags;
    uint32_t cflags;
} QEMU_ALIGNED(32) CPUJumpCacheEntry;

QEMU_BUILD_BUG_ON(sizeof(CPUJumpCacheEntry) != 32);

typedef struct CPUJumpCache {
    /* Must be first for call_rcu(). */
    struct rcu_head rcu;
    uint32_t gen;
    CPUJumpCacheEntry array[TB_JMP_CACHE_SIZE] QEMU_ALIGNED(64);
    TranslationBlock *tb[TB_JMP_CACHE_SIZE];
} CPUJumpCache;

QEMU_BUILD_BUG_ON(offsetof(CPUJumpCache, array) % 64 != 0);

#endif /* EXEC_TB_JMP_CACHE_H */
