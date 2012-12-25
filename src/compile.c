#include "detail/state.h"
#include "rose/bytevector.h"
#include "rose/compile.h"
#include "rose/eq.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#define ins_apply(state)\
        r_list ((state), 1,\
                reserved ((state), INS_APPLY))

#define ins_arg(state, next)\
        r_list ((state), 2,\
                reserved ((state), INS_ARG), (next))

#define ins_assign(state, var, next)\
        r_list ((state), 3,\
                reserved ((state), INS_ASSIGN), (var), (next))

#define ins_bind(state, var, next)\
        r_list ((state), 3,\
                reserved ((state), INS_BIND), (var), (next))

#define ins_branch(state, then_c, else_c)\
        r_list ((state), 3,\
                reserved ((state), INS_BRANCH), (then_c), (else_c))

#define ins_close(state, vars, body, next)\
        r_list ((state), 4,\
                reserved ((state), INS_CLOSE), (vars), (body), (next))

#define ins_const(state, datum, next)\
        r_list ((state), 3,\
                reserved ((state), INS_CONST), (datum), (next))

#define ins_frame(state, next, ret)\
        r_list ((state), 3,\
                reserved ((state), INS_FRAME), (next), (ret))

#define ins_halt(state)\
        r_list ((state), 1,\
                reserved ((state), INS_HALT))

#define ins_refer(state, var, next)\
        r_list ((state), 3,\
                reserved ((state), INS_REFER), (var), (next))

#define ins_return(state)\
        r_list ((state), 1,\
                reserved ((state), INS_RETURN))

static rsexp compile          (RState* state, rsexp expr, rsexp next);
static rsexp compile_sequence (RState* state, rsexp seq, rsexp next);

static rbool form_eq_p (RState* state, RReservedWord name, rsexp expr)
{
    return r_eq_p (state, reserved (state, name), r_car (expr));
}

static void missing_or_extra_expression (RState* state, rsexp expr)
{
    r_error_format (state, "missing or extra expression(s) in ~s", expr);
}

static void bad_expression (RState* state, rsexp expr)
{
    r_error_format (state, "bad expression ~s", expr);
}

static void bad_variable (RState* state, rsexp var, rsexp expr)
{
    r_error_format (state, "bad variable ~s in ~s", var, expr);
}

static rsexp check_expr_length_eq (RState* state,
                                   rsexp   expr,
                                   ruint   expected)
{
    rsexp length;

    length = r_length (state, expr);

    if (r_failure_p (length)) {
        bad_expression (state, expr);
        goto exit;
    }

    if (r_uint_from_sexp (length) != expected) {
        missing_or_extra_expression (state, expr);
        goto exit;
    }

exit:
    return length;
}

static rsexp check_expr_length_within (RState* state,
                                       rsexp   expr,
                                       ruint   lower,
                                       ruint   upper)
{
    rsexp length;
    ruint unboxed;

    length = r_length (state, expr);

    if (r_failure_p (length)) {
        bad_expression (state, expr);
        goto exit;
    }

    unboxed = r_uint_from_sexp (length);

    if (lower < unboxed || unboxed > upper) {
        missing_or_extra_expression (state, expr);
        goto exit;
    }

exit:
    return length;
}

static rsexp compile_quote (RState* state, rsexp expr, rsexp next)
{
    return r_failure_p (check_expr_length_eq (state, expr, 2))
           ? R_FAILURE
           : ins_const (state, r_cadr (expr), next);
}

static rsexp compile_assign_or_define (RState* state, rsexp expr, rsexp next)
{
    rsexp length;
    rsexp var;
    rsexp val;
    rsexp code;

    length = check_expr_length_eq (state, expr, 3u);

    if (r_failure_p (length)) {
        code = R_FAILURE;
        goto exit;
    }

    if (!r_eq_p (state, length, r_uint_to_sexp (3u))) {
        code = R_FAILURE;
        goto exit;
    }

    var = r_cadr (expr);

    if (!r_symbol_p (var)) {
        bad_variable (state, var, expr);
        code = R_FAILURE;
        goto exit;
    }

    val = r_caddr (expr);
    code = compile (state, val, next);

exit:
    return code;
}

static rsexp compile_assign (RState* state, rsexp expr, rsexp next)
{
    rsexp var = r_cadr (expr);
    next = ins_assign (state, var, next);
    return compile_assign_or_define (state, expr, next);
}

static rsexp compile_define (RState* state, rsexp expr, rsexp next)
{
    rsexp var = r_cadr (expr);
    next = ins_bind (state, var, next);
    return compile_assign_or_define (state, expr, next);
}

static rsexp compile_conditional (RState* state, rsexp expr, rsexp next)
{
    rsexp length;
    rsexp then_code;
    rsexp else_code;
    rsexp code;

    length = check_expr_length_within (state, expr, 3, 4);

    if (r_failure_p (length)) {
        code = R_FAILURE;
        goto exit;
    }

    then_code = compile (state, r_caddr (expr), next);

    if (r_failure_p (then_code)) {
        code = R_FAILURE;
        goto exit;
    }

    if (r_uint_from_sexp (length) == 3u)
        else_code = ins_const (state, R_UNSPECIFIED, next);
    else
        else_code = compile (state, r_cadddr (expr), next);

    if (r_failure_p (else_code)) {
        code = R_FAILURE;
        goto exit;
    }

    code = ins_branch (state, then_code, else_code);
    code = compile (state, r_cadr (expr), code);

exit:
    return code;
}

static rsexp compile_lambda (RState* state, rsexp expr, rsexp next)
{
    rsexp vars      = r_cadr (expr);
    rsexp body      = r_reverse (state, r_cddr (expr));
    rsexp ret       = ins_return (state);
    rsexp body_code = compile_sequence (state, body, ret);

    return ins_close (state, vars, body_code, next);
}

static rbool tail_p (RState* state, rsexp next)
{
    return r_eq_p (state, reserved (state, INS_RETURN), r_car (next));
}

static rsexp compile_application (RState* state, rsexp expr, rsexp next)
{
    rsexp proc = r_car (expr);
    rsexp args = r_cdr (expr);
    rsexp code = ins_apply (state);

    code = compile (state, proc, code);

    while (!r_null_p (args)) {
        code = ins_arg (state, code);
        code = compile (state, r_car (args), code);
        args = r_cdr (args);
    }

    return tail_p (state, next) ? code : ins_frame (state, next, code);
}

static rsexp compile_sequence (RState* state, rsexp seq, rsexp next)
{
    rsexp expr;

    while (!r_null_p (seq) && !r_failure_p (next)) {
        expr = r_car (seq);
        seq = r_cdr (seq);
        next = compile (state, expr, next);
    }

    return next;
}

static rsexp compile (RState* state, rsexp expr, rsexp next)
{
    rsexp code;

    r_gc_scope_open (state);

    if (r_symbol_p (expr)) {
        code = ins_refer (state, expr, next);
        goto exit;
    }

    if (!r_pair_p (expr)) {
        code = ins_const (state, expr, next);
        goto exit;
    }

    if (form_eq_p (state, KW_QUOTE, expr)) {
        code = compile_quote (state, expr, next);
        goto exit;
    }

    if (form_eq_p (state, KW_DEFINE, expr)) {
        code = compile_define (state, expr, next);
        goto exit;
    }

    if (form_eq_p (state, KW_SET_X, expr)) {
        code = compile_assign (state, expr, next);
        goto exit;
    }

    if (form_eq_p (state, KW_IF, expr)) {
        code = compile_conditional (state, expr, next);
        goto exit;
    }

    if (form_eq_p (state, KW_LAMBDA, expr)) {
        code = compile_lambda (state, expr, next);
        goto exit;
    }

    code = compile_application (state, expr, next);
    goto exit;

    bad_expression (state, expr);
    code = R_FAILURE;

exit:
    r_gc_scope_close_and_protect (state, code);

    r_format (state, "expression: ~s~%code to: ~s~%", expr, code);

    return code;
}

rsexp r_compile (RState* state, rsexp program)
{
    return compile_sequence (state, program, ins_halt (state));
}
