from os import environ

base = Environment(tools=['default', 'quex'],
                   ENV=environ,
                   LIBS=['gc', 'gmp', 'm'],
                   CPPFLAGS='-ggdb -Wall',
                   CPPPATH=['#include/',
                            '#build/src/',
                            environ['QUEX_PATH']])

clang = base.Clone(CC='clang', CXX='clang++')
gcc   = base.Clone(CC='gcc',   CXX='g++')
env   = clang

rose_lib = env.SConscript(dirs='src',
                          exports='env',
                          variant_dir='build/src/')

rsi_bin = env.SConscript(dirs='tools/rsi',
                         exports=['env', 'rose_lib'],
                         variant_dir='build/tools/rsi/')

test = env.SConscript(dirs='test',
                      exports=['env', 'rose_lib'],
                      variant_dir='build/test/')

env.Depends(target=rsi_bin, dependency=rose_lib)
env.Depends(target=test,    dependency=rose_lib)

env.Install(target='lib/', source=rose_lib)
env.Install(target='bin/', source=rsi_bin)


# vim:ft=python
