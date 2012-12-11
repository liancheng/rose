#ifndef __ROSE_ERROR_H__
#define __ROSE_ERROR_H__

#include "rose/sexp.h"
#include "rose/state.h"

#include <stdarg.h>
#include <setjmp.h>

R_BEGIN_DECLS

#define __INLINE_ERROR(n)       (((n) << R_TAG_BITS) | R_INLINE_ERROR_TAG)
#define R_ERROR_INTERNAL        0
#define R_ERROR_API             1024
#define R_ERROR_USER            4096
#define R_ERROR_UNKNOWN         (__INLINE_ERROR (R_ERROR_API + 0))
#define R_ERROR_OOM             (__INLINE_ERROR (R_ERROR_API + 1))

typedef struct RError RError;

rsexp r_error_new             (RState*       state,
                               rsexp         message,
                               rsexp         irritants);
rbool r_error_p               (rsexp         obj);
rsexp r_error_get_message     (rsexp         error);
rsexp r_error_get_irritants   (rsexp         error);
void  r_error_set_message_x   (rsexp         error,
                               rsexp         message);
void  r_error_set_irritants_x (rsexp         error,
                               rsexp         irritants);
rsexp r_error_printf          (RState*       state,
                               rconstcstring format,
                               ...);
rsexp r_error_format          (RState*       state,
                               rconstcstring format,
                               ...);
rsexp r_error                 (RState*       state,
                               rconstcstring message);
rsexp r_last_error            (RState*       state);
rsexp r_set_last_error_x      (RState*       state,
                               rsexp         error);
rsexp r_clear_last_error_x    (RState*       state);
rsexp r_inherit_errno_x       (RState*       state,
                               rint          errnum);

typedef struct RNestedJump RNestedJump;

struct RNestedJump {
    RNestedJump* previous;
    jmp_buf      buf;
};

#define r_try(jmp, state)\
        (jmp).previous = (state)->error_jmp;\
        (state)->error_jmp = &(jmp);\
        if (0 == setjmp ((state)->error_jmp->buf))

#define r_catch else

#define r_end_try(state)\
        (state)->error_jmp = jmp.previous;

void r_raise (RState* state);

#define ensure(stmt)\
        do {\
            rsexp __res__ = stmt;\
            if (r_failure_p (__res__))\
                return __res__;\
        }\
        while (0)

R_END_DECLS

#endif  /* __ROSE_ERROR_H__ */
