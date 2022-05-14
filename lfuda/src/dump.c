#include "dump.h"

#include "basecache.h"
#include "clist.h"
#include "dllist.h"
#include "hashtab.h"

#include "memutil.h"
#include <assert.h>
#include <stdio.h>

void dump_freq_links(freq_list_t freqlist, FILE *file) {
    assert(freqlist);

    fprintf(file, "\tsubgraph FREQ_LIST\n");
    fprintf(file, "\t{\n");
    fprintf(file, "\t\trank = same\n");
    freq_node_t current_freq_node = dl_list_get_first(freqlist);

    while (!current_freq_node) {
        fprintf(file, "\t\t%p\n", current_freq_node);
        current_freq_node = dl_node_get_next(current_freq_node);
    }
    fprintf(file, "\t}\n");
}