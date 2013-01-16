# GRAMMAR:
#
# set_expression: 
#                 [: set_term :]
#                 traditional character set
#                 \P '{' propperty string '}'
#                 '{' identifier '}'
#
# set_term:
#                 "alnum" 
#                 "alpha" 
#                 "blank" 
#                 "cntrl" 
#                 "digit" 
#                 "graph" 
#                 "lower" 
#                 "print" 
#                 "punct" 
#                 "space" 
#                 "upper" 
#                 "xdigit"
#                 "union"        '(' set_term [ ',' set_term ]+ ')'
#                 "intersection" '(' set_term [ ',' set_term ]+ ')'
#                 "difference"   '(' set_term [ ',' set_term ]+ ')'
#                 "inverse"      '(' set_term ')'
#                 set_expression
# 
import quex.engine.codec_db.core as codec_db
import quex.input.regular_expression.traditional_character_set as traditional_character_set
import quex.input.regular_expression.property                  as property
import quex.input.regular_expression.case_fold_expression      as case_fold_expression
#
from quex.engine.state_machine.core          import StateMachine
from quex.exception                          import RegularExpressionException
from quex.engine.misc.file_in                import read_until_letter, \
                                                    read_identifier, \
                                                    skip_whitespace, \
                                                    verify_word_in_list, \
                                                    check, \
                                                    error_msg
from quex.input.regular_expression.auxiliary import __snap_until, \
                                                    __debug_entry, \
                                                    __debug_exit, \
                                                    snap_replacement
special_character_set_db = {
    # The closing ']' is to trigger the end of the traditional character set
    "alnum":  "a-zA-Z0-9]",
    "alpha":  "a-zA-Z]",
    "blank":  " \\t]",
    "cntrl":  "\\x00-\\x1F\\x7F]", 
    "digit":  "0-9]",
    "graph":  "\\x21-\\x7E]",
    "lower":  "a-z]",
    "print":  "\\x20-\\x7E]", 
    "punct":  "!\"#$%&'()*+,-./:;?@[\\]_`{|}~\\\\]",
    "space":  " \\t\\r\\n]",
    "upper":  "A-Z]",
    "xdigit": "a-fA-F0-9]",
}

def do(stream, PatternDict):
    trigger_set = snap_set_expression(stream, PatternDict)

    if trigger_set is None: 
        raise RegularExpressionException("Regular Expression: character_set_expression called for something\n" + \
                                         "that does not start with '[:', '[' or '\\P'")
    if trigger_set.is_empty():
        raise RegularExpressionException("Regular Expression: Character set expression results in empty set.")

    # Create state machine that triggers with the trigger set to SUCCESS
    # NOTE: The default for the ELSE transition is FAIL.
    sm = StateMachine()
    sm.add_transition(sm.init_state_index, trigger_set, AcceptanceF=True)

    return __debug_exit(sm, stream)

def snap_set_expression(stream, PatternDict):
    assert     stream.__class__.__name__ == "StringIO" \
            or stream.__class__.__name__ == "file"

    __debug_entry("set_expression", stream)

    result = snap_property_set(stream)
    if result is not None: return result

    x = stream.read(2)
    if   x == "\\C":
        return case_fold_expression.do(stream, PatternDict, snap_set_expression=snap_set_expression)

    elif x == "[:":
        result = snap_set_term(stream, PatternDict)
        skip_whitespace(stream)
        x = stream.read(2)
        if x != ":]":
            raise RegularExpressionException("Missing closing ':]' for character set expression.\n" + \
                                             "found: '%s'" % x)
    elif x[0] == "[":
        stream.seek(-1, 1)
        result = traditional_character_set.do(stream)   

    elif x[0] == "{":
        stream.seek(-1, 1)
        result = snap_replacement(stream, PatternDict, StateMachineF=False)   

    else:
        result = None

    return __debug_exit(result, stream)

def snap_property_set(stream):
    position = stream.tell()
    x = stream.read(2)
    if   x == "\\P": 
        stream.seek(position)
        return property.do(stream)
    elif x == "\\N": 
        stream.seek(position)
        return property.do_shortcut(stream, "N", "na") # UCS Property: Name
    elif x == "\\G": 
        stream.seek(position)
        return property.do_shortcut(stream, "G", "gc") # UCS Property: General_Category
    elif x == "\\E": 
        skip_whitespace(stream)
        if check(stream, "{") == False:
            error_msg("Missing '{' after '\\E'.", stream)
        encoding_name = __snap_until(stream, "}").strip()
        return codec_db.get_supported_unicode_character_set(encoding_name, FH=stream)
    else:
        stream.seek(position)
        return None

def snap_set_term(stream, PatternDict):
    global special_character_set_db

    __debug_entry("set_term", stream)    

    operation_list     = [ "union", "intersection", "difference", "inverse"]
    character_set_list = special_character_set_db.keys()

    skip_whitespace(stream)
    position = stream.tell()

    # if there is no following '(', then enter the 'snap_expression' block below
    word = read_identifier(stream)

    if word in operation_list: 
        set_list = snap_set_list(stream, word, PatternDict)
        # if an error occurs during set_list parsing, an exception is thrown about syntax error

        L      = len(set_list)
        result = set_list[0]

        if word == "inverse":
            # The inverse of multiple sets, is to be the inverse of the union of these sets.
            if L > 1:
                for character_set in set_list[1:]:
                    result.unite_with(character_set)
            return __debug_exit(result.inverse(), stream)

        if L < 2:
            raise RegularExpressionException("Regular Expression: A %s operation needs at least\n" % word + \
                                             "two sets to operate on them.")
            
        if   word == "union":
            for set in set_list[1:]:
                result.unite_with(set)
        elif word == "intersection":
            for set in set_list[1:]:
                result.intersect_with(set)
        elif word == "difference":
            for set in set_list[1:]:
                result.subtract(set)

    elif word in character_set_list:
        reg_expr = special_character_set_db[word]
        result   = traditional_character_set.do_string(reg_expr)

    elif word != "":
        verify_word_in_list(word, character_set_list + operation_list, 
                            "Unknown keyword '%s'." % word, stream)
    else:
        stream.seek(position)
        result = snap_set_expression(stream, PatternDict)

    return __debug_exit(result, stream)

def __snap_word(stream):
    try:    the_word = read_until_letter(stream, ["("]) 
    except: 
        raise RegularExpressionException("Missing opening bracket.")
    stream.seek(-1,1)
    return the_word.strip()

def snap_set_list(stream, set_operation_name, PatternDict):
    __debug_entry("set_list", stream)

    skip_whitespace(stream)
    if stream.read(1) != "(": 
        raise RegularExpressionException("Missing opening bracket '%s' operation." % set_operation_name)

    set_list = []
    while 1 + 1 == 2:
        skip_whitespace(stream)
        result = snap_set_term(stream, PatternDict)
        if result is None: 
            raise RegularExpressionException("Missing set expression list after '%s' operation." % set_operation_name)
        set_list.append(result)
        skip_whitespace(stream)
        tmp = stream.read(1)
        if tmp != ",": 
            if tmp != ")":
                stream.seek(-1, 1)
                raise RegularExpressionException("Missing closing ')' after after '%s' operation." % set_operation_name)
            return __debug_exit(set_list, stream)


   
