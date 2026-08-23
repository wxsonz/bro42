#!/usr/bin/env bash
# ft_bro installer - an educational walkthrough, not a silent installer.
#
# PLAN.md asks for a setup that teaches. So every step EXPLAINS what it is
# about to do and why, then asks, then does it. Nothing is appended to a
# dotfile without showing you the exact line first. --dry-run changes nothing.

set -euo pipefail

DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="${HOME}/.local/bin"
DRY=0
YES=0
for arg in "$@"; do
	case "$arg" in
		--dry-run) DRY=1 ;;
		-y|--yes)  YES=1 ;;
		-h|--help) echo "usage: ./install.sh [--dry-run] [-y]"; exit 0 ;;
		*) echo "unknown option: $arg" >&2; exit 2 ;;
	esac
done

b() { printf '\033[1m%s\033[0m\n' "$*"; }
dim() { printf '\033[2m%s\033[0m\n' "$*"; }
run() {
	if [ "$DRY" = 1 ]; then dim "   would run: $*"; else "$@" >/dev/null; fi
}

# Only speak in the past tense when something actually happened.
did() { [ "$DRY" = 1 ] || dim "   $*"; }

ask() {
	[ "$YES" = 1 ] && return 0
	[ "$DRY" = 1 ] && return 0
	printf '   proceed? [Y/n] '
	read -r reply </dev/tty || reply=y
	case "$reply" in [nN]*) echo "   skipped."; return 1 ;; esac
	return 0
}

# If ft_bro was fetched by piping this script from a URL, there is no checkout
# to install from - so make one first, in a place the user chooses.
if [ ! -f "$DIR/bro" ] || [ ! -d "$DIR/engine" ]; then
	b "ft_bro is not checked out here"
	cat <<EOF
   This script is running outside an ft_bro checkout, so there is nothing to
   install yet. Clone it first, then run this script from inside it:

     git clone ${FT_BRO_ORIGIN:-<repository url>} ft_bro
     cd ft_bro && ./install.sh

EOF
	exit 1
fi

echo
b "ft_bro setup"
[ "$DRY" = 1 ] && dim "(dry run - nothing will be changed)"
echo

# ---------------------------------------------------------------- step 1
b "1. What your shell is, and why it matters"
cat <<EOF
   Your login shell is: ${SHELL:-unknown}
   Currently running  : $(ps -p $$ -o comm= 2>/dev/null || echo bash)

   A shell finds commands by looking through the directories listed in \$PATH,
   in order. Typing "bro" only works if a file called bro lives in one of them.
EOF
echo
if ask; then :; fi
echo

# ---------------------------------------------------------------- step 2
b "2. Checking python3"
if command -v python3 >/dev/null 2>&1; then
	dim "   found: $(command -v python3) ($(python3 --version 2>&1))"
else
	echo "   python3 is required and was not found on \$PATH." >&2
	exit 127
fi
echo

# ---------------------------------------------------------------- step 3
b "3. Building the test engine"
cat <<EOF
   ft_bro's engine is C. It is compiled once here; the final link happens
   later, per libft, because the engine links against YOUR library.
EOF
# The redirect must live INSIDE run, or in dry-run mode it swallows the
# "would run" notice and the step claims to have done something it did not.
if ask; then run make -C "$DIR"; did "built."; fi
echo

# ---------------------------------------------------------------- step 4
b "4. Making bro executable"
cat <<EOF
   chmod +x sets the execute permission bit. Without it the shell refuses to
   run the file even when it can see it.
EOF
if ask; then run chmod +x "$DIR/bro"; did "done."; fi
echo

# ---------------------------------------------------------------- step 5
b "5. Putting bro on your \$PATH"
cat <<EOF
   A symlink is a file that points at another file. Linking rather than
   copying means bro keeps working when you pull updates - there is only ever
   one real copy, at:
     $DIR/bro
EOF
if ask; then
	run mkdir -p "$BIN"
	run ln -sf "$DIR/bro" "$BIN/bro"
	did "linked: $BIN/bro -> $DIR/bro"
fi
echo

# ---------------------------------------------------------------- step 6
case ":${PATH}:" in
	*":${BIN}:"*)
		b "6. \$PATH already contains $BIN"
		dim "   nothing to change."
		;;
	*)
		b "6. $BIN is NOT on your \$PATH"
		rc="${HOME}/.zshrc"
		[ -n "${BASH_VERSION:-}" ] && rc="${HOME}/.bashrc"
		line="export PATH=\"\$HOME/.local/bin:\$PATH\""
		cat <<EOF
   To make "bro" work from anywhere, this exact line needs to be added to
   $rc:

     $line

   It prepends your personal bin directory to the list of places the shell
   searches. Nothing else in the file is touched.
EOF
		if ask; then
			if [ "$DRY" = 1 ]; then dim "   would append to $rc"
			else printf '\n# added by ft_bro install.sh\n%s\n' "$line" >> "$rc"
				dim "   appended. Run:  source $rc"
			fi
		fi
		;;
esac
echo

b "Done."
cat <<EOF
   cd into any libft and run:

     bro                    everything, then open the dashboard
     bro ft_split           one function
     bro explain overlap    a concept card
     bro defense            peer-evaluation questions

EOF
