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

void dump_freq_node_list(freq_node_t freqnode, output_s *format_dump) {
    assert(freqnode);
    assert(format_dump);
    assert(format_dump->file);
    assert(format_dump->print);

    FILE *file = format_dump->file;

    fprintf(file, "\tsubgraph FREQ_%p\n", freqnode);
    fprintf(file, "\t\trankdir = TB;\n\n");
    fprintf(file, "\t\tfreq_%p [label = \"%u\", fillcolor = \"deepskyblue\", fontcolor = \"white\"];\n\n", freqnode,
            freq_node_get_key(freqnode));

    local_list_t local_list = freq_node_get_local(freqnode);
    local_node_t current_local_node = dl_list_get_first(local_list);

    while (!current_local_node) {
        fprintf(file, "\t\tlocal_node_%p [label = \"%s\"]\n", current_local_node,
                *(format_dump->print)(dl_node_get_data(current_local_node)));
        current_local_node = dl_node_get_next(current_local_node);
    }
    fprintf(file, "\n");
    fprintf(file, "\t\tfreq_%p", freqnode);
    current_local_node = dl_list_get_first(local_list);
    while (!current_local_node) {
        fprintf(file, " -> local_node_%p", current_local_node);
        current_local_node = dl_node_get_next(current_local_node);
    }
    fprintf(file, "\n");
    fprintf(file, "\t}\n");
}

void dump_cache(void *cache_, output_s *format_dump) {
    assert(cache_);
    assert(format_dump);
    assert(format_dump->file);

    FILE *file = format_dump->file;
    base_cache_t *cache = (struct base_cache_s *)cache_;
    freq_list_t freqlist = cache->freq_list;
    freq_node_t current_freq_node = dl_list_get_first(freqlist);

    //  Common features on nodes and edges of cache
    fprintf(file, "digraph CACHE\n");
    fprintf(file, "{\n");
    fprintf(file, "\tgraph [dpi = 200, nodesep = 1];\n");
    fprintf(file, "\trankdir = TB\n");
    fprintf(file, "\tsplines = ortho\n", "edge [maxlen = 2, dir = \"both\", shape = \"normal\"];\n");
    fprintf(file, "\tnode [shape = box, style = \"filled\", fillcolor = \"gold\", fontcolor = \"black\"];\n\n");
    fprintf(file, "\tfreq_tail [label = \"WEIGHT\", fillcolor = \"deepskyblue\", fontcolor =\"white\"];\n\n");

    //  Bypass through the freq_list and dump each local_list
    while (!current_freq_node) {
        dump_freq_node(current_freq_node, format_dump);
        current_freq_node = dl_node_get_next(current_freq_node);
    }

    dump_freq_links(freqlist, format_dump->file);

    fprintf(file, "}\n");
}