from SCons.Builder import Builder
from SCons.Script import File
from SCons.Script import Flatten
from SCons.Script import Mkdir

from os import path

# NOTE: Do not support `--language dot'
quex_opt = {
    'QUEXTKNPREFIX': '--token-id-prefix',
    'QUEXLANG':      '--language',
    'QUEXTKNPOLICY': '--token-policy',
    'QUEXEXT':       '--file-extension-scheme',
    'QUEXOUTDIR':    '--output-directory',
}


quex_file_ext = {
    'pp': ('.hpp', '.cpp'),
    'xx': ('.hxx', '.cxx'),
    'cc': ('.hh',  '.cc'),
}


def quex_generator(source, target, env, for_signature):
    Mkdir(env['QUEXOUTDIR'])

    cmd = ['quex']
    cmd += ['--mode-files', str(source[0])]
    cmd += ['--engine', env['QUEXENGINENS'] + env['QUEXENGINE']]
    cmd += [[quex_opt[key], env[key]] for key in quex_opt if key in env]

    return ' '.join(Flatten(cmd))


def quex_emitter(source, target, env):

    target_prefix = env['QUEXENGINENS'].replace('::', '_') + env['QUEXENGINE']
    target_prefix = path.join(env['QUEXOUTDIR'], target_prefix)

    header_ext = '.h'
    source_ext = '.c'

    if env['QUEXLANG'] == 'C++':
        if 'QUEXEXT' in env:
            header_ext = quex_file_ext[env['QUEXEXT']][0]
            source_ext = quex_file_ext[env['QUEXEXT']][1]
        else:
            header_ext = ''
            source_ext = '.cpp'

    target = [
        File(''.join([target_prefix, '',               source_ext])),
        File(''.join([target_prefix, '-configuration', header_ext])),
        File(''.join([target_prefix, '',               header_ext])),
        File(''.join([target_prefix, '-token',         header_ext])),
        File(''.join([target_prefix, '-token_ids',     header_ext])),
    ]

    return (target, source)


def generate(env):
    env.SetDefault(QUEXLANG='C++',
                   QUEXTKNPREFIX='QUEX_TKN_',
                   QUEXTKNPOLICY='queue',
                   QUEXOUTDIR='.',
                   QUEXENGINENS='',
                   QUEXENGINE='Lexer',
                   QUEXSTARTMODE='MAIN')

    env['BUILDERS']['Quex'] = Builder(generator=quex_generator,
                                      emitter=quex_emitter,
                                      src_suffix='qx')


def exists(env):
    env.Detect(['quex'])

# vim:ft=python ts=4 sw=4 et
