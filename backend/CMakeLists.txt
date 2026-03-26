cmake_minimum_required(VERSION 3.20)
project(WFM_Server CXX)

# C++20 standard (C++ Core Guidelines requirement)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings (applied after Google Test is fetched)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(WFM_COMPILE_OPTIONS -Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Find required packages
find_package(Threads REQUIRED)

# ── External dependencies via FetchContent ───────────────────────────────────
include(FetchContent)

# Google Test
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Crow HTTP framework — use system installation (brew install crow)
find_package(Crow REQUIRED
    PATHS /opt/homebrew/lib/cmake/Crow /usr/local/lib/cmake/Crow)

# nlohmann/json (header-only)
FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

# Source files for models
set(MODEL_SOURCES
    src/models/ticket.cpp
)

# Source files for core router module
set(ROUTER_SOURCES
    src/core/router/confidence_evaluator.cpp
    src/core/router/routing_engine.cpp
)

# Source files for core simulator module
set(SIMULATOR_SOURCES
    src/core/simulator/cost_calculator.cpp
    src/core/simulator/threshold_optimizer.cpp
)

# Source files for core copilot module
set(COPILOT_SOURCES
    src/core/copilot/context_generator.cpp
)

# Source files for core rlhf module
set(RLHF_SOURCES
    src/core/rlhf/golden_dataset_manager.cpp
)

# Source files for core analytics module
set(ANALYTICS_SOURCES
    src/core/analytics/drift_detector.cpp
    src/core/analytics/metrics_aggregator.cpp
)

# Source files for core deployment module
set(DEPLOYMENT_SOURCES
    src/core/deployment/evm_calculator.cpp
    src/core/deployment/phase_tracker.cpp
    src/core/deployment/milestone_manager.cpp
)

# Create library for router module (for testing)
add_library(wfm_router STATIC
    ${MODEL_SOURCES}
    ${ROUTER_SOURCES}
)

target_include_directories(wfm_router PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(wfm_router PRIVATE ${WFM_COMPILE_OPTIONS})

# Create library for simulator module (for testing)
add_library(wfm_simulator STATIC
    ${MODEL_SOURCES}
    ${SIMULATOR_SOURCES}
)

target_include_directories(wfm_simulator PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(wfm_simulator PRIVATE ${WFM_COMPILE_OPTIONS})

# Create library for copilot module (for testing)
add_library(wfm_copilot STATIC
    ${MODEL_SOURCES}
    ${COPILOT_SOURCES}
)

target_include_directories(wfm_copilot PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(wfm_copilot PRIVATE ${WFM_COMPILE_OPTIONS})

# Create library for rlhf module (for testing)
add_library(wfm_rlhf STATIC
    ${MODEL_SOURCES}
    ${RLHF_SOURCES}
)

target_include_directories(wfm_rlhf PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(wfm_rlhf PRIVATE ${WFM_COMPILE_OPTIONS})

# Create library for analytics module (for testing)
add_library(wfm_analytics STATIC
    ${MODEL_SOURCES}
    ${ANALYTICS_SOURCES}
)

target_include_directories(wfm_analytics PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(wfm_analytics PRIVATE ${WFM_COMPILE_OPTIONS})

# Create library for deployment module (for testing)
add_library(wfm_deployment STATIC
    ${MODEL_SOURCES}
    ${DEPLOYMENT_SOURCES}
)

target_include_directories(wfm_deployment PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(wfm_deployment PRIVATE ${WFM_COMPILE_OPTIONS})

# Enable testing
enable_testing()

# Router tests
add_executable(router_test
    tests/core/router_test.cpp
)

target_link_libraries(router_test
    wfm_router
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

target_include_directories(router_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Register test with CTest
include(GoogleTest)
gtest_discover_tests(router_test)

# Simulator tests
add_executable(simulator_test
    tests/core/simulator_test.cpp
)

target_link_libraries(simulator_test
    wfm_simulator
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

target_include_directories(simulator_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Register simulator tests with CTest
gtest_discover_tests(simulator_test)

# Copilot tests
add_executable(copilot_test
    tests/core/copilot_test.cpp
)

target_link_libraries(copilot_test
    wfm_copilot
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

target_include_directories(copilot_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Register copilot tests with CTest
gtest_discover_tests(copilot_test)

# RLHF tests
add_executable(rlhf_test
    tests/core/rlhf_test.cpp
)

target_link_libraries(rlhf_test
    wfm_rlhf
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

target_include_directories(rlhf_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Register rlhf tests with CTest
gtest_discover_tests(rlhf_test)

# Analytics tests
add_executable(analytics_test
    tests/core/analytics_test.cpp
)

target_link_libraries(analytics_test
    wfm_analytics
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

target_include_directories(analytics_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Register analytics tests with CTest
gtest_discover_tests(analytics_test)

# Deployment tests
add_executable(deployment_test
    tests/core/deployment_test.cpp
)

target_link_libraries(deployment_test
    wfm_deployment
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

target_include_directories(deployment_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Register deployment tests with CTest
gtest_discover_tests(deployment_test)

# Custom target to run tests with verbose output
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose --output-on-failure
    DEPENDS router_test simulator_test copilot_test rlhf_test analytics_test deployment_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ── API controller sources ────────────────────────────────────────────────────
set(CONTROLLER_SOURCES
    src/api/router_controller.cpp
    src/api/simulator_controller.cpp
    src/api/copilot_controller.cpp
    src/api/rlhf_controller.cpp
    src/api/analytics_controller.cpp
    src/api/deployment_controller.cpp
)

# ── uber_wfm_backend executable ──────────────────────────────────────────────
add_executable(uber_wfm_backend
    src/main.cpp
    ${MODEL_SOURCES}
    ${ROUTER_SOURCES}
    ${SIMULATOR_SOURCES}
    ${COPILOT_SOURCES}
    ${RLHF_SOURCES}
    ${ANALYTICS_SOURCES}
    ${DEPLOYMENT_SOURCES}
    ${CONTROLLER_SOURCES}
)

target_include_directories(uber_wfm_backend PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(uber_wfm_backend
    Crow::Crow
    nlohmann_json::nlohmann_json
    Threads::Threads
)

target_compile_options(uber_wfm_backend PRIVATE ${WFM_COMPILE_OPTIONS})

# Print configuration
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
