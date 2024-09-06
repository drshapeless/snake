#include "helper.h"

#include <stdio.h>

#include "allocator.h"

char *readfile(const char *filename, unsigned int *length) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "failed to open file %s\n", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);

    int len = ftell(fp);
    fseek(fp, 0, 0);

    char *buffer = slMalloc(sizeof(char) * len);
    size_t ret = fread(buffer, sizeof(char), len, fp);
    if (ret == 0) {
        fprintf(stderr, "fread fucked up, do something in readfile function\n");
    }

    fclose(fp);

    /* Remember to free the return pointer */
    *length = len;
    return buffer;
}
