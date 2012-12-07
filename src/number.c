#include "detail/state.h"
#include "detail/number.h"
#include "detail/number_reader.h"
#include "detail/port.h"
#include "detail/sexp.h"
#include "rose/gmp.h"
#include "rose/memory.h"
#include "rose/number.h"
#include "rose/port.h"

#include <assert.h>
#include <string.h>

static void write_fixnum (RState* state, rsexp port, rsexp obj)
{
    RFixnum* fixnum = fixnum_from_sexp (obj);
    FILE*    stream = port_to_file (port);

    mpq_out_str (stream, 10, fixnum->real);

    if (0 != mpq_cmp_ui (fixnum->imag, 0u, 1u)) {
        if (0 < mpq_cmp_ui (fixnum->imag, 0u, 1u))
            r_write_char (port, '+');

        mpq_out_str (stream, 10, fixnum->imag);
        r_write_char (port, 'i');
    }
}

static void write_flonum (RState* state, rsexp port, rsexp obj)
{
    RFlonum* flonum = flonum_from_sexp (obj);

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
    RFixnum* lhs_num = fixnum_from_sexp (lhs);
    RFixnum* rhs_num = fixnum_from_sexp (rhs);

    return mpq_cmp (lhs_num->real, rhs_num->real) == 0
        && mpq_cmp (lhs_num->imag, rhs_num->imag) == 0;
}

static rbool flonum_eqv_p (RState* state, rsexp lhs, rsexp rhs)
{
    RFlonum* lhs_num = flonum_from_sexp (lhs);
    RFlonum* rhs_num = flonum_from_sexp (rhs);

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
    RFixnum* fixnum = r_object_new (state, RFixnum, R_FIXNUM_TAG);
    mpq_inits (fixnum->real, fixnum->imag, NULL);
    return fixnum;
}

void init_fixnum_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (RFixnum);
    type->name         = "fixnum";
    type->ops.write    = write_fixnum;
    type->ops.display  = write_fixnum;
    type->ops.eqv_p    = fixnum_eqv_p;
    type->ops.equal_p  = fixnum_eqv_p;
    type->ops.mark     = NULL;
    type->ops.destruct = destruct_fixnum;

    state->builtin_types [R_FIXNUM_TAG] = type;
}

void init_flonum_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (RFlonum);
    type->name         = "flonum";
    type->ops.write    = write_flonum;
    type->ops.display  = write_flonum;
    type->ops.eqv_p    = flonum_eqv_p;
    type->ops.equal_p  = flonum_eqv_p;
    type->ops.mark     = NULL;
    type->ops.destruct = NULL;

    state->builtin_types [R_FLONUM_TAG] = type;
}

rbool r_fixnum_p (rsexp obj)
{
    return r_type_tag (obj) == R_FIXNUM_TAG;
}

rbool r_flonum_p (rsexp obj)
{
    return r_type_tag (obj) == R_FLONUM_TAG;
}

rsexp r_string_to_number (RState* state, rconstcstring text)
{
    RNumberReader reader;
    rsexp         res;

    r_number_reader_init (state, &reader);
    res = r_number_read (&reader, text);

    return res;
}

rsexp r_fixnum_new (RState* state, mpq_t real, mpq_t imag)
{
    rsexp number = try_small_int (real, imag);

    if (!r_false_p (number))
        return number;

    RFixnum* fixnum = fixnum_new (state);
    mpq_set (fixnum->real, real);
    mpq_set (fixnum->imag, imag);

    return fixnum_to_sexp (fixnum);
}

rsexp r_fixreal_new (RState* state, mpq_t real)
{
    RFixnum* fixnum = fixnum_new (state);
    mpq_set (fixnum->real, real);
    return fixnum_to_sexp (fixnum);
}

rsexp r_fixnum_normalize (rsexp obj)
{
    assert (r_fixnum_p (obj));

    rsexp smi = try_small_int (fixnum_from_sexp (obj)->real,
                               fixnum_from_sexp (obj)->imag);

    return r_false_p (smi) ? obj : smi;
}

rsexp r_flonum_new (RState* state, double real, double imag)
{
    RFlonum* flonum = r_object_new (state, RFlonum, R_FLONUM_TAG);

    flonum->real = real;
    flonum->imag = imag;

    return flonum_to_sexp (flonum);
}

void r_fixnum_set_real_x (rsexp obj, mpq_t real)
{
    mpq_set (fixnum_from_sexp (obj)->real, real);
}

void r_fixnum_set_imag_x (rsexp obj, mpq_t imag)
{
    mpq_set (fixnum_from_sexp (obj)->imag, imag);
}

void r_flonum_set_real_x (rsexp obj, double real)
{
    flonum_from_sexp (obj)->real = real;
}

void r_flonum_set_imag_x (rsexp obj, double imag)
{
    flonum_from_sexp (obj)->imag = imag;
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
