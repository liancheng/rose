#include "rose/array.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct RArray {
    ruint     size;
    ruint     capacity;
    rpointer* data;
};

static void r_array_expand (RArray* array, ruint size)
{
    if (size < array->capacity)
        return;

    while (array->capacity < size)
        array->capacity <<= 1;

    array->data = realloc (array->data, array->capacity);
}

RArray* r_array_new ()
{
    RArray* array = malloc (sizeof (RArray));

    array->size     = 0u;
    array->capacity = 0u;
    array->data     = NULL;

    return array;
}

RArray* r_array_sized_new (ruint reserved_size)
{
    RArray* array = malloc (sizeof (RArray));
    uint    shift = 0u;

    while ((1u << shift++) < reserved_size)
        ;

    array->size     = 0u;
    array->capacity = 1u << shift;
    array->data     = malloc (sizeof (rpointer) * array->capacity);

    return array;
}

void r_array_free (RArray* array)
{
    free (array->data);
    free (array);
}

void r_array_push_back_n (RArray* array, rpointer values, uint n)
{
    r_array_expand (array, array->size + n);
    memcpy (array->data + array->size, values, sizeof (rpointer) * n);
    array->size += n;
}

void r_array_push_back (RArray* array, rpointer value)
{
    r_array_push_back_n (array, &value, 1u);
}

void r_array_pop_back (RArray* array)
{
    if (0 == array->size)
        return;

    array->size--;
}

void r_array_push_front_n (RArray* array, rpointer values, uint n)
{
    r_array_expand (array, array->size + n);
    memmove (array->data, array->data + n, sizeof (rpointer) * array->size);
    memcpy (array->data, values, sizeof (rpointer) * n);
    array->size += n;
}

void r_array_push_front (RArray* array, rpointer value)
{
    r_array_push_front_n (array, &value, 1u);
}

uint r_array_size (RArray* array)
{
    return array->size;
}

rpointer r_array_get_element (RArray* array, uint index)
{
    assert (index < array->size);
    return array->data [index];
}

void r_array_set_element_x (RArray* array, uint index, rpointer value)
{
    assert (index < array->size);
    array->data [index] = value;
}

void r_array_clear (RArray* array)
{
    free (array->data);
    array->size = 0u;
    array->capacity = 0u;
}
