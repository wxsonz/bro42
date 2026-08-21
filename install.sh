#!/usr/bin/env bash
# ==============================================================================
# ft_bro (bro) - Educational Installation Walkthrough
# ==============================================================================

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_BIN="$HOME/.local/bin"
LAUNCHER_PATH="$INSTALL_BIN/bro"
FULL_LAUNCHER_PATH="$INSTALL_BIN/ft_bro"
COMP_LAUNCHER_PATH="$INSTALL_BIN/ft_companion"

# Handle clean uninstall
if [[ "$1" == "--uninstall" || "$1" == "-u" ]]; then
	echo "🗑️  Removing ft_bro, bro, and ft_companion global commands..."
	rm -f "$LAUNCHER_PATH" "$FULL_LAUNCHER_PATH" "$COMP_LAUNCHER_PATH"
	echo "✅ ft_bro and bro removed cleanly."
	exit 0
fi

echo "=============================================================================="
echo "            🚀 Welcome to the ft_bro (bro) Educational Setup!                 "
echo "=============================================================================="
echo ""

# ------------------------------------------------------------------------------
# STEP 1: Shell & Environment Detection
# ------------------------------------------------------------------------------
echo "👉 [1/5] Shell & Environment Detection"
DETECTED_SHELL="$(basename "$SHELL")"
echo "  • Detected Shell : $SHELL ($DETECTED_SHELL)"
echo "  • Educational Note: In Unix, the shell is the program that interprets your"
echo "    commands. Configuration files like ~/.zshrc or ~/.bashrc define your user"
echo "    environment, variables, and aliases."
echo ""

# ------------------------------------------------------------------------------
# STEP 2: Compiling Core Tester Engine
# ------------------------------------------------------------------------------
echo "👉 [2/5] Compiling tester engine with cc & make"
echo "  • Running Command: make -C $DIR"
make -C "$DIR" >/dev/null
echo "  • Educational Note: We compile the C testing engine natively using cc with"
echo "    -Wall -Wextra -Werror flags, ensuring strict compilation compliance."
echo ""

# ------------------------------------------------------------------------------
# STEP 3: Setting up $PATH and Directories
# ------------------------------------------------------------------------------
echo "👉 [3/5] Setting up user binary directory (~/.local/bin)"
mkdir -p "$INSTALL_BIN"
echo "  • Target Path    : $INSTALL_BIN"
echo "  • Educational Note: \$PATH is an environment variable containing a list of"
echo "    directories where your shell looks for executable programs. When you type"
echo "    a command (like 'bro' or 'ft_bro'), the shell searches \$PATH"
echo "    directories in order so you don't have to type './' or absolute paths."
echo "    Using ~/.local/bin requires NO root/sudo privileges on 42 cluster machines."
echo ""

# ------------------------------------------------------------------------------
# STEP 4: Creating Global Launchers (`bro` & `ft_bro`)
# ------------------------------------------------------------------------------
echo "👉 [4/5] Creating Global Launchers (bro & ft_bro)"
cat << 'EOF' > "$LAUNCHER_PATH"
#!/usr/bin/env bash
DIR="__INSTALL_DIR__"
exec "$DIR/bro" "$@"
EOF

# Substitute the actual installation directory
sed -i "s|__INSTALL_DIR__|$DIR|g" "$LAUNCHER_PATH"

chmod +x "$LAUNCHER_PATH"
chmod +x "$DIR/bro"

# Create 'ft_bro' and 'ft_companion' symlinks
ln -sf "$LAUNCHER_PATH" "$FULL_LAUNCHER_PATH"
ln -sf "$LAUNCHER_PATH" "$COMP_LAUNCHER_PATH"

echo "  • Commands Installed: $LAUNCHER_PATH (Primary command: 'bro')"
echo "                        $FULL_LAUNCHER_PATH (Full name: 'ft_bro')"
echo "  • Educational Note: The wrapper forwards your current working directory"
echo "    (\$PWD) and all arguments (\$@) to the tester engine, so it knows which"
echo "    Libft you want to test automatically!"
echo ""

# ------------------------------------------------------------------------------
# STEP 5: Verification & PATH Check
# ------------------------------------------------------------------------------
echo "👉 [5/5] Checking \$PATH configuration"
if [[ ":$PATH:" != *":$INSTALL_BIN:"* ]]; then
	echo "  ⚠️  Note: $INSTALL_BIN is not yet in your current \$PATH."
	echo "  👉 To add it, add this line to your ~/.zshrc or ~/.bashrc:"
	echo "     export PATH=\"\$HOME/.local/bin:\$PATH\""
	echo "  👉 Or reload your shell: source ~/.zshrc (or source ~/.bashrc)"
else
	echo "  ✅ $INSTALL_BIN is already in your \$PATH!"
fi

echo ""
echo "=============================================================================="
echo "  🎉 Installation Complete! How to use \`bro\` (ft_bro):                         "
echo "=============================================================================="
echo "  1. Navigate to your Libft directory:  cd ~/my_libft"
echo "  2. Run the companion:                 bro"
echo "  3. Test a single function:            bro ft_split"
echo "  4. Macro & build checks:              bro macro"
echo "  5. Terminal-only mode:                bro --no-web"
echo "  6. Clean uninstall anytime:           ./install.sh --uninstall"
echo "=============================================================================="
echo ""
