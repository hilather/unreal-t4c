#!/usr/bin/env bash
set -euo pipefail

LH_BUILD_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
LH_PROJECT_ROOT="$(cd -- "$LH_BUILD_DIR/.." && pwd)"
LH_PROJECT="$LH_PROJECT_ROOT/Lighthaven.uproject"
# local.env is trusted shell configuration, loaded only when UE_ROOT is unset/empty.
if [[ -z "${UE_ROOT:-}" && -f "$LH_BUILD_DIR/local.env" ]]; then
    # shellcheck source=/dev/null
    source "$LH_BUILD_DIR/local.env"
fi
if [[ -z "${UE_ROOT:-}" ]]; then
    echo "UE_ROOT not set: export UE_ROOT or set it in build/local.env." >&2
    exit 1
fi
if [[ ! -f "$UE_ROOT/Engine/Binaries/Linux/UnrealEditor" ]]; then
    echo "Engine missing: $UE_ROOT/Engine/Binaries/Linux/UnrealEditor (check UE_ROOT)." >&2
    exit 1
fi
UE_ROOT="$(cd -- "$UE_ROOT" && pwd)"
export UE_ROOT
# Installed-engine UBT uses .NET ApplicationData for its logs and traces.
# An absent user config directory can resolve to an empty path, causing writes
# relative to the read-only engine mount. Keep the default inside the project.
if [[ -z "${XDG_CONFIG_HOME:-}" ]]; then
    export XDG_CONFIG_HOME="$LH_PROJECT_ROOT/Saved/BuildEnvironment/config"
    mkdir -p -- "$XDG_CONFIG_HOME"
fi
# Keep UBA's writable store beside the project's other generated build data.
# Respect an explicit caller root, as with XDG_CONFIG_HOME above.
if [[ -z "${UBA_ROOT:-}" ]]; then
    export UBA_ROOT="$LH_PROJECT_ROOT/Saved/UBA"
    mkdir -p -- "$UBA_ROOT"
fi
if ! command -v python3 >/dev/null 2>&1; then
    echo "python3 is required to read Build.version and Automation reports." >&2
    exit 1
fi
python3 - "$UE_ROOT/Engine/Build/Build.version" <<'PY'
import json, sys
try:
    with open(sys.argv[1], encoding="utf-8") as stream:
        v = json.load(stream)
    version = ".".join(str(v[key]) for key in ("MajorVersion", "MinorVersion", "PatchVersion"))
    print(f"Unreal Engine {version} ({sys.argv[1]})")
    if version != "5.8.3":
        print("Warning: project engine pin is 5.8.3; this installation differs.", file=sys.stderr)
except (OSError, ValueError, KeyError) as error:
    sys.exit(f"Cannot read engine version: {error}")
PY
