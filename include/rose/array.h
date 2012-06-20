#ifndef __ROSE_ARRAY_H__
#define __ROSE_ARRAY_H__

#include "rose/types.h"

typedef struct RArray RArray;

RArray*  r_array_new ();
RArray*  r_array_sized_new     (ruint    reserved_size);
void     r_array_free          (RArray*  array);
void     r_array_push_back_n   (RArray*  array,
                                rpointer values,
                                uint     n);
void     r_array_push_back     (RArray*  array,
                                rpointer value);
void     r_array_pop_back      (RArray*  array);
void     r_array_push_front_n  (RArray*  array,
                                rpointer values,
                                uint     n);
void     r_array_push_front    (RArray*  array,
                                rpointer value);
uint     r_array_size          (RArray*  array);
rpointer r_array_get_element   (RArray*  array,
                                uint     index);
void     r_array_set_element_x (RArray*  array,
                                uint     index,
                                rpointer value);
void     r_array_clear         (RArray*  array);


#endif  //  __ROSE_ARRAY_H__
