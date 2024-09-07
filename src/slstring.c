#include "slstring.h"

#include <string.h>

bool stringsEqual(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}
