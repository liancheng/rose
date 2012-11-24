#include "detail/state.h"
#include "detail/gmp.h"
#include "detail/number.h"
#include "detail/number_reader.h"
#include "detail/port.h"
#include "detail/sexp.h"
#include "rose/memory.h"
#include "rose/number.h"
#include "rose/port.h"

#include <assert.h>
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

static rsexp try_small_int (mpq_t real, mpq_t imag)
{
    rint smi;

    /* If imaginary part is not 0... */
    if (0 != mpq_cmp_ui (imag, 0u, 1u))
        return R_FALSE;

    mpq_canonicalize (real);

    /* If the denominator is not 1... */
    if (0 != mpz_cmp_ui (mpq_denref (real), 1u))
        return R_FALSE;

    /* If the number is too large (to fit into a signed int)... */
    if (!mpz_fits_sint_p (mpq_numref (real)))
        return R_FALSE;

    smi = mpz_get_si (mpq_numref (real));

    /* If the number doesn't fit into the range of small integers... */
    if (smi > R_SMI_MAX || smi < R_SMI_MIN)
        return R_FALSE;

    return r_int_to_sexp (smi);
}

static rbool fixnum_eqv_p (RState* state, rsexp lhs, rsexp rhs)
{
    RFixnum* lhs_num = FIXNUM_FROM_SEXP (lhs);
    RFixnum* rhs_num = FIXNUM_FROM_SEXP (rhs);

    return mpq_cmp (lhs_num->real, rhs_num->real) == 0
        && mpq_cmp (lhs_num->imag, rhs_num->imag) == 0;
}

static rbool flonum_eqv_p (RState* state, rsexp lhs, rsexp rhs)
{
    RFlonum* lhs_num = FLONUM_FROM_SEXP (lhs);
    RFlonum* rhs_num = FLONUM_FROM_SEXP (rhs);

    return lhs_num->real == rhs_num->real
        && lhs_num->imag == rhs_num->imag;
}

static void destruct_fixnum (RState* state, RObject* obj)
{
    RFixnum* fixnum = r_cast (RFixnum*, obj);
    mpq_clears (fixnum->real, fixnum->imag, NULL);
}

static RFixnum* fixnum_new (RState* state)
{
    RObject* obj = r_object_new (state, R_FIXNUM_TAG);
    RFixnum* fixnum = r_cast (RFixnum*, obj);

    mpq_inits (fixnum->real, fixnum->imag, NULL);

    return fixnum;
}

RTypeInfo* init_fixnum_type_info (RState* state)
{
    RTypeInfo* type = R_NEW0 (state, RTypeInfo);

    type->size         = sizeof (RFixnum);
    type->name         = "fixnum";
    type->ops.write    = write_fixnum;
    type->ops.display  = write_fixnum;
    type->ops.eqv_p    = fixnum_eqv_p;
    type->ops.equal_p  = fixnum_eqv_p;
    type->ops.mark     = NULL;
    type->ops.destruct = destruct_fixnum;

    return type;
}

RTypeInfo* init_flonum_type_info (RState* state)
{
    RTypeInfo* type = R_NEW0 (state, RTypeInfo);

    type->size         = sizeof (RFlonum);
    type->name         = "flonum";
    type->ops.write    = write_flonum;
    type->ops.display  = write_flonum;
    type->ops.eqv_p    = flonum_eqv_p;
    type->ops.equal_p  = flonum_eqv_p;
    type->ops.mark     = NULL;
    type->ops.destruct = NULL;

    return type;
}

rbool r_fixnum_p (rsexp obj)
{
    return r_type_tag (obj) == R_FIXNUM_TAG;
}

rbool r_flonum_p (rsexp obj)
{
    return r_type_tag (obj) == R_FLONUM_TAG;
}

rsexp r_string_to_number (RState* state, rchar const* text)
{
    return r_number_read (r_number_reader_new (state), text);
}

rsexp r_fixnum_new (RState* state, mpq_t real, mpq_t imag)
{
    rsexp number = try_small_int (real, imag);

    if (!r_false_p (number))
        return number;

    RFixnum* fixnum = fixnum_new (state);
    mpq_set (fixnum->real, real);
    mpq_set (fixnum->imag, imag);

    return FIXNUM_TO_SEXP (fixnum);
}

rsexp r_fixreal_new (RState* state, mpq_t real)
{
    RFixnum* fixnum = fixnum_new (state);
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

rsexp r_flonum_new (RState* state, double real, double imag)
{
    RObject* obj = r_object_new (state, R_FLONUM_TAG);
    RFlonum* flonum = r_cast (RFlonum*, obj);

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
    assert (n >= R_SMI_MIN && n <= R_SMI_MAX);
    return (n << R_SMI_BITS) | R_SMI_TAG;
}

rint r_int_from_sexp (rsexp obj)
{
    assert (r_small_int_p (obj));
    return (r_cast (rint, obj)) >> R_SMI_BITS;
}

rbool r_byte_p (rsexp obj)
{
    if (!r_small_int_p (obj))
        return FALSE;

    rint i = r_int_from_sexp (obj);

    return i >= 0 && i <= 255;
}

rbool r_number_p (rsexp obj)
{
    return r_small_int_p (obj) || r_fixnum_p (obj) || r_flonum_p (obj);
}

rbool r_exact_p (rsexp obj)
{
    return r_small_int_p (obj) || r_fixnum_p (obj);
}
