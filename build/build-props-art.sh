#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/lh-env.sh"
: "${BLENDER_ROOT:?Set BLENDER_ROOT to the pinned Blender directory}"
cd "$LH_PROJECT_ROOT"
# Fast replay needs no generated exports, including in a fresh hydrated clone.
if python3 Source/Lighthaven/Visual/Props/props_receipt.py; then exit 0; fi
mkdir -p Saved/ArtExport/props-config
if [[ ${LH_PROPS_SKIP_EXPORT:-0} != 1 ]]; then
XDG_CONFIG_HOME="$PWD/Saved/ArtExport/props-config" "$BLENDER_ROOT/blender" --background --factory-startup --threads 8 --python-exit-code 1 --python artsource/props/build.py
XDG_CONFIG_HOME="$PWD/Saved/ArtExport/props-config" "$BLENDER_ROOT/blender" --background --factory-startup --threads 8 --python-exit-code 1 --python artsource/props/preview.py -- --view icons
fi
python3 artsource/props/validate_glb.py
XDG_CONFIG_HOME="$PWD/Saved/ArtExport/props-config" "$BLENDER_ROOT/blender" --background --factory-startup --threads 8 --python-exit-code 1 --python artsource/props/validate_blender.py
mkdir -p Saved/DerivedDataCache Saved/BuildEnvironment/config
XDG_CONFIG_HOME="$PWD/Saved/BuildEnvironment/config" "$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$LH_PROJECT" -run=PythonScript -EnablePlugins=PythonScriptPlugin \
 "-script=$PWD/Source/Lighthaven/Visual/Props/import_props.py" '-DDC=(Local)' "-LocalDataCachePath=$PWD/Saved/DerivedDataCache" \
 -nullrhi -unattended -nosound -nop4 -NoAnalytics -NoCrashDialog '-ini:Engine:[CrashReportClient]:bStartCRCFromEngineHandler=False' '-ini:Engine:[ConsoleVariables]:HomeScreen.EnableHomeScreen=0'
# Commandlet must produce a valid receipt, not merely return zero.
python3 Source/Lighthaven/Visual/Props/props_receipt.py --verify
