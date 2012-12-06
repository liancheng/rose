from os import environ

env = Environment(tools=['default', 'quex'],
                  ENV=environ,
                  CC='clang',
                  CXX='clang++',
                  LIBS=['gc', 'gmp', 'm'],
                  CPPFLAGS='-ggdb -Wall -Werror',
                  CPPDEFINES='_GNU_SOURCE',
                  CPPPATH=['#include/',
                           '#build/src/',
                           environ['QUEX_PATH']])

env.MergeFlags("!pkg-config glib-2.0 --cflags --libs")

rose_lib = env.SConscript(dirs='src',
                          exports='env',
                          variant_dir='build/src/')

rsi_bin = env.SConscript(dirs='tools/rsi/',
                         exports=['env', 'rose_lib'],
                         variant_dir='build/tools/rsi/')

test = env.SConscript(dirs='test',
                      exports=['env', 'rose_lib'],
                      variant_dir='build/test/')

env.Depends(target=rsi_bin,
            dependency=rose_lib)

env.Install(target='lib/',
            source=rose_lib)

env.Depends(target=test,
            dependency=rose_lib)

env.Install(target='bin/',
            source=rsi_bin)

# vim:ft=python
