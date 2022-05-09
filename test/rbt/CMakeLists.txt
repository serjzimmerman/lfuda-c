set(LFUC_SOURCES
  src/rbt.c
)

add_executable(rbt ${LFUC_SOURCES})
target_include_directories(rbt PRIVATE ${LFUDA_COMMON_DIR})
target_link_libraries(rbt lfuda)