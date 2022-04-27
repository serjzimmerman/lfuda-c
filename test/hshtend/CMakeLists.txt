# Hash Table end-to-end tests (hsht)
# It is copied from my previous implementation of hash table (Segei Zimmerman)

set(HSHTEND_SOURCES
  src/spair.c
  src/main.c
  src/counter.c
  src/util.c
)

add_executable(hshtend ${HSHTEND_SOURCES})
target_include_directories(hshtend PRIVATE ${LFUDA_COMMON_DIR})
target_link_libraries(hshtend lfuda)

install(TARGETS hshtend DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)