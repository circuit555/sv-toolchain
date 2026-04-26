#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Generate compile_commands.json from a Ninja build graph.

This script queries Ninja's compdb output for all rules, keeps only true
compilation commands (those containing '-c'), and writes a root-level
compile_commands.json suitable for clangd/clang-tidy.
"""

from __future__ import annotations

import argparse
import json
import shlex
import subprocess
import sys
from pathlib import Path


def run_checked(cmd: list[str], cwd: Path) -> str:
    try:
        completed = subprocess.run(
            cmd,
            cwd=str(cwd),
            check=True,
            text=True,
            capture_output=True,
        )
    except FileNotFoundError:
        print("error: required tool not found: {}".format(cmd[0]), file=sys.stderr)
        sys.exit(1)
    except subprocess.CalledProcessError as exc:
        stderr = exc.stderr.strip()
        if stderr:
            print(stderr, file=sys.stderr)
        else:
            print("error: command failed: {}".format(" ".join(cmd)), file=sys.stderr)
        sys.exit(exc.returncode)

    return completed.stdout


def is_compile_command(command: str) -> bool:
    try:
        tokens = shlex.split(command)
    except ValueError:
        return " -c " in command
    return "-c" in tokens


def main() -> int:
    repo_root = Path(__file__).resolve().parent.parent

    parser = argparse.ArgumentParser(
        description="Generate compile_commands.json from Ninja compdb output"
    )
    parser.add_argument(
        "--build-file",
        default="build/build.ninja",
        help="Path to Ninja build file relative to repo root",
    )
    parser.add_argument(
        "--output",
        default="compile_commands.json",
        help="Output compile_commands.json path relative to repo root",
    )
    args = parser.parse_args()

    build_file = repo_root / args.build_file
    output_file = repo_root / args.output

    if not build_file.exists():
        print(
            "error: build file not found: {}\n"
            "hint: run 'modi' first to generate Ninja files".format(build_file),
            file=sys.stderr,
        )
        return 1

    rules_stdout = run_checked(
        ["ninja", "-f", args.build_file, "-t", "rules"],
        cwd=repo_root,
    )
    rules = [line.strip() for line in rules_stdout.splitlines() if line.strip()]

    if not rules:
        print("error: no Ninja rules found in {}".format(build_file), file=sys.stderr)
        return 1

    compdb_stdout = run_checked(
        ["ninja", "-f", args.build_file, "-t", "compdb", *rules],
        cwd=repo_root,
    )

    try:
        entries = json.loads(compdb_stdout)
    except json.JSONDecodeError as exc:
        print("error: failed to parse Ninja compdb JSON: {}".format(exc), file=sys.stderr)
        return 1

    compile_entries = []
    for entry in entries:
        command = entry.get("command", "")
        if not isinstance(command, str) or not command:
            continue
        if not is_compile_command(command):
            continue
        compile_entries.append(entry)

    if not compile_entries:
        print(
            "error: no compilation entries found in Ninja compdb output",
            file=sys.stderr,
        )
        return 1

    output_file.write_text(json.dumps(compile_entries, indent=2) + "\n", encoding="utf-8")
    print("wrote {} entries to {}".format(len(compile_entries), output_file))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
