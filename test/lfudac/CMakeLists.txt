set(LFUDAC_SOURCES
  src/lfudac.c
)

add_executable(lfudac ${LFUDAC_SOURCES})
target_include_directories(lfudac PRIVATE ${LFUDA_COMMON_DIR})
target_link_libraries(lfudac lfuda)

install(TARGETS lfudac DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(BASH_PROGRAM)
    add_test(NAME TestLFU-DA.TestEndToEnd COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh "$<TARGET_FILE:lfudac>" ${CMAKE_CURRENT_SOURCE_DIR})
endif()