# Diagnostic captures

This directory holds selected diagnostic logs captured manually from Y2KBox.

Capture logs normally and add selected captures here as `.log.gz` files.
Full (`on`) captures cannot be manually stopped: Ctrl+Alt+L is ignored while
Full mode is active, and the capture automatically stops after two seconds.
The hotkey remains available to start Full mode and to manually stop Simple mode.
Completed Full captures are compressed by Y2KBox after stderr is restored using zlib level 9 (maximum compression) in a background worker, keeping compression off the QEMU main thread. Simple
captures remain uncompressed `.log` files; compress those manually when they
are worth keeping. Automatic capture compression writes to the launch working
directory and does not add files to this repository. These are ordinary text
logs inside standard gzip archives; no special viewer or automated upload
process is required.

Use filenames that identify the session and capture mode, for example:

- `session-126-simple.log.gz`
- `session-126-full.log.gz`

When asking a coding agent to investigate a capture, give it the exact
repository-relative filename. The agent can read the compressed log from its
local repository checkout without requiring the entire log to be pasted or
uploaded into the chat.

On systems with gzip:

```sh
gzip -dc logs/captures/session-126-simple.log.gz
```

To keep the repository useful, commit captures that provide diagnostic value
for an investigation rather than every temporary capture. Avoid including
unrelated personal data or secrets in logs.

## Joint Operations: D3D8 environment-mapped canopy investigation

Full-mode texgen diagnostics now emit these records when a validated draw uses
3D texture-coordinate transforms or nonzero texture-coordinate generation:

- `VMVGA-D3D9-TEXGEN-MATRIX`: guest WORLD, VIEW and TEXTURE0 matrices, each
  tagged with frame, FIFO/event sequence, context and validity.
- `VMVGA-D3D9-TEXGEN-STATE`, `VMVGA-D3D9-TEXGEN-MATERIAL`, and
  `VMVGA-D3D9-TEXGEN-LIGHT`: transform flags/generation, material colors and
  shininess, plus enabled light data.
- `VMVGA-CUBEMAP-SHADOW`: bound cube texture SID and per-face level-zero
  dimensions, pitch, byte count and FNV-1a checksum of the CPU shadow. Hashing
  is capped at 2 MiB per record; missing or over-budget faces are explicit.

These checksums identify CPU-shadow contents only; they do not guarantee that
the shadow is current with GPU-rendered contents or that the face orientation
is correct. These records are Full-mode/capture gated and do not change
rendering, synchronization, upload policy or GPU scheduling.

## Full-mode resource and draw records

The opt-in `on` capture adds high-volume records that are absent from `simple`:

- `VMVGA-D3D9-BUFFER-DEFER`: each guest buffer write range and the merged deferred range.
- `VMVGA-D3D9-BUFFER-UPLOAD`: the native D3D9 Lock range, flags/mode, decision reason, flush cause, host capacity and copied bytes. Direct full-buffer DISCARD uploads and deferred range flushes are separate records.
- `VMVGA-RESOURCE-UPDATE`: guest-backed image/buffer subresource, update box, row/plane layout, byte count and selected backend. The record is emitted after guest MOB bytes reach the CPU shadow, before native upload completion.
- `VMVGA-D3D9-GEOMETRY`: FIFO-sequenced draw-range summary with render target, viewport, primitive/index range, stream count, and CPU-shadow raw vertex-input position/TEXCOORD0 UV bounds for FLOAT1–FLOAT4 elements. Both indexed and non-indexed draws are decoded. `bounds_reason` identifies unsupported formats, missing shadows, invalid ranges/indices, instancing, or the 65,536-vertex safety cap. These are raw vertex-input values, not screen-space bounds.
- `VMVGA-FRAME`: capture-local `frame_id` boundaries at guest `SVGA_3D_CMD_PRESENT` commands. Draws before the next guest PRESENT belong to the current ID. This is a guest-command frame boundary, not a host SDL presentation/vblank identifier; `seq` remains the event/FIFO correlation sequence and is not a frame number.
- `VMVGA-D3D9-STATE-SNAPSHOT`: one snapshot for each validated D3D9 draw command, including `frame`, `seq`, context, viewport/scissor, all bound render-target slots with live dimensions/format, shader bindings, every valid texture/sampler state, bound texture SIDs with live dimensions/format/mip count, and every valid render state. It records the guest context state at draw submission so the active state can be reconstructed without guessing from isolated bind events; it does not claim that native host state application succeeded.
- `Y2KBOX-DIAG pixel_snapshot=one_png_on_stop`: Full mode writes one PNG beside the log when the build has the existing optional libpng and Pixman support (`CONFIG_PNG` and `CONFIG_PIXMAN`). It copies the last CPU-side guest `DisplaySurface` into RGB before encoding, so it does not call `SDL_RenderReadPixels`/`glReadPixels` or force a GPU readback. Direct GPU scanout is skipped. Builds without those existing features log `pixel_snapshot=unavailable`; after capture, the log records `pixel_snapshot_result=saved`, `not-saved`, or `unavailable`. No new image dependency is added.
- `VMVGA-RENDER-CORRELATION`: readback submit/ready/retire events tagged with the last observed FIFO sequence. Since readback polling can be asynchronous, `last_fifo_seq` is a correlation hint, not always the originating command.

The graphics records do not dump texture-resource pixels or force GPU readbacks. The separate one-PNG snapshot captures only the CPU-side guest display surface. Do not infer that a SID is the minimap/HUD from its dimensions or draw pattern alone; use the resource updates, draw bindings, geometry evidence, snapshot and owner-visible capture together. The added records are full-mode only; simple-mode output remains compact.
