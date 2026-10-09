#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/lh-env.sh"
[[ $# == 0 ]] || { echo "Usage: $0" >&2; exit 2; }
[[ $(id -u) != 0 ]] || { echo 'Unreal refuses root; run as normal host user' >&2; exit 1; }
cd "$LH_PROJECT_ROOT"
for map in L_LighthavenTempleDistrict L_TempleB{1,2,3,4}; do
    file="Content/Lighthaven/Maps/$map.umap"
    [[ -f $file ]] || { echo "Missing map: $file" >&2; exit 1; }
    if head -c 100 "$file" | grep -q 'version https://git-lfs'; then
        echo "LFS pointer: $file; hydrate real maps with git lfs pull on host" >&2; exit 1
    fi
done
mkdir -p Saved/BuildEnvironment/config Config/Lighthaven
report=$(mktemp -d "$PWD/Saved/ArrivalReview.XXXXXX")
rm -f Saved/ArrivalSafety.tsv
set +e
XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config" \
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$LH_PROJECT" \
    '-ExecCmds=Automation RunTests Lighthaven.Integration.Wave3.ArrivalSafety; Quit' \
    -DDC-ForceMemoryCache -nullrhi -unattended -nop4 -nosound \
    '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0' \
    "-ReportExportPath=$report" "-abslog=$report/editor.log" '-TestExit=Automation Test Queue Empty'
editor_status=$?
set -e
python3 - "$report/index.json" Saved/ArrivalSafety.tsv Config/Lighthaven/ReviewedArrivals.tsv "$editor_status" <<'PY'
import json, pathlib, sys
report, evidence, output, status = sys.argv[1:]
data = json.loads(pathlib.Path(report).read_text(encoding='utf-8-sig'))
tests = data.get('tests', [])
if len(tests) != 1 or tests[0].get('state') not in ('Success', 'Fail'):
    sys.exit('No completed arrival test; reviewed list unchanged')
rows = [line.split('\t') for line in pathlib.Path(evidence).read_text().splitlines()]
if len(rows) != 9 or len({r[0] for r in rows}) != 9 or any(len(r)!=3 or r[2] not in ('PASS','FAIL') or len(r[1])!=40 for r in rows):
    sys.exit('Invalid/incomplete evidence; reviewed list unchanged')
passed = sorted('\t'.join(r[:2]) for r in rows if r[2]=='PASS')
target = pathlib.Path(output); temp = target.with_suffix('.tmp')
temp.write_text(''.join(r+'\n' for r in passed)); temp.replace(target)
for key, digest, result in rows: print(f'{result} {key} {digest}')
print(f'Reviewed {len(passed)}/9; regenerate all maps and run LHValidateWorld. Evidence: {report}')
if status != '0' or len(passed)!=9 or tests[0]['state']!='Success': sys.exit(1)
PY
