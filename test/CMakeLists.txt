add_subdirectory(lfuc)
add_subdirectory(lfudac)

if(NOT MSVC)
add_subdirectory(hshtend)
endif()

# If there is Google Test available, then add test targets
if (NOT NOGTEST AND GTEST_FOUND AND CMAKE_MINOR_VERSION GREATER_EQUAL 20)
add_subdirectory(tl)
add_subdirectory(hsht)
add_subdirectory(rbt)
endif()