#!/bin/bash
# SpaceTimeDB Server Reducer Tests
# Run: ./test_reducers.sh

set -e

echo "=== SpaceTimeDB Server Tests ==="
echo ""

# Configuration
MODULE_NAME="eon"
SPACETIME_CLI="spacetime"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0

pass() {
    echo -e "${GREEN}✓ PASS${NC}: $1"
    ((PASSED++))
}

fail() {
    echo -e "${RED}✗ FAIL${NC}: $1"
    ((FAILED++))
}

info() {
    echo -e "${YELLOW}INFO${NC}: $1"
}

# Check if spacetime CLI is available
if ! command -v $SPACETIME_CLI &> /dev/null; then
    echo "Error: spacetime CLI not found. Install from https://spacetimedb.com"
    exit 1
fi

# Build the module
echo "Building server module..."
cd "$(dirname "$0")/../../Server/eonserver"

if cargo build --release 2>/dev/null; then
    pass "Module builds successfully"
else
    fail "Module build failed"
    exit 1
fi

echo ""
echo "=== Syntax and Structure Tests ==="

# Test 1: Check all required tables exist
info "Checking table definitions..."
if grep -q 'table(name = instance' src/lib.rs && \
   grep -q 'table(name = player' src/lib.rs && \
   grep -q 'table(name = item_definition' src/lib.rs && \
   grep -q 'table(name = inventory_item' src/lib.rs && \
   grep -q 'table(name = world_item' src/lib.rs; then
    pass "All required tables defined"
else
    fail "Missing table definitions"
fi

# Test 2: Check all required reducers exist
info "Checking reducer definitions..."
REQUIRED_REDUCERS=(
    "create_instance"
    "delete_instance"
    "register_player"
    "join_instance"
    "leave_instance"
    "update_player_position"
    "add_item_to_inventory"
    "remove_item_from_inventory"
    "collect_world_item"
    "spawn_world_item"
)

for reducer in "${REQUIRED_REDUCERS[@]}"; do
    if grep -q "fn $reducer" src/lib.rs; then
        pass "Reducer '$reducer' exists"
    else
        fail "Reducer '$reducer' missing"
    fi
done

# Test 3: Check for proper error handling
info "Checking error handling patterns..."
if grep -q 'log::warn!' src/lib.rs && grep -q 'log::info!' src/lib.rs; then
    pass "Logging implemented"
else
    fail "Missing logging implementation"
fi

# Test 4: Check for proper ownership validation
info "Checking ownership validation..."
if grep -q 'owner_identity == ctx.sender' src/lib.rs; then
    pass "Ownership validation present"
else
    fail "Missing ownership validation"
fi

# Test 5: Check inventory max_stack handling
info "Checking inventory stack limits..."
if grep -q 'max_stack' src/lib.rs && grep -q '.min(item_def.max_stack)' src/lib.rs; then
    pass "Stack limit handling implemented"
else
    fail "Missing stack limit handling"
fi

# Test 6: Check for player count validation
info "Checking instance player count validation..."
if grep -q 'max_players' src/lib.rs && grep -q 'current_players >= instance.max_players' src/lib.rs; then
    pass "Player count validation implemented"
else
    fail "Missing player count validation"
fi

echo ""
echo "=== Test Summary ==="
echo -e "${GREEN}Passed${NC}: $PASSED"
echo -e "${RED}Failed${NC}: $FAILED"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}Some tests failed.${NC}"
    exit 1
fi
