from   quex.engine.misc.file_in          import EndOfStreamException, \
                                                check, \
                                                check_or_die, \
                                                error_msg, \
                                                get_current_line_info_number, \
                                                read_integer, \
                                                read_namespaced_name, \
                                                read_until_closing_bracket, \
                                                skip_whitespace, \
                                                verify_word_in_list
import quex.blackboard                   as     blackboard
from   quex.output.cpp.token_id_maker    import TokenInfo
from   quex.blackboard                   import setup as Setup
from   quex.blackboard                   import QuexSetup
from   quex.engine.unicode_db.parser     import ucs_property_db
from   quex.engine.utf8                  import __read_one_utf8_code_from_stream
from   quex.engine.generator.action_info import UserCodeFragment 
import quex.input.regular_expression.snap_backslashed_character as snap_backslashed_character



def parse(fh, CodeFragmentName, 
          ErrorOnFailureF=True, AllowBriefTokenSenderF=True, ContinueF=True):
    """RETURNS: An object of class UserCodeFragment containing
                line number, filename, and the code fragment.

                None in case of failure.
    """
    assert Setup.__class__ == QuexSetup
    assert type(ErrorOnFailureF)        == bool
    assert type(AllowBriefTokenSenderF) == bool

    skip_whitespace(fh)

    word = fh.read(2)
    if len(word) >= 1 and word[0] == "{":
        fh.seek(-1, 1) # unput the second character
        return __parse_normal(fh, CodeFragmentName)

    elif AllowBriefTokenSenderF and word == "=>":
        return __parse_brief_token_sender(fh, ContinueF)

    elif not ErrorOnFailureF:
        fh.seek(-2,1)
        return None
    else:
        error_msg("Missing code fragment after %s definition." % CodeFragmentName, fh)

def __parse_normal(fh, code_fragment_name):
    LanguageDB = Setup.language_db

    line_n = get_current_line_info_number(fh) + 1
    code   = read_until_closing_bracket(fh, "{", "}")
    return UserCodeFragment(code, fh.name, line_n, LanguageDB)

def __read_token_identifier(fh):
    """Parses a token identifier that may contain a namespace specification.

       Returns "", if no valid specification could be found.
    """
    identifier, name_space_list, dummy = read_namespaced_name(fh, "token identifier")
    if identifier == "": return ""
    if len(name_space_list) == 0: return identifier
    return reduce(lambda x, y: x + "::" + y, name_space_list + [identifier])

def __parse_brief_token_sender(fh, ContinueF):
    # shorthand for { self.send(TKN_SOMETHING); QUEX_SETTING_AFTER_SEND_CONTINUE_OR_RETURN(); }
    LanguageDB = Setup.language_db
    
    position = fh.tell()
    line_n   = get_current_line_info_number(fh) + 1
    try: 
        skip_whitespace(fh)
        position = fh.tell()

        code = __parse_token_id_specification_by_character_code(fh)
        if code != -1: 
            code = __create_token_sender_by_character_code(fh, code)
        else:
            skip_whitespace(fh)
            identifier = __read_token_identifier(fh)
            skip_whitespace(fh)
            if identifier in ["GOTO", "GOSUB", "GOUP"]:
                code = __create_mode_transition_and_token_sender(fh, identifier)
            else:
                code = __create_token_sender_by_token_name(fh, identifier)
                check_or_die(fh, ";")

        if code != "": 
            if ContinueF: code += "QUEX_SETTING_AFTER_SEND_CONTINUE_OR_RETURN();\n"
            return UserCodeFragment(code, fh.name, line_n, LanguageDB)
        else:
            return None

    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while parsing token shortcut.", fh)

def read_character_code(fh):
    # NOTE: This function is tested with the regeression test for feature request 2251359.
    #       See directory $QUEX_PATH/TEST/2251359.
    pos = fh.tell()
    
    start = fh.read(1)
    if start == "":  
        fh.seek(pos); return -1

    elif start == "'": 
        # read an utf-8 char an get the token-id
        # Example: '+'
        if check(fh, "\\"):
            # snap_backslashed_character throws an exception if 'backslashed char' is nonsense.
            character_code = snap_backslashed_character.do(fh, ReducedSetOfBackslashedCharactersF=True)
        else:
            character_code = __read_one_utf8_code_from_stream(fh)

        if character_code is None:
            error_msg("Missing utf8-character for definition of character code by character.", fh)

        elif fh.read(1) != '\'':
            error_msg("Missing closing ' for definition of character code by character.", fh)

        return character_code

    if start == "U":
        if fh.read(1) != "C": fh.seek(pos); return -1
        # read Unicode Name 
        # Example: UC MATHEMATICAL_MONOSPACE_DIGIT_FIVE
        skip_whitespace(fh)
        ucs_name = __read_token_identifier(fh)
        if ucs_name == "": fh.seek(pos); return -1
        # Get the character set related to the given name. Note, the size of the set
        # is supposed to be one.
        character_code = ucs_property_db.get_character_set("Name", ucs_name)
        if type(character_code) in [str, unicode]:
            verify_word_in_list(ucs_name, ucs_property_db["Name"].code_point_db,
                                "The string %s\ndoes not identify a known unicode character." % ucs_name, 
                                fh)
        elif type(character_code) not in [int, long]:
            error_msg("%s relates to more than one character in unicode database." % ucs_name, fh) 
        return character_code

    fh.seek(pos)
    character_code = read_integer(fh)
    if character_code is not None: return character_code

    # Try to interpret it as something else ...
    fh.seek(pos)
    return -1               

def __parse_function_argument_list(fh, ReferenceName):
    argument_list = []
    position = fh.tell()
    try:
        # Read argument list
        if check(fh, "(") == False:
            return []

        text = ""
        while 1 + 1 == 2:
            tmp = fh.read(1)
            if   tmp == ")": 
                break
            elif tmp in ["(", "[", "{"]:
                closing_bracket = {"(": ")", "[": "]", "{": "}"}[tmp]
                text += tmp + read_until_closing_bracket(fh, tmp, closing_bracket) + closing_bracket
            elif tmp == "\"":
                text += tmp + read_until_closing_bracket(fh, "", "\"", IgnoreRegions = []) + "\"" 
            elif tmp == "'":
                text += tmp + read_until_closing_bracket(fh, "", "'", IgnoreRegions = []) + "'" 
            elif tmp == ",":
                argument_list.append(text)
                text = ""
            elif tmp == "":
                fh.seek(position)
                error_msg("End of file reached while parsing argument list for %s." % ReferenceName, fh)
            else:
                text += tmp

        if text != "": argument_list.append(text)

        argument_list = map(lambda arg:    arg.strip(), argument_list)
        argument_list = filter(lambda arg: arg != "",   argument_list)
        return argument_list

    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while parsing token shortcut.", fh)

def __parse_token_id_specification_by_character_code(fh):
    ## pos = fh.tell(); print "##input:", fh.read(3); fh.seek(pos)
    character_code = read_character_code(fh)
    ## print "##cc:", character_code
    if character_code == -1: return -1
    check_or_die(fh, ";")
    return character_code

def __create_token_sender_by_character_code(fh, CharacterCode):
    # The '--' will prevent the token name from being printed
    prefix_less_token_name = "UCS_0x%06X" % CharacterCode
    token_id_str           = "0x%06X" % CharacterCode 
    blackboard.token_id_db["--" + prefix_less_token_name] = \
            TokenInfo(prefix_less_token_name, CharacterCode, None, fh.name, get_current_line_info_number(fh)) 
    return "self_send(%s);\n" % token_id_str

def cut_token_prefix_or_die(fh, TokenName):
    global Setup
    if TokenName.find(Setup.token_id_prefix) == 0: 
        return TokenName[len(Setup.token_id_prefix):]

    if TokenName.find(Setup.token_id_prefix_plain) == 0:
        return TokenName[len(Setup.token_id_prefix_plain):]

    error_msg("Token identifier does not begin with token prefix '%s'\n" % Setup.token_id_prefix + \
              "found: '%s'" % TokenName, fh)

def token_id_db_verify_or_enter_token_id(fh, TokenName):
    global Setup

    prefix_less_TokenName = cut_token_prefix_or_die(fh, TokenName)

    # Occasionally add token id automatically to database
    if not blackboard.token_id_db.has_key(prefix_less_TokenName):
        # DO NOT ENFORCE THE TOKEN ID TO BE DEFINED, BECAUSE WHEN THE TOKEN ID
        # IS DEFINED IN C-CODE, THE IDENTIFICATION IS NOT 100% SAFE.
        msg = "Token id '%s' defined implicitly." % TokenName
        if TokenName in blackboard.token_id_db.keys():
            msg += "\nNOTE: '%s' has been defined in a token { ... } section!" % \
                   (Setup.token_id_prefix + TokenName)
            msg += "\nNote, that tokens in the token { ... } section are automatically prefixed."
            error_msg(msg, fh, DontExitF=True)
        else:
            blackboard.token_id_implicit_list.append([prefix_less_TokenName, fh.name, get_current_line_info_number(fh)])

        # Enter the implicit token id definition in the database
        blackboard.token_id_db[prefix_less_TokenName] = \
                TokenInfo(prefix_less_TokenName, None, None, fh.name, get_current_line_info_number(fh)) 

def __create_token_sender_by_token_name(fh, TokenName):
    assert type(TokenName) in [str, unicode]

    # Enter token_id into database, if it is not yet defined.
    token_id_db_verify_or_enter_token_id(fh, TokenName)

    # Parse the token argument list
    argument_list = __parse_function_argument_list(fh, TokenName)

    # Create the token sender
    explicit_member_names_f = False
    for arg in argument_list:
        if arg.find("=") != -1: explicit_member_names_f = True

    assert blackboard.token_type_definition is not None, \
           "A valid token_type_definition must have been parsed at this point."

    if not explicit_member_names_f:
        # There are only two allowed cases for implicit token member names:
        #  QUEX_TKN_XYZ(Lexeme)     --> call take_text(Lexeme, LexemeEnd)
        #  QUEX_TKN_XYZ(Begin, End) --> call to take_text(Begin, End)
        if   len(argument_list) == 2:
            return "QUEX_NAME_TOKEN(take_text)(self_write_token_p(), &self, (%s), (%s));\n" % \
                   (argument_list[0], argument_list[1]) + \
                   "self_send(%s);\n" % (TokenName)

        elif len(argument_list) == 1:
            if argument_list[0] == "Lexeme":
                return "QUEX_NAME_TOKEN(take_text)(self_write_token_p(), &self, self.buffer._lexeme_start_p, self.buffer._input_p);\n" \
                       "self_send(%s);\n" % (TokenName)
            elif argument_list[0] == "LexemeNull":
                return "QUEX_NAME_TOKEN(take_text)(self_write_token_p(), &self, LexemeNull, LexemeNull);\n" \
                       "self_send(%s);\n" % (TokenName)
            else:
                error_msg("If one unnamed argument is specified it must be 'Lexeme'\n"          + \
                          "or 'LexemeNull'. Found '%s'.\n" % argument_list[0]                     + \
                          "To cut parts of the lexeme, please, use the 2 argument sender, e.g.\n" + \
                          "QUEX_TKN_MY_ID(Lexeme + 1, LexemeEnd - 2);\n"                             + \
                          "Alternatively, use named parameters such as 'number=...'.", fh)

        elif len(argument_list) == 0:
            return "self_send(%s);\n" % TokenName

        else:
            error_msg("Since 0.49.1, there are only the following brief token senders that can take\n"
                      "unnamed token arguments:\n"
                      "     one argument:   'Lexeme'   =>  token.take_text(..., LexemeBegin, LexemeEnd);\n"
                      "     two arguments:  Begin, End =>  token.take_text(..., Begin, End);\n"
                      + "Found: " + repr(argument_list)[1:-1] + ".", fh)

        # Returned from Function if implicit member names

    member_value_pairs = map(lambda x: x.split("="), argument_list)
    txt = ""
    for member, value in member_value_pairs:
        if value == "":
            error_msg("One explicit argument name mentioned requires all arguments to\n"  + \
                      "be mentioned explicitly. Value '%s' mentioned without argument.\n"   \
                      % member, fh)

        if Setup.token_class_file != "":
            error_msg("Member assignments in brief token senders are inadmissible\n" + \
                      "with manually written token classes. User provided file '%s'.\n" % Setup.token_class_file + \
                      "Found member assignment: '%s' = '%s'." % (member, value), fh)
        else:
            member_name = member.strip()
            verify_word_in_list(member_name, blackboard.token_type_definition.get_member_db(), 
                                "No member:   '%s' in token type description." % member_name, 
                                fh)
            idx = value.find("Lexeme")
            if idx != -1:
                if idx != 0 and value[idx-1] == "(":
                    pass
                else:
                    error_msg("Assignment of token member '%s' with 'Lexeme' directly being involved. The\n" % member_name + 
                              "'Lexeme' points into the text buffer and it is not owned by the token object.\n"
                              "\n"
                              "Proposals:\n\n"
                              "   (1) Use '(Lexeme)', i.e. surround 'Lexeme' by brackets to indicate\n"
                              "       that you are aware of the danger. Do this, if at the end of the\n"
                              "       process, the member can be assumed to relate to an object that\n"
                              "       is not directly dependent anymore on 'Lexeme'. This is particularly\n"
                              "       true if the member is of type 'std::string'. Its constructor\n"
                              "       creates a copy of the zero terminated string.\n\n"
                              "   (2) Use token senders without named arguments, for example\n"
                              "          \"%s(Lexeme+1, LexemeEnd-2)\"\n" % TokenName + 
                              "          \"%s(Lexeme)\"\n" % TokenName + 
                              "       These token senders create a copy of the lexeme and let the token\n"
                              "       own it.", fh)

            access = blackboard.token_type_definition.get_member_access(member_name)
            txt += "self_write_token_p()->%s = %s;\n" % (access, value.strip())


    # Box the token, stamp it with an id and 'send' it
    txt += "self_send(%s);\n" % TokenName
    return txt

def __create_mode_transition_and_token_sender(fh, Command):
    assert Command in ["GOTO", "GOSUB", "GOUP"]

    position     = fh.tell()
    LanguageDB   = Setup.language_db
    target_mode  = ""
    token_sender = ""
    if check(fh, "("):
        skip_whitespace(fh)
        if Command != "GOUP":
            target_mode = __read_token_identifier(fh)
            skip_whitespace(fh)

        if check(fh, ")"):
            token_sender = ""

        elif Command == "GOUP" or check(fh, ","):
            skip_whitespace(fh)
            token_name = __read_token_identifier(fh)
            skip_whitespace(fh)

            if check(fh, ","):
                error_msg("Missing opening '(' after token name specification.\n" 
                          "Note, that since version 0.50.1 the syntax for token senders\n"
                          "inside brief mode transitions is like:\n\n"
                          "     => GOTO(MYMODE, QUEX_TKN_MINE(Argument0, Argument1, ...));\n", fh)

            token_sender = __create_token_sender_by_token_name(fh, token_name) 

            if check(fh, ")") == False:
                error_msg("Missing closing ')' or ',' after '%s'." % Command, fh)

        else:
            fh.seek(position)
            error_msg("Missing closing ')' or ',' after '%s'." % Command, fh)

    if check(fh, ";") == False:
        error_msg("Missing ')' or ';' after '%s'." % Command, fh)

    if Command in ["GOTO", "GOSUB"] and target_mode == "": 
        error_msg("Command %s requires at least one argument: The target mode." % Command, fh)

    # Code for mode change
    if   Command == "GOTO":  txt = LanguageDB.MODE_GOTO(target_mode)
    elif Command == "GOSUB": txt = LanguageDB.MODE_GOSUB(target_mode)
    else:                    txt = LanguageDB.MODE_GOUP()

    # Code for token sending
    txt += token_sender

    return txt

