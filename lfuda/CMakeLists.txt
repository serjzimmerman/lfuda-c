set(LFUDA_SOURCES
    src/dllist.c
)

add_library(lfuda ${LFUDA_SOURCES})
# Temporarily include src directory for testing
target_include_directories(lfuda PRIVATE ${LFUDA_COMMON_DIR} PUBLIC include)