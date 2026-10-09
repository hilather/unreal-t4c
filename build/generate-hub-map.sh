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
for map in L_LighthavenTempleDistrict; do
    path="Content/Lighthaven/Maps/$map.umap"
    if [[ -e "$path" && ! -w "$path" ]]; then
        echo "lockable LFS map is read-only: run git lfs lock $path or, as the single local writer, chmod u+w $path" >&2
        exit 1
    fi
done
"$editor" "$LH_PROJECT" -run=LHGenerateHubMap -DDC='(Local)' "-LocalDataCachePath=$LH_PROJECT_ROOT/Saved/DerivedDataCache" -unattended -nullrhi -nosplash -nosound -log
