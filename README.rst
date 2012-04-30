Naming Conventions
==================

These naming conventions are borrowed from GLib.  Summarized as bellow.

Type Names
----------

Simple typedef types

    Use lowercase names with the ``r`` prefix, no ``_`` between words.  For example::

        rword
        rsexp
        rpointer
        rconstpointer

Compound structure types

    Use ``CamelCase`` names with the ``R`` prefix.  For example::

        RContext
        RError
        RString
        RVector

Variable Names
--------------

Use lowercase names, words should be separated by ``_``.  For example::

    global_env
    hash_value
    prompt_string

.. note::

    No Hungarian notation!  Please!

Function Names
--------------

Operations on compund structured types

    Use lowercase names, prefixed with ``r_``, and words should be separated by ``_``.  For example::

        r_context_new

Macro Names
-----------

Uppercase names with ``_`` as word separator.

.. vim: ft=rst ts=4 sw=4 et wrap
