#!/usr/bin/env bash
# Drop into the Linux dev container, with this repo mounted at /work.
#   ./dev.sh              -> interactive shell
#   ./dev.sh <command>    -> run one command and exit
#
# --cap-add=SYS_PTRACE and --security-opt seccomp=unconfined let gdb attach.
# Publisher and receiver both run inside this one container, so multicast
# over loopback stays within a single network namespace.
set -euo pipefail

# Only ask for a TTY when we actually have one (lets scripts/CI use this too).
TTY_FLAGS="-i"
[ -t 0 ] && TTY_FLAGS="-it"

# macOS is case-insensitive but the Linux VM is not: /Users/sam and /Users/SAM
# both open here, yet only the real casing is mounted by Colima. The wrong one
# mounts an EMPTY dir and cmake reports a missing CMakeLists.txt. Resolve the
# true on-disk path so either spelling works.
REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$(python3 -c 'import os,sys; print(os.path.realpath(sys.argv[1]))' "$REPO_DIR" 2>/dev/null || echo "$REPO_DIR")"
case "$REPO_DIR" in
    "$HOME"/*|"$HOME") ;;
    *) # Re-anchor under the canonical $HOME when only the case differs.
       SUFFIX="${REPO_DIR#/Users/*/}"
       [ -d "$HOME/$SUFFIX" ] && REPO_DIR="$HOME/$SUFFIX" ;;
esac

exec docker run --rm $TTY_FLAGS \
    -v "$REPO_DIR":/work \
    -w /work \
    --cap-add=SYS_PTRACE \
    --security-opt seccomp=unconfined \
    feed-handler-dev "$@"
