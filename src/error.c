#include "detail/sexp.h"
#include "rose/error.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/writer.h"

#include <assert.h>
#include <gc/gc.h>

struct RError {
    RType* type;
    rsexp  message;
    rsexp  irritants;
};

#define ERROR_FROM_SEXP(obj) (*((RError*) obj))
#define ERROR_TO_SEXP(error) ((rsexp) error)

static void r_error_write (rsexp port, rsexp obj)
{
    assert (r_error_p (obj));

    r_format (port,
              "error: ~s, irritants: ~s",
              r_error_get_message (obj),
              r_error_get_irritants (obj));
}

static void r_error_display (rsexp port, rsexp obj)
{
    assert (r_error_p (obj));

    r_format (port,
              "error: ~a, irritants: ~a",
              r_error_get_message (obj),
              r_error_get_irritants (obj));
}

static RType* r_error_type_info ()
{
    static RType* type = NULL;

    if (!type) {
        type = GC_MALLOC_ATOMIC (sizeof (RType));

        type->cell_size  = sizeof (RError);
        type->name       = "port";
        type->write_fn   = r_error_write;
        type->display_fn = r_error_display;
    }

    return type;
}

rsexp r_error_new (rsexp message, rsexp irritants)
{
    assert (r_string_p (message));

    RError* res = GC_NEW (RError);

    res->type      = r_error_type_info ();
    res->message   = message;
    res->irritants = irritants;

    return ERROR_TO_SEXP (res);
}

rboolean r_error_p (rsexp obj)
{
    return r_cell_p (obj) &&
           R_CELL_TYPE (obj) == r_error_type_info ();
}

rsexp r_error_get_message (rsexp error)
{
    assert (r_error_p (error));
    return ERROR_FROM_SEXP (error).message;
}

rsexp r_error_get_irritants (rsexp error)
{
    assert (r_error_p (error));
    return ERROR_FROM_SEXP (error).irritants;
}

void r_error_set_message_x (rsexp error, rsexp message)
{
    assert (r_error_p (error) && r_string_p (message));
    ERROR_FROM_SEXP (error).message = message;
}

void r_error_set_irritants_x (rsexp error, rsexp irritants)
{
    assert (r_error_p (error));
    ERROR_FROM_SEXP (error).irritants = irritants;
}
