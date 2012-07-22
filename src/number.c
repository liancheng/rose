#include "detail/state.h"
#include "detail/gmp.h"
#include "detail/number.h"
#include "detail/number_reader.h"
#include "detail/port.h"
#include "detail/sexp.h"
#include "rose/number.h"
#include "rose/port.h"

#include <gc/gc.h>
#include <string.h>

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
    mpq_clears (fixnum->real, fixnum->imag, NULL);
}

static rsexp try_small_int (mpq_t real, mpq_t imag)
{
    rint smi;

    if (0 != mpq_cmp_ui (imag, 0u, 1u))
        return R_FALSE;

    mpq_canonicalize (real);

    if (0 != mpz_cmp_ui (mpq_denref (real), 1u))
        return R_FALSE;

    if (!mpz_fits_sint_p (mpq_numref (real)))
        return R_FALSE;

    smi = mpz_get_si (mpq_numref (real));

    if (smi > SMI_MAX || smi < SMI_MIN)
        return R_FALSE;

    return r_int_to_sexp (smi);
}

static RType* r_fixnum_type_info ()
{
    static RType type = {
        .size    = sizeof (RFixnum),
        .name    = "fixnum",
        .write   = r_fixnum_write,
        .display = r_fixnum_write,
    };

    return &type;
}

static RType* r_flonum_type_info ()
{
    static RType type = {
        .size    = sizeof (RFlonum),
        .name    = "flonum",
        .write   = r_flonum_write,
        .display = r_flonum_write,
    };

    return &type;
}

rbool r_number_p (rsexp obj)
{
    return r_smi_p (obj)
        || r_fixnum_p (obj)
        || r_flonum_p (obj);
}

rbool r_byte_p (rsexp obj)
{
    return r_smi_p (obj)
        && r_int_from_sexp (obj) >= 0
        && r_int_from_sexp (obj) <= 255;
}

rbool r_fixnum_p (rsexp obj)
{
    return r_cell_p (obj) &&
           R_SEXP_TYPE (obj) == r_fixnum_type_info ();
}

rbool r_flonum_p (rsexp obj)
{
    return r_cell_p (obj) &&
           R_SEXP_TYPE (obj) == r_flonum_type_info ();
}

rsexp r_string_to_number (char const* text)
{
    return r_number_read (r_number_reader_new (), text);
}

rsexp r_fixnum_new (mpq_t real, mpq_t imag)
{
    rsexp number = try_small_int (real, imag);

    if (!r_false_p (number))
        return number;

    RFixnum* fixnum = GC_NEW_ATOMIC (RFixnum);

    fixnum->type = r_fixnum_type_info ();
    mpq_inits (fixnum->real, fixnum->imag, NULL);
    mpq_set (fixnum->real, real);
    mpq_set (fixnum->imag, imag);

    GC_REGISTER_FINALIZER (fixnum, r_fixnum_finalize, NULL, NULL, NULL);

    return FIXNUM_TO_SEXP (fixnum);
}

rsexp r_flonum_new (double real, double imag)
{
    RFlonum* flonum = GC_NEW_ATOMIC (RFlonum);

    flonum->type = r_flonum_type_info ();
    flonum->real = real;
    flonum->imag = imag;

    return FLONUM_TO_SEXP (flonum);
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
