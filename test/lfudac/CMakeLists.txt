set(LFUDAC_SOURCES
  src/lfudac.c
)

add_executable(lfudac ${LFUDAC_SOURCES})
target_include_directories(lfudac PRIVATE ${LFUDA_COMMON_DIR})
target_link_libraries(lfudac lfuda)