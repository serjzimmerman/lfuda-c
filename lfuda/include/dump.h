#ifndef DUMP_CACHE_H
#define DUMP_CACHE_H

#include "basecache.h"
#include "clist.h"
#include "dllist.h"
#include "hashtab.h"

#include "memutil.h"
#include <stdio.h>

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

typedef char *(*format_print_index)(void *index);

//  Structure for correct dump of index in the neccessary file
//  File must be opend before function of dump and closed after it
typedef struct {
    FILE *file;
    format_print_index print;
} output_s;

//  Dump links of freq nodes
void dump_freq_links(freq_list_t freqlist, FILE *file);

//  Graph dump with using graphvis as API
void dump_cache(void *cache_, output_s *format_dump);

//  Dump local list corresponding to the weight
void dump_freq_node_list(freq_node_t freqnode, output_s *format_dump);

#ifdef __cplusplus
}
#endif

#endif