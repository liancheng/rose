#include "detail/context.h"
#include "detail/gmp.h"
#include "detail/number.h"
#include "detail/number_reader.h"
#include "detail/port.h"
#include "detail/sexp.h"
#include "rose/port.h"

#include <gc/gc.h>
#include <string.h>

#define FIXNUM_TO_SEXP(ptr)     (((rsexp) (ptr)) | R_FIXNUM_TAG)
#define FIXNUM_FROM_SEXP(obj)   ((RFixnum*) ((obj) & (~R_FIXNUM_TAG)))

#define FLONUM_TO_SEXP(ptr)     (((rsexp) (ptr)) | R_FLONUM_TAG)
#define FLONUM_FROM_SEXP(obj)   ((RFlonum*) ((obj) & (~R_FLONUM_TAG)))

static void r_fixnum_write (rsexp port, rsexp obj)
{
    RFixnum* fixnum = FIXNUM_FROM_SEXP (obj);
    FILE*    stream = PORT_TO_FILE (port);

    mpq_out_str (stream, 10, fixnum->real);

    if (0 != mpq_cmp_ui (fixnum->imag, 0u, 1u)) {
        if (0 < mpq_cmp_ui (fixnum->imag, 0u, 1u))
            r_write_char (port, '+');

        mpq_out_str (stream, 10, fixnum->imag);
        r_write_char (port, 'i');
    }
}

static void r_flonum_write (rsexp port, rsexp obj)
{
    RFlonum* flonum = FLONUM_FROM_SEXP (obj);

    r_port_printf (port, "%f", flonum->real);

    if (flonum->imag != 0.) {
        if (flonum->imag > 0.)
            r_write_char (port, '+');

        r_port_printf (port, "%f", flonum->imag);
        r_write_char (port, 'i');
    }
}

static void r_fixnum_finalize (rpointer obj, rpointer client_data)
{
    RFixnum* fixnum = obj;
    r_fixnum_clear (fixnum);
}

void r_register_fixnum_type (RContext* context)
{
    RType* type = GC_NEW (RType);

    type->cell_size  = 0;
    type->name       = "fixnum";
    type->write_fn   = r_fixnum_write;
    type->display_fn = r_fixnum_write;

    context->tc3_types [R_FIXNUM_TAG] = type;
}

void r_register_flonum_type (RContext* context)
{
    RType* type = GC_NEW (RType);

    type->cell_size  = 0;
    type->name       = "flonum";
    type->write_fn   = r_flonum_write;
    type->display_fn = r_flonum_write;

    context->tc3_types [R_FLONUM_TAG] = type;
}

rsexp r_string_to_number (char const* text)
{
    return r_number_read (r_number_reader_new (), text);
}

rsexp r_fixnum_new ()
{
    RFixnum* fixnum = GC_NEW (RFixnum);

    r_fixnum_init (fixnum);
    GC_REGISTER_FINALIZER (fixnum, r_fixnum_finalize, NULL, NULL, NULL);

    return FIXNUM_TO_SEXP (fixnum);
}

rsexp r_flonum_new ()
{
    RFlonum* flonum = GC_NEW (RFlonum);

    flonum->real = 0.;
    flonum->imag = 0.;

    return FLONUM_TO_SEXP (flonum);
}

void r_fixnum_init (RFixnum* fixnum)
{
    mpq_inits (fixnum->real, fixnum->imag, NULL);
}

void r_fixnum_clear (RFixnum* fixnum)
{
    mpq_clears (fixnum->real, fixnum->imag, NULL);
}

void r_fixnum_set_real_x (rsexp obj, mpq_t real)
{
    mpq_set (FIXNUM_FROM_SEXP (obj)->real, real);
}

void r_fixnum_set_imag_x (rsexp obj, mpq_t imag)
{
    mpq_set (FIXNUM_FROM_SEXP (obj)->imag, imag);
}

void r_flonum_set_real_x (rsexp obj, double real)
{
    FLONUM_FROM_SEXP (obj)->real = real;
}

void r_flonum_set_imag_x (rsexp obj, double imag)
{
    FLONUM_FROM_SEXP (obj)->imag = imag;
}
