# Diagnostic captures

This directory holds selected diagnostic logs captured manually from Y2KBox.

Capture logs normally, compress the ones worth keeping with gzip, and add the
`.log.gz` files here. These are ordinary text logs inside standard gzip
archives; no special viewer or automated upload process is required.

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
