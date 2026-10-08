#!/usr/bin/env bash
set -euo pipefail
# shellcheck source=build/lh-env.sh
source "$(dirname -- "${BASH_SOURCE[0]}")/lh-env.sh"
if [[ $# -gt 1 ]]; then
    echo "Usage: $0 [test-filter] (default: Lighthaven)" >&2
    exit 2
fi
filter="${1:-Lighthaven}"
if [[ "$filter" == *';'* || "$filter" == *$'\n'* || "$filter" == *$'\r'* ]]; then
    echo "Test filter cannot contain command separators or newlines." >&2
    exit 2
fi
if [[ "$(id -u)" == 0 ]]; then
    echo "Unreal refuses to run as root; run on the host as a normal user" >&2
    exit 1
fi
editor="$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd"
[[ -x "$editor" ]] || { echo "Command editor missing or not executable: $editor" >&2; exit 1; }
mkdir -p -- "$LH_PROJECT_ROOT/Saved"
report_dir="$(mktemp -d "$LH_PROJECT_ROOT/Saved/Automation.XXXXXX")"
cd -- "$LH_PROJECT_ROOT"
echo "Automation evidence: $report_dir"
"$editor" "$LH_PROJECT" "-ExecCmds=Automation RunTests $filter; Quit" \
    -unattended -nullrhi -nosplash -nosound -NoSound -log \
    "-ReportExportPath=$report_dir" "-abslog=$report_dir/editor.log" \
    "-TestExit=Automation Test Queue Empty" 2>&1 | tee "$report_dir/console.log"
# A zero editor exit is insufficient: require completed test evidence.
python3 - "$report_dir/index.json" <<'PY'
import json, sys
try:
    with open(sys.argv[1], encoding="utf-8-sig") as stream:
        report = json.load(stream)
    tests = report["tests"]
    if not isinstance(tests, list) or not tests:
        raise ValueError("no tests ran")
    states = [test["state"] for test in tests]
    if any(state != "Success" for state in states):
        raise ValueError(f"failed, skipped, incomplete or unknown test states: {states}")
    if report.get("failed", 0) or report.get("notRun", 0) or report.get("inProcess", 0):
        raise ValueError("report contains failed or unfinished tests")
    print(f"Automation: {len(states)} completed successful tests")
except (OSError, ValueError, KeyError, TypeError) as error:
    sys.exit(f"Automation failed: {error}; inspect exported report and editor logs")
PY
