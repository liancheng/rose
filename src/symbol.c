#include "detail/state.h"
#include "detail/hash.h"
#include "detail/sexp.h"
#include "rose/memory.h"
#include "rose/port.h"
#include "rose/symbol.h"

#include <assert.h>
#include <string.h>

#define QUARK_BLOCK_SIZE     1024
#define QUARK_TO_SEXP(quark) ((rsexp) ((quark << R_TAG_BITS) | R_SYMBOL_TAG))
#define QUARK_FROM_SEXP(obj) (obj >> R_TAG_BITS)

typedef rword rquark;

struct RSymbolTable {
    RHashTable* quark_ht;
    rcstring*   quarks;
    rquark      quark_seq_id;
};

static rquark quark_new (RState* state, rcstring symbol, RSymbolTable* st)
{
    rquark quark;
    rcstring* new_quarks;

    if (0 == st->quark_seq_id % QUARK_BLOCK_SIZE) {
        new_quarks = r_realloc (state,
                                st->quarks,
                                st->quark_seq_id + QUARK_BLOCK_SIZE);
        memset (new_quarks + st->quark_seq_id, 0, QUARK_BLOCK_SIZE);
        st->quarks = new_quarks;
    }

    quark = st->quark_seq_id++;
    st->quarks [quark] = symbol;
    r_hash_table_put (st->quark_ht, symbol, (rpointer) quark);

    return quark;
}

static rquark string_to_quark_internal (RState*        state,
                                        rconstcstring  symbol,
                                        rbool          duplicate,
                                        RSymbolTable*  st)
{
    rquark quark = r_cast (rquark, r_hash_table_get (st->quark_ht, symbol));

    if (!quark) {
        rcstring str = duplicate
                     ? r_strdup (state, symbol)
                     : r_cast (rcstring, symbol);

        quark = quark_new (state, str, st);
    }

    return quark;
}

static rquark quark_from_symbol (RState* state, rconstcstring symbol)
{
    if (!symbol)
        return 0;

    return string_to_quark_internal (state, symbol, TRUE, state->symbol_table);
}

static rquark static_symbol_to_quark (RState* state, rconstcstring symbol)
{
    if (!symbol)
        return 0;

    return string_to_quark_internal (state, symbol, FALSE, state->symbol_table);
}

static rconstcstring quark_to_symbol (RState* state, rquark quark)
{
    RSymbolTable* st = state->symbol_table;
    return quark < st->quark_seq_id ? st->quarks [quark] : NULL;
}

static ruint str_hash (rconstpointer str)
{
    ruint h = 5381;
    rconstcstring p;

    for (p = r_cast (rconstcstring, str); *p; ++p)
        h = (h << 5) + h + *p;

    return h;
}

static rbool str_equal (rconstpointer lhs, rconstpointer rhs)
{
    return 0 == strcmp (r_cast (rconstcstring, lhs),
                        r_cast (rconstcstring, rhs));
}

static void write_symbol (RState* state, rsexp port, rsexp obj)
{
    r_port_puts (port, r_symbol_name (r_port_get_state (port), obj));
}

RSymbolTable* r_symbol_table_new (RState* state)
{
    RSymbolTable* res;
    rsize size;

    res  = r_new0 (state, RSymbolTable);
    size = sizeof (rquark) * QUARK_BLOCK_SIZE;

    res->quark_ht     = r_hash_table_new (str_hash, str_equal);
    res->quarks       = r_calloc (state, 1u, size);
    res->quark_seq_id = 1;

    return res;
}

void r_symbol_table_free (RState* state, RSymbolTable* symbol_table)
{
    r_free (state, symbol_table->quarks);
    r_hash_table_free (symbol_table->quark_ht);
}

rsexp r_symbol_new (RState* state, rconstcstring symbol)
{
    rquark quark = quark_from_symbol (state, symbol);
    return QUARK_TO_SEXP (quark);
}

rsexp r_symbol_new_static (RState* state, rconstcstring symbol)
{
    rquark quark = static_symbol_to_quark (state, symbol);
    return QUARK_TO_SEXP (quark);
}

rconstcstring r_symbol_name (RState* state, rsexp obj)
{
    assert (r_symbol_p (obj));
    return quark_to_symbol (state, QUARK_FROM_SEXP (obj));
}

RTypeInfo* init_symbol_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = 0;
    type->name         = "symbol";
    type->ops.write    = write_symbol;
    type->ops.display  = write_symbol;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = NULL;
    type->ops.mark     = NULL;
    type->ops.destruct = NULL;

    return type;
}
