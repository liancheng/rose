#include "detail/reader.h"
#include "rose/port.h"
#include "rose/reader.h"

#include <gc/gc.h>

extern int rose_yyparse (RReaderState* state);

static void r_lexer_finalize (rpointer obj, rpointer client_data)
{
    QUEX_NAME (destruct) ((RLexer*) obj);
}

static RLexer* lexer_new ()
{
    RLexer* lexer;

    lexer = GC_NEW (RLexer);
    QUEX_NAME (construct_memory) (lexer, NULL, 0, NULL, NULL, FALSE);
    GC_REGISTER_FINALIZER (lexer, r_lexer_finalize, NULL, NULL, NULL);

    return lexer;
}

RReaderState* r_reader_new (RContext* context)
{
    RReaderState* reader;

    reader = GC_NEW (RReaderState);
    memset (reader, 0, sizeof (RReaderState));

    reader->context    = context;
    reader->input_port = r_current_input_port (context);
    reader->tree       = R_NULL;
    reader->error_type = R_UNDEFINED;
    reader->last_error = R_UNDEFINED;
    reader->lexer      = lexer_new ();

    return reader;
}

RReaderState* r_reader_from_file (char const* filename, RContext* context)
{
    return r_reader_from_port (r_open_input_file (filename, context), context);
}

RReaderState* r_reader_from_string (char const* string, RContext* context)
{
    return r_reader_from_port (r_open_input_string (string, context), context);
}

RReaderState* r_reader_from_port (rsexp port, RContext* context)
{
    RReaderState* reader = r_reader_new (context);
    reader->input_port = port;
    return reader;
}

rsexp r_read (RReaderState* reader)
{
    rose_yyparse (reader);
    return reader->tree;
}

rboolean r_reader_error_p (RReaderState* reader)
{
    return !r_undefined_p (reader->error_type);
}

rsexp r_reader_last_error (RReaderState* reader)
{
    return reader->last_error;
}

void r_reader_clear_error (RReaderState* reader)
{
    reader->error_type = R_UNDEFINED;
    reader->last_error = R_UNDEFINED;
}
