#ifndef __ROSE_DETAIL_COMPILE_H__
#define __ROSE_DETAIL_COMPILE_H__

#include "detail/state.h"
#include "rose/compile.h"
#include "rose/pair.h"

R_BEGIN_DECLS

#define emit_apply(r)\
        r_list ((r), 1, (r)->i.apply)

#define emit_arg(r, next)\
        r_list ((r), 2, (r)->i.arg, (next))

#define emit_assign(r, var, next)\
        r_list ((r), 3, (r)->i.assign, (var), (next))

#define emit_bind(r, var, next)\
        r_list ((r), 3, (r)->i.bind, (var), (next))

#define emit_branch(r, then_c, else_c)\
        r_list ((r), 3, (r)->i.branch, (then_c), (else_c))

#define emit_capture_cc(r, next)\
        r_list ((r), 2, (r)->i.capture_cc, (next))

#define emit_close(r, formals, body, next)\
        r_list ((r), 4, (r)->i.close, (formals), (body), (next))

#define emit_constant(r, datum, next)\
        r_list ((r), 3, (r)->i.constant, (datum), (next))

#define emit_frame(r, next, ret)\
        r_list ((r), 3, (r)->i.frame, (next), (ret))

#define emit_halt(r)\
        r_list ((r), 1, (r)->i.halt)

#define emit_refer(r, var, next)\
        r_list ((r), 3, (r)->i.refer, (var), (next))

#define emit_restore_cc(r, stack, var)\
        r_list ((r), 3, (r)->i.restore_cc, (stack), (var))

#define emit_return(r)\
        r_list ((r), 1, (r)->i.return_)

R_END_DECLS

#endif  //  __ROSE_DETAIL_COMPILE_H__
