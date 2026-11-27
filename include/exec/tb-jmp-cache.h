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
#define TB_JMP_CACHE_WAYS 2

#ifdef CONFIG_SOFTMMU
/*
 * Split the 14-bit jump-cache set index evenly into 7 page-bucket bits
 * (128 buckets, 512 KiB stride on 4 KiB targets) and 7 intra-page set bits
 * (128 sets per page x 2 ways = 256 slots per page).  Only the bottom
 * TB_JMP_ADDR_BITS vary for addresses on the same guest page so TLB page
 * invalidations scan one contiguous 128-set (8 KiB) slice and clear only
 * entries matching the invalidated guest page.
 */
#define TB_JMP_ADDR_BITS 7
#define TB_JMP_PAGE_BITS (TB_JMP_CACHE_BITS - TB_JMP_ADDR_BITS)
#define TB_JMP_PAGE_SIZE (1 << TB_JMP_ADDR_BITS)
#define TB_JMP_ADDR_MASK (TB_JMP_PAGE_SIZE - 1)
#define TB_JMP_PAGE_MASK (TB_JMP_CACHE_SIZE - TB_JMP_PAGE_SIZE)
#endif /* CONFIG_SOFTMMU */

/*
 * Invalidated in parallel; all accesses to 'tc_ptr' must be atomic.
 * A valid entry is written only by the owning CPU while concurrent
 * invalidators only clear 'tc_ptr' to NULL (and live entries are never
 * moved between ways), therefore the cached TB metadata fields
 * (pc, gen, cflags, cs_base, flags) and the cold side table
 * CPUJumpCache::tb[h][way] are always consistent whenever tc_ptr != NULL
 * and gen matches.
 *
 * Each entry is exactly 32 bytes and two ways form one 64-byte cache line
 * (CPUJumpCacheSet), so both ways are fetched in a single cache line and
 * probed with 64-bit packed comparisons on (gen, cflags) and (cs_base, flags).
 */
typedef struct CPUJumpCacheEntry {
    const void *tc_ptr;
    vaddr pc;
    uint32_t gen;
    uint32_t cflags;
    uint32_t cs_base;
    uint32_t flags;
} QEMU_ALIGNED(32) CPUJumpCacheEntry;

QEMU_BUILD_BUG_ON(sizeof(CPUJumpCacheEntry) != 32);
QEMU_BUILD_BUG_ON(offsetof(CPUJumpCacheEntry, cflags) !=
                  offsetof(CPUJumpCacheEntry, gen) + 4);
QEMU_BUILD_BUG_ON(offsetof(CPUJumpCacheEntry, flags) !=
                  offsetof(CPUJumpCacheEntry, cs_base) + 4);

typedef struct CPUJumpCacheSet {
    CPUJumpCacheEntry way[TB_JMP_CACHE_WAYS];
} QEMU_ALIGNED(64) CPUJumpCacheSet;

QEMU_BUILD_BUG_ON(sizeof(CPUJumpCacheSet) != 64);

typedef struct CPUJumpCache {
    /* Must be first for call_rcu(). */
    struct rcu_head rcu;
    uint32_t gen;
    CPUJumpCacheSet sets[TB_JMP_CACHE_SIZE] QEMU_ALIGNED(64);
    TranslationBlock *tb[TB_JMP_CACHE_SIZE][TB_JMP_CACHE_WAYS];
    uint8_t victim_bits[TB_JMP_CACHE_SIZE / 8];
} CPUJumpCache;

QEMU_BUILD_BUG_ON(offsetof(CPUJumpCache, sets) % 64 != 0);

#endif /* EXEC_TB_JMP_CACHE_H */
