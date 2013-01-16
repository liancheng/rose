from   quex.engine.analyzer.core       import Analyzer
from   quex.engine.analyzer.state.core import AnalyzerState, \
                                              get_input_action
import quex.engine.generator.state.transition.core  as transition_block
import quex.engine.generator.state.entry            as entry
import quex.engine.generator.state.drop_out         as drop_out
from   quex.blackboard import E_StateIndices, \
                              E_EngineTypes,  \
                              setup as Setup

LanguageDB = None

def do(code, TheState, TheAnalyzer):
    global LanguageDB
    assert isinstance(TheState, AnalyzerState)
    assert isinstance(TheAnalyzer, Analyzer)

    LanguageDB = Setup.language_db
    txt        = []

    # (*) Entry _______________________________________________________________
    if not TheState.init_state_forward_f:
        entry.do(txt, TheState, TheAnalyzer)
    else:
        # There is something special about the init state in forward direction:
        # It does not increment the input pointer initially. But when it is entered
        # from other states, is has to do so. Solution: Implement init state entry
        # as 'prologue' here (without increment) and epilogue (with increment) after 
        # the state. 
        txt.append(LanguageDB.LABEL_INIT_STATE_TRANSITION_BLOCK())

    # (*) Access the triggering character _____________________________________
    input_do(txt, TheState, TheAnalyzer)
    LanguageDB.STATE_DEBUG_INFO(txt, TheState)

    # (*) Transition Map ______________________________________________________
    transition_block.do(txt, 
                        TheState.transition_map, 
                        TheState.index, 
                        TheState.engine_type, 
                        TheState.init_state_f, 
                        TheAnalyzer=TheAnalyzer)

    # (*) Drop Out ____________________________________________________________
    drop_out.do(txt, TheState, TheAnalyzer)

    # ( ) Init state prologue (if necessary)
    if TheState.init_state_forward_f:
        init_state_forward_epilog(txt, TheState, TheAnalyzer)

    # (*) Cleaning Up _________________________________________________________
    for i, x in enumerate(txt):
        assert not isinstance(x, list), repr(txt[i-2:i+2])
        assert not x is None, txt[i-2:i+2]

    code.extend(txt)

def input_do(txt, TheState, TheAnalyzer, ForceInputDereferencingF=False):
    """Generate the code fragment that accesses the 'input' character for
       the subsequent transition map. In general this consists of 

            -- increment/decrement (if not init state forward)
            -- dereference the input pointer

       The initial state in forward lexing is an exception! The input pointer
       is not increased, since it already stands on the right position from
       the last analyzis step. When the init state is entered from any 'normal'
       state it enters via the 'epilog' generated in the function 
       init_state_forward_epilog().
    """
    LanguageDB = Setup.language_db
    action = get_input_action(TheAnalyzer.engine_type, TheState, ForceInputDereferencingF)
    LanguageDB.ACCESS_INPUT(txt, action)

def init_state_forward_entry(txt, TheState):
    global LanguageDB

    entry._accepter(txt, TheState.entry.get_accepter())

def init_state_forward_epilog(txt, TheState, TheAnalyzer):
    assert TheState.init_state_forward_f
    global LanguageDB

    entry.do(txt, TheState, TheAnalyzer)
    txt.extend([
        "\n", 
        "    %s\n" % LanguageDB.INPUT_P_INCREMENT(),
        "    %s\n" % LanguageDB.GOTO(E_StateIndices.INIT_STATE_TRANSITION_BLOCK),
    ])
    return txt

def UNUSED_is_state_entered_from_some_other_state(TheState):
    """___________________________________________________________________________
       THIS FUNCTION MAY POTENTIALLY BE USED ONCE WE THINK THAT '--BUFFER-ONLY' OR
       SO, WOULD NOT REQUIRE RELOAD AND THE STATE ROUTER WAS NOT IMPLEMENTED.

       Currently, the entry to the init state is always needed, since the state
       router after the reload requires an address for the init state.
    
       RETURNS: True  -- if state is entered from some other state.
                False -- if state is not entered at all from any other state.
    """
    if TheState.engine_type != E_EngineTypes.BACKWARD_INPUT_POSITION:
        # There is possibly always reload involved and reload requires a jump
        # to the state where the buffer border was hit.
        # => Whenever reload is possible, the state is targetted.
        # In backward input position detection, no reload is possible. The whole
        # lexeme is inside the buffer. Backward input position happens only inside the
        # current lexeme.
        return True

    door_tree_root = TheState.entry.door_tree_root
    if len(door_tree_root.child_list) != 0:   # Childs are there for entries from other states ...
        return True
    elif len(door_tree_root.door_list) == 0:  # No childs, no doors => no entries from other states
        return False 
    elif len(door_tree_root.door_list) == 1 and door_tree_root.door_list[0] is E_StateIndices.NONE: 
        # Only entry is from state 'NONE' => no entries from other states.
        return False
    return True
