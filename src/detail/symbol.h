#ifndef __ROSE_DETAIL_SYMBOL_H__
#define __ROSE_DETAIL_SYMBOL_H__

#include "rose/sexp.h"
#include "rose/symbol.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

#define quark_to_sexp(q)    (r_cast (rsexp, r_set_tag_x ((q), R_TAG_SYMBOL)))
#define quark_from_sexp(q)  (r_cast (GQuark, ((q) >> R_TAG_BITS)))

#define R_QUOTE             quark_to_sexp (1)
#define R_LAMBDA            quark_to_sexp (2)
#define R_IF                quark_to_sexp (3)
#define R_SET_X             quark_to_sexp (4)
#define R_QUASIQUOTE        quark_to_sexp (5)
#define R_DEFINE            quark_to_sexp (6)
#define R_UNQUOTE           quark_to_sexp (7)
#define R_UNQUOTE_SPLICING  quark_to_sexp (8)
#define R_CALL_CC           quark_to_sexp (9)

#define R_OP_APPLY          quark_to_sexp (10)
#define R_OP_ARG            quark_to_sexp (11)
#define R_OP_ASSIGN         quark_to_sexp (12)
#define R_OP_BRANCH         quark_to_sexp (13)
#define R_OP_CAPTURE_CC     quark_to_sexp (14)
#define R_OP_CLOSE          quark_to_sexp (15)
#define R_OP_CONSTANT       quark_to_sexp (16)
#define R_OP_BIND           quark_to_sexp (17)
#define R_OP_FRAME          quark_to_sexp (18)
#define R_OP_HALT           quark_to_sexp (19)
#define R_OP_REFER          quark_to_sexp (20)
#define R_OP_RESTORE_CC     quark_to_sexp (21)
#define R_OP_RETURN         quark_to_sexp (22)

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_DETAIL_SYMBOL_H__ */
