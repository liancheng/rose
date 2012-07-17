# Naming Conventions

These naming conventions are basically borrowed from GLib, with some personal
flavor.  Summarized as bellow.

## Type Names

### Simple types

Use lowercase with prefix `r`, no `_` between words.  For example:

    rword
    rsexp
    rpointer
    rconstpointer

### Structure types

Use `CamelCase` with prefix `R`.  All structures should be typedef'ed.  For
example:

    typedef struct RPair RPair;
    typedef struct RHashTable RHashTable;

Variable Names
--------------

Use lowercase, words should be separated by `_`.  For example::

    global_env
    hash_value
    prompt_string

**Die Hungarian notation! Die!**

## Function Names

### General rule

Function names use lowercase, words should be separated by `_`.  Public
interface function names must be prefixed with `r_`.

### "Object methods"

Almost all ADT (abstract data type) structures in ROSE have "method" functions.
For example, the ADT `RHashTable` has these methods:

    RHashTable* r_hash_table_new  ();
    void        r_hash_table_free (RHashTable*);
    rpointer    r_hash_table_get  (RHashTable*, rconstpointer);
    void        r_hash_table_put  (RHashTable*, rpointer, rpointer);
    ...

A C++ equivalence for the hash table case can be:

    class RHashTable {
    public:
        RHashTable ();
        ~RHashTable ();
        rpointer get (rconstpointer);
        void put (rpointer, rpointer);
    };

There, you see the rule:

*   All functions share a prefix converted directly from the ADT name.
*   "Constructor" is named `<prefix>_new`.
*   "Destructor" is named `<prefixed>_free`.

Macro Names
-----------

Use uppercase, with `_` as word separator.  Macros that appear as public
interface must be named with the `R_` prefix.
