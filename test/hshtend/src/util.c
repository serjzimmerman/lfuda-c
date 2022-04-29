#include "util.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

unsigned long sdbm(const char *str) {
    unsigned long hash = 0;
    int c;

    while ((c = *str++))
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

long long getnc(char *dst, long long n) {
    long long i = 0, c = 0;

    assert(dst);

    while ((c = getchar()) == '\n') {
    }

    for (i = 0; i < n && c != EOF; i++) {
        *(dst++) = c;
        c = getchar();
    }

    *dst = '\0';

    return i;
}

long long strtokn(char *src, const char *const delim, char **token) {
    static char *curr = NULL;

    size_t len = strlen(delim);

    if (src) {
        curr = src;
    }

    if (!curr && !src) {
        (*token) = NULL;
        return 0;
    }

    if (!curr && src) {
        curr = src;
    }

    int c = 0;
    size_t i = 0;

    while (*curr != '\0') {
        for (i = 0; i < len; i++) {
            if (*curr == delim[i]) {
                curr++;
                break;
            }
        }
        if (i == len) {
            break;
        } else {
            c++;
        }
    }

    if (*curr == '\0') {
        curr = NULL;
        *token = NULL;
        return c;
    }

    *token = curr;

    while (*curr != '\0') {
        for (i = 0; i < len; i++) {
            if (*curr == delim[i]) {
                *curr = '\0';
                break;
            }
        }
        c++;
        curr++;
        if (i < len) {
            break;
        }
    }

    return c;
}

long long gettokn(char *src) {
    long long r = 1;

    assert(src);

    while ((src = strchr(src, ' '))) {
        r++;
        src++;
    }

    return r;
}