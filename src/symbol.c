#include "opaque.h"
#include "scanner.h"

#include "rose/hash.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#include <assert.h>
#include <string.h>

#define QUARK_BLOCK_SIZE     1024
#define QUARK_TO_SEXP(quark) ((rsexp) ((quark << 3) | R_SEXP_SYMBOL_TAG))
#define SEXP_TO_QUARK(obj)   (obj >> 3)

typedef rword rquark;

struct RSymbolTable {
    RHashTable* quark_ht;
    char**      quarks;
    rquark      quark_seq_id;
};

static inline rquark quark_new (char* symbol, RSymbolTable* st)
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

static inline rquark string_to_quark_internal (char const*   symbol,
                                               rboolean      duplicate,
                                               RSymbolTable* st)
{
    rquark quark = (rquark) r_hash_table_get (st->quark_ht, symbol);

    if (!quark) {
        char* str = duplicate ? r_strdup (symbol) : (char*)symbol;
        quark = quark_new (str, st);
    }

    return quark;
}

static inline RSymbolTable* get_symbol_table (rsexp context)
{
    return r_opaque_get (r_context_get (context, CTX_SYMBOL_TABLE));
}

static inline rquark quark_from_symbol (char const* symbol, rsexp context)
{
    if (!symbol)
        return 0;

    return string_to_quark_internal (symbol, TRUE, get_symbol_table (context));
}

static inline rquark static_symbol_to_quark (char const* symbol, rsexp context)
{
    if (!symbol)
        return 0;

    return string_to_quark_internal (symbol, FALSE, get_symbol_table (context));
}

static inline char const* r_quark_to_symbol (rquark quark, rsexp context)
{
    RSymbolTable* st = get_symbol_table (context);
    return quark < st->quark_seq_id ? st->quarks [quark] : NULL;
}

static inline void symbol_table_finalize (rpointer obj, rpointer client_data)
{
    r_hash_table_free (((RSymbolTable*) obj)->quark_ht);
}

ruint r_str_hash (rconstpointer str)
{
    ruint h = 5381;
    char const* p;

    for (p = (char const*) str; *p; ++p)
        h = (h << 5) + h + *p;

    return h;
}

rboolean r_str_equal (rconstpointer lhs, rconstpointer rhs)
{
    return 0 == strcmp ((char const*) lhs, (char const*) rhs);
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

rsexp r_symbol_new (char const* symbol, rsexp context)
{
    rquark quark = quark_from_symbol (symbol, context);
    return QUARK_TO_SEXP (quark);
}

rsexp r_symbol_new_static (char const* symbol, rsexp context)
{
    rquark quark = static_symbol_to_quark (symbol, context);
    return QUARK_TO_SEXP (quark);
}

char const* r_symbol_name (rsexp obj, rsexp context)
{
    assert (r_symbol_p (obj));
    return r_quark_to_symbol (SEXP_TO_QUARK (obj), context);
}

rsexp r_read_symbol (rsexp port, rsexp context)
{
    RETURN_ON_EOF_OR_FAIL (port, context);

    if (TKN_IDENTIFIER != r_scanner_peek_id (port, context))
        return R_SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token (port, context);
    rsexp res = r_symbol_new ((char*) (t->text), context);
    r_scanner_free_token (t);

    return res;
}

void r_write_symbol (rsexp port, rsexp obj, rsexp context)
{
    r_port_puts (port, r_symbol_name (obj, context));
}

void r_display_symbol (rsexp port, rsexp obj, rsexp context)
{
    r_write_symbol (port, obj, context);
}
