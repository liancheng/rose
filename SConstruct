from os import environ

base = Environment(tools=['default', 'quex'],
                   ENV=environ,
                   CFLAGS='-ggdb -Wall',
                   CPPPATH=['#include/',
                            '#build/src/',
                            environ['QUEX_PATH']])

clang = base.Clone(CC='clang', CXX='clang++')
gcc   = base.Clone(CC='gcc',   CXX='g++')
env   = clang;

rsi_bin = env.SConscript(dirs='src',
                         exports='env',
                         variant_dir='build/src/')

env.Install(target='bin/',
            source=rsi_bin)

# vim:ft=python
