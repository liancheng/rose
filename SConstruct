from os import environ

env = Environment(tools=['default', 'quex'],
                  ENV=environ,
                  CPPPATH=['#include',
                           environ['QUEX_PATH']])

env.SConscript(dirs='src',
               exports='env',
               variant_dir='build/src')

env.SConscript(dirs='test',
               exports='env',
               variant_dir='build/test')

# vim:ft=python
