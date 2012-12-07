#ifndef __ROSE_PAIR_H__
#define __ROSE_PAIR_H__

#include "rose/sexp.h"

#include <stdarg.h>

R_BEGIN_DECLS

typedef struct RPair RPair;

rsexp r_cons         (RState* state,
                      rsexp   car,
                      rsexp   cdr);
rsexp r_car          (rsexp   obj);
rsexp r_cdr          (rsexp   obj);
rsexp r_set_car_x    (rsexp   pair,
                      rsexp   obj);
rsexp r_set_cdr_x    (rsexp   pair,
                      rsexp   obj);
rbool r_pair_equal_p (RState* state,
                      rsexp   lhs,
                      rsexp   rhs);
rsexp r_reverse      (rsexp   list);
rsexp r_append_x     (rsexp   list,
                      rsexp   value);
rbool r_list_p       (rsexp   obj);
rsexp r_vlist        (RState* state,
                      rsize   k,
                      va_list args);
rsexp r_list         (RState* state,
                      rsize   k,
                      ...);
rsize r_length       (rsexp   list);
rsexp r_list_ref     (rsexp   list,
                      rsize   k);

#define r_caar(obj)     r_car (r_car   (obj))
#define r_cadr(obj)     r_car (r_cdr   (obj))
#define r_cdar(obj)     r_cdr (r_car   (obj))
#define r_cddr(obj)     r_cdr (r_cdr   (obj))
#define r_caaar(obj)    r_car (r_caar  (obj))
#define r_caadr(obj)    r_car (r_cadr  (obj))
#define r_cadar(obj)    r_car (r_cdar  (obj))
#define r_caddr(obj)    r_car (r_cddr  (obj))
#define r_cdaar(obj)    r_cdr (r_caar  (obj))
#define r_cdadr(obj)    r_cdr (r_cadr  (obj))
#define r_cddar(obj)    r_cdr (r_cdar  (obj))
#define r_cdddr(obj)    r_cdr (r_cddr  (obj))
#define r_caaaar(obj)   r_car (r_caaar (obj))
#define r_caaadr(obj)   r_car (r_caadr (obj))
#define r_caadar(obj)   r_car (r_cadar (obj))
#define r_caaddr(obj)   r_car (r_caddr (obj))
#define r_cadaar(obj)   r_car (r_cdaar (obj))
#define r_cadadr(obj)   r_car (r_cdadr (obj))
#define r_caddar(obj)   r_car (r_cddar (obj))
#define r_cadddr(obj)   r_car (r_cdddr (obj))
#define r_cdaaar(obj)   r_cdr (r_caaar (obj))
#define r_cdaadr(obj)   r_cdr (r_caadr (obj))
#define r_cdadar(obj)   r_cdr (r_cadar (obj))
#define r_cdaddr(obj)   r_cdr (r_caddr (obj))
#define r_cddaar(obj)   r_cdr (r_cdaar (obj))
#define r_cddadr(obj)   r_cdr (r_cdadr (obj))
#define r_cdddar(obj)   r_cdr (r_cddar (obj))
#define r_cddddr(obj)   r_cdr (r_cdddr (obj))

R_END_DECLS

#endif  /* __ROSE_PAIR_H__ */
