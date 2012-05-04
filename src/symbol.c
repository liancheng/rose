#include "context_access.h"

#include "rose/hash.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#include <assert.h>
#include <gc/gc.h>
#include <string.h>

#define QUARK_BLOCK_SIZE     1024
#define QUARK_TO_SEXP(quark) ((rsexp)((quark << 3) | R_SEXP_SYMBOL_TAG))
#define SEXP_TO_QUARK(sexp)  (sexp >> 3)

typedef rword rquark;

struct RSymbolTable {
    RHashTable* quark_ht;
    char**      quarks;
    rquark      quark_seq_id;
};

static char* gc_strdup(char const* str)
{
    char* res = GC_MALLOC_ATOMIC(strlen(str) * sizeof(char));
    strcpy(res, str);
    return res;
}

static rquark r_quark_new(char* symbol, RSymbolTable* st)
{
    rquark quark;
    char** new_quarks;

    if (0 == st->quark_seq_id % QUARK_BLOCK_SIZE) {
        new_quarks = GC_REALLOC(st->quarks,
                                st->quark_seq_id + QUARK_BLOCK_SIZE);
        memset(new_quarks + st->quark_seq_id, 0, QUARK_BLOCK_SIZE);
        st->quarks = new_quarks;
    }

    quark = st->quark_seq_id;
    st->quarks[quark] = symbol;
    r_hash_table_put(st->quark_ht, symbol, (rpointer)quark);
    ++st->quark_seq_id;

    return quark;
}

static rquark r_quark_from_string_internal(char const*   symbol,
                                           rboolean      duplicate,
                                           RSymbolTable* st)
{
    rquark quark = (rquark)r_hash_table_get(st->quark_ht, symbol);

    if (!quark) {
        char* str = duplicate ? gc_strdup(symbol) : (char*)symbol;
        quark = r_quark_new(str, st);
    }

    return quark;
}

static rquark r_quark_from_symbol(char const* symbol, RContext* context)
{
    RSymbolTable* st;

    if (!symbol)
        return 0;

    st = CONTEXT_FIELD(symbol_table, context);

    return r_quark_from_string_internal(symbol, TRUE, st);
}

static rquark r_quark_from_static_symbol(char const* symbol, RContext* context)
{
    RSymbolTable* st;

    if (!symbol)
        return 0;

    st = CONTEXT_FIELD(symbol_table, context);

    return r_quark_from_string_internal(symbol, FALSE, st);
}

static char const* r_quark_to_symbol(rquark quark, RContext* context)
{
    RSymbolTable* st = CONTEXT_FIELD(symbol_table, context);
    return quark < st->quark_seq_id ?  st->quarks[quark] : NULL;
}

ruint r_str_hash(rconstpointer str)
{
    ruint h = 5381;
    char const* p;

    for (p = (char const*)str; *p; ++p)
        h = (h << 5) + h + *p;

    return h;
}

rboolean r_str_equal(rconstpointer lhs, rconstpointer rhs)
{
    return 0 == strcmp((char const*)lhs, (char const*)rhs);
}

RSymbolTable* r_symbol_table_new()
{
    RSymbolTable* res;
    rsize size;

    res  = GC_NEW(RSymbolTable);
    size = sizeof(rquark) * QUARK_BLOCK_SIZE;

    res->quark_ht     = r_hash_table_new(r_str_hash, r_str_equal);
    res->quarks       = GC_MALLOC_ATOMIC(size);
    res->quark_seq_id = 1;

    memset(res->quarks, 0, size);

    return res;
}

rsexp r_symbol_new(char const* symbol, RContext* context)
{
    rquark quark = r_quark_from_symbol(symbol, context);
    return QUARK_TO_SEXP(quark);
}

rsexp r_static_symbol(char const* symbol, RContext* context)
{
    rquark quark = r_quark_from_static_symbol(symbol, context);
    return QUARK_TO_SEXP(quark);
}

char const* r_symbol_name(rsexp sexp, RContext* context)
{
    assert(R_SYMBOL_P(sexp));
    return r_quark_to_symbol(SEXP_TO_QUARK(sexp), context);
}

rsexp r_keywords_init(RContext* context)
{
    static char* keywords[] = {
        "and",      "=>",
        "begin",    "case",
        "cond",     "define",
        "delay",    "do",
        "else",     "if",
        "lambda",   "let",
        "let*",     "letrec",
        "or",       "quasiquote",
        "quote",    "set!",
        "unquote",  "unquote-splicing",
    };

    rsexp vec;
    rsexp symbol;
    rsize i;

    vec = r_vector_new(N_KEYWORD);

    for (i = 0; i < N_KEYWORD; ++i) {
        symbol = r_static_symbol(keywords[i], context);
        r_vector_set_x(vec, i, symbol);
    }

    return vec;
}

rsexp r_keyword(ruint name, RContext* context)
{
    assert(name < N_KEYWORD);
    rsexp vec = (rsexp)CONTEXT_FIELD(keywords, context);
    return r_vector_ref(vec, name);
}

void r_write_symbol(FILE* output, rsexp sexp, RContext* context)
{
    fprintf(output, "%s", r_symbol_name(sexp, context));
}
