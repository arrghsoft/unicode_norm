#!/bin/zsh
set -e

echo "===================================="
echo " unicode_norm Automated Test Script "
echo "===================================="

BIN=./build/unicode_norm

# Step 1: Build
echo "[1] Building unicode_norm..."
rm -rf build
cmake -S . -B build -DUNICODE_NORM_LIBRARY_STATIC_LINK=ON > /dev/null
cmake --build build > /dev/null

if [ ! -f "$BIN" ]; then
    echo "✗ Build failed: binary not found."
    exit 1
fi
echo "✔ Build completed."


# Step 2: Test directory
echo "[2] Creating test files..."

TEST_DIR=test_env
rm -rf "$TEST_DIR"
mkdir -p "$TEST_DIR"
mkdir -p "$TEST_DIR/ko"
mkdir -p "$TEST_DIR/ja"
mkdir -p "$TEST_DIR/latin"
mkdir -p "$TEST_DIR/sub"
mkdir -p "$TEST_DIR/sub/ko"
mkdir -p "$TEST_DIR/sub/ja"
mkdir -p "$TEST_DIR/sub/latin"

printf "hello world" > "$TEST_DIR/ko/$(printf '\xe1\x84\x92\xe1\x85\xa6\xe1\x86\xaf\xe1\x84\x85\xe1\x85\xa9')" # String 헬로 (NFD)
printf "hello world" > "$TEST_DIR/ja/$(printf '\xe3\x81\xaf\xe3\x81\xb2\xe3\x82\x99\xe3\x81\xbb\xe3\x82\x9a')" # String はびぽ (NFD)
printf "hello world" > "$TEST_DIR/latin/$(printf '\x61\x61\xcc\x80\x61\xcc\x81\x61\xcc\x82\x61\xcc\x88\x61\xcc\x8c')" # 문자열 aàáâäǎ (NFD)

printf "hello world" > "$TEST_DIR/sub/ko/$(printf '\xe1\x84\x92\xe1\x85\xa6\xe1\x86\xaf\xe1\x84\x85\xe1\x85\xa9')" # String 헬로 (NFD)
printf "hello world" > "$TEST_DIR/sub/ja/$(printf '\xe3\x81\xaf\xe3\x81\xb2\xe3\x82\x99\xe3\x81\xbb\xe3\x82\x9a')" # String はびぽ (NFD)
printf "hello world" > "$TEST_DIR/sub/latin/$(printf '\x61\x61\xcc\x80\x61\xcc\x81\x61\xcc\x82\x61\xcc\x88\x61\xcc\x8c')" # 문자열 aàáâäǎ (NFD)
echo "✔ Test files created."


# Step 3: Dry-run
echo "[3] Dry-run mode test (no changes)..."
$BIN -d "$TEST_DIR" > /dev/null
echo "✔ Dry-run success"


# Step 4: rename (NFC)
echo "[4] Running NFC normalization..."
$BIN -f NFC -r "$TEST_DIR" > /dev/null

echo "✔ NFC Rename completed"


# Step 5: assert
echo "[5] Checking NFC rename results..."

declare -A EXPECTED_MAP

EXPECTED_MAP[ko]="ed97aceba19c0a"
EXPECTED_MAP[ja]="e381afe381b3e381bd0a"
EXPECTED_MAP[latin]="61c3a0c3a1c3a2c3a4c78e0a"
EXPECTED_MAP[sub/ko]="ed97aceba19c0a"
EXPECTED_MAP[sub/ja]="e381afe381b3e381bd0a"
EXPECTED_MAP[sub/latin]="61c3a0c3a1c3a2c3a4c78e0a"

# Directories to test
DIRS=("ko" "ja" "latin" "sub/ko" "sub/ja" "sub/latin")

for d in "${DIRS[@]}"; do
    EXPECTED="${EXPECTED_MAP[$d]}"
    ACTUAL=$(ls "$TEST_DIR/$d" | xxd -p | tr -d '\n')

    if [[ "$EXPECTED" == "$ACTUAL" ]]; then
        echo "✔ CHECKED: $d"
    else
        echo "✗ FAIL: $d"
        echo "EXPECTED: $EXPECTED"
        echo "ACTUAL  : $ACTUAL"
        exit 1
    fi
done

echo "✔ NFC File checks passed."

# Step 6: rename (NFD)
echo "[6] Running NFD normalization..."
$BIN -f NFD -r "$TEST_DIR" > /dev/null

echo "✔ NFD Rename completed"

# Step 7: assert
echo "[7] Checking NFD rename results..."

EXPECTED_MAP[ko]="e18492e185a6e186afe18485e185a90a"
EXPECTED_MAP[ja]="e381afe381b2e38299e381bbe3829a0a"
EXPECTED_MAP[latin]="6161cc8061cc8161cc8261cc8861cc8c0a"
EXPECTED_MAP[sub/ko]="e18492e185a6e186afe18485e185a90a"
EXPECTED_MAP[sub/ja]="e381afe381b2e38299e381bbe3829a0a"
EXPECTED_MAP[sub/latin]="6161cc8061cc8161cc8261cc8861cc8c0a"

# Directories to test
DIRS=("ko" "ja" "latin" "sub/ko" "sub/ja" "sub/latin")

for d in "${DIRS[@]}"; do
    EXPECTED="${EXPECTED_MAP[$d]}"
    ACTUAL=$(ls "$TEST_DIR/$d" | xxd -p | tr -d '\n')

    if [[ "$EXPECTED" == "$ACTUAL" ]]; then
        echo "✔ CHECKED: $d"
    else
        echo "✗ FAIL: $d"
        echo "EXPECTED: $EXPECTED"
        echo "ACTUAL  : $ACTUAL"
        exit 1
    fi
done


# Step 7: Clean up
echo "[7] Cleaning temporary files..."
rm -rf "$TEST_DIR"

echo "===================================="
echo " All tests PASSED 🌟"
echo "===================================="
