# Test application for Doubly Linked List (tl)

set(TL_SOURCES
  src/tl.cc
)

add_executable(tl ${TL_SOURCES})
target_include_directories(tl PRIVATE ${LFUDA_COMMON_DIR} ${GTEST_INCLUDE_DIRECTORIES})
target_link_libraries(tl lfuda ${GTEST_BOTH_LIBRARIES})

gtest_discover_tests(tl)