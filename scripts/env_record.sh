#!/usr/bin/env bash
set -euo pipefail

mkdir -p docs

{
    echo "# Environment Record"
    echo "Generated on: $(date)"
    echo ""
    echo "## CPU Info"
    echo '```text'
    lscpu | grep -E 'Model name|^CPU\(s\)|Thread|L1d|L2|L3'
    echo '```'
    echo ""
    echo "## Memory Info"
    echo '```text'
    free -h
    echo '```'
    echo ""
    echo "## Block Devices"
    echo '```text'
    lsblk -d -o NAME,ROTA,SIZE,MODEL
    echo '```'
    echo ""
    echo "## Kernel Perf Event Paranoid"
    echo '```text'
    cat /proc/sys/kernel/perf_event_paranoid
    echo '```'
} > docs/environment.md

echo "Environment record successfully written to docs/environment.md"
