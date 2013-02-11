#ifndef __ROSE_PAIR_H__
#define __ROSE_PAIR_H__

#include "rose/sexp.h"

#include <stdarg.h>

R_BEGIN_DECLS

typedef rsexp (*RBinaryFunc) (RState*, rsexp, rsexp);

rbool r_pair_p       (rsexp obj);
rsexp r_cons         (RState* r,
                      rsexp car,
                      rsexp cdr);
rsexp r_car          (rsexp obj);
rsexp r_cdr          (rsexp obj);
void  r_set_car_x    (rsexp pair,
                      rsexp obj);
void  r_set_cdr_x    (rsexp pair,
                      rsexp obj);
rbool r_pair_equal_p (RState* r,
                      rsexp lhs,
                      rsexp rhs);
rsexp r_reverse      (RState* r,
                      rsexp list);
rsexp r_reverse_x    (RState* r,
                      rsexp list);
rsexp r_append_x     (RState* r,
                      rsexp list,
                      rsexp value);
rbool r_list_p       (rsexp obj);
rsexp r_vlist        (RState* r,
                      rsize k,
                      va_list args);
rsexp r_list         (RState* r,
                      rsize k,
                      ...);
rsexp r_length       (RState* r,
                      rsexp list);
rsexp r_list_ref     (RState* r,
                      rsexp list,
                      rsize k);
rsexp r_fold         (RState* r,
                      RBinaryFunc proc,
                      rsexp nil,
                      rsexp list);
rsexp r_list_copy    (RState* r,
                      rsexp list);
rsexp r_last_pair    (RState* r,
                      rsexp list);

#define r_caar(o)   r_car (r_car   ((o)))
#define r_cadr(o)   r_car (r_cdr   ((o)))
#define r_cdar(o)   r_cdr (r_car   ((o)))
#define r_cddr(o)   r_cdr (r_cdr   ((o)))
#define r_caaar(o)  r_car (r_caar  ((o)))
#define r_caadr(o)  r_car (r_cadr  ((o)))
#define r_cadar(o)  r_car (r_cdar  ((o)))
#define r_caddr(o)  r_car (r_cddr  ((o)))
#define r_cdaar(o)  r_cdr (r_caar  ((o)))
#define r_cdadr(o)  r_cdr (r_cadr  ((o)))
#define r_cddar(o)  r_cdr (r_cdar  ((o)))
#define r_cdddr(o)  r_cdr (r_cddr  ((o)))
#define r_caaaar(o) r_car (r_caaar ((o)))
#define r_caaadr(o) r_car (r_caadr ((o)))
#define r_caadar(o) r_car (r_cadar ((o)))
#define r_caaddr(o) r_car (r_caddr ((o)))
#define r_cadaar(o) r_car (r_cdaar ((o)))
#define r_cadadr(o) r_car (r_cdadr ((o)))
#define r_caddar(o) r_car (r_cddar ((o)))
#define r_cadddr(o) r_car (r_cdddr ((o)))
#define r_cdaaar(o) r_cdr (r_caaar ((o)))
#define r_cdaadr(o) r_cdr (r_caadr ((o)))
#define r_cdadar(o) r_cdr (r_cadar ((o)))
#define r_cdaddr(o) r_cdr (r_caddr ((o)))
#define r_cddaar(o) r_cdr (r_cdaar ((o)))
#define r_cddadr(o) r_cdr (r_cdadr ((o)))
#define r_cdddar(o) r_cdr (r_cddar ((o)))
#define r_cddddr(o) r_cdr (r_cdddr ((o)))

R_END_DECLS

#endif /* __ROSE_PAIR_H__ */
