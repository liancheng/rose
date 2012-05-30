#include "scanner.h"

#include "rose/context.h"
#include "rose/error.h"
#include "rose/port.h"
#include "rose/read.h"

#include <gc/gc.h>

RReaderState* r_reader_new (rsexp context)
{
    RReaderState* reader;

    reader = GC_NEW (RReaderState);
    memset (reader, 0, sizeof (RReaderState));

    reader->context    = context;
    reader->input_port = r_current_input_port (context);
    reader->tree       = R_SEXP_UNDEFINED;
    reader->error_type = R_SEXP_UNDEFINED;
    reader->last_error = R_SEXP_UNDEFINED;
    reader->scanner    = r_scanner_new ();

    return reader;
}
