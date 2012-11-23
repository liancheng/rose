#ifndef __ROSE_HASH_TABLE_H__
#define __ROSE_HASH_TABLE_H__

#include "rose/types.h"

typedef struct RHashTable RHashTable;

typedef ruint (*RHashFunc)      (rconstpointer data);
typedef rbool (*RHashEqualPred) (rconstpointer lhs,
                                 rconstpointer rhs);
typedef void  (*RHashDestruct)  (rconstpointer data);

RHashTable* r_hash_table_new      (RHashFunc      hash_fn,
                                   RHashEqualPred equal_fn);
RHashTable* r_hash_table_new_full (RHashFunc      hash_fn,
                                   RHashEqualPred equal_fn,
                                   RHashDestruct  key_destroy_fn,
                                   RHashDestruct  value_destroy_fn);
void        r_hash_table_free     (RHashTable*    hash_table);
rpointer    r_hash_table_get      (RHashTable*    hash_table,
                                   rconstpointer  key);
void        r_hash_table_put      (RHashTable*    hash_table,
                                   rpointer       key,
                                   rpointer       value);
rbool       r_hash_table_delete   (RHashTable*    hash_table,
                                   rconstpointer  key);
void        r_hash_table_clear    (RHashTable*    hash_table);

#endif  /* __ROSE_HASH_TABLE_H__ */
