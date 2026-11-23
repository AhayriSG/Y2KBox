#!/usr/bin/env bash
# Token-optimised CI + Codacy status for AhayriSG/Y2KBox.
# Run by the Claude Code hooks in .claude/settings.json. Prints NOTHING (exit 0, ~2 ms) once the current HEAD's
# CI run has completed and been reported once. Never hardcode secrets: CODACY_API_TOKEN (and the optional
# GITHUB_TOKEN, only used to raise the API rate limit) are read from the environment.
set -u

REPO="AhayriSG/Y2KBox"
HEAD_FULL=$(git rev-parse HEAD 2>/dev/null || echo none)
HEAD_SHA=${HEAD_FULL:0:7}
CACHE_FILE="/tmp/y2kbox_ci_checked_${HEAD_SHA}"

[ -f "$CACHE_FILE" ] && exit 0
[ "$HEAD_FULL" = "none" ] && exit 0

# Optional auth only raises the API rate limit (unauthenticated = 60 requests/hour per IP). Never stored in files.
GH_T="${GITHUB_TOKEN:-${GH_TOKEN:-}}"
if [ -z "$GH_T" ] && command -v gh >/dev/null 2>&1; then GH_T=$(gh auth token 2>/dev/null || true); fi

gh_api() {
  local -a headers=(-H "Accept: application/vnd.github+json")
  if [ -n "$GH_T" ]; then
    headers+=(-H "Authorization: Bearer $GH_T")
  fi
  curl -s --max-time 8 "${headers[@]}" \
    "https://api.github.com/repos/${REPO}/$1"
}

# Prefer the run for HEAD itself; fall back to the latest run (e.g. HEAD pushed seconds ago, no run yet).
RUN_JSON=$(gh_api "actions/runs?head_sha=${HEAD_FULL}&per_page=1")
CI_LINE=$(printf '%s' "$RUN_JSON" | python3 -c "
import sys, json
try:
    d = json.load(sys.stdin)
    runs = d.get('workflow_runs')
    if runs is None and 'rate limit' in str(d.get('message', '')).lower():
        print('RATE_LIMIT'); sys.exit()
except Exception:
    runs = None
if runs is None:
    print('ERROR')
elif not runs:
    print('NO_RUN')
else:
    r = runs[0]
    print('%s|%s|%s|%s' % (r['head_sha'][:7], r['status'], r['conclusion'], r['id']))
" 2>/dev/null || echo "ERROR")

if [ "$CI_LINE" = "RATE_LIMIT" ]; then
  echo "=== CI & Codacy Update (HEAD ${HEAD_SHA}) ==="
  echo "GitHub API rate limit hit: CI status unknown. Set GITHUB_TOKEN (or run 'gh auth login') to raise the limit. Do not poll."
  exit 0
fi

if [ "$CI_LINE" = "NO_RUN" ] || [ "$CI_LINE" = "ERROR" ]; then
  LATEST=$(gh_api "actions/runs?per_page=1" | python3 -c "
import sys, json
try:
    r = json.load(sys.stdin)['workflow_runs'][0]
    print('%s|%s|%s|%s' % (r['head_sha'][:7], r['status'], r['conclusion'], r['id']))
except Exception:
    print('ERROR')
" 2>/dev/null || echo "ERROR")
  IFS='|' read -r RUN_SHA RUN_STATUS RUN_CONCLUSION RUN_ID <<< "$LATEST"
  echo "=== CI & Codacy Update (HEAD ${HEAD_SHA}) ==="
  echo "GitHub Actions: no run for HEAD yet (latest run is for ${RUN_SHA:-?}: status=${RUN_STATUS:-?}, conclusion=${RUN_CONCLUSION:-?}). Push done? Do not poll."
  exit 0
fi

IFS='|' read -r RUN_SHA RUN_STATUS RUN_CONCLUSION RUN_ID <<< "$CI_LINE"
echo "=== CI & Codacy Update (${RUN_SHA}) ==="
echo "GitHub Actions: status=${RUN_STATUS}, conclusion=${RUN_CONCLUSION} (run_id=${RUN_ID}) https://github.com/${REPO}/actions/runs/${RUN_ID}"

if [ "$RUN_STATUS" = "completed" ]; then
  if [ "$RUN_CONCLUSION" = "success" ]; then
    echo "Binaries (Linux + Windows): https://github.com/${REPO}/actions/runs/${RUN_ID}#artifacts"
  else
    # Failed: show the failed jobs/steps and their annotations (configure/build errors) - cheap, bounded output.
    gh_api "actions/runs/${RUN_ID}/jobs" | python3 -c "
import sys, json
try:
    jobs = json.load(sys.stdin).get('jobs', [])
except Exception:
    jobs = []
for j in jobs:
    if j.get('conclusion') not in ('failure', 'cancelled', 'timed_out'):
        continue
    steps = [s['name'] for s in j.get('steps', []) if s.get('conclusion') == 'failure']
    print('  FAILED job=%s step=%s check_run_id=%s' % (j['name'], ','.join(steps) or '?', j['id']))
" 2>/dev/null | head -6
    for JID in $(gh_api "actions/runs/${RUN_ID}/jobs" | python3 -c "
import sys, json
try:
    print(' '.join(str(j['id']) for j in json.load(sys.stdin).get('jobs', []) if j.get('conclusion') == 'failure'))
except Exception:
    pass" 2>/dev/null); do
      gh_api "check-runs/${JID}/annotations" | python3 -c "
import sys, json
try:
    for a in [x for x in json.load(sys.stdin) if x.get('annotation_level') == 'failure'][:3]:
        print('  annotation: ' + (a.get('message') or '')[:700].replace('\n', ' | '))
except Exception:
    pass" 2>/dev/null
    done
    echo "  Fix this regression before new feature work (see CLAUDE.md rule 7)."
  fi
fi

# Codacy (optional). Only possible where the network allows app.codacy.com (not from the Claude sandbox).
CODACY_OK=1
if [ -z "${CODACY_API_TOKEN:-}" ]; then
  echo "Codacy: skipped (CODACY_API_TOKEN not set in the environment)"
else
  FAIL_FILE="/tmp/y2kbox_codacy_fail_${HEAD_SHA}"
  CODACY_OUT=$(curl -s --max-time 8 -X POST \
    "https://app.codacy.com/api/v3/analysis/organizations/gh/AhayriSG/repositories/Y2KBox/issues/search?limit=15" \
    -H "api-token: ${CODACY_API_TOKEN}" -H "Content-Type: application/json" \
    -d '{"branchName": "main", "levels": ["Error", "Warning"]}' | python3 -c "
import sys, json
data = json.load(sys.stdin).get('data', [])
print('Codacy open issues (Error/Warning, first 15): %d' % len(data))
for i in data[:8]:
    p = i.get('patternInfo', {})
    print('  - %s:%s [%s] %s' % (i.get('filePath'), i.get('lineNumber'), p.get('level'), (i.get('message') or '')[:160]))
" 2>/dev/null) || CODACY_OK=0
  if [ "$CODACY_OK" -eq 1 ] && [ -n "$CODACY_OUT" ]; then
    echo "$CODACY_OUT"
  else
    N=$(( $(cat "$FAIL_FILE" 2>/dev/null || echo 0) + 1 )); echo "$N" > "$FAIL_FILE"
    echo "Codacy: request failed or unparseable (attempt ${N}/2)"
    [ "$N" -ge 2 ] && CODACY_OK=1      # stop retrying (and spending tokens) after two failures
  fi
fi

if [ "$RUN_SHA" = "$HEAD_SHA" ] && [ "$RUN_STATUS" = "completed" ] && [ "$CODACY_OK" -eq 1 ]; then
  rm -f /tmp/y2kbox_ci_checked_* /tmp/y2kbox_codacy_fail_*
  touch "$CACHE_FILE"
fi
exit 0
