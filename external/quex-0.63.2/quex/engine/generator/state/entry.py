from quex.engine.analyzer.state.core                   import AnalyzerState
from quex.engine.analyzer.mega_state.path_walker.state import PathWalkerState
from quex.blackboard import setup as Setup,  \
                            E_EngineTypes

from operator import attrgetter

def do(txt, TheState, TheAnalyzer, UnreachablePrefixF=True, LabelF=True):
    """Writes code for the state entry into 'txt'.

       RETURNS: True -- if further code for the transition block and the 
                        drop out is required.
                False -- if no further code is required.
    """
    assert isinstance(TheState, AnalyzerState)
    assert type(UnreachablePrefixF) == bool
    assert type(LabelF) == bool
    LanguageDB = Setup.language_db

    txt.append("\n\n")
    if      UnreachablePrefixF \
        and (    (not TheState.init_state_f) \
              or (TheState.engine_type == E_EngineTypes.BACKWARD_INPUT_POSITION)): 
        txt.append("    %s\n" % LanguageDB.UNREACHABLE)

    if TheAnalyzer.engine_type == E_EngineTypes.BACKWARD_INPUT_POSITION:
        BIPD_ID = TheAnalyzer.state_machine_id
    else:
        BIPD_ID = None

    doit(txt, TheState, TheState.entry.door_tree_root, LastChildF=False, BIPD_ID=BIPD_ID)

    return True

def doit(txt, TheState, Node, LastChildF=False, BIPD_ID=None):
    LanguageDB = Setup.language_db
    LastI      = len(Node.child_list) - 1
    for i, child in enumerate(sorted(Node.child_list, key=attrgetter("door_id"))):
        doit(txt, TheState, child, LastChildF=(i==LastI), BIPD_ID=BIPD_ID)
    
    # If the door can be a 'goto' target, the label needs to be defined.
    if TheState.init_state_f and BIPD_ID is not None:
        txt.append(LanguageDB.LABEL_BACKWARD_INPUT_POSITION_DETECTOR(BIPD_ID))
    elif Node.door_id.door_index != 0:
        txt.append(LanguageDB.LABEL_BY_DOOR_ID(Node.door_id))
    else:
        # 'Door 0' is needed if:  -- There is transition to 'Door 0'.
        #                         -- A reload procedure is involved. Reload requires
        #                            to go back to Door 0 after reload.
        #                         -- 'Door 0' has more than one child. The second
        #                            child needs to goto 'Door 0'.
        #                         -- The state is a PathWalkerState which is uniform.
        # Reload is involved if:      The transition map is not empty.
        #                         and Not in Backward Input Position Detection Mode.
        transition_id_list     = TheState.entry.transition_db.get(Node.door_id)
        has_transition_f       = (transition_id_list is not None) and len(transition_id_list) != 0
        has_reload_f           = len(TheState.transition_map) != 0 and BIPD_ID is None
        has_multiple_childs_f  = len(Node.child_list) > 1
        is_uniform_path_walker_state_f = isinstance(TheState, PathWalkerState) and \
                                         TheState.uniform_entry_door_id_along_all_paths is not None
        if has_transition_f or has_reload_f or has_multiple_childs_f or is_uniform_path_walker_state_f:
            txt.append(LanguageDB.LABEL_BY_DOOR_ID(Node.door_id))

    comment_door(txt, Node, TheState.entry)

    action_txt = [ LanguageDB.COMMAND(command) for command in Node.common_command_list ]
    if Node.parent is not None and not LastChildF: 
        action_txt.append("    ")
        action_txt.append(LanguageDB.GOTO_BY_DOOR_ID(Node.parent.door_id))
    txt.extend(action_txt)
    txt.extend("\n")

def comment_door(txt, Node, TheEntry):
    LanguageDB = Setup.language_db

    # If the door is entered by another state, write a comment from where it is entered.
    transition_id_list = TheEntry.transition_db[Node.door_id]
    if len(transition_id_list) != 0:
        txt.append(" ")
        LanguageDB.COMMENT(txt, "".join([ "(%s from %s) " % (x.state_index, x.from_state_index) for x in transition_id_list])[:-1])
    else:
        txt.append("\n") 

