option(HASHTAB_USE_N_OPTIMIZATION OFF)

set(LFUDA_SOURCES
    src/dllist.c
    src/hashtab.c
)

add_library(lfuda ${LFUDA_SOURCES})
# Temporarily include src directory for testing
target_include_directories(lfuda PRIVATE ${LFUDA_COMMON_DIR} PUBLIC include)

if(${HASHTAB_USE_N_OPTIMIZATION})
target_compile_definitions(lfuda PUBLIC HASHTAB_USE_N_OPTIMIZATION)
endif()