#
# Taken from gtest example, modified
#
#####################################



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
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.x
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


#needed for easygl
add_compile_definitions(PLATFORM=X11)

FetchContent_GetProperties(spdlog)
if (NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif()

if(NOT TARGET spdlog)
  find_package(spdlog REQUIRED)
endif()

# Add git commit history to build output
execute_process(COMMAND
  git describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

add_compile_definitions(GIT_COMMIT="${GIT_COMMIT}")

enable_testing()

#
# TEST SUITES
##################

add_executable(
  data_init_tests
  circuit.cpp
  file_read_test.cc
  data_init_test.cc
  switch_conn_test.cc
)

#
# ACTUAL PROGRAM
##################

add_executable(
  a1
  main.cpp
  circuit.cpp
  easygl/graphics.cpp
)
target_link_libraries(
  a1
  spdlog::spdlog
  X11
)
target_link_libraries(
  data_init_tests
  GTest::gtest_main
  spdlog::spdlog
)

include(GoogleTest)
gtest_discover_tests(data_init_tests)
