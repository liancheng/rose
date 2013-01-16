import sys
import os
sys.path.insert(0, os.environ["QUEX_PATH"])

from   quex.blackboard                    import setup, \
                                                 E_Compression
import quex.blackboard                    as     blackboard
from   quex.input.command_line.GetPot     import GetPot
import quex.input.command_line.validation as     validation
from   quex.input.setup                   import SETUP_INFO,               \
                                                 SetupParTypes,            \
                                                 global_extension_db,      \
                                                 global_character_type_db, \
                                                 command_line_args_defined, \
                                                 command_line_arg_position, \
                                                 E_Files

from   quex.output.cpp.token_id_maker     import parse_token_id_file

from   quex.engine.misc.file_in           import error_msg,                \
                                                 verify_word_in_list,      \
                                                 read_namespaced_name,     \
                                                 read_integer,             \
                                                 open_file_or_die
import quex.engine.codec_db.core            as codec_db
from   quex.engine.generator.languages.core import db as quex_core_engine_generator_languages_db
from   quex.engine.generator.action_info    import CodeFragment

from   quex.DEFINITIONS import QUEX_VERSION

from   StringIO import StringIO
from   operator import itemgetter

class ManualTokenClassSetup:
    """Class to mimik as 'real' TokenTypeDescriptor as defined in 
       quex.input.files.token_type.py. Names and functions must remain
       as they are for compatibility.
    """
    def __init__(self, FileName, ClassName, NameSpace, ClassNameSafe, TokenIDType):
        
        self.__file_name       = FileName
        self.class_name        = ClassName
        self.name_space        = NameSpace
        self.class_name_safe   = ClassNameSafe

        self.column_number_type = CodeFragment("size_t")
        self.line_number_type   = CodeFragment("size_t")
        self.token_id_type      = CodeFragment(TokenIDType)

    def get_file_name(self):
        return self.__file_name

    def manually_written(self):
        return True

def do(argv):
    global setup

    try:    
        idx = argv.index("--token-class-file")
        if idx + 1 < len(argv): idx += 1
        else:                   idx  = None
    except: 
        idx = None 

    if idx is not None:
        extra_argv = __extract_extra_options_from_file(argv[idx])
        if extra_argv is not None: argv.extend(extra_argv)

    command_line = __interpret_command_line(argv)
    if command_line is None:
        return False

    return __perform_setup(command_line, argv)

def __perform_setup(command_line, argv):
    """RETURN:  True, if process needs to be started.
                False, if job is done.
    """
    global setup

    # (*) Classes and their namespace
    __setup_analyzer_class(setup)
    __setup_token_class(setup)
    __setup_token_id_prefix(setup)
    __setup_lexeme_null(setup)       # Requires 'token_class_name_space'

    # (*) Output programming language        
    setup.language = setup.language.upper()
    verify_word_in_list(setup.language,
                        quex_core_engine_generator_languages_db.keys(),
                        "Programming language '%s' is not supported." % setup.language)
    setup.language_db  = quex_core_engine_generator_languages_db[setup.language]
    setup.extension_db = global_extension_db[setup.language]

    # Is the output file naming scheme provided by the extension database
    # (Validation must happen immediately)
    if setup.extension_db.has_key(setup.output_file_naming_scheme) == False:
        error_msg("File extension scheme '%s' is not provided for language '%s'.\n" \
                  % (setup.output_file_naming_scheme, setup.language) + \
                  "Available schemes are: %s." % repr(setup.extension_db.keys())[1:-1])

    # Before file names can be prepared, determine the output directory
    # If 'source packaging' is enabled and no output directory is specified
    # then take the directory of the source packaging.
    if setup.source_package_directory != "" and setup.output_directory == "":
        setup.output_directory = setup.source_package_directory

    if setup.buffer_codec in ["utf8", "utf16"]:
        setup.buffer_codec_transformation_info = setup.buffer_codec + "-state-split"

    elif setup.buffer_codec_file != "":
        try: 
            setup.buffer_codec = os.path.splitext(os.path.basename(setup.buffer_codec_file))[0]
        except:
            error_msg("cannot interpret string following '--codec-file'")

        setup.buffer_codec_transformation_info = codec_db.get_codec_transformation_info(FileName=setup.buffer_codec_file)

    elif setup.buffer_codec != "unicode":
        setup.buffer_codec_transformation_info = codec_db.get_codec_transformation_info(setup.buffer_codec)

    if setup.buffer_codec != "unicode":
        setup.buffer_element_size_irrelevant = True
    
    # (*) Output files
    if setup.language not in ["DOT"]:
        prepare_file_names(setup)

    if setup.buffer_byte_order == "<system>": 
        setup.buffer_byte_order = sys.byteorder 
        setup.byte_order_is_that_of_current_system_f = True
    else:
        setup.byte_order_is_that_of_current_system_f = False

    if setup.buffer_element_size == "wchar_t":
        error_msg("Since Quex version 0.53.5, 'wchar_t' can no longer be specified\n"
                  "with option '--buffer-element-size' or '-bes'. Please, specify\n"
                  "'--buffer-element-type wchar_t' or '--bet'.")

    if setup.buffer_element_type == "wchar_t":
        setup.converter_ucs_coding_name = "WCHAR_T"

    make_numbers(setup)

    # (*) Determine buffer element type and size (in bytes)
    if setup.buffer_element_size == -1:
        if global_character_type_db.has_key(setup.buffer_element_type):
            setup.buffer_element_size = global_character_type_db[setup.buffer_element_type][3]
        elif setup.buffer_element_type == "":
            setup.buffer_element_size = 1
        else:
            # If the buffer element type is defined, then here we know that it is 'unknown'
            # and Quex cannot know its size on its own.
            setup.buffer_element_size = -1

    if setup.buffer_element_type == "":
        if setup.buffer_element_size in [1, 2, 4]:
            setup.buffer_element_type = { 
                1: "uint8_t", 2: "uint16_t", 4: "uint32_t",
            }[setup.buffer_element_size]
        elif setup.buffer_element_size == -1:
            pass
        else:
            error_msg("Buffer element type cannot be determined for size '%i' which\n" \
                      % setup.buffer_element_size + 
                      "has been specified by '-b' or '--buffer-element-size'.")

    setup.converter_f = False
    if setup.converter_iconv_f or setup.converter_icu_f:
        setup.converter_f = True

    # The only case where no converter helper is required is where ASCII 
    # (Unicode restricted to [0, FF] is used.
    setup.converter_helper_required_f = True
    if setup.converter_f == False and setup.buffer_element_size == 1 and setup.buffer_codec == "unicode":
        setup.converter_helper_required_f = False

    validation.do(setup, command_line, argv)

    if setup.converter_ucs_coding_name == "": 
        if global_character_type_db.has_key(setup.buffer_element_type):
            if setup.buffer_byte_order == "little": index = 1
            else:                                   index = 2
            setup.converter_ucs_coding_name = global_character_type_db[setup.buffer_element_type][index]

    if setup.token_id_foreign_definition_file != "": 
        CommentDelimiterList = [["//", "\n"], ["/*", "*/"]]
        # Regular expression to find '#include <something>' and extract the 'something'
        # in a 'group'. Note that '(' ')' cause the storage of parts of the match.
        IncludeRE            = "#[ \t]*include[ \t]*[\"<]([^\">]+)[\">]"
        #
        parse_token_id_file(setup.token_id_foreign_definition_file, 
                            setup.token_id_prefix, 
                            CommentDelimiterList, IncludeRE)
        if setup.token_id_prefix_plain != setup.token_id_prefix:
            # The 'plain' name space less token indices are also supported
            parse_token_id_file(setup.token_id_foreign_definition_file, 
                                setup.token_id_prefix_plain, 
                                CommentDelimiterList, IncludeRE)

    # (*) Compression Types
    compression_type_list = []
    for name, ctype in [("compression_template_f",         E_Compression.TEMPLATE),
                        ("compression_template_uniform_f", E_Compression.TEMPLATE_UNIFORM),
                        ("compression_path_f",             E_Compression.PATH),
                        ("compression_path_uniform_f",     E_Compression.PATH_UNIFORM)]:
        if command_line_args_defined(command_line, name):
            compression_type_list.append((command_line_arg_position(name), ctype))
    compression_type_list.sort(key=itemgetter(0))
    setup.compression_type_list = map(lambda x: x[1], compression_type_list)

    # (*) return setup ___________________________________________________________________
    return True

def __get_float(MemberName):
    ValueStr = setup.__dict__[MemberName]
    if type(ValueStr) == float: return ValueStr
    try:
        return float(ValueStr)
    except:
        option_name = repr(SETUP_INFO[MemberName][0])[1:-1]
        error_msg("Cannot convert '%s' into an floating point number for '%s'" % (ValueStr, option_name))

def prepare_file_names(setup):
    setup.output_file_stem = ""
    if setup.analyzer_name_space != ["quex"]:
        for name in setup.analyzer_name_space:
            setup.output_file_stem += name + "_"
    setup.output_file_stem += setup.analyzer_class_name

    setup.output_code_file                       = __prepare_file_name("",               E_Files.SOURCE) 
    setup.output_header_file                     = __prepare_file_name("",               E_Files.HEADER)
    setup.output_configuration_file              = __prepare_file_name("-configuration", E_Files.HEADER)
    setup.output_token_id_file                   = __prepare_file_name("-token_ids",     E_Files.HEADER)
    setup.output_token_class_file                = __prepare_file_name("-token",         E_Files.HEADER)
    if setup.token_class_only_f == False:
        setup.output_token_class_file_implementation = __prepare_file_name("-token",     E_Files.HEADER_IMPLEMTATION)
    else:
        setup.output_token_class_file_implementation = __prepare_file_name("-token",     E_Files.SOURCE)

    if   setup.buffer_codec == "utf8":
        setup.output_buffer_codec_header   = "quex/code_base/converter_helper/from-utf8"
        setup.output_buffer_codec_header_i = "quex/code_base/converter_helper/from-utf8.i"

    elif setup.buffer_codec == "utf16":
        setup.output_buffer_codec_header   = "quex/code_base/converter_helper/from-utf16"
        setup.output_buffer_codec_header_i = "quex/code_base/converter_helper/from-utf16.i"

    elif setup.buffer_codec == "utf32":
        setup.output_buffer_codec_header   = "quex/code_base/converter_helper/from-utf32"
        setup.output_buffer_codec_header_i = "quex/code_base/converter_helper/from-utf32.i"

    elif setup.buffer_codec != "unicode":
        # Note, that the name may be set to 'None' if the conversion is utf8 or utf16
        # See Internal engine character encoding'
        setup.output_buffer_codec_header = \
            __prepare_file_name("-converter-%s" % setup.buffer_codec, E_Files.HEADER)
        setup.output_buffer_codec_header_i = \
            __prepare_file_name("-converter-%s" % setup.buffer_codec, E_Files.HEADER_IMPLEMTATION)
    else:
        setup.output_buffer_codec_header   = "quex/code_base/converter_helper/from-unicode-buffer"
        setup.output_buffer_codec_header_i = "quex/code_base/converter_helper/from-unicode-buffer.i"

def make_numbers(setup):
    setup.compression_template_min_gain = __get_integer("compression_template_min_gain")
    setup.buffer_limit_code             = __get_integer("buffer_limit_code")
    setup.path_limit_code               = __get_integer("path_limit_code")

    setup.token_id_counter_offset    = __get_integer("token_id_counter_offset")
    setup.token_queue_size           = __get_integer("token_queue_size")
    setup.token_queue_safety_border  = __get_integer("token_queue_safety_border")
    setup.buffer_element_size        = __get_integer("buffer_element_size")

def __get_integer(MemberName):
    ValueStr = setup.__dict__[MemberName]
    if type(ValueStr) == int: return ValueStr
    result = read_integer(StringIO(ValueStr))
    if result is None:
        option_name = repr(SETUP_INFO[MemberName][0])[1:-1]
        error_msg("Cannot convert '%s' into an integer for '%s'.\n" % (ValueStr, option_name) + \
                  "Use prefix '0x' for hexadecimal numbers.\n" + \
                  "           '0o' for octal numbers.\n"       + \
                  "           '0b' for binary numbers.\n"      + \
                  "           '0r' for roman numbers.\n"      + \
                  "           and no prefix for decimal numbers.")
    return result

def __prepare_file_name(Suffix, ContentType):
    global setup
    assert ContentType in E_Files

    # Language + Extenstion Scheme + ContentType --> name of extension
    ext = setup.extension_db[setup.output_file_naming_scheme][ContentType]

    file_name = setup.output_file_stem + Suffix + ext

    if setup.output_directory == "": return file_name
    else:                            return os.path.normpath(setup.output_directory + "/" + file_name)

def __setup_analyzer_class(setup):
    """ X0::X1::X2::ClassName --> analyzer_class_name = ClassName
                                  analyzer_name_space = ["X0", "X1", "X2"]
        ::ClassName --> analyzer_class_name = ClassName
                        analyzer_name_space = []
        ClassName --> analyzer_class_name = ClassName
                      analyzer_name_space = ["quex"]
    """
    if setup.analyzer_class.find("::") == -1:
        setup.analyzer_class = "quex::%s" % setup.analyzer_class

    setup.analyzer_class_name, \
    setup.analyzer_name_space, \
    setup.analyzer_name_safe   = \
         read_namespaced_name(setup.analyzer_class, 
                              "analyzer engine (options -o, --engine, --analyzer-class)")

    if setup.show_name_spaces_f:
        print "Analyzer: {"
        print "     class_name:  %s;" % setup.analyzer_class_name
        print "     name_space:  %s;" % repr(setup.analyzer_name_space)[1:-1]
        print "     name_prefix: %s;" % setup.analyzer_name_safe   
        print "}"

    setup.analyzer_derived_class_name,       \
    setup.analyzer_derived_class_name_space, \
    setup.analyzer_derived_class_name_safe = \
         read_namespaced_name(setup.analyzer_derived_class_name, 
                              "derived analyzer class (options --derived-class, --dc)",
                              AllowEmptyF=True)

def __setup_lexeme_null(setup):
    if len(setup.external_lexeme_null_object) != 0:
        lexeme_null_object = setup.external_lexeme_null_object
        default_name_space = setup.analyzer_name_space
    elif setup.token_class_only_f:
        lexeme_null_object = "LexemeNullObject"
        default_name_space = setup.token_class_name_space
    else:
        lexeme_null_object = "LexemeNullObject"
        default_name_space = setup.analyzer_name_space

    if lexeme_null_object.find("::") == -1:
        # By default, setup the token in the analyzer's namespace
        if len(setup.analyzer_name_space) != 0:
            name_space = reduce(lambda x, y: "%s::%s" % (x, y), default_name_space)
        else:
            name_space = ""
        lexeme_null_object = "%s::%s" % (name_space, lexeme_null_object)

    setup.lexeme_null_name,        \
    setup.lexeme_null_namespace,   \
    setup.lexeme_null_name_safe  = \
         read_namespaced_name(lexeme_null_object, 
                              "lexeme null object (options --lexeme-null-object, --lno)")
    setup.lexeme_null_full_name_cpp = "::" 
    for name in setup.lexeme_null_namespace:
        setup.lexeme_null_full_name_cpp += name + "::"
    setup.lexeme_null_full_name_cpp += setup.lexeme_null_name

def __setup_token_class(setup):
    """ X0::X1::X2::ClassName --> token_class_name = ClassName
                                  token_name_space = ["X0", "X1", "X2"]
        ::ClassName --> token_class_name = ClassName
                        token_name_space = []
        ClassName --> token_class_name = ClassName
                      token_name_space = analyzer_name_space
    """
    if setup.token_class.find("::") == -1:
        # By default, setup the token in the analyzer's namespace
        if len(setup.analyzer_name_space) != 0:
            analyzer_name_space = reduce(lambda x, y: "%s::%s" % (x, y), setup.analyzer_name_space)
        else:
            analyzer_name_space = ""
        setup.token_class = "%s::%s" % (analyzer_name_space, setup.token_class)

    # Token classes and derived classes have the freedom not to open a namespace,
    # thus no check 'if namespace == empty'.
    setup.token_class_name,       \
    setup.token_class_name_space, \
    setup.token_class_name_safe = \
         read_namespaced_name(setup.token_class, 
                              "token class (options --token-class, --tc)")

    if setup.show_name_spaces_f:
        print "Token: {"
        print "     class_name:  %s;" % setup.token_class_name
        print "     name_space:  %s;" % repr(setup.token_class_name_space)[1:-1]
        print "     name_prefix: %s;" % setup.token_class_name_safe   
        print "}"

    if setup.token_class_file != "":
        blackboard.token_type_definition = \
                ManualTokenClassSetup(setup.token_class_file,
                                      setup.token_class_name,
                                      setup.token_class_name_space,
                                      setup.token_class_name_safe,
                                      setup.token_id_type)

    #if len(setup.token_class_name_space) == 0:
    #    setup.token_class_name_space = deepcopy(setup.analyzer_name_space)

def __setup_token_id_prefix(setup):
    setup.token_id_prefix_plain,      \
    setup.token_id_prefix_name_space, \
    dummy                           = \
         read_namespaced_name(setup.token_id_prefix, 
                              "token prefix (options --token-id-prefix)")

    if len(setup.token_id_prefix_name_space) != 0 and setup.language.upper() == "C":
         error_msg("Token id prefix cannot contain a namespaces if '--language' is set to 'C'.")

def __extract_extra_options_from_file(FileName):
    """Extract an option section from a given file. The quex command line 
       options may be given in a section surrounded by '<<<QUEX-OPTIONS>>>'
       markers. For example:

           <<<QUEX-OPTIONS>>>
              --token-class-file      Common-token
              --token-class           Common::Token
              --token-id-type         uint32_t
              --buffer-element-type   uint8_t
              --lexeme-null-object    ::Common::LexemeNullObject
              --foreign-token-id-file Common-token_ids
           <<<QUEX-OPTIONS>>>

       This function extracts those options and builds a new 'argv' array, i.e.
       an array of strings are if they would come from the command line.
    """
    MARKER = "<<<QUEX-OPTIONS>>>"
    fh     = open_file_or_die(FileName)

    while 1 + 1 == 2:
        line = fh.readline()
        if line == "":
            return None # Simply no starting marker has been found
        elif line.find(MARKER) != -1: 
            pos = fh.tell()
            break

    result = []

    while 1 + 1 == 2:
        line = fh.readline()
        if line == "":
            fh.seek(pos)
            error_msg("Missing terminating '%s'." % MARKER, fh)

        if line.find(MARKER) != -1: 
            break
        
        idx = line.find("-")
        if idx == -1: continue
        options = line[idx:].split()
        result.extend(options)

    if len(result) == 0: return None

    if setup.message_on_extra_options_f:
        if len(result) < 2: arg_str = result[0]
        else:               arg_str = reduce(lambda x, y: "%s %s" % (x.strip(), y.strip()), result)
        print "## Command line options from file '%s'" % FileName
        print "## %s" % arg_str
        print "## (suppress this message with --no-message-on-extra-options)"

    return result

def __interpret_command_line(argv):
    command_line = GetPot(argv)

    if command_line.search("--version", "-v"):
        print "Quex - Fast Universal Lexical Analyzer Generator"
        print "Version " + QUEX_VERSION
        print "(C) 2005-2012 Frank-Rene Schaefer"
        print "ABSOLUTELY NO WARRANTY"
        return None

    if command_line.search("--help", "-h"):
        print "Quex - Fast Universal Lexical Analyzer Generator"
        print "Please, consult the quex documentation for further help, or"
        print "visit http://quex.org"
        print "(C) 2005-2012 Frank-Rene Schaefer"
        print "ABSOLUTELY NO WARRANTY"
        return None

    for variable_name, info in SETUP_INFO.items():
        # Some parameters are not set on the command line. Their entry is not associated
        # with a description list.
        if type(info) != list: continue

        if info[1] == SetupParTypes.FLAG:
            setup.__dict__[variable_name] = command_line.search(info[0])        

        elif info[1] == SetupParTypes.NEGATED_FLAG:
            setup.__dict__[variable_name] = not command_line.search(info[0])        

        elif info[1] == SetupParTypes.LIST:
            if not command_line.search(info[0]):
                setup.__dict__[variable_name] = []
            else:
                the_list = command_line.nominus_followers(info[0])
                if len(the_list) == 0:
                    error_msg("Option %s\nnot followed by anything." % repr(info[0])[1:-1])

                if setup.__dict__.has_key(variable_name):
                    for element in the_list:
                        if element not in setup.__dict__[variable_name]:
                            setup.__dict__[variable_name].extend(the_list)        
                else:
                    setup.__dict__[variable_name] = list(set(the_list))

        elif command_line.search(info[0]):
            if not command_line.search(info[0]):
                setup.__dict__[variable_name] = info[1]
            else:
                value = command_line.follow("--EMPTY--", info[0])
                if value == "--EMPTY--":
                    error_msg("Option %s\nnot followed by anything." % repr(info[0])[1:-1])
                setup.__dict__[variable_name] = value
    return command_line
