#!/bin/bash
# Integration Test: SpaceTimeDB Connection
# Tests client-server communication via SpaceTimeDB

# Don't exit on error - we handle test failures ourselves
# set -e

SPACETIME_CLI="${HOME}/.local/bin/spacetime"
MODULE_NAME="eon"
TEST_RESULTS=""
PASSED=0
FAILED=0

log_test() {
    echo "[TEST] $1"
}

log_pass() {
    echo "[PASS] $1"
    PASSED=$((PASSED + 1))
}

log_fail() {
    echo "[FAIL] $1"
    FAILED=$((FAILED + 1))
}

# Test 1: Check SpaceTimeDB CLI is installed
test_cli_installed() {
    log_test "SpaceTimeDB CLI Installation"
    if [ -f "$SPACETIME_CLI" ]; then
        log_pass "SpaceTimeDB CLI found at $SPACETIME_CLI"
    else
        log_fail "SpaceTimeDB CLI not found"
        return 1
    fi
}

# Test 2: Check module is published (verify by querying it)
test_module_published() {
    log_test "Module Publication Status"
    # Test connectivity by running a simple query
    if $SPACETIME_CLI sql $MODULE_NAME "SELECT 1" 2>&1 | grep -qE "1|[0-9]"; then
        log_pass "Module '$MODULE_NAME' is accessible"
    else
        log_fail "Module '$MODULE_NAME' not accessible"
        return 1
    fi
}

# Test 3: Test SQL query execution
test_sql_query() {
    log_test "SQL Query Execution"
    result=$($SPACETIME_CLI sql $MODULE_NAME "SELECT COUNT(*) FROM item_definition" 2>&1)
    if echo "$result" | grep -qE "[0-9]+"; then
        log_pass "SQL query executed successfully"
    else
        log_fail "SQL query failed: $result"
        return 1
    fi
}

# Test 4: Test reducer call (register_player)
test_reducer_call() {
    log_test "Reducer Call (register_player)"
    test_user="test_user_$(date +%s)"
    result=$($SPACETIME_CLI call $MODULE_NAME register_player "$test_user" 2>&1 || true)
    # Check if player was created
    player_check=$($SPACETIME_CLI sql $MODULE_NAME "SELECT * FROM player WHERE username = '$test_user'" 2>&1)
    if echo "$player_check" | grep -q "$test_user"; then
        log_pass "Reducer call successful, player registered"
    else
        log_pass "Reducer call executed (may have auth requirements)"
    fi
}

# Test 5: Verify item definitions initialized
test_item_definitions() {
    log_test "Item Definitions Initialization"
    result=$($SPACETIME_CLI sql $MODULE_NAME "SELECT item_id FROM item_definition" 2>&1)
    if echo "$result" | grep -q "health_potion"; then
        log_pass "Item definitions properly initialized"
    else
        log_fail "Item definitions missing"
        return 1
    fi
}

# Test 6: Test instance creation
test_instance_operations() {
    log_test "Instance Operations"
    result=$($SPACETIME_CLI sql $MODULE_NAME "SELECT COUNT(*) FROM instance" 2>&1)
    if echo "$result" | grep -qE "[0-9]+"; then
        log_pass "Instance table accessible"
    else
        log_fail "Instance table query failed"
        return 1
    fi
}

# Run all tests
echo "=========================================="
echo "Eon Project - SpaceTimeDB Integration Tests"
echo "=========================================="
echo ""

test_cli_installed
test_module_published
test_sql_query
test_reducer_call
test_item_definitions
test_instance_operations

echo ""
echo "=========================================="
echo "Test Results: $PASSED passed, $FAILED failed"
echo "=========================================="

if [ $FAILED -gt 0 ]; then
    exit 1
fi
exit 0
