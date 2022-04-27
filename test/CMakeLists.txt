add_subdirectory(lfuc)

# If there is Google Test available, then add test targets
if (GTEST_FOUND)
add_subdirectory(tl)
# add_subdirectory(hsht)
endif()