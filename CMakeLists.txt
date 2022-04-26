cmake_minimum_required(VERSION 3.20)
project(lfuda)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra)
endif()

set(LFUDA_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(LFUDA_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)

find_package(GTest)
if (GTEST_FOUND)
enable_testing()
else()
message(WARNING "Unable to find Google Test, testing disabled")
endif()

add_subdirectory(lfuda)
add_subdirectory(test)