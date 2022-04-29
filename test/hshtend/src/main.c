#include "counter.h"
#include "hashtab.h"
#include "memutil.h"
#include "util.h"

#undef NDEBUG

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

const struct cmd_args_t { char *input_path; } cmd_args_default = {NULL};

const char *const usage_string = "Usage: test [-i <path>]\n";

int handle_input(int argc, char **argv, struct cmd_args_t *args) {
    int c = 0, p = 0;

    *args = cmd_args_default;

    while ((c = getopt(argc, argv, "o:i:h")) != -1) {
        switch (c) {
        case 'i':
            args->input_path = optarg;
            p += 2;
            break;
        case 'h':
            fprintf(stderr, usage_string);
            return -1;
        case '?':
            if (optopt == 'i' || optopt == 'o') {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            }
            return -1;
        default:
            exit(EXIT_FAILURE);
        }
    }

    if (argc > p + 1) {
        fprintf(stderr, "Invalid arguments. Run -h for help.\n");
        return -1;
    }

    return 0;
}

char *get_buf_from_file(struct cmd_args_t args, int *fd, size_t *len) {
    struct stat st;

    int fdi = open(args.input_path, O_RDONLY);
    if (fdi == -1) {
        fprintf(stderr, "Could not open input file %s\n", args.input_path);
        exit(EXIT_FAILURE);
    }
    if (fstat(fdi, &st) == -1) {
        fprintf(stderr, "Could not open input file %s\n", args.input_path);
        exit(EXIT_FAILURE);
    }

    char *buf = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fdi, 0);
    *fd = fdi;
    *len = st.st_size;

    return buf;
}

void application_loop_read_file(struct cmd_args_t args) {
    char *tok = NULL;
    int fdi = 0;
    size_t len = 0;

    char *buf = get_buf_from_file(args, &fdi, &len);
    struct counter_s *counter = counter_init(NULL);

    strtokn(buf, " \n", &tok);
    strtokn(NULL, " \n", &tok);

    long long l = atoll(tok);
    long long c = 0;

    c += strtokn(NULL, " ", &tok);
    while (tok && c <= l + 1) {
        counter_item_add(counter, tok);
        c += strtokn(NULL, " \n", &tok);
    }

    l = atoll(tok);
    c = 0;

    c += strtokn(NULL, " ", &tok);
    while (tok && c <= l + 1) {
        printf((tok ? "%d " : "%d"), counter_item_get_count(counter, tok));
        c += strtokn(NULL, " ", &tok);
    }

#if 0
    fprintf(stderr, "\nCollisions: \t%ld\nInserts: \t%ld\nSize: \t\t%ld\n",
            hash_table_get_collisions(counter_get_hashtable(counter)),
            hash_table_get_inserts(counter_get_hashtable(counter)),
            hash_table_get_size(counter_get_hashtable(counter)));
#endif
    counter_free(counter, 1);

    munmap(buf, len);
    close(fdi);
}

void application_loop_read_stdin() {
    long long a, l;
    char *buf, *tok;

    struct counter_s *counter = counter_init(NULL);

    int res = scanf("%lld %lld", &a, &l);
    if (res != 2) {
        fprintf(stderr, "Invalid input\n");
        exit(EXIT_FAILURE);
    }

    buf = calloc_checked(l + 1, sizeof(char));
    if (!buf) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    res = getnc(buf, l);
    if (res != l) {
        fprintf(stderr, "Error while reading from file. Read %d characters out of %lld", res, l);
        exit(EXIT_FAILURE);
    }

    tok = strtok(buf, " ");
    while (tok) {
        counter_item_add(counter, tok);
        tok = strtok(NULL, " ");
    }

    free(buf);

    res = scanf("%lld", &l);
    if (res != 1) {
        fprintf(stderr, "Invalid input\n");
        exit(EXIT_FAILURE);
    }

    buf = calloc_checked(l + 1, sizeof(char));
    if (!buf) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    res = getnc(buf, l);
    if (res != l) {
        fprintf(stderr, "Error while reading from file. Read %d characters out of %lld", res, l);
        exit(EXIT_FAILURE);
    }

    tok = strtok(buf, " ");
    while (tok) {
        printf((tok ? "%d " : "%d"), counter_item_get_count(counter, tok));
        tok = strtok(NULL, " ");
    }

    free(buf);
#if 0
    fprintf(stderr, "\nCollisions: \t%ld\nInserts: \t%ld\nSize: \t\t%ld\n",
            hash_table_get_collisions(counter_get_hashtable(counter)),
            hash_table_get_inserts(counter_get_hashtable(counter)),
            hash_table_get_size(counter_get_hashtable(counter)));
#endif
    counter_free(counter, 1);
}

int main(int argc, char **argv) {
    struct cmd_args_t args;

    if (handle_input(argc, argv, &args)) {
        exit(EXIT_FAILURE);
    }

    if (args.input_path == NULL) {
        application_loop_read_stdin();
    } else {
        application_loop_read_file(args);
    }
}