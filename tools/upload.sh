#!/usr/bin/env bash
# Push the project files to the board via mpremote.
# Usage: tools/upload.sh [/dev/ttyACM0]
set -euo pipefail

PORT="${1:-/dev/ttyACM0}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

if ! command -v mpremote >/dev/null; then
    echo "mpremote not on PATH: pip install mpremote" >&2
    exit 1
fi

cd "$ROOT"
mpremote connect "$PORT" cp -r lib :
mpremote connect "$PORT" cp config.py :
mpremote connect "$PORT" cp boot.py :
mpremote connect "$PORT" cp main.py :
mpremote connect "$PORT" reset
