#!/usr/bin/env bash
set -euo pipefail
# shellcheck source=build/lh-env.sh
source "$(dirname -- "${BASH_SOURCE[0]}")/lh-env.sh"
# No arguments selects the explicit G1 map set; arguments replace it.
if [[ $# == 0 ]]; then
    set -- /Game/Lighthaven/Maps/Dev_Combat /Game/Lighthaven/Maps/Dev_Movement
fi
for map in "$@"; do
    if [[ ( "$map" != /Game/* && "$map" != /Engine/* ) || "$map" == *'+'* || "$map" == *'.umap' || "$map" == *[[:space:]]* ]]; then
        echo "Usage: $0 [/Game/path/to/Map /Engine/path/to/Map ...] (explicit maps, without .umap or +)" >&2
        exit 2
    fi
done
maps=$(IFS=+; echo "$*")
uat="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
[[ -f "$uat" ]] || { echo "AutomationTool script missing: $uat" >&2; exit 1; }
run_id="$(date -u +%Y%m%dT%H%M%SZ)-$$"
archive="$LH_BUILD_DIR/output/Linux-$run_id"
log_dir="$LH_PROJECT_ROOT/Saved/Logs/Package-$run_id"
mkdir -p -- "$log_dir" "$archive"
cd -- "$LH_PROJECT_ROOT"
bash "$uat" BuildCookRun "-project=$LH_PROJECT" -noP4 -platform=Linux \
    -clientconfig=Development -build -cook -stage -pak -package -archive \
    "-archivedirectory=$archive" "-map=$maps" -utf8output 2>&1 | tee "$log_dir/uat.log"
echo "Archive: $archive"
echo "UAT log: $log_dir/uat.log"
