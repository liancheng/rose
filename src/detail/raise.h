#ifndef __ROSE_DETAIL_RAISE_H__
#define __ROSE_DETAIL_RAISE_H__

#include "rose/raise.h"
#include "rose/state.h"

#include <setjmp.h>

struct RErrorJmp {
    RErrorJmp* previous;
    jmp_buf    buf;
};

#define R_TRY(state)    if (0 == setjmp ((state)->error_jmp->buf))
#define R_THROW(state)  longjmp (state->error_jmp->buf, 1)

#endif  /* __ROSE_DETAIL_RAISE_H__ */
