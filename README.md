# Overview

As the name suggests, ROSE is an Obscure Scheme Evaluator.

Please refer to the [Wiki pages][wiki] for details.

## How to build ROSE on Ubuntu

### Install dependencies

ROSE depends on the following tools and libraries:

*   [Quex][quex], the lexer generator, v0.63.1 or newer
*   [SCons][scons], the construction tool, v2.1.0 or newer
*   [GLib][glib], the GNOME Library, v2.0 or newer
*   [GMP][gmp], the GNU Multiple Precesion Arithmetic Library, v5.0.2 or newer
*   [googletest][gtest], Google C++ Testing Framework, v1.6 or newer

Install all the dependencies other than Quex and googletest with the following command:

    $ sudo aptitude install libgmp-dev libglib2.0-dev scons

To install Quex, please refer to the appendix.

To setup googletest, please refer to [the official documentation of googletest][gtest-setup].

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
    #<unspecified>
    $

## Appendix

### Quex installation

Download Quex from the [download page][quex-dl].  You may find various kind of package types, choose the one that fits your system.

Take the tarball of Quex v0.63.1 as an example, extract the tarball to somewhere, for instance, `~/local`:

    $ tar xzf quex-0.63.1.tar.gz -C ~/local

Then you'll find a directory named `quex-0.63.1` under `~/local`.  Create a new envrionment variable `QUEX_HOME`, set `$HOME/local/quex-0.63.1` as its value:

    $ export QUEX_HOME=$HOME/local/quex-0.63.1

Setup the `quex` command:

    $ chmod +x $QUEX_HOME/quex-exe.py
    $ sudo ln -sf $QUEX_HOME/quex-exe.py quex

Now test `quex` from terminal:

    $ quex --version
    Quex - Fast Universal Lexical Analyzer Generator
    Version 0.63.1
    (C) 2005-2012 Frank-Rene Schaefer
    ABSOLUTELY NO WARRANTY

For the Deb and RPM packages, set `QUEX_HOME` to `/opt/quex/quex-<version>`.  For details, please refer to [Quex's online documentation][quex-doc].

[wiki]: https://github.com/liancheng/rose/wiki
[quex]: http://quex.sourceforge.net
[quex-dl]: http://sourceforge.net/projects/quex/files/DOWNLOAD/
[quex-doc]: http://quex.sourceforge.net/doc/html/intro/installation.html
[scons]: http://www.scons.org
[glib]: http://developer.gnome.org/glib/
[gmp]: http://gmplib.org/
[gtest]: http://code.google.com/p/googletest/
[gtest-setup]: http://code.google.com/p/googletest/wiki/V1_6_Primer#Setting_up_a_New_Test_Project
