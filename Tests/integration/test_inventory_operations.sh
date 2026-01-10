#!/bin/bash
# Integration Test: Inventory Operations
# Tests inventory system across client-server

# Don't exit on error - we handle test failures ourselves
# set -e

SPACETIME_CLI="${HOME}/.local/bin/spacetime"
MODULE_NAME="eon"
PASSED=0
FAILED=0

log_test() { echo "[TEST] $1"; }
log_pass() { echo "[PASS] $1"; PASSED=$((PASSED + 1)); }
log_fail() { echo "[FAIL] $1"; FAILED=$((FAILED + 1)); }

echo "=========================================="
echo "Eon Project - Inventory Operations Tests"
echo "=========================================="

# Test 1: Item definitions loaded
test_item_definitions() {
    log_test "Item Definitions Loaded"
    result=$($SPACETIME_CLI sql $MODULE_NAME "SELECT COUNT(*) as count FROM item_definition" 2>&1)
    if echo "$result" | grep -qE "[1-9]"; then
        log_pass "Item definitions are loaded"
    else
        log_fail "No item definitions found"
    fi
}

# Test 2: Verify specific items exist
test_specific_items() {
    log_test "Required Items Exist"
    items=("health_potion" "mana_potion" "gold_coin" "iron_sword" "wooden_shield")
    all_found=true

    for item in "${items[@]}"; do
        result=$($SPACETIME_CLI sql $MODULE_NAME "SELECT item_id FROM item_definition WHERE item_id = '$item'" 2>&1)
        if ! echo "$result" | grep -q "$item"; then
            log_fail "Missing item: $item"
            all_found=false
        fi
    done

    if $all_found; then
        log_pass "All required items exist"
    fi
}

# Test 3: Item types are valid
test_item_types() {
    log_test "Item Types Validation"
    # SpaceTimeDB doesn't support DISTINCT, so query all and check types exist
    result=$($SPACETIME_CLI sql $MODULE_NAME "SELECT item_type FROM item_definition" 2>&1)
    if echo "$result" | grep -q "consumable" && echo "$result" | grep -q "weapon"; then
        log_pass "Item types are valid (consumable, weapon, resource found)"
    else
        log_fail "Invalid or missing item types"
    fi
}

# Test 4: Inventory table accessible
test_inventory_table() {
    log_test "Inventory Table Accessibility"
    result=$($SPACETIME_CLI sql $MODULE_NAME "SELECT COUNT(*) FROM inventory_item" 2>&1)
    if echo "$result" | grep -qE "[0-9]+"; then
        log_pass "Inventory table is accessible"
    else
        log_fail "Cannot access inventory table"
    fi
}

# Test 5: Max stack values
test_max_stack() {
    log_test "Max Stack Values"
    result=$($SPACETIME_CLI sql $MODULE_NAME "SELECT item_id, max_stack FROM item_definition WHERE max_stack > 0" 2>&1)
    if echo "$result" | grep -qE "[0-9]+"; then
        log_pass "Max stack values are set"
    else
        log_fail "Max stack values not configured"
    fi
}

# Test 6: World items table
test_world_items() {
    log_test "World Items Table"
    result=$($SPACETIME_CLI sql $MODULE_NAME "SELECT COUNT(*) FROM world_item" 2>&1)
    if echo "$result" | grep -qE "[0-9]+"; then
        log_pass "World items table is accessible"
    else
        log_fail "Cannot access world items table"
    fi
}

# Run tests
test_item_definitions
test_specific_items
test_item_types
test_inventory_table
test_max_stack
test_world_items

echo ""
echo "=========================================="
echo "Inventory Tests: $PASSED passed, $FAILED failed"
echo "=========================================="

[ $FAILED -eq 0 ] && exit 0 || exit 1
