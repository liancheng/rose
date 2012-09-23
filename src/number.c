#include "detail/state.h"
#include "detail/gmp.h"
#include "detail/number.h"
#include "detail/number_reader.h"
#include "detail/port.h"
#include "detail/sexp.h"
#include "rose/number.h"
#include "rose/port.h"

#include <assert.h>
#include <gc/gc.h>
#include <string.h>

static void write_fixnum (rsexp port, rsexp obj)
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

static void write_flonum (rsexp port, rsexp obj)
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

static void fixnum_finalize (rpointer obj, rpointer client_data)
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

    if (smi > R_SMI_MAX || smi < R_SMI_MIN)
        return R_FALSE;

    return r_int_to_sexp (smi);
}

static RType* fixnum_type_info ()
{
    static RType type = {
        .size    = sizeof (RFixnum),
        .name    = "fixnum",
        .write   = write_fixnum,
        .display = write_fixnum,
    };

    return &type;
}

static RType* flonum_type_info ()
{
    static RType type = {
        .size    = sizeof (RFlonum),
        .name    = "flonum",
        .write   = write_flonum,
        .display = write_flonum,
    };

    return &type;
}

static RFixnum* fixnum_new ()
{
    RFixnum* fixnum = GC_NEW_ATOMIC (RFixnum);

    fixnum->type = fixnum_type_info ();
    mpq_inits (fixnum->real, fixnum->imag, NULL);
    GC_REGISTER_FINALIZER (fixnum, fixnum_finalize, NULL, NULL, NULL);

    return fixnum;
}

rbool r_fixnum_p (rsexp obj)
{
    return r_boxed_p (obj) &&
           R_SEXP_TYPE (obj) == fixnum_type_info ();
}

rbool r_flonum_p (rsexp obj)
{
    return r_boxed_p (obj) &&
           R_SEXP_TYPE (obj) == flonum_type_info ();
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

    RFixnum* fixnum = fixnum_new ();
    mpq_set (fixnum->real, real);
    mpq_set (fixnum->imag, imag);

    return FIXNUM_TO_SEXP (fixnum);
}

rsexp r_fixreal_new (mpq_t real)
{
    RFixnum* fixnum = fixnum_new ();
    mpq_set (fixnum->real, real);
    return FIXNUM_TO_SEXP (fixnum);
}

rsexp r_fixnum_normalize (rsexp obj)
{
    assert (r_fixnum_p (obj));

    rsexp smi = try_small_int (FIXNUM_FROM_SEXP (obj)->real,
                               FIXNUM_FROM_SEXP (obj)->imag);

    return r_false_p (smi) ? obj : smi;
}

rsexp r_flonum_new (double real, double imag)
{
    RFlonum* flonum = GC_NEW_ATOMIC (RFlonum);

    flonum->type = flonum_type_info ();
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

rsexp r_int_to_sexp (rint n)
{
    if (n >= R_SMI_MIN && n <= R_SMI_MAX)
        return (n << R_SMI_BITS) | R_SMI_TAG;

    mpq_t real;
    mpq_init (real);
    mpq_set_si (real, n, 1);

    return r_fixreal_new (real);
}
