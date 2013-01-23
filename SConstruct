from os import environ

environ['QUEX_PATH'] = Dir('#external/quex-0.63.2').abspath
environ['PATH'] = ':'.join([environ['PATH'],
                            environ['QUEX_PATH']])

cc = environ.get('CC', 'gcc')
cxx = environ.get('CXX', 'g++')
coverage = ARGUMENTS.get('coverage', 0)

env = Environment(tools=['default', 'quex', 'lcov', 'genhtml'],
                  ENV=environ,
                  CC=cc,
                  CXX=cxx,
                  LIBS=['gmp', 'm'],
                  CPPFLAGS='-ggdb -Wall -Werror',
                  CPPDEFINES=['_GNU_SOURCE'],
                  CPPPATH=['#include/',
                           '#build/src/',
                           environ['QUEX_PATH']])

env.MergeFlags("!pkg-config glib-2.0 --cflags --libs")

if coverage:
    env.AppendUnique(LIBS=['gcov',
                           'pthread'])

rose_lib = env.SConscript(dirs='src',
                          exports=['env',
                                   'coverage'],
                          variant_dir='build/src/')

rsi_bin = env.SConscript(dirs='tools/rsi/',
                         exports=['env',
                                  'rose_lib',
                                  'coverage'],
                         variant_dir='build/tools/rsi/')

test = env.SConscript(dirs='test',
                      exports=['env',
                               'rose_lib',
                               'coverage'],
                      variant_dir='build/test/')

env.Install(target='bin/',
            source=rsi_bin)

# vim:ft=python ts=4 sw=4 sts=4 et
