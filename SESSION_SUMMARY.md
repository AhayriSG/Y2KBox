# Y2KBox - session summary (end of session 2, 2026-11-17)

Purpose: paste/point a NEW session at this file so it starts without re-reading the old chat.
The repo is the source of truth: https://github.com/AhayriSG/Y2KBox (branch `main`, last push by this session: `ee601d2`).
Read in this order: `CLAUDE.md` -> `HANDOFF.md` -> `TASKS.md` -> `WORKLOG.md`. Then `git pull`.

## 1. What the project is (owner constraints)
QEMU fork that emulates ONLY 32-bit Windows XP guests. TCG/MTTCG only (no KVM/Xen/WHPX/IOMMU...). Hosts: Windows + Linux,
x86-64-v3 minimum (AVX-512 gave no gain, stay v3). Fixed machine: 2048 MB RAM below 4 GB (not configurable), guest CPU `xp-p4`
(Pentium 4 class, SSE2 max), 1 vCPU default (max 2), no user-facing -m/-smp/-cpu flags (own front end later: wxWidgets, qcow2 +
floppy + CD/ISO). RTL8139 default NIC. Performance matters for ALL instruction classes up to SSE2, not just SSE2.
3D plan (owner decision): PRIMARY = qemus/qemu-vmvga (VMware SVGA II + SVGA3D via DXVK; XP driver = VMware Tools 10.0.12,
SVGA 12.01.08); VBoxVGA 5.2.x kept as frozen alternative (T16). Extra reference: PCBox PR #202 (NV11 real-GPU route).

## 2. Done (all on `main`)
- Stripping: tests, docs, all non-x86 targets/boards/user-mode, Xen, KVM/HVF/WHPX/..., non-x86 TCG backends + TCI, macOS/BSD code,
  1015 dead C files, 724 headers, non-x86 firmware, contrib. Tree 171 MB -> 41 MB; devices 439 -> 244; `cpu.c` 11085 -> 6205 lines.
- Machine: single type `pc-i440fx-11.2` (alias `pc`), minimal `pc_memory_init` (all RAM < 4 GB), hotplug/DIMM/NVDIMM/CXL/HMAT/VIOT/
  VMGenID/VMClock removed, only CPU `xp-p4` (verified in-guest: SSE2 yes, SSE3+/AVX no), default RAM 2 GiB, 1 vCPU.
- Perf: `qemu_ld_vec`/`qemu_st_vec` TCG ops (128-bit loads 2.7x faster, mixed mem loop 6.20 -> 2.82 s);
  **x87 host fast path** (add/sub/mul/div/sqrt on normal operands; x87add 65.3 -> 21.7 ns, x87div 153.9 -> 33.5 ns) verified
  bit-identical to softfloat on 7.2M random ops (`xp/bench/x87cmp.sh`, kill switch env `XPQEMU_NO_HX87=1`); found+fixed a stock
  QEMU bug (fld m32/m64 rounded to the control-word precision; real hardware does not). Baseline hotspots still open:
  call/ret ~12 ns per pair, rep movs/stos ~2 GB/s.
- CI (`.github/workflows/build.yml`): Linux + Windows (MSYS2 UCRT64) build, boot check, `vectest`, x87 differential test, binary
  artifacts. Last pushed (`ee601d2`) added SDL2 (`--enable-sdl`), a default firmware search in `<exe dir>/pc-bios` and `<exe dir>`
  (`util/datadir.c`), a packaged-layout smoke test, README download notes. **Its CI result was not checked** (see section 4).
- Workflow tooling: `scripts/session_status.sh` + `.claude/settings.json` hooks, `CLAUDE.md` rule 7 (no local builds, no polling
  after push), `.gitignore` for `.claude/settings.local.json` and `.env*`.
- Docs structure: `HANDOFF.md` (context/decisions/findings/references), `TASKS.md` (claim protocol + queue), `WORKLOG.md`.

## 3. Task queue state (see TASKS.md for details)
T01 fixed machine PAUSED (2a, 2b done; remaining: dead above-4G code, remove -m/-smp/-machine/-cpu CLI, rename machine) |
T02 CPU PAUSED (only xp-p4 done; remaining: delete `max`/`base` types, `-cpu`, SSE3+ translators) | T03 Linux+Windows-only PAUSED
(pass 1) | T04 inline PSHUFD/shuffles TODO (another agent claimed then paused) | T05 IOMMU/SEV/SGX/TDX/VFIO + KVM call sites TODO |
T06 more pruning TODO | T07 Windows build DONE | T08 boot XP TODO (needs an XP ISO from the owner) | T09-T11 qemu-vmvga integration,
host 3D backend (Linux dxvk-native / Windows?), XP driver validation TODO (TCG backdoor-port #GP exemption is part of T09) |
T12 own front end TODO | T13a more x87, T13b call/ret, T13c rep-string TODO | T14 thread pinning TODO | T15 virtio eval TODO | T16 VBoxVGA frozen.

## 4. PENDING at the moment this session stopped (not done yet)
1. **Check CI for `ee601d2`** (SDL + packaged layout). The owner reported: `-display sdl` rejected and `bios-256k.bin` not found on
   the Windows artifact; these were the fixes just pushed, unverified.
2. **README cleanup (owner request):** remove the "Development status" table and "References" section from README.md (redundant
   with TASKS/WORKLOG/HANDOFF); keep title, badges, short intro, Download, pointers. Then remove instructions elsewhere that say to
   keep the README status table updated (CLAUDE.md rule 3, TASKS.md protocol step 4, HANDOFF section 10 checklist).
3. **Codacy loop refactor (owner chose "your choice"; recommended option):** the Claude sandbox cannot reach app.codacy.com. Remove
   the direct Codacy API calls and `CODACY_API_TOKEN` from `scripts/session_status.sh`; instead read GitHub check runs
   (`/commits/<sha>/check-runs`, annotations) and commit statuses (`/commits/<sha>/status`) for HEAD, since the Codacy GitHub App
   reports there. Cache only when the Actions run is completed and Codacy has reported (or the commit is >15 min old). Do NOT add
   `codacy/analysis-cli-action` (unverified action name; duplicates the app). Update HANDOFF section 7 and CLAUDE.md rules 2 and 7.
   Last query of check-runs failed only because of a shell syntax slip (dash has no `${var:0:7}`); redo it.
4. Owner may rotate secrets: a GitHub classic token and a Codacy API key were pasted in chat. Tokens are NOT stored in the repo.

## 5. Gotchas learned
- Sandbox shell is `dash`: no `<(...)`, no `{a,b}` expansion, no `${var:0:7}`; use `bash` scripts. Commands time out at 300 s.
- Sandbox network allowlist blocks app.codacy.com and artifact downloads; GitHub API works (unauthenticated limit is 60/h and was
  exhausted: use a token via env var). `ghp_` tokens must never be written to files.
- Parallel agents also push to `main`: always `git fetch` + `git rebase` before pushing; use `git revert`, not file copies (a revert once
  swapped TASKS.md/WORKLOG.md - check `head -1`).
- Benchmarks: the sandbox is noisy; use runs >= 1 s, min of 3; keep loop counters off the code page (SMC makes everything 50x slower).
- Compare a changed binary against the SAME binary with its kill switch, not an old build.
- Prune/dead-code scripts live in `xp/strip/`; the dead-code method only for config-driven dirs (not host-dependent backends).

## 6. Starter prompt for the new session
"Read CLAUDE.md, HANDOFF.md, TASKS.md, WORKLOG.md and SESSION_SUMMARY.md in AhayriSG/Y2KBox (git pull first). Do the PENDING items
in SESSION_SUMMARY.md section 4, then continue the highest-priority unclaimed task in TASKS.md. Follow CLAUDE.md rule 7: do not
build locally, do not poll CI after pushing - commit, push, and end your turn. I will provide a GitHub token via environment/connector."
