# Test application for Hash Table (hsht)

set(HSHT_SOURCES
  src/hsht.cc
)

add_executable(hsht ${HSHT_SOURCES})
target_include_directories(hsht PRIVATE ${LFUDA_COMMON_DIR} ${GTEST_INCLUDE_DIRECTORIES})
target_link_libraries(hsht lfuda ${GTEST_BOTH_LIBRARIES})

gtest_discover_tests(hsht)