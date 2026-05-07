#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"

cd "$ROOT"

CC=gcc
CFLAGS="-Wall -Wextra -Wpedantic -std=c11 -I src"
LDFLAGS="-lsqlite3"
TARGET="server"

SRCS=(
    src/main.c
    src/server.c
    src/router.c
    src/handlers/visit.c
    src/handlers/stats.c
    src/handlers/health.c
    src/handlers/globe.c
    src/handlers/heatmap.c
    src/services/logger.c
    src/services/geo.c
    src/services/analytics.c
    src/storage/db.c
    src/utils/json.c
    src/utils/net.c
)

echo "==> Building visitor-tracker server..."
$CC $CFLAGS "${SRCS[@]}" $LDFLAGS -o "$TARGET"

echo "Build successful. Run with: ./$TARGET"
echo "Server will listen on http://localhost:8080"
