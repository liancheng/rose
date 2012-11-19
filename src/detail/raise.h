#ifndef __ROSE_DETAIL_RAISE_H__
#define __ROSE_DETAIL_RAISE_H__

#include "rose/raise.h"
#include "rose/state.h"

#include <setjmp.h>

typedef struct RNestedJump RNestedJump;

struct RNestedJump {
    RNestedJump* previous;
    jmp_buf      buf;
};

#define R_TRY(jmp, state)\
        (jmp).previous = (state)->error_jmp;\
        (state)->error_jmp = &(jmp);\
        if (0 == setjmp ((state)->error_jmp->buf))

#define R_CATCH else

#define R_END_TRY(state)\
        (state)->error_jmp = jmp.previous;

#define R_RAISE(state)  longjmp (state->error_jmp->buf, 1)

#endif  /* __ROSE_DETAIL_RAISE_H__ */
