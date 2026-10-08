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
"$editor" "$LH_PROJECT" -run=LHGenerateDevMaps -unattended -nullrhi -nosplash -nosound -log
