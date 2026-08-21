#!/usr/bin/env bash
# ft_bro / bro - Universal Launcher

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR=""
ARGS=()

# Check if user asked for local server
if [[ "$1" == "--serve" || "$1" == "serve" || "$1" == "web" ]]; then
	echo "🚀 Starting ft_bro local server at http://localhost:4242 (Press Ctrl+C to stop)..."
	exec python3 "$DIR/web/server.py"
fi

# Parse target directory and test arguments
for arg in "$@"; do
	if [[ -d "$arg" && "$arg" != "--"* ]]; then
		TARGET_DIR="$(cd "$arg" && pwd)"
	else
		ARGS+=("$arg")
	fi
done

# If no explicit directory argument was passed:
if [[ -z "$TARGET_DIR" ]]; then
	if [[ "$PWD" != "$DIR" ]]; then
		# User ran `bro` inside another directory -> $PWD IS the target!
		TARGET_DIR="$PWD"
	elif [[ -d "$DIR/_dev/reference/libft42git" ]]; then
		# Developer testing inside repo root with local reference
		TARGET_DIR="$DIR/_dev/reference/libft42git"
	else
		TARGET_DIR="$PWD"
	fi
fi

export FT_BRO_DIR="$DIR"
export FT_BRO_TARGET="$TARGET_DIR"
export FT_COMPANION_DIR="$DIR"
export FT_COMPANION_TARGET="$TARGET_DIR"

# 1. Ensure core tester object files and stub library exist
if [[ ! -f "$DIR/src/main.o" || ! -f "$DIR/bin/libstubs.a" ]]; then
	echo "⚙️ Compiling ft_bro test engine..."
	make -C "$DIR" build_objs >/dev/null 2>&1 || make -C "$DIR" all >/dev/null 2>&1
fi

# 2. Build student's library if Makefile exists
if [[ -f "$TARGET_DIR/Makefile" ]]; then
	if ! make -C "$TARGET_DIR" bonus >/dev/null 2>&1 && ! make -C "$TARGET_DIR" all >/dev/null 2>&1 && ! make -C "$TARGET_DIR" >/dev/null 2>&1; then
		echo -e "\033[1;33m⚠️  Warning: Makefile build in $(basename "$TARGET_DIR") failed, falling back to direct .c compilation\033[0m" >&2
	fi
fi

# 3. Locate or build student library
STUDENT_LIB=""
TEMP_LIB=""
TEMP_OBJ_DIR=""

C_FILES=($(find "$TARGET_DIR" -maxdepth 3 -name "*.c" -not -path "*/.git/*" 2>/dev/null || true))

if [[ -f "$TARGET_DIR/libft.a" ]]; then
	TEMP_LIB="/tmp/ft_bro_temp_$$.a"
	cp "$TARGET_DIR/libft.a" "$TEMP_LIB"
	STUDENT_LIB="$TEMP_LIB"
elif [[ -f "$TARGET_DIR/build/libft.a" ]]; then
	TEMP_LIB="/tmp/ft_bro_temp_$$.a"
	cp "$TARGET_DIR/build/libft.a" "$TEMP_LIB"
	STUDENT_LIB="$TEMP_LIB"
fi

if [[ ${#C_FILES[@]} -gt 0 ]]; then
	TEMP_OBJ_DIR="/tmp/ft_bro_objs_$$"
	mkdir -p "$TEMP_OBJ_DIR"
	if [[ -z "$TEMP_LIB" ]]; then
		TEMP_LIB="/tmp/ft_bro_temp_$$.a"
	fi
	for c_file in "${C_FILES[@]}"; do
		obj="$TEMP_OBJ_DIR/$(basename "${c_file%.c}.o")"
		if ! cc -Wall -Wextra -Werror -I"$TARGET_DIR" -I"$TARGET_DIR/includes" -I"$TARGET_DIR/inc" -I"$DIR/include" -c "$c_file" -o "$obj" 2>"$TEMP_OBJ_DIR/err.log"; then
			if ! cc -I"$TARGET_DIR" -I"$TARGET_DIR/includes" -I"$TARGET_DIR/inc" -I"$DIR/include" -c "$c_file" -o "$obj" >/dev/null 2>&1; then
				echo -e "\033[1;33m⚠️  Warning: Failed to compile $(basename "$c_file"):\033[0m" >&2
				cat "$TEMP_OBJ_DIR/err.log" >&2
			fi
		fi
	done
	if ls "$TEMP_OBJ_DIR"/*.o >/dev/null 2>&1; then
		ar rcs "$TEMP_LIB" "$TEMP_OBJ_DIR"/*.o >/dev/null 2>&1 || true
		STUDENT_LIB="$TEMP_LIB"
	fi
fi

# 4. Link tester engine: student's libft.a is searched BEFORE fallback libstubs.a
mkdir -p "$DIR/bin"
CORE_OBJS=(
	"$DIR/src/main.o"
	"$DIR/src/framework/runner.o"
	"$DIR/src/framework/assertions.o"
	"$DIR/src/framework/macro_runner.o"
	"$DIR/src/framework/malloc_hook.o"
	"$DIR/src/framework/json_reporter.o"
	"$DIR/src/framework/browser_launcher.o"
	"$DIR/src/framework/utils.o"
	"$DIR"/src/tests_part1/*.o
	"$DIR"/src/tests_part2/*.o
	"$DIR"/src/tests_part3/*.o
)

if [[ -n "$STUDENT_LIB" && -f "$STUDENT_LIB" ]]; then
	cc -Wall -Wextra -Werror -g3 -I"$DIR/include" \
		"${CORE_OBJS[@]}" \
		"$STUDENT_LIB" \
		"$DIR/bin/libstubs.a" \
		-o "$DIR/bin/ft_bro_bin"
else
	cc -Wall -Wextra -Werror -g3 -I"$DIR/include" \
		"${CORE_OBJS[@]}" \
		"$DIR/bin/libstubs.a" \
		-o "$DIR/bin/ft_bro_bin"
fi

chmod +x "$DIR/bin/ft_bro_bin"

# Cleanup temp files on exit
cleanup() {
	if [[ -n "$TEMP_LIB" && -f "$TEMP_LIB" ]]; then
		rm -f "$TEMP_LIB"
	fi
	if [[ -n "$TEMP_OBJ_DIR" && -d "$TEMP_OBJ_DIR" ]]; then
		rm -rf "$TEMP_OBJ_DIR"
	fi
}
trap cleanup EXIT

# 5. Execute tester binary with target directory and any filters
"$DIR/bin/ft_bro_bin" "$TARGET_DIR" "${ARGS[@]}"
