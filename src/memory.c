#include "rose/memory.h"

#include <stdlib.h>
#include <string.h>

rpointer r_memdup(rpointer src, rsize byte_size)
{
    rpointer res = NULL;

    if (src) {
        res = malloc(byte_size);
        memcpy(res, src, byte_size);
    }

    return res;
}
