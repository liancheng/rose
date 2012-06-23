#include "detail/hash.h"

#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_MINIMUM_SHIFT 3
#define UNUSED_HASH_VALUE        0
#define TOMBSTONE_HASH_VALUE     1
#define UNUSED_HASH_P(h)         (UNUSED_HASH_VALUE == h)
#define TOMBSTONE_HASH_P(h)      (TOMBSTONE_HASH_VALUE == h)
#define REAL_HASH_P(h)           ((h) > 1)


struct RHashTable {
    rsize          size;
    rsize          n_occupied;
    rsize          n_nodes;
    ruint          mod;
    ruint          mask;

    RHashFunction  hash_fn;
    REqualFunction key_equal_fn;
    RDestructor    key_destructor;
    RDestructor    value_destructor;

    ruint*         hashes;
    rpointer*      keys;
    rpointer*      values;
};

static const size_t prime_mod [] = {
    1,
    2,
    3,
    7,
    13,
    31,
    61,
    127,
    251,
    509,
    1021,
    2039,
    4093,
    8191,
    16381,
    32749,
    65521,
    131071,
    262139,
    524287,
    1048573,
    2097143,
    4194301,
    8388593,
    16777213,
    33554393,
    67108859,
    134217689,
    268435399,
    536870909,
    1073741789,
    2147483647
};

static ruint r_direct_hash (rconstpointer data)
{
    return (ruint)data;
}

static rint r_direct_euqal (rconstpointer lhs, rconstpointer rhs)
{
    return lhs == rhs;
}

static void r_hash_table_set_shift (RHashTable* hash_table, rint shift)
{
    ruint i;
    ruint mask;

    hash_table->size = 1 << shift;
    hash_table->mod = prime_mod [shift];

    for (i = 0; i < shift; ++i) {
        mask <<= 1;
        mask |= 1;
    }

    hash_table->mask = mask;
}

static rint r_find_closest_shift (rint n)
{
    rint i;

    for (i = 0; n; ++i)
        n >>= 1;

    return i;
}

static void r_hash_table_set_shift_from_size (RHashTable* hash_table,
                                              rint        size)
{
    rint shift = r_find_closest_shift (size);

    if (shift < HASH_TABLE_MINIMUM_SHIFT)
        shift = HASH_TABLE_MINIMUM_SHIFT;

    r_hash_table_set_shift (hash_table, shift);
}

static rsize r_hash_table_get_node (RHashTable*   hash_table,
                                    rconstpointer key,
                                    ruint*        hash_return)
{
    ruint hash_value      = hash_table->hash_fn (key);
    rsize node_index      = hash_value % hash_table->mod;
    ruint node_hash       = hash_table->hashes [node_index];
    rsize step            = 0;
    rint  have_tombstone  = 0;
    ruint first_tombstone = 0;

    if (UNUSED_HASH_VALUE == hash_value)
        hash_value = 2;

    *hash_return = hash_value;

    while (!UNUSED_HASH_P (node_hash)) {
        if (hash_value == node_hash) {
            if (hash_table->key_equal_fn (key, hash_table->keys [node_index]))
                return node_index;
        }
        else if (TOMBSTONE_HASH_P (node_hash) && !have_tombstone) {
            first_tombstone = node_index;
            have_tombstone = 1;
        }

        // Linear probing
        ++step;
        node_index += step;
        node_index &= hash_table->mask;
        node_hash   = hash_table->hashes [node_index];
    }

    if (have_tombstone)
        return first_tombstone;

    return node_index;
}

static void r_hash_table_resize (RHashTable* hash_table)
{
    rsize     old_size;
    ruint*    new_hashes;
    rpointer* new_keys;
    rpointer* new_values;
    rsize     i;

    old_size = hash_table->size;
    r_hash_table_set_shift_from_size (hash_table,
                                      hash_table->n_nodes * 2);

    new_hashes = malloc (sizeof (ruint) * hash_table->size);
    new_keys   = malloc (sizeof (rpointer) * hash_table->size);

    if (hash_table->keys == hash_table->values)
        new_values = new_keys;
    else
        new_values = malloc (sizeof (rpointer) * hash_table->size);

    for (i = 0; i < old_size; ++i) {
        ruint node_hash = hash_table->hashes [i];
        rsize node_index = node_hash % hash_table->mod;
        ruint step = 0;

        if (REAL_HASH_P (node_hash))
            continue;

        while (!UNUSED_HASH_P (new_hashes [node_index])) {
            ++step;
            node_index += step;
            node_index &= hash_table->mask;
        }

        new_hashes [node_index] = node_hash;
        new_keys   [node_index] = hash_table->keys [i];
        new_values [node_index] = hash_table->values [i];
    }

    if (hash_table->values != hash_table->keys)
        free (hash_table->values);

    free (hash_table->keys);
    free (hash_table->hashes);

    hash_table->hashes = new_hashes;
    hash_table->keys   = new_keys;
    hash_table->values = new_values;

    hash_table->n_occupied = hash_table->n_nodes;
}

static void r_hash_table_maybe_resize (RHashTable* hash_table)
{
    rsize occupied = hash_table->n_occupied;
    rsize size     = hash_table->size;

    if ((size > hash_table->n_nodes * 4 &&
         size > 1 << HASH_TABLE_MINIMUM_SHIFT) ||
        (size <= occupied + occupied / 16))
    {
        r_hash_table_resize (hash_table);
    }
}

static void r_hash_table_put_node (RHashTable* hash_table,
                                   rsize       node_index,
                                   ruint       key_hash,
                                   rpointer    key,
                                   rpointer    value)
{
    if (hash_table->keys == hash_table->values && key != value) {
        hash_table->values = malloc (sizeof (rpointer) * hash_table->size);
        memcpy (hash_table->values,
                hash_table->keys,
                sizeof (rpointer) * hash_table->size);
    }

    ruint    old_hash  = hash_table->hashes [node_index];
    rpointer old_key   = hash_table->keys   [node_index];
    rpointer old_value = hash_table->values [node_index];

    if (REAL_HASH_P (old_hash)) {
        hash_table->values [node_index] = value;
    }
    else {
        hash_table->hashes [node_index] = key_hash;
        hash_table->keys   [node_index] = key;
        hash_table->values [node_index] = value;

        ++hash_table->n_nodes;

        if (UNUSED_HASH_P (old_hash)) {
            ++hash_table->n_occupied;
            r_hash_table_maybe_resize (hash_table);
        }
    }

    if (REAL_HASH_P (old_hash)) {
        if (hash_table->key_destructor)
            hash_table->key_destructor (old_key);

        if (hash_table->value_destructor)
            hash_table->value_destructor (old_value);
    }
}

RHashTable* r_hash_table_new (RHashFunction  hash_fn,
                              REqualFunction equal_fn)
{
    return r_hash_table_new_full (hash_fn, equal_fn, NULL, NULL);
}

RHashTable* r_hash_table_new_full (RHashFunction  hash_fn,
                                   REqualFunction equal_fn,
                                   RDestructor    key_destructor,
                                   RDestructor    value_destructor)
{
    RHashTable* hash_table = malloc (sizeof (RHashTable));

    r_hash_table_set_shift (hash_table, HASH_TABLE_MINIMUM_SHIFT);

    hash_table->n_occupied       = 0;
    hash_table->n_nodes          = 0;
    hash_table->hash_fn          = hash_fn  ? hash_fn  : r_direct_hash;
    hash_table->key_equal_fn     = equal_fn ? equal_fn : r_direct_euqal;
    hash_table->key_destructor   = key_destructor;
    hash_table->value_destructor = value_destructor;

    hash_table->hashes = malloc (sizeof (ruint) * hash_table->size);
    hash_table->keys   = malloc (sizeof (rpointer) * hash_table->size);
    hash_table->values = hash_table->keys;

    return hash_table;
}

void r_hash_table_free (RHashTable* hash_table)
{
    r_hash_table_clear (hash_table);
    free (hash_table);
}

rpointer r_hash_table_get (RHashTable*   hash_table,
                           rconstpointer key)
{
    ruint hash_value;
    rsize node_index;

    node_index = r_hash_table_get_node (hash_table, key, &hash_value);

    return REAL_HASH_P (hash_table->hashes [node_index])
           ? hash_table->values [node_index]
           : NULL;
}

void r_hash_table_put (RHashTable* hash_table,
                       rpointer    key,
                       rpointer    value)
{
    ruint hash_value;
    rsize node_index;

    node_index = r_hash_table_get_node (hash_table, key, &hash_value);

    r_hash_table_put_node (hash_table, node_index, hash_value, key, value);
}

static void r_hash_table_delete_node (RHashTable*   hash_table,
                                      rconstpointer key,
                                      rsize         node_index)
{
    rconstpointer old_key   = hash_table->keys   [node_index];
    rconstpointer old_value = hash_table->values [node_index];

    hash_table->hashes [node_index] = TOMBSTONE_HASH_VALUE;
    hash_table->keys   [node_index] = NULL;
    hash_table->values [node_index] = NULL;

    --hash_table->n_occupied;

    if (hash_table->key_destructor)
        hash_table->key_destructor (old_key);

    if (hash_table->value_destructor)
        hash_table->value_destructor (old_value);
}

rboolean r_hash_table_delete (RHashTable*   hash_table,
                              rconstpointer key)
{
    ruint node_hash;
    rsize node_index;

    node_index = r_hash_table_get_node (hash_table, key, &node_hash);

    if (!REAL_HASH_P (hash_table->hashes[node_index]))
        return FALSE;

    r_hash_table_delete_node (hash_table, key, node_index);
    r_hash_table_maybe_resize (hash_table);

    return TRUE;
}

void r_hash_table_clear (RHashTable* hash_table)
{
    rsize i;

    hash_table->n_nodes = 0;
    hash_table->n_occupied = 0;

    for (i = 0; i < hash_table->size; ++i) {
        if (REAL_HASH_P (hash_table->hashes[i])) {
            if (hash_table->key_destructor)
                hash_table->key_destructor (hash_table->keys[i]);

            if (hash_table->value_destructor)
                hash_table->value_destructor (hash_table->values[i]);

            hash_table->hashes [i] = UNUSED_HASH_VALUE;
            hash_table->keys   [i] = NULL;
            hash_table->values [i] = NULL;
        }
        else if (TOMBSTONE_HASH_P (hash_table->hashes[i]))
            hash_table->hashes [i] = UNUSED_HASH_VALUE;
    }

    r_hash_table_maybe_resize (hash_table);
}
