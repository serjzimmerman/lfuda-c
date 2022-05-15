add_subdirectory(lfuc)
add_subdirectory(lfudac)

if(NOT MSVC)
add_subdirectory(hshtend)
endif()

# If there is Google Test available, then add test targets
if (GTEST_FOUND)
add_subdirectory(tl)
add_subdirectory(hsht)
add_subdirectory(rbt)
endif()