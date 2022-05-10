# Test application for Red-Black tree (rbt)

set(RBT_SOURCES
  src/rbt.cc
)

add_executable(rbt ${RBT_SOURCES})
target_include_directories(rbt PRIVATE ${LFUDA_COMMON_DIR} ${GTEST_INCLUDE_DIRECTORIES})
target_link_libraries(rbt lfuda ${GTEST_BOTH_LIBRARIES})

gtest_discover_tests(rbt)