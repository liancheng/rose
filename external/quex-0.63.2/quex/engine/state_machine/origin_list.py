from quex.engine.state_machine.state_core_info import StateCoreInfo
from quex.blackboard                           import E_PreContextIDs, E_AcceptanceIDs

class StateOriginList(object):
    __slots__ = ('__list')

    def __init__(self, List=None):
        self.__list = []
        if List is not None: self.merge(List)

    def clone(self):
        return StateOriginList([x.clone() for x in self.__list])

    def get_list(self):
        return self.__list

    def __iter__(self):
        for x in self.__list:
            yield x

    def __len__(self):
        return len(self.__list)

    def __add(self, Origin):
        """Check if origin has already been mentioned, else append the new origin.
        """
        # NOT: if not Origin.is_meaningful(): return
        #      We need even non-meaningful origins, to detect whether a state can be 
        #      combined with another during Hopcroft optimization.
            
        PatternID = Origin.pattern_id()
        for same in (origin for origin in self.__list if origin.pattern_id() == PatternID):
            same.merge(Origin)
            return
        self.__list.append(Origin.clone())

    def get_the_only_one(self):
        """Returns a origin that belongs to the list. If there is no origin on
           the list, then one is created and then returned.
        """
        L = len(self.__list)
        if   L == 0: 
            new_origin = StateCoreInfo(E_AcceptanceIDs.FAILURE, -1L, AcceptanceF=False)
            self.__list.append(new_origin)
            # NOTE: Here the object is in a state where there is a 'nonsense origin'. It is
            #       expected from the caller to fix this.
            return new_origin

        assert L == 1, "Calling function not permitted on aggregated states."
        return self.__list[0]

    def remove_the_only_one(self):
        assert len(self.__list) == 1
        del self.__list[0]

    def add(self, X, StateIndex=None, 
            StoreInputPositionF   = False, 
            AcceptanceF           = False, 
            RestoreInputPositionF = False, 
            PreContextID          = E_PreContextIDs.NONE):
        """Add the StateMachineID and the given StateIdx to the list of origins of 
           this state.
           NOTE: The rule is that by default the 'input_position_store_f' flag
                 follows the acceptance state flag (i.e. by default any acceptance
                 state stores the input position). Thus when an origin is  added
                 to a state that is an acceptance state, the 'input_position_store_f'
                 has to be raised for all incoming origins.      
        """
        assert type(X) == long or X == E_AcceptanceIDs.FAILURE or X.__class__ == StateCoreInfo
        assert StateIndex is None or type(StateIndex) == long
        assert StoreInputPositionF is not None
            
        if isinstance(X.__class__, StateCoreInfo):
            self.__add(X.clone())
        else:
            self.__add(StateCoreInfo(PatternID             = X, 
                                     StateIndex            = StateIndex, 
                                     AcceptanceF           = AcceptanceF,
                                     PreContextID          = PreContextID,
                                     StoreInputPositionF   = StoreInputPositionF, 
                                     RestoreInputPositionF = RestoreInputPositionF))

    def merge(self, OriginList):
        for origin in OriginList: 
            self.__add(origin)

    def merge_clear(self, OriginList):
        for origin in OriginList: 
            new_origin = origin.clone()
            new_origin.set_input_position_store_f(False)
            new_origin.set_input_position_restore_f(False)
            new_origin.set_pre_context_id(E_PreContextIDs.NONE)
            new_origin.set_acceptance_f(False)
            self.__add(new_origin)

    def set(self, OriginList, ArgumentIsYoursF=False):
        assert type(OriginList) == list
        if ArgumentIsYoursF: 
            self.__list = OriginList
            return
        self.__list = []
        self.merge(OriginList)

    def clear(self):
        self.__list = []

    def DELETED_adapt(self, PatternID, StateIndex):
        """Adapts all origins so that their original state is 'StateIndex' in state machine
           'PatternID'. Post- and pre-condition flags remain, and so the store input 
           position flag.
        """
        for origin in self.__list:
            origin.set_pattern_id(PatternID)
            origin.state_index = StateIndex 

    def delete_meaningless(self):
        """Deletes origins that are not concerned with one of the three:
           -- post-conditions
           -- pre-conditions/also trivials
           -- store input positions

           NOTE: This function is only to be used for single patterns not for
                 combined state machines. During the NFA to DFA translation
                 more than one state is combined into one. This maybe reflected
                 in the origin list. However, only at the point when the 
                 pattern state machine is ready, then the origin states are something
                 meaningful. The other information has to be kept.
                 
           NOTE: After applying this fuction to a single pattern, there should only
                 be one origin for each state.
        """
        self.__list = filter(lambda origin:
                                    origin.post_contexted_acceptance_f()            or
                                    origin.pre_context_id() != E_PreContextIDs.NONE or
                                    origin.input_position_store_f(),
                                    self.__list)

    def get_absolute_acceptance_origin(self):
        """There can be only one 'origin' that wins without any pre-condition/pre-context.
           This is the one with the lowest pattern id (the one which has been written first).
        """
        iterable = (origin for origin in self.__list                       \
                    if     origin.pre_context_id() == E_PreContextIDs.NONE \
                       and origin.is_acceptance())
        try:
            result = min((origin for origin in iterable), key=lambda x: x.pattern_id())
            return result
        except:
            return None

    def get_conditional_acceptance_iterable(self, MaxAcceptanceID):
        """There can be more than one conditional acceptance for a state, because it is only
           known at runtime which pre-context is actually fulfilled.
        """
        # Conditional acceptance on a pre-context is simply nonsense.
        if MaxAcceptanceID == E_AcceptanceIDs.FAILURE:
            return (origin for origin in self.__list                  \
                    if origin.pre_context_id() != E_PreContextIDs.NONE)
        else:
            return (origin for origin in self.__list                       \
                    if     origin.pre_context_id() != E_PreContextIDs.NONE \
                       and origin.pattern_id() < MaxAcceptanceID)

    def get_store_iterable(self):
        """Return list of origins which tell something about storing the input
           position. Those cannot be acceptance origins.
        """
        return (origin for origin in self.__list if origin.input_position_store_f())

    def delete_dominated(self):
        """This function is a simplification in order to allow the Hopcroft Minimization
           to be more efficient. It 'simulates' the code generation where the first unconditional
           pattern matches. The remaining origins of a state are redundant.

           This function is to be seen in analogy with the function 'get_acceptance_detector'. 
           Except for the fact that it requires the 'end of core pattern' markers of post
           conditioned patterns. If the markers are not set, the store input position commands
           are not called properly, and when restoring the input position bad bad things happen 
           ... i.e. segmentation faults.
        """
        # NOTE: Acceptance origins sort before non-acceptance origins
        self.__list.sort(key=lambda x: (not x.is_acceptance(), x.pattern_id()))
        new_origin_list = []
        unconditional_acceptance_found_f = False
        for origin in self.__list:

            if origin.is_acceptance():
                # Only append acceptance origins until the first unconditional acceptance state 
                # is found. 
                if not unconditional_acceptance_found_f:
                    if origin.pre_context_id() == E_PreContextIDs.NONE:
                        unconditional_acceptance_found_f = True # prevent entering this part again
                    new_origin_list.append(origin)

            else:
                # Non-Acceptance origins do not harm in any way. Actually, the origins
                # with 'origin.input_position_store_f() == True' **need**
                # to be in there. See the comment at the entry of this function.
                new_origin_list.append(origin)

        self.__list = new_origin_list 

    def get_string(self):
        txt = "" 
        if len(self.__list) == 0: 
            return txt + "\n"

        # for origin in sorted(self.__list, key=attrgetter("state_machine_id")):
        for origin in self.__list:
            txt += repr(origin) + ", "
        txt = (txt[:-2] + "\n").replace("L","")     
        return txt
