# (C) 2009 Frank-Rene Schaefer
import os
import sys
from   quex.engine.state_machine.core                  import StateMachine
import quex.engine.state_machine.utf8_state_split      as utf8_state_split
import quex.engine.state_machine.utf16_state_split     as utf16_state_split
import quex.engine.state_machine.algorithm.beautifier            as beautifier
from   quex.engine.misc.file_in                        import error_msg
from   quex.blackboard                                 import setup as Setup

sys.path.append(os.environ["QUEX_PATH"])

def try_this(X, FH=None):
    """Transforms a given state machine from 'Unicode Driven' to another
       character encoding type.
    
       RETURNS: Transformed state machine. It may be the same as it was 
                before if there was no transformation actually.
    """
    if X is None: return X

    assert isinstance(X, StateMachine)
    assert X.is_DFA_compliant()

    TrafoInfo = Setup.buffer_codec_transformation_info
    if TrafoInfo is None: return X

    if isinstance(TrafoInfo, (str, unicode)):
        if   TrafoInfo == "utf8-state-split":  return __DFA(utf8_state_split.do(X))
        elif TrafoInfo == "utf16-state-split": return __DFA(utf16_state_split.do(X))
        else:                                  assert False
    
    # Pre-condition SM is transformed inside the member function
    X.transform(TrafoInfo)
    return __DFA(X)

def __DFA(SM):
    if   SM is None:            return None
    elif SM.is_DFA_compliant(): return SM

    result = beautifier.do(SM)
    return result

def do_set(number_set, TrafoInfo, FH=-1):
    """RETURNS: True  transformation successful
                False transformation failed, number set possibly in inconsistent state!
    """
    assert TrafoInfo is not None
    assert TrafoInfo.__class__.__name__ == "NumberSet"

    if type(TrafoInfo) == str:
        if TrafoInfo == "utf8-state-split": 
            result = utf8_state_split.do_set(number_set)
            if result is None:
                error_msg("Operation 'number set transformation' failed 'utf8'.\n" + \
                          "The given number set results in a state sequence not a single transition.", FH) 
            return result
        # Other transformations are not supported
        assert False
    
    return number_set.transform(TrafoInfo)
        

