# Y2KBox active session context

> Rolling working summary derived from `WORKLOG.md`. This is a navigation and continuity aid, not a replacement for the append-only historical record. `TASKS.md` remains authoritative for task status; `HANDOFF.md` remains authoritative for project decisions and operating instructions. The ten sessions below are the latest ten entries currently recorded in `WORKLOG.md` (Sessions 100–109).

## How to use this file

- **New coding-agent session / fresh sandbox:** follow `AGENTS.md` (canonical shared policy); use `CLAUDE.md` or `GEMINI.md` only for tool-specific integration. Read this summary and `WORKLOG_INDEX.md`, plus only HANDOFF sections 0-3 and the selected task entry in `TASKS.md`. Do not read entire HANDOFF/TASKS/WORKLOG files by default. Use the index for relevant historical sections and verify old claims against current source and owner decisions.
- **Continuing in the same coding-agent session:** do not reread onboarding files or the entire `WORKLOG.md` without a reason. Use this rolling summary for recent context, then consult `HANDOFF.md` and relevant `TASKS.md` entries. Open only specific historical sections needed to verify an older decision or result.
- A new user prompt, task, or turn does **not** by itself make a new coding-agent session. A genuinely new agent runtime/sandbox with no retained working context does. After context compaction in the same runtime, use this summary rather than restarting the full-history read.
- At the end of each work session, append the session to `WORKLOG.md` first, then refresh this file so it contains exactly the latest ten worklog sessions. Never rewrite or trim `WORKLOG.md`.

## Current project state and working constraints

- Repository: [AhayriSG/Y2KBox](https://github.com/AhayriSG/Y2KBox), `main`.
- This summary can lag behind `main`; verify the current branch HEAD and task status before making changes.
- Y2KBox targets 32-bit Windows XP. Linux product runtime is KVM-only; Windows product runtime is TCG-only. Performance optimization work targets Windows TCG.
- VMware SVGA/SVGA3D via VMVGA is the sole active display/3D path. SDL is a fixed-size virtual monitor: guest resolution controls window client size; host resizing must not change guest geometry; fullscreen toggle is Ctrl+Alt+F.
- Keep the owner-approved guest/device constraints in `HANDOFF.md`; do not infer that a standalone test or green CI proves in-game correctness.
- **Build/CI workflow:** Gemini 4 Neon alone has standing authorization for ordinary local QEMU builds and standard checks; all other agents need explicit owner authorization. Gemini 4 Neon also has standing authorization for relevant, bounded benchmarks/A-B; other agents still need explicit authorization. GitHub Actions remains authoritative; report local results separately. Markdown-only commits run only the lightweight detector; Linux/Windows jobs are skipped. After pushing, do not poll CI.
- Diagnostic captures are independent sessions: every Ctrl+Alt+L start resets counters and writes a new timestamped `.log`; stopping ends that capture interval without restarting the VM.

## Latest ten worklog sessions

## Current investigation / next actions

1. **T13e-F1 / Windows TCG profiling remains active.** The owner supplied a dry-weather capture on 2026-11-23 with `capture_ms=23526`; `cpu.exec` reported 3,415 intervals, 23.515560 s total, 19.554 ms max, 3,164 intervals over 1 ms, 1,064 over 10 ms, and none over 100 ms. This is an interval summary, not workload attribution; inspect the complete capture and compare only equivalent scenes/settings before proposing code changes.
2. **Hitman 2: Silent Assassin D3D8 visual issue remains open.** The owner supplied a sub-second menu capture while investigating static-background rendering. Analyze that capture and current VMVGA/D3D8 path; do not claim a fix or infer visual correctness from CI.
3. **Diagnostic capture coverage remains under investigation (T19).** Separate timestamped sessions and per-capture counter resets have been exercised, but unusually small later log files compared with the first capture need category-by-category comparison before declaring all output coverage complete.
4. **T25 guest OpenGL investigation is PAUSED (low feasibility; awaiting independent reassessment).** Native XP GDI Generic exposes OpenGL 1.1.0 with three extensions; COD1 with software GL creates/makes-current a context but fails its feature check (HANDOFF 8d / Session 104). VMware documents software-emulated OpenGL in XP guests (8e / 105), and conventional XP ICD discovery requires the active display driver's OPENGL_GETINFO escape (8f / 106). QindieGL is a concrete but unproven app-local GL-to-D3D9 lead: its source lacks `wglGetExtensionsStringEXT`, and a historical minimal-entry-point experiment reportedly reached COD1 startup with severe graphics corruption (8h / 108). TitaniumGL is a separate lead: the owner observed COD1 loading its DLL, creating/making-current an OpenGL 1.4 context, and seeing numerous extensions plus renderer `VMware SVGA II/119MB VRAM`; COD1 still rejects the card, with the same broad result in hardware and software modes. The `0 PFDs found` line and failed capability check are unresolved (8i / 109). Do not prototype, modify the guest/driver, build, or test while T25 remains paused. See HANDOFF 8g–8i / Sessions 107–109.
5. Keep the F1 crowded-grid, FIFA residual hitches, and Hitman 2 D3D8 rendering issue as distinct investigations unless evidence connects them. Do not reopen the resolved general FlatView scene-transition stall without new evidence.
6. Update the selected task entry and append the next worklog session when new implementation, measurements, or verified conclusions are produced.

## Key diagnostic interpretation rules

- Performance counters can be nested. Do not add child phase totals to enclosing command/FIFO totals.
- Counters whose implementation uses `qemu_perf_stats_mark()` measure intervals between marks (event cadence), not operation execution time.
- Overlapping lock dimensions (aggregate, vertex/index, full/partial, flags, size buckets) are not additive.
- A green CI run proves build/test coverage only; it does not prove XP guest gameplay or visual correctness.
- Keep the exact same repeatable owner workload for performance comparisons. Distinguish measured facts, owner observations, hypotheses, and verified fixes.


### Session 100 — Start guest OpenGL investigation and audit the SVGA3D path
- Began T25 (IN PROGRESS), targeting OpenGL 1.1 baseline through OpenGL 2.1 maximum.
- Corrected the protocol model: Mesa translates OpenGL into SVGA3D commands; there is no separate raw OpenGL FIFO. Y2KBox already whitelists some legacy commands used by SVGA3D, but complete GL 2.1 semantics and caps are unverified.
- The latest QEMU diagnostic log has no WGL/PFD output. A guest-side probe is needed to enumerate pixel formats, distinguish generic software from ICD formats, and test context creation.
- Mesa's documented SVGA guest driver stack is Linux-specific; historical Workstation 6 documentation describes XP OpenGL as software-emulated but does not establish VMware Tools 10.0.12 behavior. README Fixed target wording was previewed and awaits owner approval. No source changes, diagnostic utility, builds, tests, guest validation, or CI polling.

### Session 101 — Clarify the guest OpenGL target in the README
- Updated the README Fixed target to include OpenGL 1.1 as the baseline and OpenGL 2.1 as the maximum, explicitly noting that guest OpenGL is not yet implemented.
- T25 remains an investigation: a complete 2.1 path must also preserve compatibility with older 1.x applications, and the Call of Duty 1 WGL pixel-format failure still needs an XP-side probe.
- Documentation-only; no emulator source changes, diagnostic utility, build, benchmark, guest test, or CI polling.

### Session 102 — Build an XP-side COD1 WGL pixel-format probe
- Identified COD1's GLW_ChoosePFD routine as the custom id Tech 3 PFD enumerator, not Windows ChoosePixelFormat. It rejects generic formats by default, requires RGBA/window/OpenGL/double-buffer flags, at least 15 depth bits, and at least 4 stencil bits when stencil is requested.
- Added xp/tools/glprobe.c and its README. It mirrors the game's filtering/best-match logic for 32/24/8 and 32/24/0, explains per-format rejections, compares software-allowed selection, and tests WGL context creation plus GL vendor/renderer/version.
- T25 now points to the probe. Next is an owner-run XP capture; the utility has not been compiled or run. No QEMU build, guest test, benchmark, or CI polling.

### Session 103 — Match Mesa's VGPU9 baseline and identify the COD1 software fallback
- Mesa VGPU9 reports GLSL 1.20 and requires SVGA hardware version WS8_B1 plus VS/PS 3.0. Y2KBox advertises WS8_B1 and SM3.0 and implements handlers for the core legacy SVGA3D command family; semantic/capability completeness remains unproven.
- VMware Workstation Player 15 documentation says OpenGL apps in XP guests run in software-emulation mode. Added a COD1 test with +set r_allowSoftwareGL 1 to allow generic formats and see whether software OpenGL can get past pixel-format selection.
- T25 updated with protocol matching, candidate state gaps, and the software fallback test. No QEMU build, standalone probe compile/run, guest test, benchmark, or CI polling.


### Session 104 — Identify the active XP guest OpenGL implementation
- Owner-run COD1 software-OpenGL testing now reaches successful WGL context creation but fails the game's feature check.
- GLview reports Microsoft / GDI Generic / OpenGL 1.1.0 and only three extensions: `GL_EXT_bgra`, `GL_EXT_paletted_texture`, and `GL_WIN_swap_hint`. Detailed extension checks and limits are recorded in HANDOFF.md section 8d.
- Next investigate why VMware Tools/SVGA is not loading a VMware OpenGL ICD. No Visual Studio installation or guest-side compilation is required; do not repeat the GLview test.
- Documentation-only; no source changes, builds, benchmarks, additional guest tests, or CI polling.


### Session 105 — Establish VMware's documented XP OpenGL limitation
- VMware Workstation Player 15 documentation explicitly states OpenGL applications in Windows XP guests run in software-emulation mode; VMware Tools 10.0.12 is the last Tools release for XP.
- This makes the owner's GDI Generic 1.1.0 result expected; do not pursue a Tools reinstall or missing-ICD file hunt without new evidence.
- WINVBE 1.3 provides an XP software OpenGL 2.1 ICD but requires replacing the display-driver stack and offers no hardware 3D; optional isolated test only.
- T25's core requirement is a Windows XP guest OpenGL ICD that translates GL calls into SVGA3D commands. Documentation-only; no guest test, source changes, build, benchmark, or CI polling.


### Session 106 — Establish the Windows XP OpenGL ICD discovery constraint
- A conventional XP ICD must be advertised by the active display driver through the OPENGL_GETINFO escape; a registry entry alone cannot activate an arbitrary ICD alongside VMware's driver.
- The only plausible no-display-driver-replacement path is a per-application standalone OpenGL DLL. The id Tech 3 r_glDriver path may make this possible for COD1, but its exact fork behavior must be verified.
- Existing XP Direct3D 9 acceleration could provide the transport. GL4ES plus a historical ANGLE D3D9 backend is a high-risk candidate, not a known-good XP stack.
- Next gate: prove standalone DLL loading and D3D9 HAL device creation/Present before writing any GL compatibility code. Documentation-only; no guest modifications, builds, tests, or CI polling.


### Session 107 — Pause guest OpenGL work for independent reassessment
- README now describes OpenGL as a low-probability research objective and mentions application-local wrapper DLLs only as one unproven possibility.
- T25 is PAUSED pending an independent reassessment. The next reviewer should challenge the current hypotheses and investigate alternative architectures before proposing a prototype.
- HANDOFF section 8g records the reassessment brief and makes clear that no agent has been assigned or dispatched. No guest changes, builds, tests, or CI polling.

### Session 108 — Record QindieGL as a concrete but unproven OpenGL wrapper lead
- Added QindieGL to T25's technical research record, including its missing `wglGetExtensionsStringEXT` implementation and the owner's COD1 failure.
- Recorded the historical experiment that reportedly allowed COD1 to start but left substantial graphical corruption, and flagged GPL-3.0 compatibility review before any code reuse.
- T25 remained PAUSED; no patch, build, guest modification, or test was authorized.

### Session 109 — Record TitaniumGL as a separate OpenGL wrapper research lead
- Removed the QindieGL-specific paragraph from README.md while preserving the generic statement that application-local wrapper DLLs remain an unproven alternative.
- Added TitaniumGL's COD1 OpenGL 1.4/context/extension observations, the unresolved `0 PFDs found` line, and the same broad rejection in hardware and software modes to TASKS.md and HANDOFF.md section 8i.
- Kept TitaniumGL separate from QindieGL and T25 PAUSED. Documentation-only; no source changes, builds, guest modifications, additional guest tests, benchmarks, or CI polling.
