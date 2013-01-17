#include "detail/primitive.h"
#include "rose/error.h"
#include "rose/io.h"
#include "rose/read.h"

static rsexp np_read (RState* r, rsexp args)
{
    rsexp port;
    rsexp reader;

    r_match_args (r, args, 0, 1, FALSE, &port);

    if (r_undefined_p (port))
        port = r_current_input_port (r);

    ensure (reader = r_reader_new (r, port));

    return r_read (reader);
}

RPrimitiveDesc read_primitives [] = {
    { "read", np_read, 0, 1, FALSE },
    { NULL }
};
