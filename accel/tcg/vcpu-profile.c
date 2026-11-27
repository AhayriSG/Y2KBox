/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Opt-in 1 kHz host-side statistical profiler for the TCG vCPU thread.
 *
 * Active only while Y2KBox diagnostic capture (Ctrl+Alt+L) is enabled.
 * Zero runtime overhead when capture is inactive.
 *
 * On Windows x86-64 hosts, a dedicated sampler thread periodically captures
 * the vCPU thread's host RIP with SuspendThread/GetThreadContext/ResumeThread
 * (performing zero locks, allocations, or syscalls while suspended), classifies
 * JIT vs QEMU C vs OS DLL execution, tags each sample with the capture second
 * and VMVGA presented-frame count, and resolves host symbols on capture stop
 * via the executable's PE/COFF symbol table + dynamic dbghelp.dll fallback.
 */

#include "qemu/osdep.h"
#include "qemu/perf-stats.h"
#include "vcpu-profile.h"

void vcpu_profile_symbol_anchor(void)
{
    /* Non-static anchor used to self-verify PE/COFF symbol VA mapping. */
    asm volatile("" : : : "memory");
}

#if defined(_WIN32) && (defined(__x86_64__) || defined(_M_X64))

#include <windows.h>
#include "hw/core/cpu.h"
#include "system/tcg.h"
#include "tcg/tcg.h"
#include "exec/translation-block.h"
#include "qemu/atomic.h"
#include "qemu/rcu.h"
#include "qemu/thread.h"

#define VCPU_PROF_MAX_SAMPLES 131072U
#define VCPU_PROF_MAX_SECS    600U
#define VCPU_PROF_SYM_NAME_LEN 96U
#define VCPU_PROF_MOD_NAME_LEN 48U
#define VCPU_PROF_MAX_FUNC_SPAN (64ULL * 1024ULL)
#define VCPU_PROF_MAX_EXEC_SECS 16U
#define VCPU_PROF_TCG_STAT_COUNT 8U

static const QemuPerfStat vcpu_prof_tcg_stats[VCPU_PROF_TCG_STAT_COUNT] = {
    QEMU_PERF_TCG_TB_GEN_CODE,
    QEMU_PERF_TCG_TB_EXEC_LOOPS,
    QEMU_PERF_TCG_CPU_EXCEPTION,
    QEMU_PERF_TCG_CPU_INTERRUPT,
    QEMU_PERF_TCG_TLB_FILL,
    QEMU_PERF_TCG_MMIO_BQL_WAIT,
    QEMU_PERF_TCG_PIO_IN,
    QEMU_PERF_TCG_PIO_OUT,
};

typedef struct VcpuProfSample {
    uint64_t rip;
    uint32_t guest_pc;
    uint32_t frame;
    uint16_t sec;
    uint8_t in_jit;
    uint8_t cpl;
    uint8_t halted;
} VcpuProfSample;

typedef struct VcpuProfState {
    QemuThread sampler_thread;
    HANDLE vcpu_handle;
    CPUState *cpu;
    VcpuProfSample *samples;
    uint32_t sample_count;
    int64_t start_us;
    int64_t stop_us;
    int running;
    bool active;
    uint64_t tcg_sec_delta[VCPU_PROF_MAX_SECS][VCPU_PROF_TCG_STAT_COUNT];
    uint64_t tcg_sec_exec_work[VCPU_PROF_MAX_SECS];
    uint64_t tcg_last_count[VCPU_PROF_TCG_STAT_COUNT];
    uint64_t tcg_last_exec_work;
    uint32_t tcg_last_sec;
    bool tcg_snapshot_started;
} VcpuProfState;

static VcpuProfState vcpu_prof;

static void vcpu_profile_tcg_snapshot(VcpuProfState *st, uint32_t sec,
                                      bool force)
{
    uint64_t current[VCPU_PROF_TCG_STAT_COUNT];
    uint64_t exec_work;
    size_t i;

    if (sec >= VCPU_PROF_MAX_SECS) {
        sec = VCPU_PROF_MAX_SECS - 1;
    }
    if (!st->tcg_snapshot_started) {
        memset(st->tcg_last_count, 0, sizeof(st->tcg_last_count));
        st->tcg_last_exec_work = 0;
        st->tcg_last_sec = 0;
        st->tcg_snapshot_started = true;
    }
    if (!force && sec <= st->tcg_last_sec) {
        return;
    }

    for (i = 0; i < VCPU_PROF_TCG_STAT_COUNT; i++) {
        current[i] = qemu_perf_stats_get_count(vcpu_prof_tcg_stats[i]);
    }
    exec_work = qemu_perf_stats_get_work_units(QEMU_PERF_TCG_TB_EXEC_LOOPS);

    /* Attribute the accumulated delta to the interval that just ended. */
    for (i = 0; i < VCPU_PROF_TCG_STAT_COUNT; i++) {
        st->tcg_sec_delta[st->tcg_last_sec][i] =
            current[i] - st->tcg_last_count[i];
    }
    st->tcg_sec_exec_work[st->tcg_last_sec] =
        exec_work - st->tcg_last_exec_work;

    memcpy(st->tcg_last_count, current, sizeof(current));
    st->tcg_last_exec_work = exec_work;
    st->tcg_last_sec = sec;
}

static void vcpu_profile_print_tcg_sec(const VcpuProfState *st)
{
    uint32_t sec, last = MIN(st->tcg_last_sec, VCPU_PROF_MAX_SECS - 1);

    for (sec = 0; sec <= last; sec++) {
        const uint64_t *d = st->tcg_sec_delta[sec];
        if (st->tcg_sec_exec_work[sec] == 0 &&
            d[0] == 0 && d[1] == 0 && d[2] == 0 && d[3] == 0 &&
            d[4] == 0 && d[5] == 0 && d[6] == 0 && d[7] == 0) {
            continue;
        }
        fprintf(stderr,
                "Y2KBOX-PROF-TCG-SEC sec=%" PRIu32
                " tb_gen=%" PRIu64 " tb_exec_loops=%" PRIu64
                " tb_exec_work=%" PRIu64 " cpu_exception=%" PRIu64
                " cpu_interrupt=%" PRIu64 " tlb_fill=%" PRIu64
                " mmio_bql_wait=%" PRIu64 " pio_in=%" PRIu64
                " pio_out=%" PRIu64 "\n",
                sec, d[0], d[1], st->tcg_sec_exec_work[sec],
                d[2], d[3], d[4], d[5], d[6], d[7]);
    }
}

/* ---- Minimal dynamic winmm.dll timer resolution helpers ---- */

typedef UINT (WINAPI *TimePeriodFn)(UINT);

/* ---- Minimal dynamic dbghelp.dll definitions (zero link dependency) ---- */

#define VCPU_SYMOPT_UNDNAME        0x00000002U
#define VCPU_SYMOPT_DEFERRED_LOADS 0x00000004U

typedef struct VcpuDbgSymbolInfo {
    ULONG SizeOfStruct;
    ULONG TypeIndex;
    ULONG64 Reserved[2];
    ULONG Index;
    ULONG Size;
    ULONG64 ModBase;
    ULONG Flags;
    ULONG64 Value;
    ULONG64 Address;
    ULONG Register;
    ULONG Scope;
    ULONG Tag;
    ULONG NameLen;
    ULONG MaxNameLen;
    CHAR Name[1];
} VcpuDbgSymbolInfo;

typedef DWORD (WINAPI *SymSetOptionsFn)(DWORD);
typedef BOOL (WINAPI *SymInitializeFn)(HANDLE, PCSTR, BOOL);
typedef BOOL (WINAPI *SymFromAddrFn)(HANDLE, DWORD64, PDWORD64,
                                     VcpuDbgSymbolInfo *);
typedef BOOL (WINAPI *SymCleanupFn)(HANDLE);

typedef struct VcpuDbgHelp {
    HMODULE mod;
    SymInitializeFn init;
    SymFromAddrFn from_addr;
    SymCleanupFn cleanup;
    bool ready;
} VcpuDbgHelp;

static void vcpu_dbghelp_init(VcpuDbgHelp *dh)
{
    SymSetOptionsFn set_opts;

    memset(dh, 0, sizeof(*dh));
    dh->mod = LoadLibraryA("dbghelp.dll");
    if (!dh->mod) {
        return;
    }
    set_opts = (SymSetOptionsFn)(void *)GetProcAddress(dh->mod,
                                                       "SymSetOptions");
    dh->init = (SymInitializeFn)(void *)GetProcAddress(dh->mod,
                                                       "SymInitialize");
    dh->from_addr = (SymFromAddrFn)(void *)GetProcAddress(dh->mod,
                                                          "SymFromAddr");
    dh->cleanup = (SymCleanupFn)(void *)GetProcAddress(dh->mod, "SymCleanup");
    if (!dh->init || !dh->from_addr || !dh->cleanup) {
        FreeLibrary(dh->mod);
        dh->mod = NULL;
        return;
    }
    if (set_opts) {
        set_opts(VCPU_SYMOPT_UNDNAME | VCPU_SYMOPT_DEFERRED_LOADS);
    }
    if (dh->init(GetCurrentProcess(), NULL, TRUE)) {
        dh->ready = true;
    } else {
        FreeLibrary(dh->mod);
        dh->mod = NULL;
    }
}

static void vcpu_dbghelp_fini(VcpuDbgHelp *dh)
{
    if (dh->ready && dh->cleanup) {
        dh->cleanup(GetCurrentProcess());
    }
    if (dh->mod) {
        FreeLibrary(dh->mod);
    }
    memset(dh, 0, sizeof(*dh));
}

/* ---- Pure-C PE64 COFF symbol table loader (unstripped MinGW .exe) ---- */

typedef struct PeSymEntry {
    uint64_t rva;
    char *name;
} PeSymEntry;

typedef struct PeExecRange {
    uint64_t start;
    uint64_t end;
} PeExecRange;

typedef struct PeSymTable {
    uint64_t exe_base;
    uint64_t exe_end;
    PeExecRange exec_ranges[VCPU_PROF_MAX_EXEC_SECS];
    size_t exec_range_count;
    PeSymEntry *entries;
    size_t count;
    const char *self_check;
} PeSymTable;

static uint16_t pe_rd_u16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t pe_rd_u32(const uint8_t *p)
{
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static int pe_sym_cmp(const void *a, const void *b)
{
    const PeSymEntry *sa = a;
    const PeSymEntry *sb = b;
    if (sa->rva < sb->rva) {
        return -1;
    }
    if (sa->rva > sb->rva) {
        return 1;
    }
    return strcmp(sa->name, sb->name);
}

static void pe_sym_table_free(PeSymTable *tab)
{
    size_t i;
    for (i = 0; i < tab->count; i++) {
        g_free(tab->entries[i].name);
    }
    g_free(tab->entries);
    memset(tab, 0, sizeof(*tab));
}

static void pe_sym_table_load(PeSymTable *tab, uintptr_t anchor_va)
{
    char exe_path[MAX_PATH];
    HMODULE h_exe;
    FILE *fp = NULL;
    uint8_t hdr[64];
    uint8_t nt_hdr[24];
    uint8_t opt_hdr[64];
    uint8_t *sec_buf = NULL;
    uint8_t *sym_buf = NULL;
    char *strtab = NULL;
    uint32_t pe_off, symtab_off, num_syms, strtab_size = 0;
    uint16_t num_secs, opt_size;
    uint32_t size_of_image = 0;
    uint32_t *sec_va = NULL;
    uint32_t *sec_chars = NULL;
    long file_size;
    size_t cap = 0, i;
    bool anchor_matched_sec_rel = false;
    bool anchor_matched_rva = false;
    uint64_t anchor_rva;

    memset(tab, 0, sizeof(*tab));
    tab->self_check = "NONE";

    h_exe = GetModuleHandleA(NULL);
    if (!h_exe) {
        return;
    }
    tab->exe_base = (uint64_t)(uintptr_t)h_exe;
    if (anchor_va < tab->exe_base) {
        return;
    }
    anchor_rva = (uint64_t)(anchor_va - tab->exe_base);

    if (GetModuleFileNameA(h_exe, exe_path, sizeof(exe_path)) == 0) {
        return;
    }
    fp = fopen(exe_path, "rb");
    if (!fp) {
        return;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        goto out;
    }
    file_size = ftell(fp);
    if (file_size < 128 || fseek(fp, 0, SEEK_SET) != 0) {
        goto out;
    }
    if (fread(hdr, 1, 64, fp) != 64 || pe_rd_u16(hdr) != 0x5A4DU) {
        goto out;
    }
    pe_off = pe_rd_u32(hdr + 60);
    if ((uint64_t)pe_off + 24 > (uint64_t)file_size) {
        goto out;
    }
    if (fseek(fp, (long)pe_off, SEEK_SET) != 0 ||
        fread(nt_hdr, 1, 24, fp) != 24 ||
        pe_rd_u32(nt_hdr) != 0x00004550U) {
        goto out;
    }

    num_secs = pe_rd_u16(nt_hdr + 6);
    symtab_off = pe_rd_u32(nt_hdr + 12);
    num_syms = pe_rd_u32(nt_hdr + 16);
    opt_size = pe_rd_u16(nt_hdr + 20);
    if (num_secs == 0 || num_secs > 96 || opt_size < 60) {
        goto out;
    }
    if ((uint64_t)pe_off + 24 + opt_size + (uint64_t)num_secs * 40 >
        (uint64_t)file_size) {
        goto out;
    }

    if (fread(opt_hdr, 1, 60, fp) != 60) {
        goto out;
    }
    if (pe_rd_u16(opt_hdr) != 0x020BU) { /* PE32+ */
        goto out;
    }
    size_of_image = pe_rd_u32(opt_hdr + 56);
    tab->exe_end = tab->exe_base + size_of_image;

    if (symtab_off == 0 || num_syms == 0 ||
        (uint64_t)symtab_off + (uint64_t)num_syms * 18 > (uint64_t)file_size) {
        tab->self_check = "NO_COFF";
        goto out;
    }

    if (fseek(fp, (long)(pe_off + 24 + opt_size), SEEK_SET) != 0) {
        goto out;
    }
    sec_buf = g_malloc((size_t)num_secs * 40);
    if (fread(sec_buf, 40, num_secs, fp) != num_secs) {
        goto out;
    }
    sec_va = g_new0(uint32_t, num_secs);
    sec_chars = g_new0(uint32_t, num_secs);
    for (i = 0; i < num_secs; i++) {
        uint32_t vsize = pe_rd_u32(sec_buf + i * 40 + 8);
        uint32_t rsize = pe_rd_u32(sec_buf + i * 40 + 16);
        uint32_t span = vsize ? vsize : rsize;
        sec_va[i] = pe_rd_u32(sec_buf + i * 40 + 12);
        sec_chars[i] = pe_rd_u32(sec_buf + i * 40 + 36);
        if ((sec_chars[i] & 0x20000000U) && span > 0 &&
            tab->exec_range_count < VCPU_PROF_MAX_EXEC_SECS) {
            tab->exec_ranges[tab->exec_range_count].start = sec_va[i];
            tab->exec_ranges[tab->exec_range_count].end =
                (uint64_t)sec_va[i] + span;
            tab->exec_range_count++;
        }
    }

    if (fseek(fp, (long)symtab_off, SEEK_SET) != 0) {
        goto out;
    }
    sym_buf = g_malloc((size_t)num_syms * 18);
    if (fread(sym_buf, 18, num_syms, fp) != num_syms) {
        goto out;
    }

    if ((uint64_t)symtab_off + (uint64_t)num_syms * 18 + 4 <=
        (uint64_t)file_size) {
        uint8_t sb[4];
        if (fread(sb, 1, 4, fp) == 4) {
            strtab_size = pe_rd_u32(sb);
            if (strtab_size >= 4 &&
                (uint64_t)symtab_off + (uint64_t)num_syms * 18 + strtab_size <=
                (uint64_t)file_size) {
                strtab = g_malloc(strtab_size + 1);
                memcpy(strtab, sb, 4);
                if (strtab_size > 4 &&
                    fread(strtab + 4, 1, strtab_size - 4, fp) !=
                    strtab_size - 4) {
                    g_free(strtab);
                    strtab = NULL;
                    strtab_size = 0;
                } else {
                    strtab[strtab_size] = '\0';
                }
            } else {
                strtab_size = 0;
            }
        }
    }

    /* Pass 1: locate anchor to confirm section-relative vs image-RVA values. */
    for (i = 0; i < num_syms; i++) {
        const uint8_t *e = sym_buf + i * 18;
        uint32_t val = pe_rd_u32(e + 8);
        int16_t sec_num = (int16_t)pe_rd_u16(e + 12);
        uint8_t sclass = e[16];
        uint8_t naux = e[17];
        char short_name[9];
        const char *name = NULL;

        if (sec_num >= 1 && sec_num <= num_secs &&
            (sclass == 2 || sclass == 3)) {
            if (pe_rd_u32(e) != 0) {
                memcpy(short_name, e, 8);
                short_name[8] = '\0';
                name = short_name;
            } else {
                uint32_t off = pe_rd_u32(e + 4);
                if (strtab && off >= 4 && off < strtab_size) {
                    name = strtab + off;
                }
            }
            if (name && (strcmp(name, "vcpu_profile_symbol_anchor") == 0 ||
                         strcmp(name, "_vcpu_profile_symbol_anchor") == 0)) {
                uint64_t cand_sec = (uint64_t)sec_va[sec_num - 1] + val;
                if (cand_sec == anchor_rva) {
                    anchor_matched_sec_rel = true;
                } else if ((uint64_t)val == anchor_rva) {
                    anchor_matched_rva = true;
                }
            }
        }
        i += naux;
    }

    if (anchor_matched_sec_rel) {
        tab->self_check = "OK";
    } else if (anchor_matched_rva) {
        tab->self_check = "ADJ";
    } else {
        tab->self_check = "MISS";
    }

    /* Pass 2: collect executable-section function symbols. */
    cap = 4096;
    tab->entries = g_new(PeSymEntry, cap);
    for (i = 0; i < num_syms; i++) {
        const uint8_t *e = sym_buf + i * 18;
        uint32_t val = pe_rd_u32(e + 8);
        int16_t sec_num = (int16_t)pe_rd_u16(e + 12);
        uint8_t sclass = e[16];
        uint8_t naux = e[17];
        char short_name[9];
        const char *name = NULL;
        uint64_t rva;

        if (sec_num >= 1 && sec_num <= num_secs &&
            (sclass == 2 || sclass == 3) &&
            (sec_chars[sec_num - 1] & 0x20000000U)) {
            if (pe_rd_u32(e) != 0) {
                memcpy(short_name, e, 8);
                short_name[8] = '\0';
                name = short_name;
            } else {
                uint32_t off = pe_rd_u32(e + 4);
                if (strtab && off >= 4 && off < strtab_size) {
                    name = strtab + off;
                }
            }
            if (name && name[0] != '\0' && name[0] != '.' &&
                strncmp(name, "__imp_", 6) != 0) {
                rva = anchor_matched_rva
                          ? (uint64_t)val
                          : ((uint64_t)sec_va[sec_num - 1] + val);
                if (rva < size_of_image) {
                    if (tab->count == cap) {
                        cap *= 2;
                        tab->entries = g_renew(PeSymEntry, tab->entries, cap);
                    }
                    tab->entries[tab->count].rva = rva;
                    tab->entries[tab->count].name = g_strdup(name);
                    tab->count++;
                }
            }
        }
        i += naux;
    }

    if (tab->count > 1) {
        size_t w = 0;
        qsort(tab->entries, tab->count, sizeof(PeSymEntry), pe_sym_cmp);
        /* Deduplicate identical RVAs, keeping the first non-internal name. */
        for (i = 0; i < tab->count; i++) {
            if (w > 0 && tab->entries[i].rva == tab->entries[w - 1].rva) {
                g_free(tab->entries[i].name);
            } else {
                tab->entries[w++] = tab->entries[i];
            }
        }
        tab->count = w;
    }

out:
    g_free(strtab);
    g_free(sym_buf);
    g_free(sec_chars);
    g_free(sec_va);
    g_free(sec_buf);
    if (fp) {
        fclose(fp);
    }
}

static bool pe_sym_table_lookup(const PeSymTable *tab, uint64_t rip,
                                const char **out_name, uint64_t *out_sym_rva)
{
    size_t lo = 0, hi = tab->count;
    uint64_t rva;

    if (tab->count == 0 || rip < tab->exe_base || rip >= tab->exe_end ||
        strcmp(tab->self_check, "MISS") == 0) {
        return false;
    }
    rva = rip - tab->exe_base;
    if (tab->exec_range_count > 0) {
        size_t j;
        bool in_exec = false;
        for (j = 0; j < tab->exec_range_count; j++) {
            if (rva >= tab->exec_ranges[j].start &&
                rva < tab->exec_ranges[j].end) {
                in_exec = true;
                break;
            }
        }
        if (!in_exec) {
            return false;
        }
    }
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (tab->entries[mid].rva <= rva) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    if (lo == 0) {
        return false;
    }
    lo--;
    if (rva - tab->entries[lo].rva > VCPU_PROF_MAX_FUNC_SPAN) {
        return false;
    }
    *out_name = tab->entries[lo].name;
    *out_sym_rva = tab->entries[lo].rva;
    return true;
}

/* ---- Symbol resolution + aggregation ---- */

typedef struct ResolvedSym {
    char sym[VCPU_PROF_SYM_NAME_LEN];
    char mod[VCPU_PROF_MOD_NAME_LEN];
    uint64_t rva;
    uint8_t is_exe;
    uint8_t is_os;
    uint8_t resolution_kind; /* 1=PE, 2=DbgHelp, 3=unresolved RVA */
} ResolvedSym;

static void resolve_module_name(uint64_t rip, char *mod_out, size_t mod_sz,
                                uint64_t *rva_out, uint8_t *is_exe_out,
                                uint64_t exe_base)
{
    HMODULE hmod = NULL;
    char path[MAX_PATH];
    const char *base_name;

    *rva_out = rip;
    *is_exe_out = 0;
    snprintf(mod_out, mod_sz, "unknown");

    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)(uintptr_t)rip, &hmod) && hmod) {
        *rva_out = rip - (uint64_t)(uintptr_t)hmod;
        *is_exe_out = ((uint64_t)(uintptr_t)hmod == exe_base) ? 1 : 0;
        if (GetModuleFileNameA(hmod, path, sizeof(path)) > 0) {
            const char *slash = strrchr(path, '\\');
            const char *fslash = strrchr(path, '/');
            base_name = path;
            if (slash && slash + 1 > base_name) {
                base_name = slash + 1;
            }
            if (fslash && fslash + 1 > base_name) {
                base_name = fslash + 1;
            }
            snprintf(mod_out, mod_sz, "%s", base_name);
        }
    }
}

static void resolve_host_rip(const PeSymTable *pe, const VcpuDbgHelp *dh,
                             uint64_t rip, ResolvedSym *out)
{
    const char *pe_name = NULL;
    uint64_t pe_rva = 0;

    memset(out, 0, sizeof(*out));
    resolve_module_name(rip, out->mod, sizeof(out->mod), &out->rva,
                        &out->is_exe, pe->exe_base);
    out->is_os = out->is_exe ? 0 : 1;

    if (out->is_exe && pe_sym_table_lookup(pe, rip, &pe_name, &pe_rva)) {
        snprintf(out->sym, sizeof(out->sym), "%s", pe_name);
        out->rva = pe_rva;
        out->resolution_kind = 1;
        return;
    }

    if (dh->ready && dh->from_addr) {
        union {
            VcpuDbgSymbolInfo info;
            char raw[sizeof(VcpuDbgSymbolInfo) + 256];
        } buf;
        DWORD64 disp = 0;

        memset(&buf, 0, sizeof(buf));
        buf.info.SizeOfStruct = sizeof(VcpuDbgSymbolInfo);
        buf.info.MaxNameLen = 255;
        if (dh->from_addr(GetCurrentProcess(), (DWORD64)rip, &disp,
                          &buf.info) &&
            buf.info.Name[0] != '\0') {
            snprintf(out->sym, sizeof(out->sym), "%s", buf.info.Name);
            if (out->rva >= disp) {
                out->rva -= disp;
            }
            out->resolution_kind = 2;
            return;
        }
    }

    /* Quantize unresolved addresses to 64-byte buckets for aggregation. */
    out->resolution_kind = 3;
    out->rva &= ~63ULL;
    snprintf(out->sym, sizeof(out->sym), "%s+0x%" PRIx64, out->mod, out->rva);
}

typedef struct SymBucket {
    char sym[VCPU_PROF_SYM_NAME_LEN];
    char mod[VCPU_PROF_MOD_NAME_LEN];
    uint64_t rva;
    uint32_t count;
} SymBucket;

static int sym_bucket_cmp_desc(const void *a, const void *b)
{
    const SymBucket *sa = a;
    const SymBucket *sb = b;
    if (sa->count != sb->count) {
        return (sa->count > sb->count) ? -1 : 1;
    }
    return strcmp(sa->sym, sb->sym);
}

static void sym_bucket_add(GHashTable *tbl, const char *sym, const char *mod,
                           uint64_t rva)
{
    SymBucket *b = g_hash_table_lookup(tbl, sym);
    if (!b) {
        b = g_new0(SymBucket, 1);
        snprintf(b->sym, sizeof(b->sym), "%s", sym);
        snprintf(b->mod, sizeof(b->mod), "%s", mod);
        b->rva = rva;
        g_hash_table_insert(tbl, b->sym, b);
    }
    b->count++;
}

static void print_top_buckets(GHashTable *tbl, uint32_t total,
                              const char *prefix, const char *phase_tag,
                              size_t limit)
{
    GHashTableIter iter;
    gpointer key, val;
    size_t n = g_hash_table_size(tbl);
    SymBucket *arr;
    size_t i = 0;

    if (n == 0 || total == 0) {
        return;
    }
    arr = g_new(SymBucket, n);
    g_hash_table_iter_init(&iter, tbl);
    while (g_hash_table_iter_next(&iter, &key, &val)) {
        arr[i++] = *(const SymBucket *)val;
    }
    qsort(arr, n, sizeof(SymBucket), sym_bucket_cmp_desc);
    if (n > limit) {
        n = limit;
    }
    for (i = 0; i < n; i++) {
        double pct = 100.0 * (double)arr[i].count / (double)total;
        if (phase_tag) {
            fprintf(stderr,
                    "%s phase=%s rank=%zu samples=%" PRIu32 " pct=%.2f%% "
                    "sym=%s mod=%s rva=0x%" PRIx64 "\n",
                    prefix, phase_tag, i + 1, arr[i].count, pct,
                    arr[i].sym, arr[i].mod, arr[i].rva);
        } else {
            fprintf(stderr,
                    "%s rank=%zu samples=%" PRIu32 " pct=%.2f%% "
                    "sym=%s mod=%s rva=0x%" PRIx64 "\n",
                    prefix, i + 1, arr[i].count, pct,
                    arr[i].sym, arr[i].mod, arr[i].rva);
        }
    }
    g_free(arr);
}

typedef struct JitPcBucket {
    uint64_t key; /* ((uint64_t)cpl << 32) | guest_pc */
    uint32_t guest_pc;
    uint8_t cpl;
    uint32_t count;
} JitPcBucket;

static int jit_bucket_cmp_desc(const void *a, const void *b)
{
    const JitPcBucket *ja = a;
    const JitPcBucket *jb = b;
    if (ja->count != jb->count) {
        return (ja->count > jb->count) ? -1 : 1;
    }
    return (ja->guest_pc < jb->guest_pc) ? -1 : 1;
}

static void jit_bucket_add(GHashTable *tbl, uint32_t guest_pc, uint8_t cpl)
{
    uint64_t k = ((uint64_t)cpl << 32) | (guest_pc & ~15U);
    JitPcBucket *b = g_hash_table_lookup(tbl, &k);
    if (!b) {
        b = g_new0(JitPcBucket, 1);
        b->key = k;
        b->guest_pc = guest_pc & ~15U;
        b->cpl = cpl;
        g_hash_table_insert(tbl, &b->key, b);
    }
    b->count++;
}

static void print_top_jit(GHashTable *tbl, uint32_t jit_total,
                          uint32_t all_total, const char *phase_tag,
                          size_t limit)
{
    GHashTableIter iter;
    gpointer key, val;
    size_t n = g_hash_table_size(tbl);
    JitPcBucket *arr;
    size_t i = 0;

    if (n == 0 || jit_total == 0 || all_total == 0) {
        return;
    }
    arr = g_new(JitPcBucket, n);
    g_hash_table_iter_init(&iter, tbl);
    while (g_hash_table_iter_next(&iter, &key, &val)) {
        arr[i++] = *(const JitPcBucket *)val;
    }
    qsort(arr, n, sizeof(JitPcBucket), jit_bucket_cmp_desc);
    if (n > limit) {
        n = limit;
    }
    for (i = 0; i < n; i++) {
        fprintf(stderr,
                "Y2KBOX-PROF-JIT phase=%s rank=%zu samples=%" PRIu32
                " pct_jit=%.2f%% pct_all=%.2f%% cpl=%u guest_pc=0x%08"
                PRIx32 "\n",
                phase_tag, i + 1, arr[i].count,
                100.0 * (double)arr[i].count / (double)jit_total,
                100.0 * (double)arr[i].count / (double)all_total,
                (unsigned)arr[i].cpl, arr[i].guest_pc);
    }
    g_free(arr);
}

/* ---- Sampler thread ---- */

static void *vcpu_profile_thread_fn(void *arg)
{
    VcpuProfState *st = arg;
    HMODULE h_winmm;
    TimePeriodFn begin_period = NULL;
    TimePeriodFn end_period = NULL;
    bool period_set = false;

    rcu_register_thread();

    h_winmm = LoadLibraryA("winmm.dll");
    if (h_winmm) {
        begin_period = (TimePeriodFn)(void *)GetProcAddress(h_winmm,
                                                            "timeBeginPeriod");
        end_period = (TimePeriodFn)(void *)GetProcAddress(h_winmm,
                                                          "timeEndPeriod");
        if (begin_period && end_period && begin_period(1) == 0) {
            period_set = true;
        }
    }

    while (qatomic_read(&st->running)) {
        CONTEXT ctx __attribute__((aligned(16)));
        DWORD susp;
        BOOL ok = FALSE;
        uint64_t rip = 0;
        uint32_t guest_pc = 0;
        uint8_t in_jit = 0;
        uint8_t cpl = 3;
        uint8_t halted;
        int64_t now_us;
        uint32_t sec;
        uint32_t frame;

        Sleep(1);
        if (!qatomic_read(&st->running)) {
            break;
        }

        now_us = g_get_monotonic_time();
        sec = (now_us > st->start_us)
                  ? (uint32_t)((now_us - st->start_us) / 1000000LL)
                  : 0;
        if (sec >= VCPU_PROF_MAX_SECS) {
            sec = VCPU_PROF_MAX_SECS - 1;
        }
        vcpu_profile_tcg_snapshot(st, sec, false);
        if (st->sample_count >= VCPU_PROF_MAX_SAMPLES) {
            continue;
        }
        frame = (uint32_t)qemu_perf_stats_get_count(
            QEMU_PERF_VMVGA_SCREEN_TARGET_PRESENT);
        halted = qatomic_read(&st->cpu->halted) ? 1 : 0;

        /*
         * CRITICAL: Perform ZERO allocations, ZERO locks, and ZERO syscalls
         * other than GetThreadContext while the vCPU thread is suspended.
         */
        susp = SuspendThread(st->vcpu_handle);
        if (susp != (DWORD)-1) {
            memset(&ctx, 0, sizeof(ctx));
            ctx.ContextFlags = CONTEXT_CONTROL;
            ok = GetThreadContext(st->vcpu_handle, &ctx);
            ResumeThread(st->vcpu_handle);
            if (ok) {
                rip = (uint64_t)ctx.Rip;
            }
        }

        if (!ok || rip == 0) {
            continue;
        }

        if (in_code_gen_buffer((const void *)(uintptr_t)(rip - tcg_splitwx_diff))) {
            in_jit = 1;
            WITH_RCU_READ_LOCK_GUARD() {
                TranslationBlock *tb = tcg_tb_lookup((uintptr_t)rip);
                if (tb) {
                    cpl = (uint8_t)(tb->flags & 3U);
                }
            }
            if (st->cpu->cc && st->cpu->cc->get_pc) {
                guest_pc = (uint32_t)st->cpu->cc->get_pc(st->cpu);
            }
        }

        VcpuProfSample *s = &st->samples[st->sample_count++];
        s->rip = rip;
        s->guest_pc = guest_pc;
        s->frame = frame;
        s->sec = (uint16_t)sec;
        s->in_jit = in_jit;
        s->cpl = cpl;
        s->halted = halted;
    }

    if (period_set && end_period) {
        end_period(1);
    }
    if (h_winmm) {
        FreeLibrary(h_winmm);
    }
    rcu_unregister_thread();
    return NULL;
}

typedef struct SecSummary {
    uint32_t samples;
    uint32_t jit;
    uint32_t jit_cpl0;
    uint32_t qemu_c;
    uint32_t os_dll;
    uint32_t frame_min;
    uint32_t frame_max;
    uint32_t df;
    GHashTable *syms;
} SecSummary;

static void vcpu_profile_report(VcpuProfState *st)
{
    PeSymTable pe;
    VcpuDbgHelp dh;
    uint32_t i, n = st->sample_count;
    uint32_t total_jit = 0, total_jit_cpl0 = 0, total_jit_cpl3 = 0;
    uint32_t total_qemu_c = 0, total_os = 0, total_halted = 0;
    uint32_t host_pe_symbols = 0, host_dbghelp_symbols = 0;
    uint32_t host_unresolved = 0, jit_guest_pc_zero = 0;
    uint32_t max_sec = 0;
    int64_t dur_ms = (st->stop_us > st->start_us)
                         ? (st->stop_us - st->start_us) / 1000LL
                         : 0;
    GHashTable *rip_cache = NULL;
    GHashTable *all_syms = NULL;
    GHashTable *hi_syms = NULL;
    GHashTable *lo_syms = NULL;
    GHashTable *all_jit = NULL;
    GHashTable *lo_jit = NULL;
    SecSummary *secs = NULL;
    uint32_t hi_secs = 0, hi_frames = 0, hi_samples = 0;
    uint32_t hi_jit = 0, hi_cpl0 = 0, hi_qemuc = 0, hi_os = 0;
    uint32_t lo_secs = 0, lo_frames = 0, lo_samples = 0;
    uint32_t lo_jit_cnt = 0, lo_cpl0 = 0, lo_qemuc = 0, lo_os = 0;
    uint32_t prev_frame_max = 0;

    pe_sym_table_load(&pe, (uintptr_t)&vcpu_profile_symbol_anchor);
    vcpu_dbghelp_init(&dh);

    vcpu_profile_print_tcg_sec(st);

    if (n == 0) {
        fprintf(stderr,
                "Y2KBOX-PROF-META samples=0 duration_ms=%" PRId64
                " pe_syms=%zu self_check=%s dbghelp=%d\n",
                dur_ms, pe.count, pe.self_check, dh.ready ? 1 : 0);
        vcpu_dbghelp_fini(&dh);
        pe_sym_table_free(&pe);
        return;
    }

    for (i = 0; i < n; i++) {
        if (st->samples[i].sec > max_sec) {
            max_sec = st->samples[i].sec;
        }
    }
    if (max_sec >= VCPU_PROF_MAX_SECS) {
        max_sec = VCPU_PROF_MAX_SECS - 1;
    }

    secs = g_new0(SecSummary, max_sec + 1);
    for (i = 0; i <= max_sec; i++) {
        secs[i].frame_min = UINT32_MAX;
        secs[i].syms = g_hash_table_new_full(g_str_hash, g_str_equal,
                                             NULL, g_free);
    }

    /* Cache resolved non-JIT RIP -> ResolvedSym (many samples hit same RIP). */
    rip_cache = g_hash_table_new_full(g_int64_hash, g_int64_equal,
                                      g_free, g_free);
    all_syms = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
    hi_syms = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
    lo_syms = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
    all_jit = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, g_free);
    lo_jit = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, g_free);

    /* First pass: compute per-second frame deltas so we know high vs low FPS. */
    for (i = 0; i < n; i++) {
        const VcpuProfSample *s = &st->samples[i];
        SecSummary *ss = &secs[s->sec];
        if (s->frame < ss->frame_min) {
            ss->frame_min = s->frame;
        }
        if (s->frame > ss->frame_max) {
            ss->frame_max = s->frame;
        }
    }
    for (i = 0; i <= max_sec; i++) {
        if (secs[i].frame_min != UINT32_MAX) {
            uint32_t base = (i == 0) ? secs[i].frame_min : prev_frame_max;
            secs[i].df = (secs[i].frame_max >= base)
                             ? (secs[i].frame_max - base)
                             : 0;
            prev_frame_max = secs[i].frame_max;
        }
    }

    /* Second pass: resolve symbols and accumulate overall, per-sec, and phases. */
    for (i = 0; i < n; i++) {
        const VcpuProfSample *s = &st->samples[i];
        SecSummary *ss = &secs[s->sec];
        bool is_hi = (ss->df >= 5);
        bool is_lo = (ss->df <= 3);

        ss->samples++;
        if (s->halted) {
            total_halted++;
        }
        if (is_hi) {
            hi_samples++;
        } else if (is_lo) {
            lo_samples++;
        }

        if (s->in_jit) {
            const char *jit_sym = (s->cpl == 0) ? "[TCG_JIT_CPL0]"
                                                : "[TCG_JIT_CPL3]";
            total_jit++;
            ss->jit++;
            if (s->guest_pc == 0) {
                jit_guest_pc_zero++;
            }
            if (s->cpl == 0) {
                total_jit_cpl0++;
                ss->jit_cpl0++;
                if (is_hi) {
                    hi_cpl0++;
                } else if (is_lo) {
                    lo_cpl0++;
                }
            } else {
                total_jit_cpl3++;
            }
            if (is_hi) {
                hi_jit++;
                sym_bucket_add(hi_syms, jit_sym, "tcg-jit", 0);
            } else if (is_lo) {
                lo_jit_cnt++;
                sym_bucket_add(lo_syms, jit_sym, "tcg-jit", 0);
                jit_bucket_add(lo_jit, s->guest_pc, s->cpl);
            }
            sym_bucket_add(all_syms, jit_sym, "tcg-jit", 0);
            sym_bucket_add(ss->syms, jit_sym, "tcg-jit", 0);
            jit_bucket_add(all_jit, s->guest_pc, s->cpl);
        } else {
            uint64_t key_rip = s->rip;
            ResolvedSym *rs = g_hash_table_lookup(rip_cache, &key_rip);
            if (!rs) {
                uint64_t *k = g_new(uint64_t, 1);
                *k = key_rip;
                rs = g_new0(ResolvedSym, 1);
                resolve_host_rip(&pe, &dh, key_rip, rs);
                g_hash_table_insert(rip_cache, k, rs);
            }
            if (rs->resolution_kind == 1) {
                host_pe_symbols++;
            } else if (rs->resolution_kind == 2) {
                host_dbghelp_symbols++;
            } else {
                host_unresolved++;
            }
            if (rs->is_exe) {
                total_qemu_c++;
                ss->qemu_c++;
                if (is_hi) {
                    hi_qemuc++;
                } else if (is_lo) {
                    lo_qemuc++;
                }
            } else {
                total_os++;
                ss->os_dll++;
                if (is_hi) {
                    hi_os++;
                } else if (is_lo) {
                    lo_os++;
                }
            }
            sym_bucket_add(all_syms, rs->sym, rs->mod, rs->rva);
            sym_bucket_add(ss->syms, rs->sym, rs->mod, rs->rva);
            if (is_hi) {
                sym_bucket_add(hi_syms, rs->sym, rs->mod, rs->rva);
            } else if (is_lo) {
                sym_bucket_add(lo_syms, rs->sym, rs->mod, rs->rva);
            }
        }
    }

    for (i = 0; i <= max_sec; i++) {
        if (secs[i].samples == 0) {
            continue;
        }
        if (secs[i].df >= 5) {
            hi_secs++;
            hi_frames += secs[i].df;
        } else if (secs[i].df <= 3) {
            lo_secs++;
            lo_frames += secs[i].df;
        }
    }

    fprintf(stderr,
            "Y2KBOX-PROF-META samples=%" PRIu32 " duration_ms=%" PRId64
            " jit=%" PRIu32 "(%.1f%%) jit_cpl0=%" PRIu32 "(%.1f%%)"
            " jit_cpl3=%" PRIu32 "(%.1f%%) qemu_c=%" PRIu32 "(%.1f%%)"
            " os_dll=%" PRIu32 "(%.1f%%) halted=%" PRIu32 "(%.1f%%)"
            " host_pe=%" PRIu32 " host_dbghelp=%" PRIu32
            " host_unresolved=%" PRIu32 " jit_guest_pc_zero=%" PRIu32
            " pe_syms=%zu self_check=%s dbghelp=%d\n",
            n, dur_ms,
            total_jit, 100.0 * total_jit / n,
            total_jit_cpl0, 100.0 * total_jit_cpl0 / n,
            total_jit_cpl3, 100.0 * total_jit_cpl3 / n,
            total_qemu_c, 100.0 * total_qemu_c / n,
            total_os, 100.0 * total_os / n,
            total_halted, 100.0 * total_halted / n,
            host_pe_symbols, host_dbghelp_symbols, host_unresolved,
            jit_guest_pc_zero,
            pe.count, pe.self_check, dh.ready ? 1 : 0);

    /* Per-second timeline */
    for (i = 0; i <= max_sec; i++) {
        SecSummary *ss = &secs[i];
        const SymBucket *top1 = NULL, *top2 = NULL;
        GHashTableIter iter;
        gpointer k, v;

        if (ss->samples == 0) {
            continue;
        }
        g_hash_table_iter_init(&iter, ss->syms);
        while (g_hash_table_iter_next(&iter, &k, &v)) {
            const SymBucket *b = v;
            if (!top1 || b->count > top1->count) {
                top2 = top1;
                top1 = b;
            } else if (!top2 || b->count > top2->count) {
                top2 = b;
            }
        }
        fprintf(stderr,
                "Y2KBOX-PROF-SEC sec=%" PRIu32 " df=%" PRIu32
                " samples=%" PRIu32 " jit=%.1f%% cpl0=%.1f%% qemu_c=%.1f%%"
                " os=%.1f%% top1=%s(%.1f%%) top2=%s(%.1f%%)\n",
                i, ss->df, ss->samples,
                100.0 * ss->jit / ss->samples,
                100.0 * ss->jit_cpl0 / ss->samples,
                100.0 * ss->qemu_c / ss->samples,
                100.0 * ss->os_dll / ss->samples,
                top1 ? top1->sym : "none",
                top1 ? (100.0 * top1->count / ss->samples) : 0.0,
                top2 ? top2->sym : "none",
                top2 ? (100.0 * top2->count / ss->samples) : 0.0);
    }

    /* Overall Top 30 */
    print_top_buckets(all_syms, n, "Y2KBOX-PROF-TOP", NULL, 30);

    /* High-FPS vs Low-FPS phase comparison */
    if (hi_samples > 0) {
        fprintf(stderr,
                "Y2KBOX-PROF-PHASE phase=high_fps secs=%" PRIu32
                " frames=%" PRIu32 " samples=%" PRIu32 " jit=%.1f%%"
                " cpl0=%.1f%% qemu_c=%.1f%% os=%.1f%%\n",
                hi_secs, hi_frames, hi_samples,
                100.0 * hi_jit / hi_samples,
                100.0 * hi_cpl0 / hi_samples,
                100.0 * hi_qemuc / hi_samples,
                100.0 * hi_os / hi_samples);
        print_top_buckets(hi_syms, hi_samples, "Y2KBOX-PROF-PHASE-TOP",
                          "high_fps", 15);
    }
    if (lo_samples > 0) {
        fprintf(stderr,
                "Y2KBOX-PROF-PHASE phase=low_fps secs=%" PRIu32
                " frames=%" PRIu32 " samples=%" PRIu32 " jit=%.1f%%"
                " cpl0=%.1f%% qemu_c=%.1f%% os=%.1f%%\n",
                lo_secs, lo_frames, lo_samples,
                100.0 * lo_jit_cnt / lo_samples,
                100.0 * lo_cpl0 / lo_samples,
                100.0 * lo_qemuc / lo_samples,
                100.0 * lo_os / lo_samples);
        print_top_buckets(lo_syms, lo_samples, "Y2KBOX-PROF-PHASE-TOP",
                          "low_fps", 15);
    }

    /* Hottest guest PCs when executing inside TCG JIT */
    print_top_jit(all_jit, total_jit, n, "all", 15);
    if (lo_jit_cnt > 0 && lo_samples > 0) {
        print_top_jit(lo_jit, lo_jit_cnt, lo_samples, "low_fps", 15);
    }

    for (i = 0; i <= max_sec; i++) {
        g_hash_table_destroy(secs[i].syms);
    }
    g_free(secs);
    g_hash_table_destroy(lo_jit);
    g_hash_table_destroy(all_jit);
    g_hash_table_destroy(lo_syms);
    g_hash_table_destroy(hi_syms);
    g_hash_table_destroy(all_syms);
    g_hash_table_destroy(rip_cache);
    vcpu_dbghelp_fini(&dh);
    pe_sym_table_free(&pe);
}

static void vcpu_profile_start(void)
{
    CPUState *cpu;
    DWORD tid;
    HANDLE h;

    if (vcpu_prof.active) {
        return;
    }
    if (!tcg_enabled()) {
        fprintf(stderr,
                "Y2KBOX-PROF-STATUS available=0 reason=not_tcg\\n");
        return;
    }
    cpu = first_cpu;
    if (!cpu || !cpu->thread) {
        fprintf(stderr,
                "Y2KBOX-PROF-STATUS available=0 reason=no_vcpu_thread\\n");
        return;
    }
    tid = cpu->thread->tid ? (DWORD)cpu->thread->tid : (DWORD)cpu->thread_id;
    if (tid == 0) {
        fprintf(stderr,
                "Y2KBOX-PROF-STATUS available=0 reason=no_thread_id\\n");
        return;
    }

    h = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT |
                   THREAD_QUERY_INFORMATION, FALSE, tid);
    if (!h) {
        fprintf(stderr,
                "Y2KBOX-PROF-STATUS available=0 reason=open_thread_failed"
                " winerr=%lu\\n", (unsigned long)GetLastError());
        return;
    }

    memset(&vcpu_prof, 0, sizeof(vcpu_prof));
    vcpu_prof.vcpu_handle = h;
    vcpu_prof.cpu = cpu;
    vcpu_prof.samples = g_new(VcpuProfSample, VCPU_PROF_MAX_SAMPLES);
    vcpu_prof.sample_count = 0;
    vcpu_prof.start_us = g_get_monotonic_time();
    qatomic_set(&vcpu_prof.running, 1);
    vcpu_prof.active = true;

    qemu_thread_create(&vcpu_prof.sampler_thread, "vcpu-prof",
                       vcpu_profile_thread_fn, &vcpu_prof,
                       QEMU_THREAD_JOINABLE);
    fprintf(stderr,
            "Y2KBOX-PROF-STATUS available=1 sampler=windows_x64_tcg\\n");
}

static void vcpu_profile_stop(void)
{
    if (!vcpu_prof.active) {
        return;
    }
    vcpu_prof.stop_us = g_get_monotonic_time();
    qatomic_set(&vcpu_prof.running, 0);
    qemu_thread_join(&vcpu_prof.sampler_thread);
    {
        uint32_t sec = (vcpu_prof.stop_us > vcpu_prof.start_us)
                           ? (uint32_t)((vcpu_prof.stop_us -
                                         vcpu_prof.start_us) / 1000000LL)
                           : 0;
        vcpu_profile_tcg_snapshot(&vcpu_prof, sec, true);
    }
    if (vcpu_prof.vcpu_handle) {
        CloseHandle(vcpu_prof.vcpu_handle);
        vcpu_prof.vcpu_handle = NULL;
    }

    vcpu_profile_report(&vcpu_prof);

    g_free(vcpu_prof.samples);
    vcpu_prof.samples = NULL;
    vcpu_prof.sample_count = 0;
    vcpu_prof.active = false;
}

void vcpu_profile_register_hooks(void)
{
    qemu_perf_stats_set_vcpu_profile_hooks(vcpu_profile_start,
                                           vcpu_profile_stop);
}

#else

void vcpu_profile_register_hooks(void)
{
    /* Host-side SuspendThread/GetThreadContext sampler is Windows x64 only. */
}

#endif
