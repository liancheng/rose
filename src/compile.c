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

static rsexp compile (RState* r, rsexp expr, rsexp next);

static rsexp compile_sequence (RState* r, rsexp seq, rsexp next);

static rbool form_eq_p (RState* r, rsexp name, rsexp expr)
{
    return r_eq_p (r, name, r_car (expr));
}

static rbool tail_p (RState* r, rsexp next)
{
    return r_eq_p (r, r->i.return_, r_car (next));
}

static rbool validate_quotation (RState* r, rsexp expr)
{
    if (!r_eq_p (r, r_length (r, expr), r_uint_to_sexp (2u))) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    return TRUE;
}

static rsexp compile_quotation (RState* r, rsexp expr, rsexp next)
{
    rsexp code;

    if (!validate_quotation (r, expr))
        return R_FAILURE;

    r_gc_scope_open (r);

    ensure_or_goto (code = emit_constant (r, r_cadr (expr), next), exit);

exit:
    r_gc_scope_close_and_protect (r, code);

    return code;
}

static rbool validate_assignment (RState* r, rsexp expr)
{
    if (!r_eq_p (r, r_length (r, expr), r_uint_to_sexp (3u))) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    if (!r_symbol_p (r_cadr (expr))) {
        r_error_code (r, R_ERR_BAD_VARIABLE, r_cadr (expr), expr);
        return FALSE;
    }

    return TRUE;
}

static rsexp compile_assignment (RState* r, rsexp expr, rsexp next)
{
    rsexp var, val, code;

    if (!validate_assignment (r, expr))
        return R_FAILURE;

    var = r_cadr (expr);
    val = r_caddr (expr);

    r_gc_scope_open (r);

    ensure_or_goto (code = emit_assign (r, var, next), exit);
    ensure_or_goto (code = compile (r, val, code), exit);

exit:
    r_gc_scope_close_and_protect (r, code);

    return code;
}

static rbool validate_variable_definition (RState* r, rsexp expr)
{
    if (!r_eq_p (r, r_length (r, expr), r_uint_to_sexp (3u))) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    return TRUE;
}

static rsexp compile_variable_definition (RState* r,
                                          rsexp expr,
                                          rsexp next)
{
    rsexp code;

    if (!validate_variable_definition (r, expr))
        return R_FAILURE;

    r_gc_scope_open (r);

    ensure_or_goto (code = emit_bind (r, r_cadr (expr), next), exit);
    ensure_or_goto (code = compile (r, r_caddr (expr), code), exit);

exit:
    r_gc_scope_close_and_protect (r, code);

    return code;
}

static rbool validate_lambda_formals (RState* r,
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
        r_error_code (r, R_ERR_BAD_FORMALS, formals, expr);
        return FALSE;
    }

    while (r_pair_p (formals)) {
        formal = r_car (formals);

        if (!r_symbol_p (formal)) {
            r_error_code (r, R_ERR_BAD_FORMALS, formals, expr);
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

    r_error_code (r, R_ERR_BAD_FORMALS, formals, expr);
    return FALSE;
}

static rbool validate_lambda (RState* r, rsexp expr)
{
    rsexp length;
    rsize u_length;

    length = r_length (r, expr);

    if (r_failure_p (length)) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return R_FAILURE;
    }

    u_length = r_uint_from_sexp (length);

    if (u_length < 3) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return R_FAILURE;
    }

    return validate_lambda_formals (r, expr, r_cadr (expr));
}

static rsexp compile_lambda (RState* r, rsexp expr, rsexp next)
{
    rsexp formals;
    rsexp body;
    rsexp code;

    if (!validate_lambda (r, expr))
        return R_FAILURE;

    formals = r_cadr (expr);
    body = r_reverse_x (r, r_cddr (expr));

    r_gc_scope_open (r);

    ensure_or_goto (code = emit_return (r), exit);
    ensure_or_goto (code = compile_sequence (r, body, code), exit);
    ensure_or_goto (code = emit_close (r, formals, code, next), exit);

exit:
    r_gc_scope_close_and_protect (r, code);

    return code;
}

static rbool validate_procedure_formals (RState* r, rsexp expr)
{
    rsexp formals;
    rsexp name;

    if (!r_pair_p (r_cadr (expr))) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    formals = r_cdadr (expr);
    name = r_caadr (expr);

    if (!r_symbol_p (name)) {
        r_error_code (r, R_ERR_BAD_VARIABLE, name, expr);
        return FALSE;
    }

    return validate_lambda_formals (r, expr, formals);
}

static rbool validate_procedure_definition (RState* r, rsexp expr)
{
    rsexp length;

    length = r_length (r, expr), FALSE;

    if (r_failure_p (length)) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    if (r_uint_from_sexp (length) < 3) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    return validate_procedure_formals (r, expr);
}

static rsexp compile_procedure_definition (RState* r,
                                           rsexp expr,
                                           rsexp next)
{
    rsexp var, body, formals;
    rsexp bind, code;

    if (!validate_procedure_definition (r, expr))
        return R_FAILURE;

    var = r_caadr (expr);
    body = r_reverse_x (r, r_cddr (expr));
    formals = r_cdadr (expr);

    r_gc_scope_open (r);

    ensure_or_goto (bind = emit_bind (r, var, next), exit);
    ensure_or_goto (code = emit_return (r), exit);
    ensure_or_goto (code = compile_sequence (r, body, code), exit);
    ensure_or_goto (code = emit_close (r, formals, code, bind), exit);

exit:
    r_gc_scope_close_and_protect (r, code);

    return code;
}

static rsexp compile_definition (RState* r, rsexp expr, rsexp next)
{
    if (!r_pair_p (r_cdr (expr))) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return R_FAILURE;
    }

    return r_symbol_p (r_cadr (expr))
           ? compile_variable_definition (r, expr, next)
           : compile_procedure_definition (r, expr, next);
}

static rsize validate_contional (RState* r, rsexp expr)
{
    rsexp length;
    rsize u_length;

    length = r_length (r, expr);

    if (r_failure_p (length)) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return 0;
    }

    u_length = r_uint_from_sexp (length);

    if (u_length < 3 || u_length > 4) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return 0;
    }

    return u_length;
}

static rsexp compile_conditional (RState* r, rsexp expr, rsexp next)
{
    rsize length;
    rsexp then_expr, then_code;
    rsexp else_expr, else_code;
    rsexp test_expr, code;

    length = validate_contional (r, expr);

    if (length == 0)
        return R_FAILURE;

    test_expr = r_cadr (expr);
    then_expr = r_caddr (expr);
    else_expr = length == 3 ? R_UNSPECIFIED : r_cadddr (expr);

    r_gc_scope_open (r);

    ensure_or_goto (then_code = compile (r, then_expr, next), exit);
    ensure_or_goto (else_code = compile (r, else_expr, next), exit);

    ensure_or_goto (code = emit_branch (r, then_code, else_code), exit);
    ensure_or_goto (code = compile (r, test_expr, code), exit);

exit:
    r_gc_scope_close_and_protect (r, code);

    return code;
}

static rbool validate_application (RState* r, rsexp expr)
{
    if (!r_list_p (expr)) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    return TRUE;
}

static rsexp compile_application (RState* r, rsexp expr, rsexp next)
{
    rsexp proc, args, code;

    if (!validate_application (r, expr))
        return R_FAILURE;

    proc = r_car (expr);
    args = r_cdr (expr);

    r_gc_scope_open (r);

    ensure_or_goto (code = emit_apply (r), exit);
    ensure_or_goto (code = compile (r, proc, code), exit);

    while (!r_null_p (args)) {
        ensure_or_goto (code = emit_arg (r, code), exit);
        ensure_or_goto (code = compile (r, r_car (args), code), exit);
        args = r_cdr (args);
    }

    if (!tail_p (r, next))
        code = emit_frame (r, next, code);

exit:
    r_gc_scope_close_and_protect (r, code);

    return code;
}

static rbool validate_call_cc (RState* r, rsexp expr)
{
    if (!r_eq_p (r, r_length (r, expr), r_uint_to_sexp (2u))) {
        r_error_code (r, R_ERR_BAD_SYNTAX, expr);
        return FALSE;
    }

    return TRUE;
}

static rsexp compile_call_cc (RState* r, rsexp expr, rsexp next)
{
    rsexp code;

    if (!validate_call_cc (r, expr))
        return R_FAILURE;

    r_gc_scope_open (r);

    ensure_or_goto (code = emit_apply (r), exit);
    ensure_or_goto (code = compile (r, r_cadr (expr), code), exit);
    ensure_or_goto (code = emit_arg (r, code), exit);
    ensure_or_goto (code = emit_capture_cc (r, code), exit);

    if (!tail_p (r, next))
        code = emit_frame (r, next, code);

exit:
    r_gc_scope_close_and_protect (r, code);

    return code;
}

static rsexp compile (RState* r, rsexp expr, rsexp next)
{
    rsexp code;

    r_gc_scope_open (r);

    if (r_symbol_p (expr)) {
        code = emit_refer (r, expr, next);
        goto exit;
    }

    if (!r_pair_p (expr)) {
        code = emit_constant (r, expr, next);
        goto exit;
    }

    if (form_eq_p (r, r->kw.quote, expr)) {
        code = compile_quotation (r, expr, next);
        goto exit;
    }

    if (form_eq_p (r, r->kw.define, expr)) {
        code = compile_definition (r, expr, next);
        goto exit;
    }

    if (form_eq_p (r, r->kw.set_x, expr)) {
        code = compile_assignment (r, expr, next);
        goto exit;
    }

    if (form_eq_p (r, r->kw.if_, expr)) {
        code = compile_conditional (r, expr, next);
        goto exit;
    }

    if (form_eq_p (r, r->kw.lambda, expr)) {
        code = compile_lambda (r, expr, next);
        goto exit;
    }

    if (form_eq_p (r, r->kw.call_cc, expr)) {
        code = compile_call_cc (r, expr, next);
        goto exit;
    }

    code = compile_application (r, expr, next);
    goto exit;

    r_error_code (r, R_ERR_BAD_SYNTAX, expr);
    code = R_FAILURE;

exit:
    r_gc_scope_close_and_protect (r, code);

    return code;
}

static rsexp compile_sequence (RState* r, rsexp seq, rsexp next)
{
    rsexp expr;

    r_gc_scope_open (r);

    while (!r_null_p (seq) && !r_failure_p (next)) {
        expr = r_car (seq);
        seq = r_cdr (seq);
        next = compile (r, expr, next);
    }

    r_gc_scope_close_and_protect (r, next);

    return next;
}

rsexp r_compile (RState* r, rsexp program)
{
    rsexp halt = emit_halt (r);

    return r_failure_p (halt)
           ? R_FAILURE
           : compile_sequence (r, program, halt);
}

rsexp r_compile_from_port (RState* r, rsexp port)
{
    rsexp reader;
    rsexp datum;
    rsexp program;

    ensure (reader = r_reader_new (r, port));

    program = R_NULL;
    datum = R_UNDEFINED;

    while (TRUE) {
        datum = r_read (reader);

        if (r_failure_p (datum))
            return R_FAILURE;

        if (r_eof_object_p (datum))
            break;

        ensure (program = r_cons (r, datum, program));
    }

    return r_compile (r, program);
}
