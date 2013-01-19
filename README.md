# Overview

As the name suggests, ROSE is an Obscure Scheme Evaluator.  Please refer to the [Wiki pages][wiki] for details.

**NOTE**: ROSE is still in an very early stage, the interpreter is barely runnable, many features required by R5RS are missing.

## How to build ROSE on Ubuntu

### Install dependencies

ROSE depends on the following tools and libraries:

*   [Quex][quex], the lexer generator, v0.63.2 or newer
*   [SCons][scons], the construction tool, v2.1.0 or newer
*   [GLib][glib], the GNOME Library, v2.0 or newer
*   [GMP][gmp], the GNU Multiple Precesion Arithmetic Library, v5.0.2 or newer
*   [googletest][gtest], Google C++ Testing Framework, v1.6 or newer

Quex and googletest have already been included in the `external` directory.  All the other dependencies can be installed with the following command:

    $ sudo aptitude install libgmp-dev libglib2.0-dev scons

### Build

Check out the code and run `scons` to build ROSE:

    $ git clone git://github.com/liancheng/rose.git
    $ cd rose
    $ scons

After the building process, you may find the ROSE interpreter executable `rsi` under `bin`:

    $ ./bin/rsi
    (display "hello world\n")
    ^D
    hello world

Here is the factorial example (input is read from the terminal):

    $ ./bin/rsi test/script/factorial.scm
    42
    1405006117752879898543142606244511569936384000000000

You may also try the famous yin-yang script:

    $ ./bin/rsi test/script/yin-yang.scm
    @*@**@***@****@*****@******@*******...

[wiki]: https://github.com/liancheng/rose/wiki
[quex]: http://quex.sourceforge.net
[scons]: http://www.scons.org
[glib]: http://developer.gnome.org/glib/
[gmp]: http://gmplib.org/
[gtest]: http://code.google.com/p/googletest/
