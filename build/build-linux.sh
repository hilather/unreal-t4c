#!/usr/bin/env bash
set -euo pipefail
# shellcheck source=build/lh-env.sh
source "$(dirname -- "${BASH_SOURCE[0]}")/lh-env.sh"
if [[ $# -gt 1 || (${1:-} != "" && ${1:-} != "--game") ]]; then
    echo "Usage: $0 [--game] (also build Lighthaven game target)" >&2
    exit 2
fi
builder="$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh"
[[ -f "$builder" ]] || { echo "Build script missing: $builder" >&2; exit 1; }
log_dir="$LH_BUILD_DIR/logs/Build-$(date -u +%Y%m%dT%H%M%SZ)-$$"
mkdir -p -- "$log_dir"
cd -- "$LH_PROJECT_ROOT"
bash "$builder" LighthavenEditor Linux Development "-Project=$LH_PROJECT" -WaitMutex 2>&1 | tee "$log_dir/editor.log"
if [[ ${1:-} == "--game" ]]; then
    bash "$builder" Lighthaven Linux Development "-Project=$LH_PROJECT" -WaitMutex 2>&1 | tee "$log_dir/game.log"
fi
echo "Build logs: $log_dir"
