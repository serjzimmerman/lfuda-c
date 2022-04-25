#ifndef COMMON_ERROR_H
#define COMMON_ERROR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define UNUSED_PARAMETER(expr) (void)(expr);

#if defined(NDEBUG)
#define MESSAGE(format, ...)                                                   \
  do {                                                                         \
    fprintf(stderr, format, ##__VA_ARGS__);                                    \
  } while (0)

#else
#define MESSAGE(errtype, format, ...)                                          \
  do {                                                                         \
    fprintf(stderr, "%s: at line %d of %s in function %s\n" format, errtype,   \
            __LINE__, __FILE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);           \
  } while (0)
#endif

#if defined(NDEBUG)
#define WARNING(format, ...)                                                   \
  do {                                                                         \
  } while (0)
#define ERROR(format, ...)                                                     \
  do {                                                                         \
    MESSAGE(format, ##__VA_ARGS__);                                            \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#else
#define WARNING(format, ...) MESSAGE("Warning", format, ##__VA_ARGS__);
#define ERROR(format, ...)                                                     \
  do {                                                                         \
    MESSAGE("Fatal error", format, ##__VA_ARGS__);                             \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#endif

#endif