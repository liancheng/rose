from os import environ

env = Environment(tools=['default', 'quex'],
                  ENV=environ)

env.Quex(source='src/r5rs.qx',
         QUEXENGINE='r5rs_lexer',
         QUEXLANG='C++',
         QUEXTKNPREFIX='TKN_',
         QUEXTKNPOLICY='single',
         QUEXOUTDIR='build')

# vim:ft=python
