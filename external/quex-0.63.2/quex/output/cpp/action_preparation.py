"""Action Preparation:

   Functions to prepare a source code fragment to be sticked into the
   lexical analyzer. This includes the following:

    -- pattern matches: 
    
       (optional) line and column counting based on the character 
       content of the lexeme. Many times, the character or line
       number count is determined by the pattern, so counting can
       be replaced by an addition of a constant (or even no count
       at all).
                            
    -- end of file/stream action:

       If not defined by the user, send 'TERMINATION' token and
       return.

    -- failure action (no match):

       If not defined by the user, abort program with a message 
       that tells that the user did not define an 'on_failure'
       handler.

(C) 2005-2011 Frank-Rene Schaefer
"""
from   quex.engine.generator.action_info       import CodeFragment, \
                                                      PatternActionInfo
from   quex.engine.generator.languages.address import get_plain_strings
from   quex.blackboard                         import setup as Setup, \
                                                      E_Count

import re

LanguageDB = None

def do(Mode, IndentationSupportF, BeginOfLineSupportF):
    """The module 'quex.output.cpp.core' produces the code for the 
       state machine. However, it requires a certain data format. This function
       adapts the mode information to this format. Additional code is added 

       -- for counting newlines and column numbers. This happens inside
          the function ACTION_ENTRY().
       -- (optional) for a virtual function call 'on_action_entry()'.
       -- (optional) for debug output that tells the line number and column number.
    """
    global LanguageDB
    LanguageDB = Setup.language_db

    assert Mode.__class__.__name__ == "Mode"
    variable_db              = {}

    # -- 'on after match' action
    on_after_match_str = ""
    require_terminating_zero_preparation_f = False
    if Mode.has_code_fragment_list("on_after_match"):
        on_after_match_str, \
        require_terminating_zero_preparation_f = get_code(Mode.get_code_fragment_list("on_after_match"), variable_db)

    # -- 'end of stream' action
    end_of_stream_action, db = __prepare_end_of_stream_action(Mode, IndentationSupportF, BeginOfLineSupportF)
    variable_db.update(db)

    # -- 'on failure' action (on the event that nothing matched)
    on_failure_action, db = __prepare_on_failure_action(Mode, BeginOfLineSupportF, require_terminating_zero_preparation_f)
    variable_db.update(db)

    # -- pattern-action pairs
    pattern_action_pair_list        = Mode.get_pattern_action_pair_list()
    indentation_counter_terminal_id = Mode.get_indentation_counter_terminal_index()

    # Assume pattern-action pairs (matches) are sorted and their pattern state
    # machine ids reflect the sequence of pattern precedence.
    for pattern_info in pattern_action_pair_list:
        action  = pattern_info.action()
        pattern = pattern_info.pattern()

        # Generated code fragments may rely on some information about the generator
        if hasattr(action, "data") and type(action.data) == dict:   
            action.data["indentation_counter_terminal_id"] = indentation_counter_terminal_id

        prepared_action, db = __prepare(Mode, action, pattern, \
                                        SelfCountingActionF=False, \
                                        BeginOfLineSupportF=BeginOfLineSupportF, 
                                        require_terminating_zero_preparation_f=require_terminating_zero_preparation_f)
        variable_db.update(db)

        pattern_info.set_action(prepared_action)
    
    return variable_db, \
           pattern_action_pair_list, \
           PatternActionInfo(None, end_of_stream_action), \
           PatternActionInfo(None, on_failure_action), \
           on_after_match_str

Lexeme_matcher = re.compile("\\bLexeme\\b", re.UNICODE)
def get_code(CodeFragmentList, variable_db={}, IndentationBase=1):
    global Lexeme_matcher 

    code_str = ""
    for code_info in CodeFragmentList:
        result = code_info.get_code()
        if type(result) == tuple: 
            result, add_variable_db = result
            variable_db.update(add_variable_db)

        if type(result) == list: code_str += "".join(get_plain_strings(result))
        else:                    code_str += result        

    # If 'Lexeme' occurs as an isolated word, then ensure the generation of 
    # a terminating zero. Note, that the occurence of 'LexemeBegin' does not
    # ensure the preparation of a terminating zero.
    require_terminating_zero_f = (Lexeme_matcher.search(code_str) is not None) 

    return pretty_code(code_str, IndentationBase), require_terminating_zero_f

def __prepare(Mode, CodeFragment_or_CodeFragments, ThePattern, 
              Default_ActionF=False, EOF_ActionF=False, SelfCountingActionF=False, BeginOfLineSupportF=False,
              require_terminating_zero_preparation_f=False):
    """-- If there are multiple handlers for a single event they are combined
    
       -- Adding debug information printer (if desired)
    
       -- The task of this function is it to adorn the action code for each pattern with
          code for line and column number counting.
    """
    assert Mode.__class__.__name__  == "Mode"
    assert ThePattern      is None or ThePattern.__class__.__name__ == "Pattern" 
    assert type(Default_ActionF)    == bool
    assert type(EOF_ActionF)        == bool
    # We assume that any state machine presented here has been propperly created
    # and thus contains some side information about newline number, character number etc.

    if type(CodeFragment_or_CodeFragments) == list:
        assert Default_ActionF or EOF_ActionF, \
               "Action code formatting: Multiple Code Fragments can only be specified for default or\n" + \
               "end of stream action."
        CodeFragmentList = CodeFragment_or_CodeFragments
    else:
        CodeFragmentList = [ CodeFragment_or_CodeFragments ]

    user_code     = ""
    variable_db   = {}

    # (*) Code to be performed on every match -- before the related action
    on_match_code = ""
    if Mode.has_code_fragment_list("on_match"):
        on_match_code, rtzp_f = get_code(Mode.get_code_fragment_list("on_match"), variable_db)
        require_terminating_zero_preparation_f = require_terminating_zero_preparation_f or rtzp_f

    # (*) Code to count line and column numbers
    lc_count_code = ""
    if not SelfCountingActionF: 
        lc_count_code = "    %s\n" % __get_line_and_column_counting(ThePattern, EOF_ActionF)

    #if (not Default_ActionF) and (not EOF_ActionF):
    #    lc_count_code += "    __QUEX_ASSERT_COUNTER_CONSISTENCY(&self.counter);\n"

    # (*) THE user defined action to be performed in case of a match
    user_code, rtzp_f = get_code(CodeFragmentList, variable_db)
    require_terminating_zero_preparation_f = require_terminating_zero_preparation_f or rtzp_f

    store_last_character_str = ""
    if BeginOfLineSupportF:
        store_last_character_str = "    %s\n" % LanguageDB.ASSIGN("me->buffer._character_before_lexeme_start", 
                                                                  LanguageDB.INPUT_P_DEREFERENCE(-1))

    set_terminating_zero_str = ""
    if require_terminating_zero_preparation_f:
        set_terminating_zero_str += "    QUEX_LEXEME_TERMINATING_ZERO_SET(&me->buffer);\n"

    txt  = ""
    txt += lc_count_code
    txt += store_last_character_str
    txt += set_terminating_zero_str
    txt += on_match_code
    txt += "    {\n"
    txt += user_code
    txt += "\n    }"

    return CodeFragment(txt), variable_db

def __prepare_end_of_stream_action(Mode, IndentationSupportF, BeginOfLineSupportF):
    if not Mode.has_code_fragment_list("on_end_of_stream"):
        # We cannot make any assumptions about the token class, i.e. whether
        # it can take a lexeme or not. Thus, no passing of lexeme here.
        txt  = "self_send(__QUEX_SETTING_TOKEN_ID_TERMINATION);\n"
        txt += "RETURN;\n"

        Mode.set_code_fragment_list("on_end_of_stream", CodeFragment(txt))

    if IndentationSupportF:
        if Mode.default_indentation_handler_sufficient():
            code = "QUEX_NAME(on_indentation)(me, /*Indentation*/0, LexemeNull);\n"
        else:
            code = "QUEX_NAME(%s_on_indentation)(me, /*Indentation*/0, LexemeNull);\n" % Mode.name

        code_fragment = CodeFragment(code)
        Mode.insert_code_fragment_at_front("on_end_of_stream", code_fragment)

    # RETURNS: end_of_stream_action, db 
    return __prepare(Mode, Mode.get_code_fragment_list("on_end_of_stream"), 
                     None, EOF_ActionF=True, BeginOfLineSupportF=BeginOfLineSupportF)

def __prepare_on_failure_action(Mode, BeginOfLineSupportF, require_terminating_zero_preparation_f):
    if not Mode.has_code_fragment_list("on_failure"):
        txt  = "QUEX_ERROR_EXIT(\"\\n    Match failure in mode '%s'.\\n\"\n" % Mode.name 
        txt += "                \"    No 'on_failure' section provided for this mode.\\n\"\n"
        txt += "                \"    Proposal: Define 'on_failure' and analyze 'Lexeme'.\\n\");\n"
        Mode.set_code_fragment_list("on_failure", CodeFragment(txt))

    # RETURNS: on_failure_action, db 
    return __prepare(Mode, Mode.get_code_fragment_list("on_failure"), 
                     None, Default_ActionF=True, 
                     BeginOfLineSupportF=BeginOfLineSupportF,
                     require_terminating_zero_preparation_f=require_terminating_zero_preparation_f) 

def __get_line_and_column_counting(ThePattern, EOF_ActionF):
    global LanguageDB

    if EOF_ActionF:
        return "__QUEX_COUNT_END_OF_STREAM_EVENT(self.counter);"

    if ThePattern is None:
        return "__QUEX_COUNT_VOID(self.counter);"

    newline_n   = ThePattern.newline_n
    character_n = ThePattern.character_n

    if   newline_n == E_Count.VOID:
        # Run the general algorithm, since not even the number of newlines in the 
        # pattern can be determined directly from the pattern
        return "__QUEX_COUNT_VOID(self.counter);"

    elif newline_n != 0:
        if ThePattern.sm.get_ending_character_set().contains_only(ord('\n')):
            # A pattern that ends with newline, lets the next column start at one.
            return "__QUEX_COUNT_NEWLINE_N_FIXED_COLUMN_N_ZERO(self.counter, %i);" % newline_n
        # TODO: Try to determine number of characters backwards to newline directly
        #       from the pattern state machine. (Those seldom cases won't bring much
        #       speed-up)
        return "__QUEX_COUNT_NEWLINE_N_FIXED_COLUMN_N_VOID(self.counter, %i);" % newline_n

    # Lexeme does not contain newline --> count only columns
    if character_n == E_Count.VOID: incr_str = "LexemeL"
    else:                           incr_str = "%i" % int(character_n)
    return "__QUEX_COUNT_NEWLINE_N_ZERO_COLUMN_N_FIXED(self.counter, %s);" % incr_str

def pretty_code(Code, Base):
    """-- Delete empty lines at the beginning
       -- Delete empty lines at the end
       -- Strip whitespace after last non-whitespace
       -- Propper Indendation based on Indentation Counts

       Base = Min. Indentation
    """
    class Info:
        def __init__(self, IndentationN, Content):
            self.indentation = IndentationN
            self.content     = Content
    info_list           = []
    no_real_line_yet_f  = True
    indentation_set     = set()
    for line in Code.split("\n"):
        line = line.rstrip() # Remove trailing whitespace
        if len(line) == 0 and no_real_line_yet_f: continue
        else:                                     no_real_line_yet_f = False

        content     = line.lstrip()
        if len(content) != 0 and content[0] == "#": indentation = 0
        else:                                       indentation = len(line) - len(content) + Base
        info_list.append(Info(indentation, content))
        indentation_set.add(indentation)

    # Discretize indentation levels
    indentation_list = list(indentation_set)
    indentation_list.sort()

    # Collect the result
    result              = []
    # Reverse so that trailing empty lines are deleted
    no_real_line_yet_f  = True
    for info in reversed(info_list):
        if len(info.content) == 0 and no_real_line_yet_f: continue
        else:                                             no_real_line_yet_f = False
        indentation_level = indentation_list.index(info.indentation)
        result.append("%s%s\n" % ("    " * indentation_level, info.content))

    return "".join(reversed(result))

