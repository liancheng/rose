#! /usr/bin/env python
import os

from   quex.engine.misc.string_handling import blue_print
from   quex.engine.misc.file_in         import get_file_content_or_die, \
                                               get_include_guard_extension
from   quex.DEFINITIONS                 import QUEX_PATH, QUEX_VERSION
import quex.blackboard                  as     blackboard
from   quex.blackboard                  import setup as Setup

def do(ModeDB):
    assert blackboard.token_type_definition is not None
    LanguageDB = Setup.language_db

    QuexClassHeaderFileTemplate = os.path.normpath(  QUEX_PATH
                                                   + Setup.language_db["$code_base"] 
                                                   + Setup.language_db["$analyzer_template_file"]).replace("//","/")
    LexerClassName = Setup.analyzer_class_name

    quex_converter_coding_name_str = Setup.converter_ucs_coding_name

    mode_id_definition_str = "" 
    # NOTE: First mode-id needs to be '1' for compatibility with flex generated engines
    for i, info in enumerate(ModeDB.items()):
        name = info[0]
        mode = info[1]
        if mode.options["inheritable"] == "only": continue
        mode_id_definition_str += "    QUEX_NAME(ModeID_%s) = %i,\n" % (name, i)

    if mode_id_definition_str != "":
        mode_id_definition_str = mode_id_definition_str[:-2]

    # -- instances of mode classes as members of the lexer
    mode_object_members_txt,     \
    mode_specific_functions_txt, \
    friend_txt                   = get_mode_class_related_code_fragments(ModeDB.values())

    # -- define a pointer that directly has the type of the derived class
    if Setup.analyzer_derived_class_name != "":
        analyzer_derived_class_name    = Setup.analyzer_derived_class_name
        derived_class_type_declaration = "class %s;" % Setup.analyzer_derived_class_name
    else:
        analyzer_derived_class_name    = Setup.analyzer_class_name
        derived_class_type_declaration = ""

    token_class_file_name = blackboard.token_type_definition.get_file_name()
    token_class_name      = blackboard.token_type_definition.class_name
    token_class_name_safe = blackboard.token_type_definition.class_name_safe

    template_code_txt = get_file_content_or_die(QuexClassHeaderFileTemplate)

    include_guard_ext = get_include_guard_extension(
            Setup.language_db.NAMESPACE_REFERENCE(Setup.analyzer_name_space) 
            + "__" + Setup.analyzer_class_name)

    if Setup.token_id_foreign_definition_file != "":
        token_id_definition_file = Setup.token_id_foreign_definition_file
    else:
        token_id_definition_file = Setup.output_token_id_file

    lexer_name_space_safe = get_include_guard_extension(LanguageDB.NAMESPACE_REFERENCE(Setup.analyzer_name_space))

    txt = blue_print(template_code_txt,
            [
                ["$$___SPACE___$$",                      " " * (len(LexerClassName) + 1)],
                ["$$CLASS_BODY_EXTENSION$$",             blackboard.class_body_extension.get_code()],
                ["$$CONVERTER_HELPER$$",                 Setup.get_file_reference(Setup.output_buffer_codec_header)],
                ["$$INCLUDE_GUARD_EXTENSION$$",          include_guard_ext],
                ["$$LEXER_CLASS_NAME$$",                 LexerClassName],
                ["$$LEXER_NAME_SPACE$$",                 lexer_name_space_safe],
                ["$$LEXER_CLASS_NAME_SAFE$$",            Setup.analyzer_name_safe],
                ["$$LEXER_CONFIG_FILE$$",                Setup.get_file_reference(Setup.output_configuration_file)],
                ["$$LEXER_DERIVED_CLASS_DECL$$",         derived_class_type_declaration],
                ["$$LEXER_DERIVED_CLASS_NAME$$",         analyzer_derived_class_name],
                ["$$QUEX_MODE_ID_DEFINITIONS$$",         mode_id_definition_str],
                ["$$MEMENTO_EXTENSIONS$$",               blackboard.memento_class_extension.get_code()],
                ["$$MODE_CLASS_FRIENDS$$",               friend_txt],
                ["$$MODE_OBJECTS$$",                     mode_object_members_txt],
                ["$$MODE_SPECIFIC_ANALYSER_FUNCTIONS$$", mode_specific_functions_txt],
                ["$$PRETTY_INDENTATION$$",               "     " + " " * (len(LexerClassName)*2 + 2)],
                ["$$QUEX_TEMPLATE_DIR$$",                QUEX_PATH + Setup.language_db["$code_base"]],
                ["$$QUEX_VERSION$$",                     QUEX_VERSION],
                ["$$TOKEN_CLASS_DEFINITION_FILE$$",      Setup.get_file_reference(token_class_file_name)],
                ["$$TOKEN_CLASS$$",                      token_class_name],
                ["$$TOKEN_CLASS_NAME_SAFE$$",            token_class_name_safe],
                ["$$TOKEN_ID_DEFINITION_FILE$$",         Setup.get_file_reference(token_id_definition_file)],
                ["$$CORE_ENGINE_CHARACTER_CODING$$",     quex_converter_coding_name_str],
                ["$$USER_DEFINED_HEADER$$",              blackboard.header.get_code() + "\n"],
             ])

    return txt

def do_implementation(ModeDB):

    FileTemplate = os.path.normpath(QUEX_PATH
                                    + Setup.language_db["$code_base"] 
                                    + "/analyzer/TXT-Cpp.i")
    func_txt = get_file_content_or_die(FileTemplate)

    func_txt = blue_print(func_txt,
            [
                ["$$CONSTRUCTOR_EXTENSTION$$",                  blackboard.class_constructor_extension.get_code()],
                ["$$CONVERTER_HELPER_I$$",                      Setup.get_file_reference(Setup.output_buffer_codec_header_i)],
                ["$$CONSTRUCTOR_MODE_DB_INITIALIZATION_CODE$$", get_constructor_code(ModeDB.values())],
                ["$$MEMENTO_EXTENSIONS_PACK$$",                 blackboard.memento_pack_extension.get_code()],
                ["$$MEMENTO_EXTENSIONS_UNPACK$$",               blackboard.memento_unpack_extension.get_code()],
                ])
    return func_txt

quex_mode_init_call_str = """
     QUEX_NAME($$MN$$).id   = QUEX_NAME(ModeID_$$MN$$);
     QUEX_NAME($$MN$$).name = "$$MN$$";
     QUEX_NAME($$MN$$).analyzer_function = $analyzer_function;
#    if      defined(QUEX_OPTION_INDENTATION_TRIGGER) \\
        && ! defined(QUEX_OPTION_INDENTATION_DEFAULT_HANDLER)
     QUEX_NAME($$MN$$).on_indentation = $on_indentation;
#    endif
     QUEX_NAME($$MN$$).on_entry       = $on_entry;
     QUEX_NAME($$MN$$).on_exit        = $on_exit;
#    if      defined(QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK)
     QUEX_NAME($$MN$$).has_base       = $has_base;
     QUEX_NAME($$MN$$).has_entry_from = $has_entry_from;
     QUEX_NAME($$MN$$).has_exit_to    = $has_exit_to;
#    endif
"""

def __get_mode_init_call(mode):
    
    analyzer_function = "QUEX_NAME(%s_analyzer_function)" % mode.name
    on_indentation    = "QUEX_NAME(%s_on_indentation)"    % mode.name
    on_entry          = "QUEX_NAME(%s_on_entry)"          % mode.name
    on_exit           = "QUEX_NAME(%s_on_exit)"           % mode.name
    has_base          = "QUEX_NAME(%s_has_base)"          % mode.name
    has_entry_from    = "QUEX_NAME(%s_has_entry_from)"    % mode.name
    has_exit_to       = "QUEX_NAME(%s_has_exit_to)"       % mode.name

    if mode.options["inheritable"] == "only": 
        analyzer_function = "QUEX_NAME(Mode_uncallable_analyzer_function)"

    if len(mode.get_code_fragment_list("on_entry")) == 0:
        on_entry = "QUEX_NAME(Mode_on_entry_exit_null_function)"

    if len(mode.get_code_fragment_list("on_exit")) == 0:
        on_exit = "QUEX_NAME(Mode_on_entry_exit_null_function)"

    if len(mode.get_code_fragment_list("on_indentation")) == 0:
        on_indentation = "QUEX_NAME(Mode_on_indentation_null_function)"

    txt = blue_print(quex_mode_init_call_str,
                [["$$MN$$",             mode.name],
                 ["$analyzer_function", analyzer_function],
                 ["$on_indentation",    on_indentation],
                 ["$on_entry",          on_entry],
                 ["$on_exit",           on_exit],
                 ["$has_base",          has_base],
                 ["$has_entry_from",    has_entry_from],
                 ["$has_exit_to",       has_exit_to]])

    return txt

def __get_mode_function_declaration(Modes, FriendF=False):

    if FriendF: prolog = "    friend "
    else:       prolog = "extern "

    def __mode_functions(Prolog, ReturnType, NameList, ArgList):
        txt = ""
        for name in NameList:
            function_signature = "%s QUEX_NAME(%s_%s)(%s);" % \
                     (ReturnType, mode.name, name, ArgList)
            txt += "%s" % Prolog + "    " + function_signature + "\n"

        return txt

    txt = ""
    on_indentation_txt = ""
    for mode in Modes:
        if mode.options["inheritable"] == "only": continue

        txt += __mode_functions(prolog, "__QUEX_TYPE_ANALYZER_RETURN_VALUE", 
                                ["analyzer_function"],
                                "QUEX_TYPE_ANALYZER*")

        # If one of the following events is specified, then we need an 'on_indentation' handler
        for event_name in ["on_indentation"]:
            if not mode.has_code_fragment_list(event_name): continue
            on_indentation_txt = __mode_functions(prolog, "void", ["on_indentation"], 
                                 "QUEX_TYPE_ANALYZER*, QUEX_TYPE_INDENTATION, QUEX_TYPE_CHARACTER*")

        for event_name in ["on_exit", "on_entry"]:
            if not mode.has_code_fragment_list(event_name): continue
            txt += __mode_functions(prolog, "void", [event_name], 
                                    "QUEX_TYPE_ANALYZER*, const QUEX_NAME(Mode)*")

        txt += "#ifdef QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK\n"
        txt += __mode_functions(prolog, "bool", ["has_base", "has_entry_from", "has_exit_to"], 
                                "const QUEX_NAME(Mode)*")
        txt += "#endif\n"

    txt += on_indentation_txt
    txt += "\n"

    return txt

def get_constructor_code(Modes):
    L = max(map(lambda m: len(m.name), Modes))

    txt = ""
    for mode in Modes:
        if mode.options["inheritable"] == "only": continue
        txt += "    __quex_assert(QUEX_NAME(ModeID_%s) %s< %i);\n" % \
               (mode.name, " " * (L-len(mode.name)), len(Modes))

    for mode in Modes:
        if mode.options["inheritable"] == "only": continue
        txt += __get_mode_init_call(mode)

    for mode in Modes:
        if mode.options["inheritable"] == "only": continue
        txt += "        me->mode_db[QUEX_NAME(ModeID_%s)]%s = &(QUEX_NAME(%s));\n" % \
               (mode.name, " " * (L-len(mode.name)), mode.name)
    return txt

def get_mode_class_related_code_fragments(Modes):
    """
       RETURNS:  -- members of the lexical analyzer class for the mode classes
                 -- static member functions declaring the analyzer functions for he mode classes 
                 -- constructor init expressions (before '{'),       
                 -- constructor text to be executed at construction time 
                 -- friend declarations for the mode classes/functions

    """
    members_txt = ""    
    for mode in Modes:
        if mode.options["inheritable"] == "only": continue
        members_txt += "        extern QUEX_NAME(Mode)  QUEX_NAME(%s);\n" % mode.name

    mode_functions_txt = __get_mode_function_declaration(Modes, FriendF=False)
    friends_txt        = __get_mode_function_declaration(Modes, FriendF=True)

    return members_txt,        \
           mode_functions_txt, \
           friends_txt

