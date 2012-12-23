#include "detail/math_workaround.h"
#include "detail/number.h"
#include "detail/number_reader.h"
#include "rose/gc.h"
#include "rose/number.h"

#include <ctype.h>
#include <string.h>

static rbool xdigit_to_uint (char ch, ruint* digit)
{
    if ('0' <= ch && ch <= '9') {
        *digit = ch - '0';
        return TRUE;
    }

    if ('a' <= ch && ch <= 'f') {
        *digit = ch - 'a' + 10;
        return TRUE;
    }

    if ('A' <= ch && ch <= 'F') {
        *digit = ch - 'A' + 10;
        return TRUE;
    }

    return FALSE;
}

static void apply_exponent (mpq_t real, rint exponent)
{
    rint exp_sign = exponent < 0 ? -1 : 1;
    mpz_t pow_z;
    mpq_t pow_q;

    exponent = abs (exponent);

    mpz_init (pow_z);
    mpq_init (pow_q);

    mpz_ui_pow_ui (pow_z, 10u, (ruint) exponent);
    mpq_set_num (pow_q, pow_z);

    if (exp_sign < 0)
        mpq_inv (pow_q, pow_q);

    mpq_mul (real, real, pow_q);
    mpq_canonicalize (real);

    mpz_clear (pow_z);
    mpq_clear (pow_q);
}

static rbool fix_exactness (RNumberReader* reader,
                            mpq_t          real,
                            mpq_t          imag,
                            double*        real_d,
                            double*        imag_d)
{
    /* If there exists the exactness prefix `#e', then the number is exact. */
    if (TRUE == reader->exact)
        return TRUE;

    /*
     * If neither `#e' nor `#i' exists, and this isn't a decimal number, then
     * the number is also exact.
     */
    if (UNKNOWN == reader->exact && TRUE != reader->decimal)
        return TRUE;

    /* Otherwise, the number is inexact. */
    *real_d = mpq_get_d (real);
    *imag_d = mpq_get_d (imag);

    return FALSE;
}

static rsexp i_am_feeling_lucky (rconstcstring text)
{
    rcstring end;
    rint number = strtol (text, &end, 10);

    if ('\0' != *end || number > R_SMI_MAX || number < R_SMI_MIN)
        return R_FALSE;

    return r_int_to_sexp (number);
}

static void feed_input (RNumberReader* reader, rconstcstring text)
{
    reader->begin = text;
    reader->end   = text + strlen (text);
    reader->pos   = text;
}

static char lookahead (RNumberReader* reader)
{
    return (reader->pos < reader->end) ? *(reader->pos) : '\0';
}

static char next_char (RNumberReader* reader)
{
    return (reader->pos < reader->end) ? *(reader->pos++) : '\0';
}

static void consume (RNumberReader* reader)
{
    if (++reader->pos > reader->end)
        reader->pos = reader->end;
}

static rbool eoi_p (RNumberReader* reader)
{
    return reader->pos == reader->end;
}

static rconstcstring mark (RNumberReader* reader)
{
    return reader->pos;
}

static rbool reset (RNumberReader* reader, rconstcstring mark)
{
    reader->pos = mark;
    return FALSE;
}

/**
 *  radix
 *      : '#' [bodx]
 *      ;
 */
static rbool read_radix (RNumberReader* reader)
{
    rconstcstring pos = mark (reader);

    if ('#' != next_char (reader))
        return reset (reader, pos);

    switch (next_char (reader)) {
        case 'b': case 'B': reader->radix =  2; return TRUE;
        case 'o': case 'O': reader->radix =  8; return TRUE;
        case 'd': case 'D': reader->radix = 10; return TRUE;
        case 'x': case 'X': reader->radix = 16; return TRUE;
    }

    return reset (reader, pos);
}

/**
 *  exactness
 *      : '#' [ei]
 *      ;
 */
static rbool read_exactness (RNumberReader* reader)
{
    rconstcstring pos = mark (reader);

    if ('#' != next_char (reader))
        return reset (reader, pos);

    switch (next_char (reader)) {
        case 'e':
            reader->exact = TRUE;
            return TRUE;

        case 'i':
            reader->exact = FALSE;
            return TRUE;
    }

    return reset (reader, pos);
}

/**
 *  prefix
 *      : radix exactness?
 *      / exactness radix?
 *      /
 *      ;
 */
static rbool read_prefix (RNumberReader* reader)
{
    if (read_radix (reader))
        read_exactness (reader);
    else if (read_exactness (reader))
        read_radix (reader);

    return TRUE;
}

/**
 *  sign
 *      : '+'
 *      / '-'
 *      ;
 */
static rbool read_sign (RNumberReader* reader, rint* sign)
{
    rconstcstring pos = mark (reader);

    switch (next_char (reader)) {
        case '+': *sign =  1; return TRUE;
        case '-': *sign = -1; return TRUE;
    }

    return reset (reader, pos);
}

/**
 *  digit
 *      : [01]      { when (radix == 2) }
 *      / [0-7]     { when (radix == 8) }
 *      / [0-9]     { when (radix == 10) }
 *      / [0-9a-f]  { when (radix == 16) }
 *      ;
 */
static rbool read_digit (RNumberReader* reader, ruint* digit)
{
    if (!xdigit_to_uint (lookahead (reader), digit))
        return FALSE;

    if (*digit >= reader->radix)
        return FALSE;

    consume (reader);
    return TRUE;
}

static rbool read_digits (RNumberReader* reader, mpz_t digits)
{
    rconstcstring pos = mark (reader);

    ruint digit;

    if (!read_digit (reader, &digit))
        return reset (reader, pos);

    mpz_set_ui (digits, digit);

    while (read_digit (reader, &digit)) {
        mpz_mul_ui (digits, digits, reader->radix);
        mpz_add_ui (digits, digits, digit);
    }

    return TRUE;
}

/**
 *  uinteger
 *      : digit+ !'.'
 *      ;
 */
static rbool read_uinteger (RNumberReader* reader, mpz_t uinteger)
{
    rconstcstring pos = mark (reader);

    if (!read_digits (reader, uinteger))
        return reset (reader, pos);

    if ('.' == lookahead (reader))
        return reset (reader, pos);

    return TRUE;
}

/**
 *  rational
 *      : uinteger '/' uinteger
 *      ;
 */
static rbool read_rational (RNumberReader* reader, mpq_t ureal)
{
    mpz_t numer;
    mpz_t denom;
    rbool success;

    rconstcstring pos = mark (reader);

    mpz_init_set_ui (numer, 0u);
    mpz_init_set_ui (denom, 1u);

    if (!read_uinteger (reader, numer))
        goto fail;

    if ('/' != next_char (reader))
        goto fail;

    if (!read_uinteger (reader, denom))
        goto fail;

    mpq_set_num (ureal, numer);
    mpq_set_den (ureal, denom);
    mpq_canonicalize (ureal);

    success = TRUE;
    goto clear;

fail:
    success = reset (reader, pos);

clear:
    mpz_clears (numer, denom, NULL);
    return success;
}

/**
 *  suffix
 *      : [esfdl] sign digit+
 *      ;
 */
static rbool read_suffix (RNumberReader* reader, rint* exponent)
{
    rint sign = 1;
    ruint digit;
    rconstcstring pos;

    pos = mark (reader);
    *exponent = 0;

    if (!strchr ("esfdl", lookahead (reader)))
        return reset (reader, pos);

    consume (reader);

    read_sign (reader, &sign);

    if (!read_digit (reader, &digit))
        return reset (reader, pos);

    for (*exponent = digit; read_digit (reader, &digit); )
        *exponent = (*exponent) * 10 + digit;

    if (sign < 0)
        *exponent = -(*exponent);

    return TRUE;
}

/**
 *  decimal_frac
 *      : '.' digit+ suffix?
 *      ;
 */
static rbool read_decimal_frac (RNumberReader* reader, mpq_t ureal)
{
    mpz_t numer;
    mpz_t denom;
    ruint digit;
    ruint size;
    rbool success;
    rint exponent = 0;
    rconstcstring pos = mark (reader);

    mpq_set_ui (ureal, 0u, 1u);
    mpz_inits (numer, denom, NULL);

    if ('.' != next_char (reader))
        goto fail;

    if (!read_digit (reader, &digit))
        goto fail;

    mpz_set_ui (numer, digit);

    for (size = 1u; read_digit (reader, &digit); ++size) {
        mpz_mul_ui (numer, numer, 10u);
        mpz_add_ui (numer, numer, digit);
    }

    mpz_ui_pow_ui (denom, 10u, size);
    mpq_set_num (ureal, numer);
    mpq_set_den (ureal, denom);
    mpq_canonicalize (ureal);

    read_suffix (reader, &exponent);
    apply_exponent (ureal, exponent);

    reader->decimal = TRUE;
    success = TRUE;
    goto clear;

fail:
    success = reset (reader, pos);

clear:
    mpz_clears (numer, denom, NULL);
    return success;
}

/**
 *  decimal_int_frac
 *      : digit+ '.' digit* suffix?
 *      ;
 */
static rbool read_decimal_int_frac (RNumberReader* reader,
                                    mpq_t          ureal)
{
    mpz_t numer;
    mpz_t denom;
    ruint digit;
    ruint size;
    rbool success;
    rint exponent = 0;
    rconstcstring pos = mark (reader);

    mpq_set_ui (ureal, 0u, 1u);
    mpz_inits (numer, denom, NULL);

    if (!read_digits (reader, numer))
        goto fail;

    if ('.' != next_char (reader))
        goto fail;

    for (size = 0u; read_digit (reader, &digit); ++size) {
        mpz_mul_ui (numer, numer, 10u);
        mpz_add_ui (numer, numer, digit);
    }

    mpz_ui_pow_ui (denom, 10u, size);
    mpq_set_num (ureal, numer);
    mpq_set_den (ureal, denom);
    mpq_canonicalize (ureal);

    read_suffix (reader, &exponent);
    apply_exponent (ureal, exponent);

    reader->decimal = TRUE;
    success = TRUE;
    goto clear;

fail:
    success = reset (reader, pos);

clear:
    mpz_clears (numer, denom, NULL);
    return success;
}

/**
 *  decimal_uint
 *      : uinteger suffix
 *      ;
 */
static rbool read_decimal_uint (RNumberReader* reader, mpq_t ureal)
{
    mpz_t numer;
    rbool success;
    rint exponent = 0;
    rconstcstring pos = mark (reader);

    mpq_set_ui (ureal, 0u, 1u);
    mpz_init (numer);

    if (!read_uinteger (reader, numer))
        goto fail;

    mpq_set_num (ureal, numer);

    if (!read_suffix (reader, &exponent))
        goto fail;

    apply_exponent (ureal, exponent);

    reader->decimal = TRUE;
    success = TRUE;
    goto clear;

fail:
    success = reset (reader, pos);

clear:
    mpz_clear (numer);
    return success;
}

/**
 *  decimal
 *      : decimal_frac
 *      / decimal_int_frac
 *      / decimal_uint
 *      ;
 */
static rbool read_decimal (RNumberReader* reader, mpq_t ureal)
{
    rconstcstring pos;

    if (10 != reader->radix)
        return FALSE;

    pos = mark (reader);

    return read_decimal_frac (reader, ureal)
        || read_decimal_int_frac (reader, ureal)
        || read_decimal_uint (reader, ureal)
        || reset (reader, pos);
}

/**
 *  ureal_uint
 *      : uinteger
 *      ;
 */
static rbool read_ureal_uint (RNumberReader* reader, mpq_t ureal)
{
    mpz_t numer;
    rbool success;
    rconstcstring pos = mark (reader);

    mpz_init (numer);
    mpq_set_ui (ureal, 0u, 1u);

    if (!read_uinteger (reader, numer))
        goto fail;

    mpq_set_num (ureal, numer);

    success = TRUE;
    goto clear;

fail:
    success = reset (reader, pos);

clear:
    mpz_clear (numer);
    return success;
}

/**
 *  ureal
 *      : rational
 *      / decimal       { when (radix == 10) }
 *      / ureal_uint
 *      ;
 */
static rbool read_ureal (RNumberReader* reader, mpq_t ureal)
{
    rconstcstring pos = mark (reader);

    return read_rational (reader, ureal)
        || read_decimal (reader, ureal)
        || read_ureal_uint (reader, ureal)
        || reset (reader, pos);
}

/**
 *  real
 *      : sign? ureal
 *      ;
 */
static rbool read_real (RNumberReader* reader, mpq_t real)
{
    rint sign = 1;
    rconstcstring pos = mark (reader);

    read_sign (reader, &sign);

    if (!read_ureal (reader, real))
        return reset (reader, pos);

    if (sign < 0)
        mpq_neg (real, real);

    return TRUE;
}

/**
 *  polar_complex
 *      : real '@' real
 *      ;
 */
static rbool read_polar_complex (RNumberReader* reader,
                                 double*        rho,
                                 double*        theta)
{
    mpq_t r;
    mpq_t t;
    rbool success;
    rconstcstring pos = mark (reader);

    mpq_inits (r, t, NULL);

    if (!read_real (reader, r))
        goto fail;

    if ('@' != next_char (reader))
        goto fail;

    if (!read_real (reader, t))
        goto fail;

    *rho    = mpq_get_d (r);
    *theta  = mpq_get_d (t);
    success = TRUE;

    goto clear;

fail:
    success = reset (reader, pos);

clear:
    mpq_clears (r, t, NULL);
    return success;
}

/**
 *  rect_ri
 *      : real sign ureal? 'i'
 *      ;
 */
static rbool read_rect_ri (RNumberReader* reader,
                           mpq_t          real,
                           mpq_t          imag)
{
    char i;
    rint sign = 1;
    rconstcstring pos = mark (reader);

    if (!read_real (reader, real))
        return reset (reader, pos);

    if (!read_sign (reader, &sign))
        return reset (reader, pos);

    if (!read_ureal (reader, imag))
        mpq_set_ui (imag, 1u, 1u);

    if (sign < 0)
        mpq_neg (imag, imag);

    i = next_char (reader);

    if ('i' != i && 'I' != i)
        return reset (reader, pos);

    return TRUE;
}

/**
 *  rect_i
 *      : sign ureal? 'i'
 *      ;
 */
static rbool read_rect_i (RNumberReader* reader,
                          mpq_t          real,
                          mpq_t          imag)
{
    char i;
    rint sign = 1;
    rconstcstring pos = mark (reader);

    mpq_set_ui (real, 0u, 1u);

    if (!read_sign (reader, &sign))
        return reset (reader, pos);

    if (!read_ureal (reader, imag))
        mpq_set_ui (imag, 1u, 1u);

    if (sign < 0)
        mpq_neg (imag, imag);

    i = next_char (reader);

    if ('i' != i && 'I' != i)
        return reset (reader, pos);

    return TRUE;
}

/**
 *  rect_r
 *      : real
 *      ;
 */
static rbool read_rect_r (RNumberReader* reader,
                          mpq_t          real,
                          mpq_t          imag)
{
    rconstcstring pos = mark (reader);

    if (!read_real (reader, real))
        return reset (reader, pos);

    mpq_set_ui (imag, 0u, 1u);

    return TRUE;
}

/**
 *  rect_complex
 *      : rect_i
 *      / rect_ri
 *      / rect_r
 *      ;
 */
static rbool read_rect_complex (RNumberReader* reader,
                                mpq_t          real,
                                mpq_t          imag)
{
    rconstcstring pos = mark (reader);

    return read_rect_i (reader, real, imag)
        || read_rect_ri (reader, real, imag)
        || read_rect_r (reader, real, imag)
        || reset (reader, pos);
}

/**
 *  number
 *      : prefix? (polar_complex / rect_complex)
 *      ;
 */
static rsexp read_number (RNumberReader* reader)
{
    rsexp number;
    double rho;
    double theta;
    mpq_t real;
    mpq_t imag;
    rconstcstring pos = mark (reader);

    mpq_inits (real, imag, NULL);

    read_prefix (reader);

    if (read_polar_complex (reader, &rho, &theta)) {
        number = r_flonum_new (reader->state,
                               rho * r_cos (theta),
                               rho * r_sin (theta));
        goto clear;
    }

    if (read_rect_complex (reader, real, imag)) {
        double r;
        double i;

        number = fix_exactness (reader, real, imag, &r, &i)
               ? r_fixnum_new (reader->state, real, imag)
               : r_flonum_new (reader->state, r, i);

        goto clear;
    }

    reset (reader, pos);
    number = R_FAILURE;

clear:
    mpq_clears (real, imag, NULL);

    return number;
}

/**
 *  start
 *      : number <EOI>
 *      ;
 */
rsexp r_number_read (RNumberReader* reader, rconstcstring text)
{
    rsexp number;
    rconstcstring pos;

    number = i_am_feeling_lucky (text);

    if (!r_false_p (number))
        return number;

    feed_input (reader, text);
    pos = mark (reader);
    number = read_number (reader);

    if (r_false_p (number)) {
        reset (reader, pos);
        return R_FAILURE;
    }

    if (!eoi_p (reader)) {
        reset (reader, pos);
        return R_FAILURE;
    }

    return number;
}

void r_number_reader_init (RState* state, RNumberReader* reader)
{
    reader->state   = state;
    reader->begin   = NULL;
    reader->end     = NULL;
    reader->pos     = NULL;
    reader->exact   = UNKNOWN;
    reader->decimal = UNKNOWN;
    reader->radix   = 10u;
}
