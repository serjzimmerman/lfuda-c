cmake_minimum_required(VERSION 3.20)
project(lfuda)

option(INSOURCEBUILD OFF)

if((${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR}) AND NOT ${INSOURCEBUILD})
    message(FATAL_ERROR "In-source building disabled. Provide -DINSOURCEBUILD option if you are sure about it.")
endif()

# Possibly support windows and MSVC compiler
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Common include directories and install directory
set(LFUDA_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(LFUDA_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)

find_package(GTest)
if(GTEST_FOUND)
    enable_testing()
else()
    message(WARNING "Unable to find Google Test, testing disabled")
endif()

set(NOLINT ON CACHE BOOL "Disable clang-tidy")

if(NOT ${NOLINT})
find_program(CLANG_TIDY_COMMAND clang-tidy)
if(CLANG_TIDY_COMMAND)

# Clang-tidy for linting
set(CMAKE_CXX_CLANG_TIDY 
    clang-tidy;
    -checks=google-*,cppcoreguidelines-*
)
set(CMAKE_C_CLANG_TIDY 
    clang-tidy;
    -checks=google-*,cppcoreguidelines-*
)
else()
message(WARNING "Unable to find clang-tidy, linter disabled")
endif()

else()
message(WARNING "-DNOLINT option provided, linter disabled")
endif()

set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(BASH_PROGRAM bash)

add_subdirectory(lfuda)
add_subdirectory(test)
add_subdirectory(util)