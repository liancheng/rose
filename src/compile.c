#include "detail/compile.h"
#include "rose/bytevector.h"
#include "rose/compile.h"
#include "rose/eq.h"
#include "rose/number.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

static rsexp compile          (RState* state, rsexp expr, rsexp next);
static rsexp compile_sequence (RState* state, rsexp seq, rsexp next);

static rbool form_eq_p (RState* state, RReservedWord name, rsexp expr)
{
    return r_eq_p (state, reserved (state, name), r_car (expr));
}

static rbool tail_p (RState* state, rsexp next)
{
    return r_eq_p (state, reserved (state, INS_RETURN), r_car (next));
}

static void missing_or_extra_expression (RState* state, rsexp expr)
{
    r_error_format (state, "missing or extra expression(s) in ~s", expr);
}

static void bad_syntax (RState* state, rsexp expr)
{
    r_error_format (state, "bad syntax ~s", expr);
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
        bad_syntax (state, expr);
        goto exit;
    }

    if (r_uint_from_sexp (length) != expected) {
        missing_or_extra_expression (state, expr);
        goto exit;
    }

exit:
    return length;
}

static rsexp compile_quote (RState* state, rsexp expr, rsexp next)
{
    rsexp length;

    ensure (length = r_length (state, expr));

    if (r_uint_from_sexp (length) != 2u) {
        bad_syntax (state, expr);
        return R_FAILURE;
    }

    return emit_const (state, r_cadr (expr), next);
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
    next = emit_assign (state, var, next);
    return compile_assign_or_define (state, expr, next);
}

static rsexp compile_define (RState* state, rsexp expr, rsexp next)
{
    rsexp var = r_cadr (expr);
    next = emit_bind (state, var, next);
    return compile_assign_or_define (state, expr, next);
}

static rsexp compile_conditional (RState* state, rsexp expr, rsexp next)
{
    rsexp length;
    ruint u_length;
    rsexp then_expr, then_code;
    rsexp else_expr, else_code;
    rsexp test_expr, code;

    expr = r_cdr (expr);
    ensure_or_error (length = r_length (state, expr),
                     bad_syntax (state, expr));

    u_length = r_uint_from_sexp (length);

    if (u_length < 2 || u_length > 3) {
        bad_syntax (state, expr);
        return R_FAILURE;
    }

    test_expr = r_car (expr);
    then_expr = r_cadr (expr);
    else_expr = u_length == 2 ? R_UNSPECIFIED : r_caddr (expr);

    ensure (then_code = compile (state, then_expr, next));
    ensure (else_code = r_unspecified_p (else_expr)
                      ? emit_const (state, R_UNSPECIFIED, next)
                      : compile (state, else_expr, next));

    ensure (code = emit_branch (state, then_code, else_code));
    ensure (code = compile (state, test_expr, code));

    return code;
}

static rsexp compile_lambda (RState* state, rsexp expr, rsexp next)
{
    rsexp vars = r_cadr (expr);
    rsexp body = r_reverse (state, r_cddr (expr));
    rsexp body_code = compile_sequence (state, body, emit_return (state));

    return emit_close (state, vars, body_code, next);
}

static rsexp compile_application (RState* state, rsexp expr, rsexp next)
{
    rsexp proc = r_car (expr);
    rsexp args = r_cdr (expr);
    rsexp code = emit_apply (state);

    code = compile (state, proc, code);

    while (!r_null_p (args)) {
        code = emit_arg (state, code);
        code = compile (state, r_car (args), code);
        args = r_cdr (args);
    }

    return tail_p (state, next) ? code : emit_frame (state, next, code);
}

static rsexp compile_call_cc (RState* state, rsexp expr, rsexp next)
{
    rsexp code;

    r_gc_scope_open (state);

    code = emit_apply (state);
    code = compile (state, r_cadr (expr), code);
    code = emit_arg (state, code);
    code = emit_capture_cc (state, code);
    code = tail_p (state, next) ? code : emit_frame (state, next, code);

    r_gc_scope_close_and_protect (state, code);

    return code;
}

static rsexp compile (RState* state, rsexp expr, rsexp next)
{
    rsexp code;

    r_gc_scope_open (state);

    if (r_symbol_p (expr)) {
        code = emit_refer (state, expr, next);
        goto exit;
    }

    if (!r_pair_p (expr)) {
        code = emit_const (state, expr, next);
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

    if (form_eq_p (state, KW_CALL_CC, expr)) {
        code = compile_call_cc (state, expr, next);
        goto exit;
    }

    code = compile_application (state, expr, next);
    goto exit;

    bad_syntax (state, expr);
    code = R_FAILURE;

exit:
    r_gc_scope_close_and_protect (state, code);

    return code;
}

static rsexp compile_sequence (RState* state, rsexp seq, rsexp next)
{
    rsexp expr;

    r_gc_scope_open (state);

    while (!r_null_p (seq) && !r_failure_p (next)) {
        expr = r_car (seq);
        seq = r_cdr (seq);
        next = compile (state, expr, next);
    }

    r_gc_scope_close_and_protect (state, next);

    return next;
}

rsexp r_compile (RState* state, rsexp program)
{
    rsexp halt = emit_halt (state);

    return r_failure_p (halt)
           ? R_FAILURE
           : compile_sequence (state, program, halt);
}
