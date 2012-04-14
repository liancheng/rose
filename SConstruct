from os import environ

gcc = Environment(tools=['default', 'quex'],
                  ENV=environ,
                  CFLAGS='-ggdb -Wall',
                  CPPPATH=['#include/',
                           '#build/src/',
                           environ['QUEX_PATH']])

clang = gcc.Clone(CC='clang', CXX='clang++')


env = clang;

env.ParseConfig('pkg-config --cflags --libs glib-2.0')

env.SConscript(dirs='src',
               exports='env',
               variant_dir='build/src/')

env.SConscript(dirs='test',
               exports='env',
               variant_dir='build/test/')

# vim:ft=python
