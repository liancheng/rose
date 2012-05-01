#include "context_access.h"
#include "rose/symbol.h"

#include <assert.h>
#include <gc/gc.h>
#include <string.h>

#define QUARK_INITIAL_COUNT     128
#define QUARK_BLOCK_SIZE        1024
#define SEXP_FROM_QUARK(quark)  ((rsexp)((quark << 3) | SEXP_SYMBOL_TAG))
#define SEXP_TO_QUARK(sexp)     (sexp >> 3)

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
        char* str = duplicate ? gc_strdup(symbol) : symbol;
        quark = r_quark_new(str, st);
    }

    return quark;
}

static rquark r_quark_from_symbol(char const* symbol, RContext* context)
{
    RSymbolTable* st;

    if (!symbol)
        return 0;

    st = CONTEXT_FIELD(RSymbolTable*, symbol_table, context);

    return r_quark_from_string_internal(symbol, TRUE, st);
}

static rquark r_quark_from_static_symbol(char const* symbol, RContext* context)
{
    RSymbolTable* st;

    if (!symbol)
        return 0;

    st = CONTEXT_FIELD(RSymbolTable*, symbol_table, context);

    return r_quark_from_string_internal(symbol, FALSE, st);
}

static char const* r_quark_to_symbol(rquark quark, RContext* context)
{
    RSymbolTable* st = CONTEXT_FIELD(RSymbolTable*, symbol_table, context);
    return quark < st->quark_seq_id ?  st->quarks[quark] : NULL;
}

ruint r_str_hash(rconstpointer str)
{
    ruint h = 5381;

    for (char const* p = (char const*)str; *p; ++p)
        h = (h << 5) + h + *p;

    return h;
}

rboolean r_str_equal(rconstpointer lhs, rconstpointer rhs)
{
    return 0 == strcmp((char const*)lhs, (char const*)rhs);
}

RSymbolTable* r_symbol_table_new()
{
    RSymbolTable* res = GC_NEW(RSymbolTable);

    res->quark_ht     = r_hash_table_new(r_str_hash, r_str_equal);
    res->quarks       = GC_MALLOC_ATOMIC(sizeof(rquark) * QUARK_INITIAL_COUNT);
    res->quark_seq_id = 1;

    memset(res->quarks, 0, sizeof(rquark) * QUARK_INITIAL_COUNT);

    return res;
}

rsexp sexp_from_symbol(char const* symbol, RContext* context)
{
    rquark quark = r_quark_from_symbol(symbol, context);
    return SEXP_FROM_QUARK(quark);
}

rsexp sexp_from_static_symbol(char const* symbol, RContext* context)
{
    rquark quark = r_quark_from_static_symbol(symbol, context);
    return SEXP_FROM_QUARK(quark);
}

char const* sexp_to_symbol(rsexp sexp, RContext* context)
{
    assert(SEXP_SYMBOL_P(sexp));
    return r_quark_to_symbol(SEXP_TO_QUARK(sexp), context);
}
