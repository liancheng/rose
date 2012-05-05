#include "keyword.h"
#include "scanner.h"

#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/read.h"
#include "rose/types.h"
#include "rose/write.h"

#include <assert.h>
#include <stdarg.h>

#define PAIR_TO_SEXP(p) (((rsexp)(p) << 2) | R_SEXP_PAIR_TAG)
#define SEXP_TO_PAIR(s) ((RPair*)((s) >> 2))

rsexp r_cons(rsexp car, rsexp cdr)
{
    RPair* pair;

    pair = GC_NEW(RPair);
    pair->car = car;
    pair->cdr = cdr;

    return PAIR_TO_SEXP(pair);
}

rsexp r_car(rsexp sexp)
{
    assert(r_pair_p(sexp));
    return SEXP_TO_PAIR(sexp)->car;
}

rsexp r_cdr(rsexp sexp)
{
    assert(r_pair_p(sexp));
    return SEXP_TO_PAIR(sexp)->cdr;
}

rsexp r_set_car(rsexp pair, rsexp sexp)
{
    assert(r_pair_p(pair));
    SEXP_TO_PAIR(pair)->car = sexp;
    return R_SEXP_UNSPECIFIED;
}

rsexp r_set_cdr(rsexp pair, rsexp sexp)
{
    assert(r_pair_p(pair));
    SEXP_TO_PAIR(pair)->cdr = sexp;
    return R_SEXP_UNSPECIFIED;
}

static rsexp r_reverse_internal(rsexp list, rsexp acc)
{
    return r_null_p(list)
           ? acc
           : r_reverse_internal(r_cdr(list),
                                r_cons(r_car(list), acc));
}

rsexp r_reverse(rsexp list)
{
    assert(r_null_p(list) || r_pair_p(list));
    return r_reverse_internal(list, R_SEXP_NULL);
}

rsexp r_append(rsexp list, rsexp sexp)
{
    rsexp tail;

    if (r_null_p(list))
        return sexp;

    assert(r_pair_p(list));

    for (tail = list; r_pair_p(r_cdr(tail)); )
        tail = r_cdr(tail);

    r_set_cdr(tail, sexp);

    return list;
}

rboolean r_list_p(rsexp sexp)
{
    return r_null_p(sexp) || (r_pair_p(sexp) && r_list_p(r_cdr(sexp)));
}

rsexp r_list(rsize count, ...)
{
    va_list args;
    rsize i;

    va_start(args, count);

    rsexp res = R_SEXP_NULL;
    for (i = 0; i < count; ++i)
        res = r_cons(va_arg(args, rsexp), res);

    va_end(args);

    return r_reverse(res);
}

rsize r_length(rsexp list)
{
    return r_null_p(list) ? 0 : 1 + r_length(r_cdr(list));
}

static void r_write_pair_cdr(rsexp output, rsexp sexp, rsexp context)
{
    if (r_pair_p(sexp)) {
        r_pprintf(output, " ");
        r_write(output, r_car(sexp), context);
        r_write_pair_cdr(output, r_cdr(sexp), context);
    }
    else if (!r_null_p(sexp)) {
        r_pprintf(output, " . ");
        r_write(output, sexp, context);
    }
}

void r_write_pair(rsexp output, rsexp sexp, rsexp context)
{
    assert(r_pair_p(sexp));

    r_pprintf(output, "(");
    r_write(output, r_car(sexp), context);
    r_write_pair_cdr(output, r_cdr(sexp), context);
    r_pprintf(output, ")");
}

void r_write_null(rsexp output, rsexp sexp, rsexp context)
{
    r_pprintf(output, "()");
}

rboolean r_pair_equal_p(rsexp lhs, rsexp rhs)
{
    return r_pair_p(lhs) &&
           r_pair_p(rhs) &&
           r_equal_p(r_car(lhs), r_car(rhs)) &&
           r_equal_p(r_cdr(lhs), r_cdr(rhs));
}

static rsexp read_abbrev(rsexp input, rsexp context)
{
    rsexp res;
    rsexp sexp;
    rtokenid id;

    RETURN_ON_EOF_OR_FAIL(input, context);

    id  = r_scanner_peek_token_id(input, context);
    res = R_SEXP_UNSPECIFIED;

    switch (id) {
        case TKN_QUOTE:    res = r_keyword(QUOTE,            context); break;
        case TKN_BACKTICK: res = r_keyword(QUASIQUOTE,       context); break;
        case TKN_COMMA:    res = r_keyword(UNQUOTE,          context); break;
        case TKN_COMMA_AT: res = r_keyword(UNQUOTE_SPLICING, context); break;
    }

    if (r_unspecified_p(res))
        return res;

    r_scanner_consume_token(input, context);
    sexp = r_read(input, context);

    if (!r_unspecified_p(res))
        res = r_list(2, res, sexp);

    return res;
}

rsexp r_read_list(rsexp input, rsexp context)
{
    rsexp res;
    rsexp sexp;
    rsexp last;

    // Handle abbreviations (quote, unquote, unquote-splicing & quasiquote).
    res = read_abbrev(input, context);

    if (!r_unspecified_p(res))
        return res;

    // Consume the `('.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_LP == r_scanner_peek_token_id(input, context))
        r_scanner_consume_token(input, context);
    else
        return R_SEXP_UNSPECIFIED;

    // Initialize the list to '().  Read data until `.' or `)', cons the list
    // in reverse order and then revert it.
    for (res = R_SEXP_NULL;
         !r_unspecified_p(sexp = r_read(input, context));
         res = r_cons(sexp, res))
        ;

    res = r_reverse(res);

    // Handle dotted improper list like (a b c . d).
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_DOT == r_scanner_peek_token_id(input, context)) {
        r_scanner_consume_token(input, context);

        last = r_read(input, context);
        if (r_unspecified_p(last))
            return R_SEXP_UNSPECIFIED;

        r_append(res, last);
    }

    // Consume the `)'.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_RP == r_scanner_peek_token_id(input, context))
        r_scanner_consume_token(input, context);
    else
        return R_SEXP_UNSPECIFIED;

    return res;
}
