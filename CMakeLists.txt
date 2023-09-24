cmake_minimum_required(VERSION 3.14)
project(a1)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.x
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_GetProperties(spdlog)
if (NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif()

if(NOT TARGET spdlog)
  find_package(spdlog REQUIRED)
endif()

enable_testing()

add_executable(
  hello_test
  hello_test.cc
)
add_executable(
  a1
  main.cpp
)
target_link_libraries(
  a1
  spdlog::spdlog
)
target_link_libraries(
  hello_test
  GTest::gtest_main
  spdlog::spdlog
)

include(GoogleTest)
gtest_discover_tests(hello_test)
