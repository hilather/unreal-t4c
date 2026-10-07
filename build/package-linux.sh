#!/usr/bin/env bash
set -euo pipefail
# shellcheck source=build/lh-env.sh
source "$(dirname -- "${BASH_SOURCE[0]}")/lh-env.sh"
if [[ $# != 1 || "$1" != /Game/* || "$1" == *'+'* || "$1" == *'.umap' ]]; then
    echo "Usage: $0 /Game/path/to/Map (one explicit map, without .umap)" >&2
    exit 2
fi
uat="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
[[ -f "$uat" ]] || { echo "AutomationTool script missing: $uat" >&2; exit 1; }
run_id="$(date -u +%Y%m%dT%H%M%SZ)-$$"
archive="$LH_BUILD_DIR/output/Linux-$run_id"
log_dir="$LH_PROJECT_ROOT/Saved/Logs/Package-$run_id"
mkdir -p -- "$log_dir" "$archive"
cd -- "$LH_PROJECT_ROOT"
bash "$uat" BuildCookRun "-project=$LH_PROJECT" -noP4 -platform=Linux \
    -clientconfig=Development -build -cook -stage -pak -package -archive \
    "-archivedirectory=$archive" "-map=$1" -utf8output 2>&1 | tee "$log_dir/uat.log"
echo "Archive: $archive"
echo "UAT log: $log_dir/uat.log"
