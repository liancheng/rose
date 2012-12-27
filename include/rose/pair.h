#ifndef __ROSE_PAIR_H__
#define __ROSE_PAIR_H__

#include "rose/sexp.h"

#include <stdarg.h>

R_BEGIN_DECLS

rbool r_pair_p       (rsexp   obj);
rsexp r_cons         (RState* state,
                      rsexp   car,
                      rsexp   cdr);
rsexp r_car          (rsexp   obj);
rsexp r_cdr          (rsexp   obj);
rsexp r_checked_car  (RState* state,
                      rsexp   obj);
rsexp r_checked_cdr  (RState* state,
                      rsexp   obj);
void  r_set_car_x    (rsexp   pair,
                      rsexp   obj);
void  r_set_cdr_x    (rsexp   pair,
                      rsexp   obj);
rbool r_pair_equal_p (RState* state,
                      rsexp   lhs,
                      rsexp   rhs);
rsexp r_reverse      (RState* state,
                      rsexp   list);
rsexp r_append_x     (RState* state,
                      rsexp   list,
                      rsexp   value);
rbool r_list_p       (rsexp   obj);
rsexp r_vlist        (RState* state,
                      rsize   k,
                      va_list args);
rsexp r_list         (RState* state,
                      rsize   k,
                      ...);
rsexp r_length       (RState* state,
                      rsexp   list);
rsexp r_list_ref     (rsexp   list,
                      rsize   k);

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

#define r_checked_caar(s, o)   r_checked_car ((s), r_checked_car   ((s), (o)))
#define r_checked_cadr(s, o)   r_checked_car ((s), r_checked_cdr   ((s), (o)))
#define r_checked_cdar(s, o)   r_checked_cdr ((s), r_checked_car   ((s), (o)))
#define r_checked_cddr(s, o)   r_checked_cdr ((s), r_checked_cdr   ((s), (o)))
#define r_checked_caaar(s, o)  r_checked_car ((s), r_checked_caar  ((s), (o)))
#define r_checked_caadr(s, o)  r_checked_car ((s), r_checked_cadr  ((s), (o)))
#define r_checked_cadar(s, o)  r_checked_car ((s), r_checked_cdar  ((s), (o)))
#define r_checked_caddr(s, o)  r_checked_car ((s), r_checked_cddr  ((s), (o)))
#define r_checked_cdaar(s, o)  r_checked_cdr ((s), r_checked_caar  ((s), (o)))
#define r_checked_cdadr(s, o)  r_checked_cdr ((s), r_checked_cadr  ((s), (o)))
#define r_checked_cddar(s, o)  r_checked_cdr ((s), r_checked_cdar  ((s), (o)))
#define r_checked_cdddr(s, o)  r_checked_cdr ((s), r_checked_cddr  ((s), (o)))
#define r_checked_caaaar(s, o) r_checked_car ((s), r_checked_caaar ((s), (o)))
#define r_checked_caaadr(s, o) r_checked_car ((s), r_checked_caadr ((s), (o)))
#define r_checked_caadar(s, o) r_checked_car ((s), r_checked_cadar ((s), (o)))
#define r_checked_caaddr(s, o) r_checked_car ((s), r_checked_caddr ((s), (o)))
#define r_checked_cadaar(s, o) r_checked_car ((s), r_checked_cdaar ((s), (o)))
#define r_checked_cadadr(s, o) r_checked_car ((s), r_checked_cdadr ((s), (o)))
#define r_checked_caddar(s, o) r_checked_car ((s), r_checked_cddar ((s), (o)))
#define r_checked_cadddr(s, o) r_checked_car ((s), r_checked_cdddr ((s), (o)))
#define r_checked_cdaaar(s, o) r_checked_cdr ((s), r_checked_caaar ((s), (o)))
#define r_checked_cdaadr(s, o) r_checked_cdr ((s), r_checked_caadr ((s), (o)))
#define r_checked_cdadar(s, o) r_checked_cdr ((s), r_checked_cadar ((s), (o)))
#define r_checked_cdaddr(s, o) r_checked_cdr ((s), r_checked_caddr ((s), (o)))
#define r_checked_cddaar(s, o) r_checked_cdr ((s), r_checked_cdaar ((s), (o)))
#define r_checked_cddadr(s, o) r_checked_cdr ((s), r_checked_cdadr ((s), (o)))
#define r_checked_cdddar(s, o) r_checked_cdr ((s), r_checked_cddar ((s), (o)))
#define r_checked_cddddr(s, o) r_checked_cdr ((s), r_checked_cdddr ((s), (o)))

R_END_DECLS

#endif  /* __ROSE_PAIR_H__ */
