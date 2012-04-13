from os import environ

gcc = Environment(tools=['default', 'quex'],
                  ENV=environ,
                  CPPPATH=['#include/',
                           '#build/src/',
                           environ['QUEX_PATH']])

clang = gcc.Clone(CC='clang', CXX='clang++')

env = clang;

env.SConscript(dirs='src',
               exports='env',
               variant_dir='build/src/')

env.SConscript(dirs='test',
               exports='env',
               variant_dir='build/test/')

# vim:ft=python
