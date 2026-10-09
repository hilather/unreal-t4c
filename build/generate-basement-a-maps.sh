#!/usr/bin/env bash
set -euo pipefail
if [[ "$(id -u)" == 0 ]]; then
    echo "Unreal refuses to run as root; run on the host as a normal user" >&2
    exit 1
fi
source "$(dirname -- "${BASH_SOURCE[0]}")/lh-env.sh"
[[ $# == 0 ]] || { echo "Usage: $0" >&2; exit 2; }
editor="$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd"
[[ -x "$editor" ]] || { echo "Command editor missing or not executable: $editor" >&2; exit 1; }
cd -- "$LH_PROJECT_ROOT"
for map in L_TempleB1 L_TempleB2; do
    path="Content/Lighthaven/Maps/$map.umap"
    if [[ -e "$path" && ! -w "$path" ]]; then
        echo "lockable LFS map is read-only: run git lfs lock $path or, as the single local writer, chmod u+w $path" >&2
        exit 1
    fi
done
cache_args=()
# Optional filesystem cache for isolated workers where Zen cannot bind a local port.
if [[ ${LH_NO_ZEN:-0} == 1 ]]; then
    cache_args+=(-ddc=InstalledNoZenLocalFallback "-LocalDataCachePath=$LH_PROJECT_ROOT/DerivedDataCache")
fi
"$editor" "$LH_PROJECT" "${cache_args[@]}" -run=LHGenerateBasementAMaps -unattended -nullrhi -nosplash -nosound -NoAnalytics -NoCrashDialog -log
