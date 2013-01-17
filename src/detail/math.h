#ifndef __ROSE_DETAIL_MATH_H__
#define __ROSE_DETAIL_MATH_H__

#include "rose/math.h"

R_BEGIN_DECLS

rsexp negate_smi (RState* r, rsexp num);
rsexp negate_flo (RState* r, rsexp num);
rsexp negate_fix (RState* r, rsexp num);

rbool smi_sum_overflow_p (int lhs, int rhs, int sum);

rsexp add_smi_smi (RState* r, rsexp lhs, rsexp rhs);
rsexp add_smi_any (RState* r, rsexp lhs, rsexp rhs);

rsexp add_flo_smi (RState* r, rsexp lhs, rsexp rhs);
rsexp add_flo_flo (RState* r, rsexp lhs, rsexp rhs);
rsexp add_flo_any (RState* r, rsexp lhs, rsexp rhs);

rsexp add_fix_smi (RState* r, rsexp lhs, rsexp rhs);
rsexp add_fix_fix (RState* r, rsexp lhs, rsexp rhs);
rsexp add_fix_flo (RState* r, rsexp lhs, rsexp rhs);
rsexp add_fix_any (RState* r, rsexp lhs, rsexp rhs);

rbool smi_product_overflow_p (int lhs, int rhs, int sum);

rsexp multiply_smi_smi (RState* r, rsexp lhs, rsexp rhs);
rsexp multiply_smi_any (RState* r, rsexp lhs, rsexp rhs);

rsexp multiply_flo_smi (RState* r, rsexp lhs, rsexp rhs);
rsexp multiply_flo_flo (RState* r, rsexp lhs, rsexp rhs);
rsexp multiply_flo_any (RState* r, rsexp lhs, rsexp rhs);

rsexp multiply_fix_smi (RState* r, rsexp lhs, rsexp rhs);
rsexp multiply_fix_fix (RState* r, rsexp lhs, rsexp rhs);
rsexp multiply_fix_flo (RState* r, rsexp lhs, rsexp rhs);
rsexp multiply_fix_any (RState* r, rsexp lhs, rsexp rhs);

rsexp invert_smi (RState* r, rsexp num);
rsexp invert_fix (RState* r, rsexp num);
rsexp invert_flo (RState* r, rsexp num);

R_END_DECLS

#endif /* __ROSE_DETAIL_MATH_H__ */
