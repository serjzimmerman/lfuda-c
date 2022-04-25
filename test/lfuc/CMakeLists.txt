set(LFUC_SOURCES
  src/lfuc.c
)

add_executable(lfuc ${LFUC_SOURCES})
target_include_directories(lfuc PRIVATE ${LFUDA_COMMON_DIR})
target_link_libraries(lfuc lfuda)