#ifndef LFUDA_DUMP_H
#define LFUDA_DUMP_H

#include "dllist.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstdio>
extern "C" {
#else
#include <stddef.h>
#include <stdio.h>
#endif

typedef void (*format_print_index)(void *index, FILE *file);

//  Structure for correct dump of index in the neccessary file
//  File must be opend before function of dump and closed after it
typedef struct {
    FILE *file;
    format_print_index print;
} output_t;

//  Graph dump with using graphvis as API
void dump_cache(void *cache_, output_t format_dump);

#ifdef __cplusplus
}
#endif

#endif