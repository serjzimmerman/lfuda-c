set(LFUC_SOURCES
  src/dump.c
)

add_executable(dump ${LFUC_SOURCES})
target_include_directories(dump PRIVATE ${LFUDA_COMMON_DIR})
target_link_libraries(dump lfuda)