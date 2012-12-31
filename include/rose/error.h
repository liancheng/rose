#ifndef __ROSE_ERROR_H__
#define __ROSE_ERROR_H__

#include "rose/sexp.h"
#include "rose/state.h"

#include <stdarg.h>
#include <setjmp.h>

R_BEGIN_DECLS

#define r_inline_error_p(obj)   (r_get_tag (obj) == R_TAG_INLINE_ERROR)

#define MAKE_INLINE_ERROR(n)    (((n) << R_TAG_BITS) | R_TAG_INLINE_ERROR)
#define R_ERROR_INTERNAL        0
#define R_ERROR_API             1024
#define R_ERROR_USER            4096
#define R_ERROR_UNKNOWN         (MAKE_INLINE_ERROR (R_ERROR_API + 0))
#define R_ERROR_OOM             (MAKE_INLINE_ERROR (R_ERROR_API + 1))

#define R_ERR_COMPILE           10000
#define R_ERR_BAD_SYNTAX        (R_ERR_COMPILE + 1)
#define R_ERR_BAD_VARIABLE      (R_ERR_COMPILE + 2)
#define R_ERR_BAD_FORMALS       (R_ERR_COMPILE + 3)
#define R_ERR_COMPILE_MAX       20000

#define R_ERR_RUNTIME           20000
#define R_ERR_WRONG_TYPE_ARG    (R_ERR_RUNTIME + 1)
#define R_ERR_UNKNOWN_INSTR     (R_ERR_RUNTIME + 2)
#define R_ERR_UNBOUND_VAR       (R_ERR_RUNTIME + 3)
#define R_ERR_WRONG_APPLY       (R_ERR_RUNTIME + 4)
#define R_ERR_WRONG_ARG_NUM     (R_ERR_RUNTIME + 5)
#define R_ERR_INDEX_OVERFLOW    (R_ERR_RUNTIME + 6)
#define R_ERR_RUNTIME_MAX       30000

rsexp r_error_new              (RState* state,
                                rsexp message,
                                rsexp irritants);
rbool r_error_p                (rsexp obj);
rsexp r_error_object_message   (rsexp error);
rsexp r_error_object_irritants (rsexp error);
rsexp r_error_printf           (RState* state,
                                rconstcstring format,
                                ...);
rsexp r_error_format           (RState* state,
                                rconstcstring format,
                                ...);
rsexp r_error                  (RState* state,
                                rconstcstring message);
rsexp r_error_code             (RState* state,
                                rint error_code,
                                ...);
rsexp r_last_error             (RState* state);
rsexp r_set_last_error_x       (RState* state,
                                rsexp error);
rsexp r_clear_last_error_x     (RState* state);
rsexp r_inherit_errno_x        (RState* state,
                                rint errnum);

typedef struct RNestedJump RNestedJump;

struct RNestedJump {
    RNestedJump* previous;
    jmp_buf      buf;
};

void r_raise (RState* state);

#define ensure(stmt)\
        do {\
            if (r_failure_p (stmt))\
                return R_FAILURE;\
        }\
        while (0)

#define ensure_or_goto(stmt, label)\
        do {\
            if (r_failure_p (stmt))\
                goto label;\
        }\
        while (0)

#define ensure_or_ret(stmt, ret)\
        do {\
            if (r_failure_p (stmt))\
                return (ret);\
        }\
        while (0)

R_END_DECLS

#endif  /* __ROSE_ERROR_H__ */
