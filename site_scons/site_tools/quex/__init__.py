"""
SCons.Tool.quex

Tool-specific initializaton for the Quex lexer generator.

For the following options, target/source dependencies are not properly
handled:

- ``--language dot``
- ``--derived-class``, ``--dc``
- ``--derived-class-file``
- ``--token-class-file``
- ``--token-class``, ``--tc``
- ``--codec-file``
"""

import SCons

from SCons.Builder import Builder
from SCons.Script import Flatten, Mkdir

from os import path


class ToolQuexWarning(SCons.Warnings.Warning):
    pass


class QuexCompilerNotFound(ToolQuexWarning):
    pass


def quex_generator(source, target, env, for_signature):
    quex_opt = {
        'QUEXLANG':   '--language',
        'QUEXEXT':    '--file-extension-scheme',
        'QUEXOUTDIR': '--output-directory',
        'QUEXTKNID':  '--foreign-token-id-file'
    }

    Mkdir(env['QUEXOUTDIR'])

    cmd = ['quex']
    cmd += ['--mode-files', str(source[0])]
    cmd += ['--engine', env['QUEXENGINENS'] + env['QUEXENGINE']]
    cmd += [env['QUEXFLAGS']]
    cmd += [[quex_opt[key], env[key]] for key in quex_opt if key in env]

    return ' '.join(Flatten(cmd))


def quex_emitter(source, target, env):
    target_prefix = env['QUEXENGINENS'].replace('::', '_') + env['QUEXENGINE']
    target_prefix = path.join(env['QUEXOUTDIR'], target_prefix)

    # C file extension scheme
    header_ext = '.h'
    source_ext = '.c'

    # C++ file extension schemes
    quex_ext_scheme = {
        '++': ('.c++', '.h++'),
        'pp': ('.hpp', '.cpp'),
        'xx': ('.hxx', '.cxx'),
        'cc': ('.hh',  '.cc'),
    }

    if env['QUEXLANG'] == 'C++':
        if 'QUEXEXT' in env:
            header_ext = quex_ext_scheme[env['QUEXEXT']][0]
            source_ext = quex_ext_scheme[env['QUEXEXT']][1]
        else:
            header_ext = ''
            source_ext = '.cpp'

    # Auto generated lexer source/header files
    target = [
        target_prefix +                    source_ext,
        target_prefix + '-configuration' + header_ext,
        target_prefix +                    header_ext,
        target_prefix + '-token'         + header_ext,
        target_prefix + '-token_ids'     + header_ext,
    ]

    if 'QUEXTKNID' in env:
        source.append(env['QUEXTKNID'])

    return (target, source)


_quex_builder = Builder(generator=quex_generator,
                        emitter=quex_emitter,
                        src_suffix='qx')


def generate(env):
    """
    Add Quex builders and construction variables to the environment.
    """

    env['Quex'] = _detect(env)

    env.SetDefault(QUEXLANG='C++',
                   QUEXOUTDIR='.',
                   QUEXENGINENS='',
                   QUEXENGINE='Lexer',
                   QUEXFLAGS='')

    env['BUILDERS']['Quex'] = _quex_builder


def _detect(env):
    """
    Try to detect the Quex compiler
    """

    try:
        return env['Quex']
    except KeyError:
        pass

    quex = env.WhereIs('quex')
    if quex:
        return quex

    raise SCons.Errors.StopError(QuexCompilerNotFound,
                                 "Cound not detect Quex compiler")

    return None


def exists(env):
    return _detect(env)

# vim:ft=python ts=4 sw=4 tw=70 et
