#include "rose/context.h"
#include "rose/error.h"
#include "rose/port.h"
#include "rose/read.h"

#include <gc/gc.h>

static void lexer_finalize (rpointer obj, rpointer client_data)
{
    QUEX_NAME (destruct) ((RLexer*) obj);
}

static RLexer* lexer_new ()
{
    RLexer* lexer;
    
    lexer = GC_NEW (RLexer);
    QUEX_NAME (construct_memory) (lexer, NULL, 0, NULL, NULL, FALSE);
    GC_REGISTER_FINALIZER (lexer, lexer_finalize, NULL, NULL, NULL);

    return lexer;
}

RReaderState* r_reader_new (rsexp context)
{
    RReaderState* reader;

    reader = GC_NEW (RReaderState);
    memset (reader, 0, sizeof (RReaderState));

    reader->context    = context;
    reader->input_port = r_current_input_port (context);
    reader->tree       = R_SEXP_NULL;
    reader->error_type = R_SEXP_UNDEFINED;
    reader->last_error = R_SEXP_UNDEFINED;
    reader->lexer      = lexer_new ();

    return reader;
}
