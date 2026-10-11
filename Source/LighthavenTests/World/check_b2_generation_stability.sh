#!/usr/bin/env bash
# Run after both targets are built and all five local maps are generated.
# This intentionally regenerates B1 and leaves generated maps for subsequent checks;
# the worker/integrator must restore their original .umap pointers before committing.
set -euo pipefail
repo=$(git rev-parse --show-toplevel)
engine=${UE_ROOT:-/home/brewerm/Downloads/unreal}
editor="$engine/Engine/Binaries/Linux/UnrealEditor-Cmd"
project="$repo/Lighthaven.uproject"
map="$repo/Content/Lighthaven/Maps/L_TempleB2.umap"
logs="$repo/Saved/B2GenerationStability"
mkdir -p "$logs" "$repo/Saved/BuildEnvironment/config" "$repo/Saved/DerivedDataCache"
export XDG_CONFIG_HOME="$repo/Saved/BuildEnvironment/config"

run_generator() {
    local run=$1
    shift
    "$editor" "$project" -run=LHGenerateBasementAMaps "$@" \
        '-DDC=(Local)' "-LocalDataCachePath=$repo/Saved/DerivedDataCache" \
        -nullrhi -unattended -nosound -nop4 -NoCrashDialog \
        '-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False' \
        '-ini:EditorSettings:[/Script/UnrealEd.CrashReportsPrivacySettings]:bSendUnattendedBugReports=False' \
        '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0' \
        > "$logs/$run.log" 2>&1 || {
            tail -n 50 "$logs/$run.log" >&2
            return 1
        }
    rg 'B2 semantic|Generated /Game/Lighthaven/Maps/L_TempleB1' "$logs/$run.log"
}

# First adoption (or changed output) establishes the paired semantic/file receipt.
# Each mutation must change the snapshot and restoration must return to the baseline.
run_generator prime -VerifyB2Fingerprint
rg -q 'B2 semantic mutation controls and restoration: passed' "$logs/prime.log"
for field in 'point intensity' 'class default' 'sky intensity' 'post process' 'visual recipe'; do
    rg -q "B2 semantic mutation $field: detected" "$logs/prime.log"
done
before=$(sha256sum "$map" | cut -d ' ' -f 1)
for run in repeat1 repeat2; do
    if [[ "$run" == repeat2 ]]; then
        run_generator "$run" -B1AllocationProbe
        rg -q 'B1 allocation probe: 97 unsaved transient pieces before B2' "$logs/$run.log"
    else
        run_generator "$run"
    fi
    rg -q 'B2 semantic content unchanged .*preserving map bytes' "$logs/$run.log"
    after=$(sha256sum "$map" | cut -d ' ' -f 1)
    if [[ "$before" != "$after" ]]; then
        printf 'FAIL: B2 bytes changed on %s (%s -> %s)\n' "$run" "$before" "$after" >&2
        exit 1
    fi
    printf 'PASS: %s B2 SHA-256 %s\n' "$run" "$after"
done
printf 'PASS: changed lighting/default/sky/exposure/recipe detected; two B1 regenerations (including 97 extra transient B1 pieces) left B2 byte-identical.\n'
