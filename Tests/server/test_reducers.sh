#!/bin/bash
# SpaceTimeDB Reducer Integration Tests
# This script tests the eonserver module by starting a local SpaceTimeDB instance
# and calling reducers via the CLI.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SERVER_DIR="$PROJECT_ROOT/Server/eonserver"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Database name for testing
TEST_DB="eon_test_$(date +%s)"

log_info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

log_pass() {
    echo -e "${GREEN}[PASS]${NC} $1"
    ((TESTS_PASSED++))
}

log_fail() {
    echo -e "${RED}[FAIL]${NC} $1"
    ((TESTS_FAILED++))
}

run_test() {
    local test_name="$1"
    local test_cmd="$2"
    local expected_success="${3:-true}"

    ((TESTS_RUN++))
    echo ""
    log_info "Running: $test_name"

    if eval "$test_cmd" > /tmp/test_output.txt 2>&1; then
        if [ "$expected_success" = "true" ]; then
            log_pass "$test_name"
            return 0
        else
            log_fail "$test_name (expected failure but succeeded)"
            cat /tmp/test_output.txt
            return 1
        fi
    else
        if [ "$expected_success" = "false" ]; then
            log_pass "$test_name (expected failure)"
            return 0
        else
            log_fail "$test_name"
            cat /tmp/test_output.txt
            return 1
        fi
    fi
}

cleanup() {
    log_info "Cleaning up..."
    # Delete test database if it exists
    spacetime delete --local "$TEST_DB" 2>/dev/null || true
}

# Trap cleanup on exit
trap cleanup EXIT

# Main test execution
main() {
    echo "============================================"
    echo "  SpaceTimeDB Reducer Integration Tests"
    echo "============================================"
    echo ""

    # Check prerequisites
    if ! command -v spacetime &> /dev/null; then
        echo "Error: spacetime CLI not found. Please install SpaceTimeDB."
        exit 1
    fi

    # Build the module
    log_info "Building eonserver module..."
    cd "$SERVER_DIR"
    spacetime build || {
        echo "Error: Failed to build eonserver module"
        exit 1
    }

    # Check if local SpaceTimeDB is running
    log_info "Checking if local SpaceTimeDB is running..."
    if ! spacetime server ping --local 2>/dev/null; then
        log_info "Starting local SpaceTimeDB server..."
        spacetime start &
        sleep 3
    fi

    # Publish the module
    log_info "Publishing eonserver to local instance as '$TEST_DB'..."
    spacetime publish --local "$TEST_DB" || {
        echo "Error: Failed to publish module"
        exit 1
    }

    # Wait for module to be ready
    sleep 2

    echo ""
    echo "============================================"
    echo "  Running Tests"
    echo "============================================"

    # Test 1: Initialize item definitions
    run_test "init_item_definitions" \
        "spacetime call --local $TEST_DB init_item_definitions"

    # Test 2: Create a game instance
    run_test "create_instance (valid)" \
        "spacetime call --local $TEST_DB create_instance 'Test Room' 4"

    # Test 3: Create instance with invalid name (empty)
    run_test "create_instance (empty name)" \
        "spacetime call --local $TEST_DB create_instance '' 4" \
        "false"

    # Test 4: Create instance with invalid max players
    run_test "create_instance (invalid max_players)" \
        "spacetime call --local $TEST_DB create_instance 'Invalid' 20" \
        "false"

    # Test 5: Query tables via SQL
    run_test "query game_instance table" \
        "spacetime sql --local $TEST_DB 'SELECT * FROM game_instance'"

    run_test "query item_definition table" \
        "spacetime sql --local $TEST_DB 'SELECT * FROM item_definition'"

    # Print summary
    echo ""
    echo "============================================"
    echo "  Test Summary"
    echo "============================================"
    echo "Tests Run:    $TESTS_RUN"
    echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
    echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
    echo ""

    if [ $TESTS_FAILED -gt 0 ]; then
        exit 1
    fi
}

main "$@"
