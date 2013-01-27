#ifndef __ROSE_DETAIL_SSTREAM_H__
#define __ROSE_DETAIL_SSTREAM_H__

#ifdef __APPLE__

#include "rose/types.h"

#include <stdio.h>

R_BEGIN_DECLS

FILE* fmemopen (void* buf, size_t size, char const* mode);
FILE* open_memstream (char** buf_ptr, size_t* size_ptr);

R_END_DECLS

#endif /* __APPLE__ */

#endif /* __ROSE_DETAIL_SSTREAM_H__ */
