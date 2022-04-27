cmake_minimum_required(VERSION 3.20)
project(lfuda)

# Possibly support windows and MSVC compiler
if(MSVC)
    add_compile_options(/W4 /WX)
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
message(WARNING "Unable to find clang-tidy, disabling linter. You will have to live without it ;)")
endif()

set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(lfuda)
add_subdirectory(test)