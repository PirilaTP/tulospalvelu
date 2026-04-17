#!/bin/bash
# Reset test data and run HkMaali
# Usage: ./run_test.sh [--reset]
#   --reset: always reset data files (default: only if missing)

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TEST_DIR="$SCRIPT_DIR/test_data"
SOURCE_DIR="$SCRIPT_DIR/../../kisat/HkKisaWinData"
BINARY="$SCRIPT_DIR/HkMaali"

if [ ! -f "$BINARY" ]; then
    echo "HkMaali not found. Run 'make' first."
    exit 1
fi

mkdir -p "$TEST_DIR"

if [ "$1" = "--reset" ] || [ ! -f "$TEST_DIR/KILP.DAT" ]; then
    echo "Resetting test data from $SOURCE_DIR..."
    cp "$SOURCE_DIR/KILP.DAT" "$TEST_DIR/"
    cp "$SOURCE_DIR/KilpSrj.xml" "$TEST_DIR/"
    cp "$SOURCE_DIR/radat1.xml" "$TEST_DIR/" 2>/dev/null
    cat > "$TEST_DIR/laskenta.cfg" << 'EOF'
Kone=MA
Emit
EOF
    echo "Done."
fi

echo "Starting HkMaali in $TEST_DIR"
echo "  Tip: set terminal to 80x50 for best display"
echo ""
cd "$TEST_DIR" && exec "$BINARY"
