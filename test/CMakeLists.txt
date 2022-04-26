add_subdirectory(lfuc)

if (GTEST_FOUND)
enable_testing()
add_subdirectory(tl)
endif()