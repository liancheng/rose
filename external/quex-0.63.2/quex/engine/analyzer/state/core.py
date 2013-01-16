from   quex.engine.state_machine.core      import State
from   quex.engine.analyzer.state.entry    import Entry
from   quex.engine.analyzer.state.drop_out import DropOutBackward, \
                                                  DropOutBackwardInputPositionDetection
from   quex.blackboard  import E_StateIndices, \
                               E_EngineTypes, \
                               E_InputActions

class AnalyzerState(object):
    """AnalyzerState consists of the following major components:

       entry -- tells what has to happen at entry to the state (depending 
                on the state from which it is entered).

       input -- determined how to access the character that is used for 
                transition triggering.

       transition_map -- telling what subsequent state is to be entered
                         dependent on the triggering character.

       drop_out -- contains information about what happens when the 
                   transition map cannot trigger on the character.
    """
    __slots__ = ("__index", 
                 "__init_state_f", 
                 "__target_index_list", 
                 "__engine_type", 
                 "__state_machine_id",
                 "input", 
                 "entry", 
                 "map_target_index_to_character_set", 
                 "transition_map", 
                 "drop_out", 
                 "_origin_list")

    def __init__(self, SM_State, StateIndex, InitStateF, EngineType, FromStateIndexList):
        assert isinstance(SM_State, State)
        assert isinstance(StateIndex, (int, long))
        assert type(InitStateF) is bool
        assert EngineType in E_EngineTypes
        assert isinstance(FromStateIndexList, set)

        self.__index        = StateIndex
        self.__init_state_f = InitStateF
        self.__engine_type  = EngineType

        if self.__init_state_f: 
            if E_StateIndices.NONE not in FromStateIndexList:
                FromStateIndexList.add(E_StateIndices.NONE)

        # Test "quex/engine/analyzer/mega_state/template/TEST/best_matching_pair.py" would not work!
        # assert len(FromStateIndexList) != 0

        # (*) Entry Action
        if   EngineType == E_EngineTypes.FORWARD: 
            self.entry = Entry(self.__index, FromStateIndexList)
        elif EngineType == E_EngineTypes.BACKWARD_PRE_CONTEXT: 
            pre_context_id_fulfilled_list = [ origin.pattern_id() for origin in SM_State.origins() \
                                                                  if origin.is_acceptance() ]
            self.entry = Entry(self.__index, FromStateIndexList, pre_context_id_fulfilled_list)
        elif EngineType == E_EngineTypes.BACKWARD_INPUT_POSITION: 
            self.entry = Entry(self.__index, FromStateIndexList)
        else:
            assert False

        # (*) Transition
        if EngineType == E_EngineTypes.BACKWARD_INPUT_POSITION:
            # During backward input detection, an acceptance SM_State triggers a
            # return from the searcher, thus no further transitions are necessary.
            # (orphaned states, also, need to be deleted).
            ## if SM_State.is_acceptance(): assert state.transitions().is_empty()
            pass

        self.transition_map                    = SM_State.transitions().get_trigger_map()
        self.__target_index_list               = SM_State.transitions().get_map().keys()
        # Currently, the following is only used for path compression. If the alternative
        # is implemented, then the following is no longer necessary.
        self.map_target_index_to_character_set = SM_State.transitions().get_map()

        # (*) Drop Out
        if   EngineType == E_EngineTypes.FORWARD: 
            # DropOut and Entry interact and require sophisticated analysis
            # => See "Analyzer.get_drop_out_object(...)"
            self.drop_out = None 

        elif EngineType == E_EngineTypes.BACKWARD_PRE_CONTEXT:
            self.drop_out = DropOutBackward()
        elif EngineType == E_EngineTypes.BACKWARD_INPUT_POSITION:
            self.drop_out = DropOutBackwardInputPositionDetection(SM_State.is_acceptance())

        self._origin_list = SM_State.origins().get_list()

    @property
    def index(self):                  return self.__index
    def set_index(self, Value):       assert isinstance(Value, long); self.__index = Value
    @property
    def init_state_f(self):           return self.__init_state_f
    @property
    def init_state_forward_f(self):   return self.__init_state_f and self.__engine_type == E_EngineTypes.FORWARD
    @property
    def engine_type(self):            return self.__engine_type
    def set_engine_type(self, Value): assert Value in E_EngineTypes; self.__engine_type = Value     
    @property
    def target_index_list(self):      return self.__target_index_list
    @property
    def transition_map_empty_f(self): 
        L = len(self.transition_map)
        if   L > 1:  return False
        elif L == 0: return True
        elif self.transition_map[0][1] == E_StateIndices.DROP_OUT: return True
        return False

    def get_string_array(self, InputF=True, EntryF=True, TransitionMapF=True, DropOutF=True):
        txt = [ "State %s:\n" % repr(self.index).replace("L", "") ]
        # if InputF:         txt.append("  .input: move position %s\n" % repr(self.input))
        if EntryF:         txt.append("  .entry:\n"); txt.append(repr(self.entry))
        if TransitionMapF: txt.append("  .transition_map:\n")
        if DropOutF:       txt.extend(["  .drop_out:\n",    repr(self.drop_out)])
        txt.append("\n")
        return txt

    def get_string(self, InputF=True, EntryF=True, TransitionMapF=True, DropOutF=True):
        return "".join(self.get_string_array(InputF, EntryF, TransitionMapF, DropOutF))

    def entries_empty_f(self):
        """The 'SetTemplateStateKey' commands cost nothing, so an easy condition for
           'all entries empty' is that the door_tree_root reports a cost of '0'.
        """
        # This function can only be called after a call to 'finish()'.
        assert self.entry.door_tree_root is not None
        return self.entry.door_tree_root.has_commands_other_than_MegaState_Control()

    def __repr__(self):
        return self.get_string()

def get_input_action(EngineType, TheState, ForceInputDereferencingF):
    if EngineType == E_EngineTypes.FORWARD:
        if TheState.init_state_f: action = E_InputActions.DEREF
        else:                     action = E_InputActions.INCREMENT_THEN_DEREF
    else:                         action = E_InputActions.DECREMENT_THEN_DEREF

    if TheState.transition_map_empty_f:
        # If the state has no further transitions then the input character does 
        # not have to be read. This is so, since without a transition map, the 
        # state immediately drops out. The drop out transits to a terminal. 
        # Then, the next action will happen from the init state where we work
        # on the same position. If required the reload happens at that moment.
        #
        # This is not true for Path Walker States, so we offer the option 
        # 'ForceInputDereferencingF'
        if not ForceInputDereferencingF:
            if   action == E_InputActions.INCREMENT_THEN_DEREF: return E_InputActions.INCREMENT
            elif action == E_InputActions.DECREMENT_THEN_DEREF: return E_InputActions.DECREMENT

    return action

