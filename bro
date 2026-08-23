#!/usr/bin/env bash
# ft_bro launcher. Deliberately tiny: resolve where we live, resolve the
# target, hand off to Python. The reference implementation's launcher grew to
# 140 lines of bash doing compilation logic and acquired a duplicate-main bug
# in the process (design/03_ORCHESTRATOR.md).

set -euo pipefail

src="${BASH_SOURCE[0]}"
while [ -L "$src" ]; do
	dir="$(cd -P "$(dirname "$src")" && pwd)"
	src="$(readlink "$src")"
	[[ $src != /* ]] && src="$dir/$src"
done
BRO_DIR="$(cd -P "$(dirname "$src")" && pwd)"

if ! command -v python3 >/dev/null 2>&1; then
	echo "bro: python3 is required but not on \$PATH" >&2
	exit 127
fi

# Do NOT cd - the target defaults to $PWD, which is the whole point of being
# able to run `bro` from inside your own libft.
exec env PYTHONPATH="$BRO_DIR${PYTHONPATH:+:$PYTHONPATH}" python3 -m ft_bro "$@"
