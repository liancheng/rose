# The 'grammar' of quex's regular expressions:
#
#  complete expression: expression
#                       expression / expression                 = post conditioned expression
#                       expression / expression /               = pre conditioned expression
#                       expression / expression / expression    = pre and post conditioned expression
# 
#  expression: term
#              term | expression
#  
#  term:  primary
#         primary term
#  
#  primary:  " non_double_quote *  "              = character string
#            [ non_rect_bracket_close ]           = set of characters
#            { identifier }                       = pattern replacement
#            ( expression )
#            non_control_character+               = lonely characters
#            primary repetition_cmd
#  
#  non_double_quote: 'anything but an unbackslashed double quote, i.e. \" is ok, 
#                     but " is not.'      
#  non_rect_bracket_close: 'anything but an unbackslashed rectangular bracket, i.e.
#                           \] is ok, but ] is not.'               
#  non_control_character: 'anything but (, ", [, or {'
#       
#  repetition_cmd: 'a repetition command such as +, *, {2}, {,2}, {2,5}'        
#
#########################################################################################       
import sys
from StringIO import StringIO

from quex.engine.misc.file_in       import error_msg, \
                                           check 

from quex.exception                 import RegularExpressionException
from quex.engine.interval_handling  import Interval, NumberSet
from quex.engine.state_machine.core import StateMachine
from quex.input.regular_expression.auxiliary import __snap_until, \
                                                    __debug_entry, \
                                                    __debug_exit, \
                                                    __debug_print, \
                                                    snap_replacement

from   quex.blackboard                              import setup as Setup
import quex.engine.utf8                             as utf8
import quex.input.regular_expression.character_set_expression   as character_set_expression
import quex.input.regular_expression.snap_backslashed_character as snap_backslashed_character
import quex.input.regular_expression.snap_character_string      as snap_character_string
import quex.input.regular_expression.case_fold_expression       as case_fold_expression
import quex.input.regular_expression.construct                  as construct
import quex.engine.state_machine.sequentialize           as sequentialize
import quex.engine.state_machine.algorithm.beautifier    as beautifier
import quex.engine.state_machine.parallelize             as parallelize
import quex.engine.state_machine.repeat                  as repeat


CONTROL_CHARACTERS = [ "+", "*", "\"", "/", "(", ")", "{", "}", "|", "[", "]", "$"] 

def do(UTF8_String_or_Stream, PatternDict, 
       AllowNothingIsNecessaryF     = False,
       AllowStateMachineTrafoF      = True): 
    assert type(AllowNothingIsNecessaryF) == bool
    assert type(PatternDict) == dict

    def __ensure_whitespace_follows(InitialPos, stream):
        tmp = stream.read(1)
        if tmp == "" or tmp.isspace(): stream.seek(-1, 1); return

        end_position = stream.tell() - 1
        stream.seek(InitialPos)
        pattern_str = stream.read(end_position - InitialPos)
        error_msg("Pattern definition '%s' not followed by whitespace.\n" % pattern_str + \
                  "Found subsequent character '%s'." % tmp, 
                  stream)

    if type(UTF8_String_or_Stream) == str: stream = StringIO(UTF8_String_or_Stream)
    else:                                  stream = UTF8_String_or_Stream    

    if PatternDict is None: PatternDict = {}

    initial_position = stream.tell()

    # -- check for the begin of line condition (BOL)
    if check(stream, '^'): begin_of_line_f = True
    else:                  begin_of_line_f = False
    
    # -- MAIN: transform the pattern into a state machine
    pre, core, post = snap_conditional_expression(stream, PatternDict)

    if core is None: 
        stream.seek(initial_position)
        return None

    # -- check for end of line condition (EOL) 
    # -- check for terminating whitespace
    end_of_line_f = False
    if check(stream, '$'): end_of_line_f = True

    __ensure_whitespace_follows(initial_position, stream)
    
    pattern = construct.do(core_sm         = core, 
                           begin_of_line_f = begin_of_line_f, 
                           pre_context     = pre,
                           end_of_line_f   = end_of_line_f,
                           post_context    = post, 
                           fh              = stream,
                           AllowNothingIsNecessaryF   = AllowNothingIsNecessaryF,
                           AllowStateMachineTrafoF    = AllowStateMachineTrafoF)
    
    return pattern

def snap_conditional_expression(stream, PatternDict):
    """conditional expression: expression
                               expression / expression                 = post conditioned expression
                               expression / expression /               = pre conditioned expression
                               expression / expression / expression    = pre and post conditioned expression
       TODO: <- ($8592) for pre-conditions
             -> ($8594) for post-conditions


        RETURNS: pre_context, core_pattern, post_context
    """                     
    __debug_entry("conditional expression", stream)    

    # -- expression
    pattern_0 = snap_expression(stream, PatternDict) 
    if pattern_0 is None: 
        return None, None, None
    
    # -- '/'
    if not check(stream, '/'): 
        # (1) expression without pre and post condition
        #     pattern_0 is already beautified by 'snap_expression()'
        return None, pattern_0, None
        
    # -- expression
    pattern_1 = snap_expression(stream, PatternDict) 
    if pattern_1 is None: 
        return None, pattern_0, None
    
    # -- '/'
    if not check(stream, '/'): 
        # (2) expression with only a post condition
        #     NOTE: setup_post_context() marks state origins!
        return None, pattern_0, pattern_1

    # -- expression
    pattern_2 = snap_expression(stream, PatternDict) 
    if pattern_2 is None: 
        # (3) expression with only a pre condition
        #     NOTE: setup_pre_context() marks the state origins!
        return pattern_0, pattern_1, None
    else:
        # (4) expression with post and pre-condition
        return pattern_0, pattern_1, pattern_2

def snap_expression(stream, PatternDict):
    """expression:  term
                    term | expression
    """              
    __debug_entry("expression", stream)    
    # -- term
    result = snap_term(stream, PatternDict) 
    if result is None: 
        return __debug_exit(None, stream)

    # -- optional '|'
    if not check(stream, '|'): 
        return __debug_exit(result, stream)
    
    position_1 = stream.tell()
    __debug_print("'|' (in expression)")

    # -- expression
    result_2 = snap_expression(stream, PatternDict) 
    __debug_print("expression(in expression):",  result_2)
    if result_2 is None:
        stream.seek(position_1) 
        return __debug_exit(result, stream)

    result = parallelize.do([result, result_2], CloneF=True)   # CloneF = False (shold be!)
    return __debug_exit(beautifier.do(result), stream)
        
def snap_term(stream, PatternDict):
    """term:  primary
              primary term 
    """
    __debug_entry("term", stream)    

    # -- primary
    result = snap_primary(stream, PatternDict) 
    __debug_print("##primary(in term):", result)
    if result is None: return __debug_exit(None, stream)
    position_1 = stream.tell()

    # -- optional 'term' 
    result_2 = snap_term(stream, PatternDict) 
    __debug_print("##term(in term):",  result_2)
    if result_2 is None: 
        stream.seek(position_1)
        return __debug_exit(result, stream)
    
    ## print "##1:", result.get_string(NormalizeF=False)
    ## print "##2:", result_2.get_string(NormalizeF=False)
    result = sequentialize.do([result, result_2], 
                              MountToFirstStateMachineF=True, 
                              CloneRemainingStateMachinesF=False)    

    return __debug_exit(beautifier.do(result), stream)
        
def snap_primary(stream, PatternDict):
    """primary:  " non_double_quote *  "              = character string
                 [ non_rect_bracket_close ]           = set of characters
                 { identifier }                       = pattern replacement
                 ( expression )
                 non_control_character+               = lonely characters
                 primary repetition_cmd
    """
    __debug_entry("primary", stream)    
    x = stream.read(1); lookahead = stream.read(1); 
    if x != "" and lookahead != "": stream.seek(-1, 1)
    if x == "": return __debug_exit(None, stream)

    # -- 'primary' primary
    if   x == "\"": result = snap_character_string.do(stream)
    elif x == "[":  
        stream.seek(-1, 1); 
        result = character_set_expression.do(stream, PatternDict)
    elif x == "{":  result = snap_replacement(stream, PatternDict)
    elif x == ".":  result = create_ALL_BUT_NEWLINE_state_machine()
    elif x == "(":  result = snap_bracketed_expression(stream, PatternDict)

    elif x.isspace():
        # a lonestanding space ends the regular expression
        stream.seek(-1, 1)
        return __debug_exit(None, stream)

    elif x in ["*", "+", "?"]:
        raise RegularExpressionException("lonely operator '%s' without expression proceeding." % x) 

    elif x == "\\":
        if lookahead == "C":
            stream.read(1)
            result = snap_case_folded_pattern(stream, PatternDict)
        elif lookahead == "R":
            result = get_expression_in_brackets(stream, PatternDict, "reverse operator", "R").get_inverse()
        elif lookahead == "A":
            result =  get_expression_in_brackets(stream, PatternDict, "anti-pattern operator", "A")
            result.transform_to_anti_pattern()
        else:
            stream.seek(-1, 1)
            trigger_set = character_set_expression.snap_property_set(stream)
            if trigger_set is None:
                stream.seek(1, 1)  # snap_property_set() leaves tream right before '\\'
                char_code = snap_backslashed_character.do(stream)
                if char_code is None:
                    raise RegularExpressionException("Backslash followed by unrecognized character code.")
                trigger_set = char_code
            result = StateMachine()
            result.add_transition(result.init_state_index, trigger_set, AcceptanceF=True)

    elif x not in CONTROL_CHARACTERS:
        # NOTE: The '\' is not inside the control characters---for a reason.
        #       It is used to define for example character codes using '\x' etc.
        stream.seek(-1, 1)
        result = snap_non_control_character(stream, PatternDict)

    else:
        # NOTE: This includes the '$' sign which means 'end of line'
        #       because the '$' sign is in CONTROL_CHARACTERS, but is not checked
        #       against. Thus, it it good to leave here on '$' because the
        #       '$' sign is handled on the very top level.
        # this is not a valid primary
        stream.seek(-1, 1)
        return __debug_exit(None, stream)

    # -- optional repetition command? 
    result_repeated = __snap_repetition_range(result, stream) 
    if result_repeated is not None: result = result_repeated
    return __debug_exit(beautifier.do(result), stream)
    # return __debug_exit(result, stream)
    
def snap_non_control_character(stream, PatternDict):
    __debug_entry("non-control characters", stream)

    # (*) read first character
    char_code = utf8.__read_one_utf8_code_from_stream(stream)
    if char_code is None:
        error_msg("Character could not be interpreted as UTF8 code or End of File reached prematurely.", 
                  stream)
    result = StateMachine()
    result.add_transition(result.init_state_index, char_code, AcceptanceF=True)
    return __debug_exit(result, stream)
    
def __snap_repetition_range(the_state_machine, stream):    
    """Snaps a string that represents a repetition range. The following 
       syntaxes are supported:
           '?'      one or none repetition
           '+'      one or arbitrary repetition
           '*'      arbitrary repetition (even zero)
           '{n}'    exactly 'n' repetitions
           '{m,n}'  from 'm' to 'n' repetitions
           '{n,}'   arbitrary, but at least 'n' repetitions
    """       
    assert the_state_machine.__class__.__name__ == "StateMachine", \
           "received object of type '%s'" % the_state_machine.__class__.__name__ + "\n" + \
           repr(the_state_machine)

    position_0 = stream.tell()
    x = stream.read(1)
    if   x == "+": result = repeat.do(the_state_machine, 1)
    elif x == "*": result = repeat.do(the_state_machine)
    elif x == "?": result = repeat.do(the_state_machine, 0, 1)
    elif x == "{":
        repetition_range_str = __snap_until(stream, "}")
        if len(repetition_range_str) and not repetition_range_str[0].isdigit():
            # no repetition range, so everything remains as it is
            stream.seek(position_0)
            return the_state_machine
            
        try:
            if repetition_range_str.find(",") == -1:
                # no ',' thus "match exactly a certain number": 
                # e.g. {4} = match exactly four repetitions
                number = int(repetition_range_str)
                result = repeat.do(the_state_machine, number, number)
                return result
            # a range of numbers is given       
            fields = repetition_range_str.split(",")
            fields = map(lambda x: x.strip(), fields)

            number_1 = int(fields[0].strip())
            if fields[1] == "": number_2 = -1                      # e.g. {2,}
            else:               number_2 = int(fields[1].strip())  # e.g. {2,5}  
            # produce repeated state machine 
            result = repeat.do(the_state_machine, number_1, number_2)
            return result
        except:
            raise RegularExpressionException("error while parsing repetition range expression '%s'" \
                                             % repetition_range_str)
    else:
        # no repetition range, so everything remains as it is
        stream.seek(position_0)
        return the_state_machine
    
    return result

def create_ALL_BUT_NEWLINE_state_machine():
    global Setup
    result = StateMachine()
    # NOTE: Buffer control characters are supposed to be filtered out by the code
    #       generator.
    trigger_set = NumberSet(Interval(ord("\n")).inverse()) 

    if Setup.get_character_value_limit() != sys.maxint:
        trigger_set.intersect_with(Interval(0, Setup.get_character_value_limit()))

    result.add_transition(result.init_state_index, trigger_set, AcceptanceF=True) 
    return result
    
def snap_bracketed_expression(stream, PatternDict):
    position = stream.tell()
    result = snap_expression(stream, PatternDict)
    if not check(stream, ")"): 
        stream.seek(position)
        remainder_txt = stream.readline().replace("\n", "").replace("\r", "")
        raise RegularExpressionException("Missing closing ')' after expression; found '%s'.\n" % remainder_txt \
                                         + "Note, that patterns end with the first non-quoted whitespace.\n" \
                                         + "Also, closing brackets in quotes do not close a syntax block.")

    if result is None:
        length = stream.tell() - position
        stream.seek(position)
        raise RegularExpressionException("expression in brackets has invalid syntax '%s'" % \
                                         stream.read(length))
    return result

def snap_case_folded_pattern(sh, PatternDict, CharacterSetF=False):
    """Parse a case fold expression of the form \C(..){ R } or \C{ R }.
       Assume that '\C' has been snapped already from the stream.

       See function ucs_case_fold_parser.get_fold_set() for details
       about case folding.
    """
    return case_fold_expression.do(sh, PatternDict, snap_expression)

def get_expression_in_brackets(stream, PatternDict, Name, TriggerChar):
    # Read over the trigger character 
    stream.read(1)
    if not check(stream, "{"):
        error_msg("Missing opening '{' after %s %s." % (Name, TriggerChar), stream)
    pattern = snap_expression(stream, PatternDict) 
    if check(stream, "}"):
        return pattern
    elif check(stream, "/") or check(stream, "$"):
        error_msg("Pre- or post contexts are not allowed in %s \\%s{...} expressions." % (Name, TriggerChar), stream)
    else:
        error_msg("Missing closing '}' %s in \\%s{...}." % (Name, TriggerChar), stream)
    return pattern
