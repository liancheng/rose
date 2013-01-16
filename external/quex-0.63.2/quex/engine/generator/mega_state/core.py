from   quex.blackboard                                   import setup as Setup, E_StateIndices
from   quex.engine.analyzer.mega_state.template.state    import TemplateState
from   quex.engine.analyzer.mega_state.path_walker.state import PathWalkerState
from   quex.engine.analyzer.mega_state.core         import MegaState_Target_DROP_OUT
from   quex.engine.generator.state.transition.code  import TextTransitionCode
import quex.engine.generator.state.drop_out         as drop_out_coder
import quex.engine.generator.state.entry            as entry_coder
import quex.engine.generator.state.transition.core  as transition_block
import quex.engine.generator.mega_state.template    as template
import quex.engine.generator.mega_state.path_walker as path_walker
from   quex.engine.generator.languages.address      import get_label, get_address
from   quex.engine.generator.languages.variable_db  import variable_db
from   quex.engine.interval_handling                import Interval
import sys

class Handler:
    def __init__(self, TheState):
        if isinstance(TheState, PathWalkerState):
            self.require_data = path_walker.require_data
            self.framework    = path_walker.framework   
            self.state_key_str      = "path_iterator - path_walker_%s_path_base" % TheState.index 
            self.debug_drop_out_str = "__quex_debug_path_walker_drop_out(%i, path_walker_%s_path_base, path_iterator);\n" \
                                      % (TheState.index, TheState.index)
        elif isinstance(TheState, TemplateState):
            self.require_data = template.require_data
            self.framework    = template.framework   
            self.state_key_str      = "state_key"
            self.debug_drop_out_str = "__quex_debug_template_drop_out(%i, state_key);\n" % TheState.index
        else:
            assert False

        self.state = TheState

    def debug_info_map_state_key_to_state_index(self, txt):
        txt.append("#   define __QUEX_DEBUG_MAP_STATE_KEY_TO_STATE(X) ( \\\n")
        for state_index in self.state.implemented_state_index_list()[:-1]:
            state_key = self.state.map_state_index_to_state_key(state_index)
            txt.append("             (X) == %i ? %i :    \\\n" % (state_key, state_index))

        state_index = self.state.implemented_state_index_list()[-1]
        state_key   = self.state.map_state_index_to_state_key(state_index)
        txt.append("             (X) == %i ? %i : 0)" % (state_key, state_index))

        if isinstance(self.state, PathWalkerState):
            txt.append("\n#   define __QUEX_DEBUG_MAP_PATH_BASE_TO_PATH_ID(PB) ( \\\n")
            for path_id in xrange(len(self.state.path_list) - 1):
                txt.append("             (PB) == path_walker_%i_path_%i ? %i :    \\\n" \
                           % (self.state.index, path_id, path_id))
            path_id = len(self.state.path_list) - 1
            txt.append("             (PB) == path_walker_%i_path_%i ? %i : 0)" \
                       % (self.state.index, path_id, path_id))

    def debug_info_undo_map_state_key_to_state_index(self, txt):
        txt.append("\n#   undef __QUEX_DEBUG_MAP_STATE_KEY_TO_STATE\n")
        if isinstance(self.state, PathWalkerState):
            txt.append("#   undef __QUEX_DEBUG_MAP_PATH_BASE_TO_PATH_ID\n")

def do(txt, TheState, TheAnalyzer):
    specific = Handler(TheState)

    specific.debug_info_map_state_key_to_state_index(txt)

    # (*) Entry _______________________________________________________________
    entry_coder.do(txt, TheState, TheAnalyzer) 

    # (*) Access input character etc. _________________________________________
    specific.framework(txt, TheState, TheAnalyzer)

    # (*) Transition Map ______________________________________________________
    prepare_transition_map(TheState, TheAnalyzer, specific.state_key_str)
    transition_block.do(txt, 
                        TheState.transition_map, 
                        TheState.index, 
                        TheAnalyzer.engine_type, 
                        TheState.init_state_f, 
                        TheAnalyzer = TheAnalyzer) 

    # (*) Drop Out ____________________________________________________________
    drop_out_scheme_implementation(txt, TheState, TheAnalyzer, 
                                   specific.state_key_str, 
                                   specific.debug_drop_out_str)

    # (*) Request necessary variable definition _______________________________
    specific.require_data(TheState, TheAnalyzer)

    specific.debug_info_undo_map_state_key_to_state_index(txt)
    return

def drop_out_scheme_implementation(txt, TheState, TheAnalyzer, StateKeyString, DebugString):
    """DropOut Section:

       The drop out section is the place where we come if the transition map
       does not trigger to another state. We also land here if the reload fails.
       The routing to the different drop-outs of the related states happens by 
       means of a switch statement, e.g.
       
       _4711: /* Address of the drop out */
           switch( state_key ) {
           case 0:
                 ... drop out of state 815 ...
           case 1: 
                 ... drop out of state 541 ...
           }

       The switch statement is not necessary if all drop outs are the same, 
       of course.
    """
    LanguageDB = Setup.language_db
    # (*) Central Label for the Templates Drop Out
    #     (The rules for having or not having a label here are complicated, 
    #      so rely on the label's usage database.)
    txt.append("%s:\n" % get_label("$drop-out", TheState.index))
    txt.append("    %s\n" % DebugString) 

    def implement_prototype(StateIndices, TheAnalyzer):
        # There **must** be at least one element, at this point in time
        assert len(StateIndices) != 0
        prototype_i = StateIndices.__iter__().next()
        prototype   = TheAnalyzer.state_db[prototype_i]
        result      = []
        drop_out_coder.do(result, prototype, TheAnalyzer, \
                          DefineLabelF=False, MentionStateIndexF=False)
        return result


    # (*) Drop Out Section(s)
    if TheState.drop_out.uniform_f:
        # uniform drop outs => no 'switch-case' required
        txt.extend(implement_prototype(TheState.implemented_state_index_list(), TheAnalyzer))
        return

    # non-uniform drop outs => route by 'state_key'
    case_list = []
    for drop_out, state_index_set in TheState.drop_out.iteritems():
        # state keys related to drop out
        state_key_list = map(lambda i: TheState.map_state_index_to_state_key(i), state_index_set)
        # drop out action
        # Implement drop-out for each state key. 'state_key_list' combines
        # states that implement the same drop-out behavior. Same drop-outs
        # are implemented only once.
        case_list.append( (state_key_list, implement_prototype(state_index_set, TheAnalyzer)) )

    case_txt = LanguageDB.SELECTION(StateKeyString, case_list)
    LanguageDB.INDENT(case_txt)
    txt.extend(case_txt)

def prepare_transition_map(TheState, TheAnalyzer, StateKeyStr):
    """Generate targets in the transition map which the code generation can 
       handle. The transition map will consist of pairs of
    
                          (Interval, TextTransitionCode)
    
       objects. 

       NOTE: A word about the reload procedure.
       
       Reload can end either with success (new data has been loaded), or failure
       (no more data available). In case of success the **only** the transition
       step has to be repeated. Nothing else is effected.  Stored positions are
       adapted automatically.
       
       By convention we redo the transition map, in case of reload success and 
       jump to the state's drop-out in case of failure. There is no difference
       here in the template state example.
    """
    # Transition map of the 'skeleton'        
    if TheState.transition_map_empty_f:
        # Transition Map Empty:
        # This happens, for example, if there are only keywords and no 
        # 'overlaying' identifier pattern. But, in this case also, there
        # must be something that catches the 'buffer limit code'. 
        # => Define an 'all drop out' trigger_map, and then later
        # => Adapt the trigger map, so that the 'buffer limit' is an 
        #    isolated single interval.
        TheState.transition_map = [ (Interval(-sys.maxint, sys.maxint), MegaState_Target_DROP_OUT) ]

    for i, info in enumerate(TheState.transition_map):
        interval, target = info
        new_target = prepare_target(target, TheState, TheAnalyzer.state_db, StateKeyStr)
        TheState.transition_map[i] = (interval, new_target)

    return

def prepare_target(Target, TheState, StateDB, StateKeyStr):
    LanguageDB = Setup.language_db

    if Target.drop_out_f:
        code = LanguageDB.GOTO_DROP_OUT(TheState.index)
        return E_StateIndices.DROP_OUT

    elif Target.target_state_index is not None:
        # NOTE: Not all transitions of from 'x' to 'Target.target_state_index' may
        #       be relevant. For example, if the transition lies on a uniform path
        #       which is implemented by the MegaState. The MegaState indicates
        #       the irrelevance by deleting the transition_id. 
        # HOWEVER: If no transition_id is found, then transition_map is erroneous!
        for from_state_index in TheState.implemented_state_index_list():
            target_entry     = StateDB[Target.target_state_index].entry
            door_id          = target_entry.get_door_id(Target.target_state_index, from_state_index)
            if door_id is not None: 
                return TextTransitionCode([LanguageDB.GOTO_BY_DOOR_ID(door_id)])
        else:
            assert False, "TransitionID was not resolved in target state's entry."

    elif Target.target_door_id is not None:
        return TextTransitionCode([LanguageDB.GOTO_BY_DOOR_ID(Target.target_door_id)])

    elif Target.scheme is not None:
        label = "template_%i_target_%i[%s]" % (TheState.index, Target.scheme_id, StateKeyStr)
        code  = LanguageDB.GOTO_BY_VARIABLE(label)
        require_scheme_variable(Target.scheme_id, Target.scheme, TheState, StateDB)
        return TextTransitionCode([code])

    else:
        assert False

def require_scheme_variable(SchemeID, Scheme, TState, StateDB):
    """Defines the transition targets for each involved state. Note, that recursion
       is handled as part of the general case, where all involved states target 
       a common door of the template state.
    """
    LanguageDB = Setup.language_db

    def get_code(AdrList):
        last_i = len(AdrList) - 1
        txt = ["{ "]
        for i, adr in enumerate(AdrList):
            if i != last_i:
                txt.append("%s, " % LanguageDB.LABEL_BY_ADDRESS(adr)) 
            else:
                txt.append("%s " % LanguageDB.LABEL_BY_ADDRESS(adr)) 
        txt.append(" }")
        return "".join(txt)

    assert len(Scheme) == len(TState.implemented_state_index_list())
    def address(Target, StateKey, TheState):
        if Target == E_StateIndices.DROP_OUT:
            # All drop outs end up at the end of the transition map, where
            # it is routed via the state_key to the state's particular drop out.
            return get_address("$drop-out", TState.index, U=True, R=True)

        from_state_index = TheState.map_state_key_to_state_index(StateKey)
        door_id          = StateDB[Target].entry.get_door_id(Target, 
                                                             FromStateIndex=from_state_index)

        if door_id is None:
            # IMPORTANT NOTE: (This case is separated to make this comment)
            #
            # A MegaState's transition map may be partly covered by the
            # MegaState's head.  This implies, that not all implemented
            # states trigger to the state mentioned in the transition map.
            # (A 'pseudo-common' .target_state_index may be split into a
            # scheme, because the entering doors differ.) As a result the
            # '.get_door_id()' may result in a totally legal 'None' for
            # a particular 'state_key'.
            # 
            # Later: 'LABEL_BY_ADDRESS(None) --> "QUEX_GOTO_LABEL_VOID"
            return None 
        else:
            return LanguageDB.ADDRESS_BY_DOOR_ID(door_id)

    address_list = [ address(target_index, state_key, TState) \
                     for state_key, target_index in enumerate(Scheme) ]

    variable_db.require_array("template_%i_target_%i", 
                              ElementN = len(TState.implemented_state_index_list()), 
                              Initial  = get_code(address_list),
                              Index    = (TState.index, SchemeID))


