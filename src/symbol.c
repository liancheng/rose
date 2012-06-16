#include "detail/context.h"
#include "detail/hash.h"
#include "detail/sexp.h"
#include "rose/port.h"
#include "rose/symbol.h"

#include <gc/gc.h>
#include <assert.h>
#include <string.h>

#define QUARK_BLOCK_SIZE     1024
#define QUARK_TO_SEXP(quark) ((rsexp) ((quark << R_TC5_BITS) | R_SYMBOL_TAG))
#define SEXP_TO_QUARK(obj)   (obj >> R_TC5_BITS)

typedef rword rquark;

struct RSymbolTable {
    RHashTable* quark_ht;
    char**      quarks;
    rquark      quark_seq_id;
};

static rquark quark_new (char* symbol, RSymbolTable* st)
{
    rquark quark;
    char** new_quarks;

    if (0 == st->quark_seq_id % QUARK_BLOCK_SIZE) {
        new_quarks = GC_REALLOC (st->quarks,
                                 st->quark_seq_id + QUARK_BLOCK_SIZE);
        memset (new_quarks + st->quark_seq_id, 0, QUARK_BLOCK_SIZE);
        st->quarks = new_quarks;
    }

    quark = st->quark_seq_id;
    st->quarks[quark] = symbol;
    r_hash_table_put (st->quark_ht, symbol, (rpointer)quark);
    ++st->quark_seq_id;

    return quark;
}

static rquark string_to_quark_internal (char const*   symbol,
                                        rboolean      duplicate,
                                        RSymbolTable* st)
{
    rquark quark = (rquark) r_hash_table_get (st->quark_ht, symbol);

    if (!quark) {
        char* str = duplicate ? GC_STRDUP (symbol) : (char*)symbol;
        quark = quark_new (str, st);
    }

    return quark;
}

static rquark quark_from_symbol (char const* symbol, RContext* context)
{
    if (!symbol)
        return 0;

    return string_to_quark_internal (symbol, TRUE, context->symbol_table);
}

static rquark static_symbol_to_quark (char const* symbol,
                                      RContext*   context)
{
    if (!symbol)
        return 0;

    return string_to_quark_internal (symbol, FALSE, context->symbol_table);
}

static char const* r_quark_to_symbol (rquark quark, RContext* context)
{
    RSymbolTable* st = context->symbol_table;
    return quark < st->quark_seq_id ? st->quarks [quark] : NULL;
}

static void symbol_table_finalize (rpointer obj, rpointer client_data)
{
    r_hash_table_free (((RSymbolTable*) obj)->quark_ht);
}

static ruint r_str_hash (rconstpointer str)
{
    ruint h = 5381;
    char const* p;

    for (p = (char const*) str; *p; ++p)
        h = (h << 5) + h + *p;

    return h;
}

static rboolean r_str_equal (rconstpointer lhs, rconstpointer rhs)
{
    return 0 == strcmp ((char const*) lhs, (char const*) rhs);
}

static void r_symbol_write (rsexp port, rsexp obj, RContext* context)
{
    r_port_puts (port, r_symbol_name (obj, context));
}

RSymbolTable* r_symbol_table_new ()
{
    RSymbolTable* res;
    rsize size;

    res  = GC_NEW (RSymbolTable);
    size = sizeof (rquark) * QUARK_BLOCK_SIZE;

    res->quark_ht     = r_hash_table_new (r_str_hash, r_str_equal);
    res->quarks       = GC_MALLOC_ATOMIC (size);
    res->quark_seq_id = 1;

    memset (res->quarks, 0, size);
    GC_REGISTER_FINALIZER (res, symbol_table_finalize, NULL, NULL, NULL);

    return res;
}

rsexp r_symbol_new (char const* symbol, RContext* context)
{
    rquark quark = quark_from_symbol (symbol, context);
    return QUARK_TO_SEXP (quark);
}

rsexp r_symbol_new_static (char const* symbol, RContext* context)
{
    rquark quark = static_symbol_to_quark (symbol, context);
    return QUARK_TO_SEXP (quark);
}

char const* r_symbol_name (rsexp obj, RContext* context)
{
    assert (r_symbol_p (obj));
    return r_quark_to_symbol (SEXP_TO_QUARK (obj), context);
}

void r_register_symbol_type (RContext* context)
{
    RType* type = GC_NEW (RType);

    type->cell_size  = 0;
    type->name       = "symbol";
    type->write_fn   = r_symbol_write;
    type->display_fn = r_symbol_write;

    context->tc5_types [R_SYMBOL_TAG >> R_TC3_BITS] = type;
}
