#ifndef COMMON_MEMUTIL_H
#define COMMON_MEMUTIL_H

#include "error.h"

#ifdef __cpluscplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

// Calloc memory and exit when there is none available
static void *calloc_checked(size_t count, size_t size) {
  void *ptr = calloc(count, size);
  if (!ptr) {
// Verbose error messages
#ifdef NDEBUG
    ERROR("Memory exhausted\n");
#else
    ERROR("Memory exhausted, cannot allocate enough memory of size: %lu\n", (unsigned long)(count) * (size));
#endif
  }
  return ptr;
}

#endif