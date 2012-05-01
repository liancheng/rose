#ifndef __ROSE_MEMORY_H__
#define __ROSE_MEMORY_H__

#include "rose/types.h"

#define R_NEW(struct_type, struct_n)\
        ((struct_type*)(malloc(struct_n * sizeof(struct_type))))

#define R_NEW0(struct_type, struct_n)\
        ((struct_type*)(calloc(struct_n, sizeof(struct_type))))

rpointer r_memdup(rpointer src, rsize byte_size);

#endif  //  __ROSE_MEMORY_H__
