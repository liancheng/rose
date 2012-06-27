#include "detail/math_workaround.h"
#include "detail/number.h"
#include "detail/number_reader.h"
#include "rose/string.h"

#include <ctype.h>
#include <gc/gc.h>
#include <string.h>

#define MARK\
        char const* mark = r_number_reader_mark (reader)

#define REWIND\
        r_number_reader_rewind (reader, mark)

#define NEXT\
        r_number_reader_next (reader)

#define LOOKAHEAD\
        r_number_reader_lookahead (reader, 0u)

#define LOOKAHEAD_N(n)\
        r_number_reader_lookahead (reader, (n))

#define CONSUME\
        r_number_reader_consume (reader, 1u)

#define CONSUME_N(n)\
        r_number_reader_consume (reader, (n))

static rboolean xdigit_to_uint (char ch, ruint* digit)
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
}

static rsexp i_am_feeling_lucky (char const* text)
{
    char* end;
    rint number = strtol (text, &end, 10);

    if ('\0' != *end || number > INT30_MAX || number < INT30_MIN)
        return R_FALSE;

    return r_int_to_sexp (number);
}

static rboolean fix_exactness (rtribool exact,
                               mpq_t    real,
                               mpq_t    imag,
                               double*  real_d,
                               double*  imag_d)
{
    if (TRUE == exact || UNKNOWN == exact)
        return TRUE;

    *real_d = mpq_get_d (real);
    *imag_d = mpq_get_d (imag);

    return FALSE;
}

RNumberReader* r_number_reader_new ()
{
    RNumberReader* reader = GC_NEW (RNumberReader);

    memset (reader, 0, sizeof (RNumberReader));

    reader->begin = NULL;
    reader->end   = NULL;
    reader->pos   = NULL;

    reader->exact = UNKNOWN;
    reader->radix = 10u;

    return reader;
}

void r_number_reader_feed_input (RNumberReader* reader, char const* text)
{
    reader->begin = text;
    reader->end   = text + strlen (text);
    reader->pos   = text;
}

char r_number_reader_lookahead (RNumberReader* reader, ruint n)
{
    return (reader->pos + n < reader->end) ? *(reader->pos + n) : '\0';
}

char r_number_reader_next (RNumberReader* reader)
{
    return (reader->pos < reader->end) ? *(reader->pos++) : '\0';
}

void r_number_reader_consume (RNumberReader* reader, ruint n)
{
    if (reader->pos + n > reader->end)
        reader->pos = reader->end;
    else
        reader->pos += n;
}

rboolean r_number_reader_eoi_p (RNumberReader* reader)
{
    return reader->pos == reader->end;
}

char const* r_number_reader_mark (RNumberReader* reader)
{
    return reader->pos;
}

rboolean r_number_reader_rewind (RNumberReader* reader, char const* mark)
{
    reader->pos = mark;
    return FALSE;
}

/**
 *  start
 *      : number <EOI>
 *      ;
 */
rsexp r_number_read (RNumberReader* reader, char const* text)
{
    rsexp number;

    number = i_am_feeling_lucky (text);

    if (!r_false_p (number))
        return number;

    r_number_reader_feed_input (reader, text);

    MARK;

    number = r_number_read_number (reader);

    if (r_false_p (number))
        return r_bool_to_sexp (REWIND);

    if (!r_number_reader_eoi_p (reader))
        return r_bool_to_sexp (REWIND);

    return number;
}

/**
 *  number
 *      : prefix? (polar_complex / rect_complex)
 *      ;
 */
rsexp r_number_read_number (RNumberReader* reader)
{
    rsexp number;
    double rho;
    double theta;
    mpq_t real;
    mpq_t imag;

    MARK;
    mpq_inits (real, imag, NULL);

    r_number_read_prefix (reader);

    if (r_number_read_polar_complex (reader, &rho, &theta)) {
        number = r_flonum_new (rho * r_cos (theta), rho * r_sin (theta));
        goto clear;
    }

    if (r_number_read_rect_complex (reader, real, imag)) {
        double r;
        double i;

        if (FALSE == fix_exactness (reader->exact, real, imag, &r, &i))
            number = r_flonum_new (r, i);
        else
            number = r_fixnum_new (real, imag);

        goto clear;
    }

    number = r_bool_to_sexp (REWIND);

clear:
    mpq_clears (real, imag, NULL);
    return number;
}

/**
 *  prefix
 *      : radix exactness?
 *      / exactness radix?
 *      /
 *      ;
 */
rboolean r_number_read_prefix (RNumberReader* reader)
{
    if (r_number_read_radix (reader)) {
        r_number_read_exactness (reader);
        return TRUE;
    }

    if (r_number_read_exactness (reader))
        r_number_read_radix (reader);

    return TRUE;
}

/**
 *  radix
 *      : '#' [bodx]
 *      ;
 */
rboolean r_number_read_radix (RNumberReader* reader)
{
    MARK;

    if ('#' != NEXT)
        return REWIND;

    switch (NEXT) {
        case 'b': case 'B': reader->radix =  2; return TRUE;
        case 'o': case 'O': reader->radix =  8; return TRUE;
        case 'd': case 'D': reader->radix = 10; return TRUE;
        case 'x': case 'X': reader->radix = 16; return TRUE;
    }

    return REWIND;
}

/**
 *  exactness
 *      : '#' [ei]
 *      ;
 */
rboolean r_number_read_exactness (RNumberReader* reader)
{
    MARK;

    if ('#' != NEXT)
        return REWIND;

    switch (NEXT) {
        case 'e': case 'E': reader->exact = TRUE;  return TRUE;
        case 'i': case 'I': reader->exact = FALSE; return TRUE;
    }

    return REWIND;
}

/**
 *  polar_complex
 *      : real '@' real
 *      ;
 */
rboolean r_number_read_polar_complex (RNumberReader* reader,
                                      double*        rho,
                                      double*        theta)
{
    mpq_t r;
    mpq_t t;
    rboolean success;

    MARK;
    mpq_inits (r, t, NULL);

    if (!r_number_read_real (reader, r))
        goto fail;

    if ('@' != NEXT)
        goto fail;

    if (!r_number_read_real (reader, t))
        goto fail;

    *rho    = mpq_get_d (r);
    *theta  = mpq_get_d (t);
    success = TRUE;

    goto clear;

fail:
    success = REWIND;

clear:
    mpq_clears (r, t, NULL);
    return success;
}

/**
 *  rect_complex
 *      : rect_i
 *      / rect_ri
 *      / rect_r
 *      ;
 */
rboolean r_number_read_rect_complex (RNumberReader* reader,
                                     mpq_t          real,
                                     mpq_t          imag)
{
    MARK;

    return r_number_read_rect_i (reader, real, imag)
        || r_number_read_rect_ri (reader, real, imag)
        || r_number_read_rect_r (reader, real, imag)
        || REWIND;
}

/**
 *  rect_i
 *      : sign ureal? 'i'
 *      ;
 */
rboolean r_number_read_rect_i (RNumberReader* reader, mpq_t real, mpq_t imag)
{
    rint sign = 1;
    char i;

    MARK;

    mpq_set_ui (real, 0u, 1u);

    if (!r_number_read_sign (reader, &sign))
        return REWIND;

    if (!r_number_read_ureal (reader, imag))
        mpq_set_ui (imag, 1u, 1u);

    if (sign < 0)
        mpq_neg (imag, imag);

    i = NEXT;

    if ('i' != i && 'I' != i)
        return REWIND;

    return TRUE;
}

/**
 *  rect_ri
 *      : real sign ureal? 'i'
 *      ;
 */
rboolean r_number_read_rect_ri (RNumberReader* reader, mpq_t real, mpq_t imag)
{
    rint sign = 1;
    char i;

    MARK;

    if (!r_number_read_real (reader, real))
        return REWIND;

    if (!r_number_read_sign (reader, &sign))
        return REWIND;

    if (!r_number_read_ureal (reader, imag))
        mpq_set_ui (imag, 1u, 1u);

    if (sign < 0)
        mpq_neg (imag, imag);

    i = NEXT;

    if ('i' != i && 'I' != i)
        return REWIND;

    return TRUE;
}

/**
 *  rect_r
 *      : real
 *      ;
 */
rboolean r_number_read_rect_r (RNumberReader* reader, mpq_t real, mpq_t imag)
{
    MARK;

    if (!r_number_read_real (reader, real))
        return REWIND;

    mpq_set_ui (imag, 0u, 1u);

    return TRUE;
}

/**
 *  real
 *      : sign? ureal
 *      ;
 */
rboolean r_number_read_real (RNumberReader* reader, mpq_t real)
{
    rint sign = 1;

    MARK;

    r_number_read_sign (reader, &sign);

    if (!r_number_read_ureal (reader, real))
        return REWIND;

    if (sign < 0)
        mpq_neg (real, real);

    return TRUE;
}

/**
 *  ureal
 *      : rational
 *      / decimal       { when (radix == 10) }
 *      / ureal_uint
 *      ;
 */
rboolean r_number_read_ureal (RNumberReader* reader, mpq_t ureal)
{
    MARK;

    return r_number_read_rational (reader, ureal)
        || r_number_read_decimal (reader, ureal)
        || r_number_read_ureal_uint (reader, ureal)
        || REWIND;
}

/**
 *  rational
 *      : uinteger '/' uinteger
 *      ;
 */
rboolean r_number_read_rational (RNumberReader* reader, mpq_t ureal)
{
    mpz_t numer;
    mpz_t denom;
    rboolean success;

    MARK;

    mpz_init_set_ui (numer, 0u);
    mpz_init_set_ui (denom, 1u);

    if (!r_number_read_uinteger (reader, numer))
        goto fail;

    if ('/' != NEXT)
        goto fail;

    if (!r_number_read_uinteger (reader, denom))
        goto fail;

    mpq_set_num (ureal, numer);
    mpq_set_den (ureal, denom);
    mpq_canonicalize (ureal);

    success = TRUE;
    goto clear;

fail:
    success = REWIND;

clear:
    mpz_clears (numer, denom, NULL);
    return success;
}

/**
 *  decimal
 *      : decimal_frac
 *      / decimal_int_frac
 *      / decimal_uint
 *      ;
 */
rboolean r_number_read_decimal (RNumberReader* reader, mpq_t ureal)
{
    if (10 != reader->radix)
        return FALSE;

    MARK;

    return r_number_read_decimal_frac (reader, ureal)
        || r_number_read_decimal_int_frac (reader, ureal)
        || r_number_read_decimal_uint (reader, ureal)
        || REWIND;
}

/**
 *  decimal_frac
 *      : '.' digit+ suffix?
 *      ;
 */
rboolean r_number_read_decimal_frac (RNumberReader* reader, mpq_t ureal)
{
    mpz_t numer;
    mpz_t denom;
    ruint digit;
    ruint size;
    rboolean success;
    rint exponent = 0;

    MARK;
    mpq_set_ui (ureal, 0u, 1u);
    mpz_inits (numer, denom, NULL);

    if ('.' != NEXT)
        goto fail;

    if (!r_number_read_digit (reader, &digit))
        goto fail;

    mpz_set_ui (numer, digit);

    for (size = 1u; r_number_read_digit (reader, &digit); ++size) {
        mpz_mul_ui (numer, numer, 10u);
        mpz_add_ui (numer, numer, digit);
    }

    mpz_ui_pow_ui (denom, 10u, size);
    mpq_set_num (ureal, numer);
    mpq_set_den (ureal, denom);
    mpq_canonicalize (ureal);

    r_number_read_suffix (reader, &exponent);
    apply_exponent (ureal, exponent);

    reader->exact = FALSE;
    success = TRUE;
    goto clear;

fail:
    success = REWIND;

clear:
    mpz_clears (numer, denom, NULL);
    return success;
}

/**
 *  decimal_int_frac
 *      : digit+ '.' digit* suffix?
 *      ;
 */
rboolean r_number_read_decimal_int_frac (RNumberReader* reader, mpq_t ureal)
{
    mpz_t numer;
    mpz_t denom;
    ruint digit;
    ruint size;
    rboolean success;
    rint exponent = 0;

    MARK;
    mpq_set_ui (ureal, 0u, 1u);
    mpz_inits (numer, denom, NULL);

    if (!r_number_read_digits (reader, numer))
        goto fail;

    if ('.' != NEXT)
        goto fail;

    for (size = 0u; r_number_read_digit (reader, &digit); ++size) {
        mpz_mul_ui (numer, numer, 10u);
        mpz_add_ui (numer, numer, digit);
    }

    mpz_ui_pow_ui (denom, 10u, size);
    mpq_set_num (ureal, numer);
    mpq_set_den (ureal, denom);
    mpq_canonicalize (ureal);

    r_number_read_suffix (reader, &exponent);
    apply_exponent (ureal, exponent);

    reader->exact = FALSE;
    success = TRUE;
    goto clear;

fail:
    success = REWIND;

clear:
    mpz_clears (numer, denom, NULL);
    return success;
}

/**
 *  decimal_uint
 *      : uinteger suffix
 *      ;
 */
rboolean r_number_read_decimal_uint (RNumberReader* reader, mpq_t ureal)
{
    mpz_t numer;
    rboolean success;
    rint exponent = 0;

    MARK;
    mpq_set_ui (ureal, 0u, 1u);
    mpz_init (numer);

    if (!r_number_read_uinteger (reader, numer))
        goto fail;

    mpq_set_num (ureal, numer);

    if (!r_number_read_suffix (reader, &exponent))
        goto fail;

    apply_exponent (ureal, exponent);

    reader->exact = FALSE;
    success = TRUE;
    goto clear;

fail:
    success = REWIND;

clear:
    mpz_clear (numer);
    return success;
}

/**
 *  ureal_uint
 *      : uinteger
 *      ;
 */
rboolean r_number_read_ureal_uint (RNumberReader* reader, mpq_t ureal)
{
    mpz_t numer;
    rboolean success;

    MARK;
    mpz_init (numer);
    mpq_set_ui (ureal, 0u, 1u);

    if (!r_number_read_uinteger (reader, numer))
        goto fail;

    mpq_set_num (ureal, numer);

    success = TRUE;
    goto clear;

fail:
    success = REWIND;

clear:
    mpz_clear (numer);
    return success;
}

/**
 *  suffix
 *      : [esfdl] sign digit+
 *      ;
 */
rboolean r_number_read_suffix (RNumberReader* reader, rint* exponent)
{
    rint sign = 1;
    ruint digit;

    MARK;
    *exponent = 0;

    if (!strchr ("esfdl", LOOKAHEAD))
        return REWIND;

    CONSUME;

    r_number_read_sign (reader, &sign);

    if (!r_number_read_digit (reader, &digit))
        return REWIND;

    for (*exponent = digit; r_number_read_digit (reader, &digit); )
        *exponent = (*exponent) * 10 + digit;

    if (sign < 0)
        *exponent = -(*exponent);

    reader->exact = FALSE;
    return TRUE;
}

/**
 *  uinteger
 *      : digit+ !'.'
 *      ;
 */
rboolean r_number_read_uinteger (RNumberReader* reader, mpz_t uinteger)
{
    MARK;

    if (!r_number_read_digits (reader, uinteger))
        return REWIND;

    if ('.' == LOOKAHEAD)
        return REWIND;

    return TRUE;
}

rboolean r_number_read_digits (RNumberReader* reader, mpz_t digits)
{
    MARK;

    ruint digit;

    if (!r_number_read_digit (reader, &digit))
        return REWIND;

    mpz_set_ui (digits, digit);

    while (r_number_read_digit (reader, &digit)) {
        mpz_mul_ui (digits, digits, reader->radix);
        mpz_add_ui (digits, digits, digit);
    }

    return TRUE;
}

/**
 *  digit
 *      : [01]      { when (radix == 2) }
 *      / [0-7]     { when (radix == 8) }
 *      / [0-9]     { when (radix == 10) }
 *      / [0-9a-f]  { when (radix == 16) }
 *      ;
 */
rboolean r_number_read_digit (RNumberReader* reader, ruint* digit)
{
    if (!xdigit_to_uint (LOOKAHEAD, digit))
        return FALSE;

    if (*digit >= reader->radix)
        return FALSE;

    CONSUME;
    return TRUE;
}

/**
 *  sign
 *      : '+'
 *      / '-'
 *      ;
 */
rboolean r_number_read_sign (RNumberReader* reader, rint* sign)
{
    MARK;

    switch (NEXT) {
        case '+': *sign =  1; return TRUE;
        case '-': *sign = -1; return TRUE;
    }

    return REWIND;
}
