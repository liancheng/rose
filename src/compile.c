#include "detail/compile.h"
#include "rose/bytevector.h"
#include "rose/compile.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/number.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

static rsexp compile (RState* state, rsexp expr, rsexp next);

static rsexp compile_sequence (RState* state, rsexp seq, rsexp next);

static rbool form_eq_p (RState* state, RReservedWord name, rsexp expr)
{
    return r_eq_p (state, reserved (state, name), r_car (expr));
}

static rbool tail_p (RState* state, rsexp next)
{
    return r_eq_p (state, reserved (state, INS_RETURN), r_car (next));
}

static rbool validate_quotation (RState* state, rsexp expr)
{
    if (!r_eq_p (state, r_length (state, expr), r_uint_to_sexp (2u))) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    return TRUE;
}

static rsexp compile_quotation (RState* state, rsexp expr, rsexp next)
{
    rsexp code;

    if (!validate_quotation (state, expr))
        return R_FAILURE;

    r_gc_scope_open (state);

    ensure_or_goto (code = emit_const (state, r_cadr (expr), next), exit);

exit:
    r_gc_scope_close_and_protect (state, code);

    return code;
}

static rbool validate_assignment (RState* state, rsexp expr)
{
    if (!r_eq_p (state, r_length (state, expr), r_uint_to_sexp (3u))) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    if (!r_symbol_p (r_cadr (expr))) {
        r_error_code (state, R_ERR_BAD_VARIABLE, r_cadr (expr), expr);
        return FALSE;
    }

    return TRUE;
}

static rsexp compile_assignment (RState* state, rsexp expr, rsexp next)
{
    rsexp var, val, code;

    if (!validate_assignment (state, expr))
        return R_FAILURE;

    var = r_cadr (expr);
    val = r_caddr (expr);

    r_gc_scope_open (state);

    ensure_or_goto (code = emit_assign (state, var, next), exit);
    ensure_or_goto (code = compile (state, val, code), exit);

exit:
    r_gc_scope_close_and_protect (state, code);

    return code;
}

static rbool validate_variable_definition (RState* state, rsexp expr)
{
    if (!r_eq_p (state, r_length (state, expr), r_uint_to_sexp (3u))) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    return TRUE;
}

static rsexp compile_variable_definition (RState* state,
                                          rsexp expr,
                                          rsexp next)
{
    rsexp code;

    if (!validate_variable_definition (state, expr))
        return R_FAILURE;

    r_gc_scope_open (state);

    ensure_or_goto (code = emit_bind (state, r_cadr (expr), next), exit);
    ensure_or_goto (code = compile (state, r_caddr (expr), code), exit);

exit:
    r_gc_scope_close_and_protect (state, code);

    return code;
}

static rbool validate_lambda_formals (RState* state,
                                      rsexp expr,
                                      rsexp formals)
{
    rsexp formal;

    /* (lambda x body) or (define (f . x) body */
    if (r_symbol_p (formals))
        return TRUE;

    /* (lambda () body) or (define (f) body */
    if (r_null_p (formals))
        return TRUE;

    if (!r_pair_p (formals)) {
        r_error_code (state, R_ERR_BAD_FORMALS, formals, expr);
        return FALSE;
    }

    while (r_pair_p (formals)) {
        formal = r_car (formals);

        if (!r_symbol_p (formal)) {
            r_error_code (state, R_ERR_BAD_FORMALS, formals, expr);
            return FALSE;
        }

        formals = r_cdr (formals);
    }

    /* (lambda (x y z) body) or (define (f x y z) body) */
    if (r_null_p (formals))
        return TRUE;

    /* (lambda (x y . z) body) or (define (f x y . z) body) */
    if (r_symbol_p (formals))
        return TRUE;

    r_error_code (state, R_ERR_BAD_FORMALS, formals, expr);
    return FALSE;
}

static rbool validate_lambda (RState* state, rsexp expr)
{
    rsexp length;
    rsize u_length;

    length = r_length (state, expr);

    if (r_failure_p (length)) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return R_FAILURE;
    }

    u_length = r_uint_from_sexp (length);

    if (u_length < 3) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return R_FAILURE;
    }

    return validate_lambda_formals (state, expr, r_cadr (expr));
}

static rsexp compile_lambda (RState* state, rsexp expr, rsexp next)
{
    rsexp formals;
    rsexp body;
    rsexp code;

    if (!validate_lambda (state, expr))
        return R_FAILURE;

    formals = r_cadr (expr);
    body = r_reverse_x (state, r_cddr (expr));

    r_gc_scope_open (state);

    ensure_or_goto (code = emit_return (state), exit);
    ensure_or_goto (code = compile_sequence (state, body, code), exit);
    ensure_or_goto (code = emit_close (state, formals, code, next), exit);

exit:
    r_gc_scope_close_and_protect (state, code);

    return code;
}

static rbool validate_procedure_formals (RState* state, rsexp expr)
{
    rsexp formals;
    rsexp name;

    if (!r_pair_p (r_cadr (expr))) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    formals = r_cdadr (expr);
    name = r_caadr (expr);

    if (!r_symbol_p (name)) {
        r_error_code (state, R_ERR_BAD_VARIABLE, name, expr);
        return FALSE;
    }

    return validate_lambda_formals (state, expr, formals);
}

static rbool validate_procedure_definition (RState* state, rsexp expr)
{
    rsexp length;

    length = r_length (state, expr), FALSE;

    if (r_failure_p (length)) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    if (r_uint_from_sexp (length) < 3) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    return validate_procedure_formals (state, expr);
}

static rsexp compile_procedure_definition (RState* state,
                                           rsexp expr,
                                           rsexp next)
{
    rsexp var, body, formals;
    rsexp bind, code;

    if (!validate_procedure_definition (state, expr))
        return R_FAILURE;

    var = r_caadr (expr);
    body = r_reverse_x (state, r_cddr (expr));
    formals = r_cdadr (expr);

    r_gc_scope_open (state);

    ensure_or_goto (bind = emit_bind (state, var, next), exit);
    ensure_or_goto (code = emit_return (state), exit);
    ensure_or_goto (code = compile_sequence (state, body, code), exit);
    ensure_or_goto (code = emit_close (state, formals, code, bind), exit);

exit:
    r_gc_scope_close_and_protect (state, code);

    return code;
}

static rsexp compile_definition (RState* state, rsexp expr, rsexp next)
{
    if (!r_pair_p (r_cdr (expr))) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return R_FAILURE;
    }

    return r_symbol_p (r_cadr (expr))
           ? compile_variable_definition (state, expr, next)
           : compile_procedure_definition (state, expr, next);
}

static rsize validate_contional (RState* state, rsexp expr)
{
    rsexp length;
    rsize u_length;

    length = r_length (state, expr);

    if (r_failure_p (length)) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return 0;
    }

    u_length = r_uint_from_sexp (length);

    if (u_length < 3 || u_length > 4) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return 0;
    }

    return u_length;
}

static rsexp compile_conditional (RState* state, rsexp expr, rsexp next)
{
    rsize length;
    rsexp then_expr, then_code;
    rsexp else_expr, else_code;
    rsexp test_expr, code;

    length = validate_contional (state, expr);

    if (length == 0)
        return R_FAILURE;

    test_expr = r_cadr (expr);
    then_expr = r_caddr (expr);
    else_expr = length == 3 ? R_UNSPECIFIED : r_cadddr (expr);

    r_gc_scope_open (state);

    ensure_or_goto (then_code = compile (state, then_expr, next), exit);
    ensure_or_goto (else_code = compile (state, else_expr, next), exit);

    ensure_or_goto (code = emit_branch (state, then_code, else_code), exit);
    ensure_or_goto (code = compile (state, test_expr, code), exit);

exit:
    r_gc_scope_close_and_protect (state, code);

    return code;
}

static rbool validate_application (RState* state, rsexp expr)
{
    if (!r_list_p (expr)) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    return TRUE;
}

static rsexp compile_application (RState* state, rsexp expr, rsexp next)
{
    rsexp proc, args, code;

    if (!validate_application (state, expr))
        return R_FAILURE;

    proc = r_car (expr);
    args = r_cdr (expr);

    r_gc_scope_open (state);

    ensure_or_goto (code = emit_apply (state), exit);
    ensure_or_goto (code = compile (state, proc, code), exit);

    while (!r_null_p (args)) {
        ensure_or_goto (code = emit_arg (state, code), exit);
        ensure_or_goto (code = compile (state, r_car (args), code), exit);
        args = r_cdr (args);
    }

    if (!tail_p (state, next))
        code = emit_frame (state, next, code);

exit:
    r_gc_scope_close_and_protect (state, code);

    return code;
}

static rbool validate_call_cc (RState* state, rsexp expr)
{
    if (!r_eq_p (state, r_length (state, expr), r_uint_to_sexp (2u))) {
        r_error_code (state, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    return TRUE;
}

static rsexp compile_call_cc (RState* state, rsexp expr, rsexp next)
{
    rsexp code;

    if (!validate_call_cc (state, expr))
        return R_FAILURE;

    r_gc_scope_open (state);

    ensure_or_goto (code = emit_apply (state), exit);
    ensure_or_goto (code = compile (state, r_cadr (expr), code), exit);
    ensure_or_goto (code = emit_arg (state, code), exit);
    ensure_or_goto (code = emit_capture_cc (state, code), exit);

    if (!tail_p (state, next))
        code = emit_frame (state, next, code);

exit:
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
        code = compile_quotation (state, expr, next);
        goto exit;
    }

    if (form_eq_p (state, KW_DEFINE, expr)) {
        code = compile_definition (state, expr, next);
        goto exit;
    }

    if (form_eq_p (state, KW_SET_X, expr)) {
        code = compile_assignment (state, expr, next);
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

    r_error_code (state, R_ERR_BAD_SYNTAX, expr);
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

rsexp r_compile_from_port (RState* state, rsexp port)
{
    rsexp reader;
    rsexp datum;
    rsexp program;

    ensure (reader = r_reader_new (state, port));

    program = R_NULL;
    datum = R_UNDEFINED;

    while (TRUE) {
        datum = r_read (reader);

        if (r_failure_p (datum))
            return R_FAILURE;

        if (r_eof_object_p (datum))
            break;

        program = r_cons (state, datum, program);
    }

    return r_compile (state, program);
}
