# Diagnostic captures

This directory holds selected diagnostic logs captured manually from Y2KBox.

Capture logs normally and add selected captures here as `.log.gz` files.
Full (`on`) captures are automatically stopped after two seconds (or earlier
with Ctrl+Alt+L) and compressed by Y2KBox after stderr is restored using zlib level 9 (maximum compression) in a background worker, keeping compression off the QEMU main thread. Simple
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

## Full-mode resource and draw records

The opt-in `on` capture adds high-volume records that are absent from `simple`:

- `VMVGA-D3D9-BUFFER-DEFER`: each guest buffer write range and the merged deferred range.
- `VMVGA-D3D9-BUFFER-UPLOAD`: the native D3D9 Lock range, flags/mode, decision reason, flush cause, host capacity and copied bytes. Direct full-buffer DISCARD uploads and deferred range flushes are separate records.
- `VMVGA-RESOURCE-UPDATE`: guest-backed image/buffer subresource, update box, row/plane layout, byte count and selected backend. The record is emitted after guest MOB bytes reach the CPU shadow, before native upload completion.
- `VMVGA-D3D9-GEOMETRY`: FIFO-sequenced draw-range summary with render target, viewport, primitive/index range and stream count. Position/UV bounds are currently explicitly unavailable (`bounds_reason=vertex-decode-not-implemented`).
- `VMVGA-RENDER-CORRELATION`: readback submit/ready/retire events tagged with the last observed FIFO sequence. Since readback polling can be asynchronous, `last_fifo_seq` is a correlation hint, not always the originating command.

These records do not dump texture pixels or force GPU readbacks. Do not infer that a SID is the minimap/HUD from its dimensions or draw pattern alone; use the resource updates, draw bindings, geometry evidence and owner-visible capture together. The added records are full-mode only; simple-mode output remains compact.
