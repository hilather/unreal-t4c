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
receipt="${map%.umap}.gen-receipt.json"
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
# Model a coordinator commit and fresh checkout using an isolated Git index/tree.
# Never stage the worker's maps/receipt in the real index. Only tracked bytes are
# copied back; the old ignored Saved receipt is absent on the first invocation.
index="$logs/checkout.index"
checkout="$logs/checkout"
rm -f "$index"
mkdir -p "$checkout"
map_relative=${map#"$repo/"}
receipt_relative=${receipt#"$repo/"}
GIT_INDEX_FILE="$index" git read-tree --empty
for path in "$map_relative" "$receipt_relative"; do
    blob=$(git hash-object -w --no-filters "$repo/$path")
    GIT_INDEX_FILE="$index" git update-index --add --cacheinfo "100644,$blob,$path"
done
tree=$(GIT_INDEX_FILE="$index" git write-tree)
GIT_INDEX_FILE="$index" git read-tree "$tree"
GIT_INDEX_FILE="$index" git checkout-index --all --force --prefix="$checkout/"
cmp "$receipt" "$checkout/$receipt_relative"
cp "$checkout/$map_relative" "$map"
cp "$checkout/$receipt_relative" "$receipt"
rm -f "$repo/Saved/MapGeneration/L_TempleB2.semantic-receipt"
python3 - "$receipt" "$map" <<'JSON_CHECK'
import hashlib, json, pathlib, sys
receipt = json.loads(pathlib.Path(sys.argv[1]).read_text(encoding='utf-8-sig'))
assert receipt['version'] == 1
assert len(receipt['content_md5']) == 32
assert receipt['map_md5'] == hashlib.md5(pathlib.Path(sys.argv[2]).read_bytes()).hexdigest()
JSON_CHECK
for run in fresh-checkout repeat2; do
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
# Negative controls: neither a matching content signature with wrong map bytes,
# nor matching map bytes with a stale content signature may authorize a skip.
for control in map_md5 content_md5; do
    python3 - "$receipt" "$control" <<'MUTATE_RECEIPT'
import json, pathlib, sys
path = pathlib.Path(sys.argv[1])
text = path.read_text(encoding='utf-8-sig')
data = json.loads(text)
path.write_text(text.replace(data[sys.argv[2]], '0' * 32))
MUTATE_RECEIPT
    run_generator "mismatch-$control"
    rg -q 'B2 semantic content or map receipt changed .*saving' "$logs/mismatch-$control.log"
    run_generator "restored-$control"
    rg -q 'B2 semantic content unchanged .*preserving map bytes' "$logs/restored-$control.log"
done
printf 'PASS: tracked JSON/map checkout without ignored receipt preserved B2; changed lighting/default/sky/exposure/recipe detected; two B1 regenerations (including 97 extra transient B1 pieces) left B2 byte-identical.\n'
