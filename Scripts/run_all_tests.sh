#!/bin/bash
# Eon Project - Complete Test Suite
# Runs all integration, unit, and performance tests

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TESTS_DIR="$PROJECT_ROOT/Tests"

TOTAL_PASSED=0
TOTAL_FAILED=0
TESTS_RUN=0

echo "=========================================="
echo "Eon Project - Complete Test Suite"
echo "=========================================="
echo "Date: $(date)"
echo ""

run_test() {
    local test_script="$1"
    local test_name="$2"

    echo "----------------------------------------"
    echo "Running: $test_name"
    echo "----------------------------------------"

    TESTS_RUN=$((TESTS_RUN + 1))

    if bash "$test_script"; then
        TOTAL_PASSED=$((TOTAL_PASSED + 1))
        echo "[SUITE] $test_name: PASSED"
    else
        TOTAL_FAILED=$((TOTAL_FAILED + 1))
        echo "[SUITE] $test_name: FAILED"
    fi
    echo ""
}

# Make test scripts executable
chmod +x "$TESTS_DIR"/integration/*.sh 2>/dev/null || true
chmod +x "$TESTS_DIR"/performance/*.sh 2>/dev/null || true

# Run Integration Tests
echo ""
echo "========== INTEGRATION TESTS =========="
echo ""

if [ -f "$TESTS_DIR/integration/test_spacetimedb_connection.sh" ]; then
    run_test "$TESTS_DIR/integration/test_spacetimedb_connection.sh" "SpaceTimeDB Connection"
fi

if [ -f "$TESTS_DIR/integration/test_multiplayer_sync.sh" ]; then
    run_test "$TESTS_DIR/integration/test_multiplayer_sync.sh" "Multiplayer Sync"
fi

if [ -f "$TESTS_DIR/integration/test_inventory_operations.sh" ]; then
    run_test "$TESTS_DIR/integration/test_inventory_operations.sh" "Inventory Operations"
fi

# Run Performance Tests
echo ""
echo "========== PERFORMANCE TESTS =========="
echo ""

if [ -f "$TESTS_DIR/performance/test_mobile_performance.sh" ]; then
    run_test "$TESTS_DIR/performance/test_mobile_performance.sh" "Mobile Performance"
fi

# Summary
echo ""
echo "=========================================="
echo "TEST SUITE SUMMARY"
echo "=========================================="
echo "Tests Run: $TESTS_RUN"
echo "Passed: $TOTAL_PASSED"
echo "Failed: $TOTAL_FAILED"
echo ""

if [ $TOTAL_FAILED -eq 0 ]; then
    echo "[SUCCESS] All tests passed!"
    echo ""
    echo "Project is ready for deployment."
    exit 0
else
    echo "[WARNING] Some tests failed."
    echo ""
    echo "Review failed tests before deployment."
    exit 1
fi
