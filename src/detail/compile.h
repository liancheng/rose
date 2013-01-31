#ifndef __ROSE_DETAIL_COMPILE_H__
#define __ROSE_DETAIL_COMPILE_H__

#include "detail/state.h"
#include "rose/compile.h"
#include "rose/pair.h"

R_BEGIN_DECLS

#define emit_apply(r)\
        r_list ((r), 1, R_OP_APPLY)

#define emit_arg(r, next)\
        r_list ((r), 2, R_OP_ARG, (next))

#define emit_assign(r, var, next)\
        r_list ((r), 3, R_OP_ASSIGN, (var), (next))

#define emit_bind(r, var, next)\
        r_list ((r), 3, R_OP_BIND, (var), (next))

#define emit_branch(r, then_c, else_c)\
        r_list ((r), 3, R_OP_BRANCH, (then_c), (else_c))

#define emit_capture_cc(r, next)\
        r_list ((r), 2, R_OP_CAPTURE_CC, (next))

#define emit_close(r, formals, body, next)\
        r_list ((r), 4, R_OP_CLOSE, (formals), (body), (next))

#define emit_constant(r, datum, next)\
        r_list ((r), 3, R_OP_CONSTANT, (datum), (next))

#define emit_frame(r, next, ret)\
        r_list ((r), 3, R_OP_FRAME, (next), (ret))

#define emit_halt(r)\
        r_list ((r), 1, R_OP_HALT)

#define emit_refer(r, var, next)\
        r_list ((r), 3, R_OP_REFER, (var), (next))

#define emit_restore_cc(r, stack, var)\
        r_list ((r), 3, R_OP_RESTORE_CC, (stack), (var))

#define emit_return(r)\
        r_list ((r), 1, R_OP_RETURN)

R_END_DECLS

#endif /* __ROSE_DETAIL_COMPILE_H__ */
