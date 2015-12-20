#ifndef __ROSE_STRING_H__
#define __ROSE_STRING_H__

#include "rose/sexp.h"
#include "rose/state.h"

#include <stdarg.h>

R_BEGIN_DECLS

rsexp r_string_new (RState* r, rconstcstring str);

rsexp r_make_string (RState* r, rsize k, rchar ch);

rbool r_string_p (rsexp obj);

rsexp r_string_ref (RState* r, rsexp obj, rsize k);

rsexp r_string_set_x (RState* r, rsexp obj, rsize k, rchar ch);

rconstcstring r_string_to_cstr (rsexp obj);

rbool r_string_equal_p (RState* r, rsexp lhs, rsexp rhs);

rsexp r_string_length_by_byte (rsexp obj);

rsexp r_string_length (rsexp obj);

rsexp r_string_format (RState* r, rconstcstring format, ...);

rsexp r_string_vformat (RState* r, rconstcstring format, va_list args);

rsexp r_string_printf (RState* r, rconstcstring format, ...);

rsexp r_string_vprintf (RState* r, rconstcstring format, va_list args);

R_END_DECLS

#endif /* __ROSE_STRING_H__ */
