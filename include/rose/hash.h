#ifndef __ROSE_HASH_TABLE_H__
#define __ROSE_HASH_TABLE_H__

#include "rose/types.h"

typedef struct RHashTable RHashTable;

typedef ruint    (*RHashFunction)  (rconstpointer data);
typedef rboolean (*REqualFunction) (rconstpointer lhs,
                                    rconstpointer rhs);
typedef void     (*RDestructor)    (rconstpointer data);

RHashTable* r_hash_table_new      (RHashFunction  hash_fn,
                                   REqualFunction equal_fn);
RHashTable* r_hash_table_new_full (RHashFunction  hash_fn,
                                   REqualFunction equal_fn,
                                   RDestructor    key_destroy_fn,
                                   RDestructor    value_destroy_fn);
void        r_hash_table_free     (RHashTable*    hash_table);
rpointer    r_hash_table_get      (RHashTable*    hash_table,
                                   rconstpointer  key);
void        r_hash_table_put      (RHashTable*    hash_table,
                                   rpointer       key,
                                   rpointer       value);
rboolean    r_hash_table_delete   (RHashTable*    hash_table,
                                   rconstpointer  key);
void        r_hash_table_clear    (RHashTable*    hash_table);

#endif  //  __ROSE_HASH_TABLE_H__
