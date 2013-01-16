"""ABSTRACT:

This module produces an object of class Analyzer. It is a representation of an
analyzer state machine (object of class StateMachine) that is suited for code
generation. In particular, track analysis results in 'decorations' for states
which help to implement efficient code.

Formally an Analyzer consists of a set of states that are related by their
transitions. Each state is an object of class AnalyzerState and has the
following components:

    * entry:          actions to be performed at the entry of the state.

    * input:          what happens to get the next character.

    * transition_map: a map that tells what state is to be entered 
                      as a reaction to the current input character.

    * drop_out:       what has to happen if no character triggers.

For administrative purposes, other data such as the 'state_index' is stored
along with the AnalyzerState object.

The goal of track analysis is to reduce the run-time effort of the lexical
analyzer. In particular, acceptance and input position storages may be spared
depending on the constitution of the state machine.

-------------------------------------------------------------------------------
(C) 2010-2011 Frank-Rene Schaefer
ABSOLUTELY NO WARRANTY
"""

import quex.engine.analyzer.track_analysis        as     track_analysis
import quex.engine.analyzer.optimizer             as     optimizer
from   quex.engine.analyzer.state.core            import AnalyzerState
from   quex.engine.analyzer.state.drop_out        import DropOut
import quex.engine.analyzer.mega_state.analyzer   as     mega_state_analyzer
import quex.engine.analyzer.position_register_map as     position_register_map
from   quex.engine.state_machine.core             import StateMachine
from   quex.blackboard  import setup as Setup
from   quex.blackboard  import E_AcceptanceIDs, \
                               E_EngineTypes, \
                               E_TransitionN, \
                               E_PreContextIDs

from   collections      import defaultdict
from   operator         import itemgetter
from   itertools        import islice, imap, izip

def do(SM, EngineType=E_EngineTypes.FORWARD):
    # Generate Analyzer from StateMachine
    analyzer = Analyzer(SM, EngineType)

    # Optimize the Analyzer
    analyzer = optimizer.do(analyzer)

    # The language database requires the analyzer for labels etc.
    if Setup.language_db is not None:
        Setup.language_db.register_analyzer(analyzer)

    # If required by the user: Combine some states into mega states.
    mega_state_analyzer.do(analyzer)

    return analyzer

class Analyzer:
    """A representation of a pattern analyzing StateMachine suitable for
       effective code generation.
    """
    def __init__(self, SM, EngineType):
        assert EngineType in E_EngineTypes
        assert isinstance(SM, StateMachine)


        self.__acceptance_state_index_list = SM.get_acceptance_state_index_list()
        self.__init_state_index = SM.init_state_index
        self.__state_machine_id = SM.get_id()
        self.__engine_type      = EngineType

        # (*) PathTrace database, Successor database
        self.__trace_db, self.__dangerous_positioning_state_set = track_analysis.do(SM)

        # (*) From/To Databases
        #
        #     from_db:  state_index --> states from which it is entered.
        #     to_db:    state_index --> states which it enters
        #
        from_db = defaultdict(set)
        to_db   = defaultdict(set)
        for from_index, state in SM.states.iteritems():
            to_db[from_index] = set(state.transitions().get_map().iterkeys())
            for to_index in state.transitions().get_map().iterkeys():
                from_db[to_index].add(from_index)
        self.__from_db = from_db
        self.__to_db   = to_db

        # (*) Prepare AnalyzerState Objects
        self.__state_db = dict([(state_index, AnalyzerState(SM.states[state_index], state_index, 
                                                            state_index == SM.init_state_index, 
                                                            EngineType, 
                                                            from_db[state_index])) 
                                 for state_index in self.__trace_db.iterkeys()])

        if EngineType != E_EngineTypes.FORWARD:
            # BACKWARD_INPUT_POSITION, BACKWARD_PRE_CONTEXT:
            #
            # DropOut and Entry do not require any construction beyond what is
            # accomplished inside the constructor of 'AnalyzerState'. No positions
            # need to be stored and restored.
            self.__position_register_map = None
            self.__position_info_db      = None
            return

        # (*) Positioning info:
        #
        #     map:  (state_index) --> (pattern_id) --> positioning info
        #
        self.__position_info_db = {}
        for state_index, trace_list in self.__trace_db.iteritems():
            self.__position_info_db[state_index] = self.multi_path_positioning_analysis(trace_list)

        # (*) Drop Out Behavior
        #     The PathTrace objects tell what to do at drop_out. From this, the
        #     required entry actions of states can be derived.
        self.__require_acceptance_storage_list = []
        self.__require_position_storage_list   = []
        for state_index, trace_list in self.__trace_db.iteritems():
            state = self.__state_db[state_index]
            # trace_list: PathTrace objects for each path that guides to state.
            self.configure_drop_out(state, trace_list)

        # (*) Entry Behavior
        #     Implement the required entry actions.
        self.configure_entries()

        # (*) Position Register Map (Used in 'optimizer.py')
        self.__position_register_map = position_register_map.do(self)

    @property
    def trace_db(self):                    return self.__trace_db
    @property
    def state_db(self):                    return self.__state_db
    @property
    def init_state_index(self):            return self.__init_state_index
    @property
    def position_register_map(self):       return self.__position_register_map
    @property
    def state_machine_id(self):            return self.__state_machine_id
    @property
    def engine_type(self):                 return self.__engine_type
    @property
    def position_info_db(self):            return self.__position_info_db
    @property
    def acceptance_state_index_list(self): return self.__acceptance_state_index_list
    @property
    def to_db(self):
        """Map: state_index --> list of states that it enters."""
        return self.__to_db
    @property
    def from_db(self):
        """Map: state_index --> list of states that enter it."""
        return self.__from_db

    def last_acceptance_variable_required(self):
        """If one entry stores the last_acceptance, then the 
           correspondent variable is required to be defined.
        """
        if self.__engine_type != E_EngineTypes.FORWARD: return False
        for entry in imap(lambda x: x.entry, self.__state_db.itervalues()):
            if entry.has_accepter(): return True
        return False

    def configure_drop_out(self, state, ThePathTraceList):
        """Every analysis step ends with a 'drop-out'. At this moment it is
           decided what pattern has won. Also, the input position pointer
           must be set so that it indicates the right location where the
           next step starts the analysis. 

           Consequently, a drop-out action contains two elements:

            -- Acceptance Checker: Dependent on the fulfilled pre-contexts a
               winning pattern is determined. 

               If acceptance depends on stored acceptances, a request is raised
               at each accepting state that is has to store its acceptance in 
               variable 'last_acceptance'.

            -- Terminal Router: Dependent on the accepted pattern the input
               position is modified and the terminal containing the pattern
               action is entered.

               If the input position is restored from a position register, 
               then the storing states are requested to store the input
               position.

           --------------------------------------------------------------------
           HINT:
           A state may be reached via multiple paths. For each path there is a
           separate PathTrace. Each PathTrace tells what has to happen in the state
           depending on the pre-contexts being fulfilled or not (if there are
           even any pre-context patterns).
        """
        assert len(ThePathTraceList) != 0
        result = DropOut()

        # (*) Acceptance Checker
        uniform_f = self.multi_path_acceptance_analysis(ThePathTraceList)
        if uniform_f:
            # (i) Uniform Acceptance Pattern for all paths through the state.
            # 
            #     Use one trace as prototype. No related state needs to store
            #     acceptance at entry. 
            prototype = ThePathTraceList[0]
            for x in prototype.acceptance_trace:
                result.accept(x.pre_context_id, x.pattern_id)
                # No further checks after unconditional acceptance necessary
                if     x.pre_context_id == E_PreContextIDs.NONE \
                   and x.pattern_id     != E_AcceptanceIDs.FAILURE: break
        else:
            # (ii) Non-Uniform Acceptance Patterns
            #
            #     Different paths to one state result in different acceptances. 
            #     There is only one way to handle this:
            #
            #       -- The acceptance must be stored in the state where it occurs, and
            #       -- It must be restored here.
            #
            result.accept(E_PreContextIDs.NONE, E_AcceptanceIDs.VOID)

            # Dependency: Related states are required to store acceptance at state entry.
            for trace in ThePathTraceList:
                self.__require_acceptance_storage_list.extend(trace.acceptance_trace)
            # Later on, a function will use the '__require_acceptance_storage_list' to 
            # implement the acceptance storage.

        # (*) Terminal Router
        for pattern_id, info in self.__position_info_db[state.index].iteritems():
            result.route_to_terminal(pattern_id, info.transition_n_since_positioning)

            if info.transition_n_since_positioning == E_TransitionN.VOID: 
                # Request the storage of the position from related states.
                self.__require_position_storage_list.append((state.index, pattern_id, info))
            # Later on, a function will use the '__require_position_storage_list' to 
            # implement the position storage.

        result.trivialize()
        state.drop_out = result

    def configure_entries(self):
        """DropOut objects may rely on acceptances and input positions being 
           stored. This storage happens at state entries.
           
           Function 'configure_drop_out()' registers which states have to store
           the input position and which ones have to store acceptances. These
           tasks are specified in the two members:

                 self.__require_acceptance_storage_list
                 self.__require_position_storage_list

           It is tried to postpone the storing as much as possible along the
           state paths from store to restore. Thus, some states may not have to
           store, and thus the lexical analyzer becomes a little faster.
        """
        self.implement_required_acceptance_storage()
        self.implement_required_position_storage()

    def implement_required_acceptance_storage(self):
        """
        Storing Acceptance / Postpone as much as possible.
        
        The stored 'last_acceptance' is only needed at the first time
        when it is restored. So, we could walk along the path from the 
        accepting state to the end of the path and see when this happens.
        
        Critical case:
        
          State V --- "acceptance = A" -->-.
                                            \
                                              State Y ----->  State Z
                                            /
          State W --- "acceptance = B" -->-'
        
        That is, if state Y is entered from state V is shall store 'A'
        as accepted, if it is entered from state W is shall store 'B'.
        In this case, we cannot walk the path further, and say that when
        state Z is entered it has to store 'A'. This would cancel the
        possibility of having 'B' accepted here. There is good news:
        
        ! During the 'configure_drop_out()' the last acceptance is restored    !
        ! if and only if there are at least two paths with differing           !
        ! acceptance patterns. Thus, it is sufficient to consider the restore  !
        ! of acceptance in the drop_out as a terminal condition.               !
        """
        postponed_db = defaultdict(set)
        for acceptance_trace in self.__require_acceptance_storage_list:
            accepting_state_index  = acceptance_trace.accepting_state_index
            path_since_positioning = acceptance_trace.path_since_positioning
            pre_context_id         = acceptance_trace.pre_context_id
            pattern_id             = acceptance_trace.pattern_id

            # Find the first place on the path where the acceptance is restored
            # - starting from the last accepting state.
            begin_i = acceptance_trace.index_of_last_acceptance_on_path_since_positioning()

            prev_state_index = None
            for state_index in islice(path_since_positioning, begin_i, None):
                if self.__state_db[state_index].drop_out.restore_acceptance_f: 
                    break
                prev_state_index = state_index

            if prev_state_index is not None:
                entry           = self.__state_db[state_index].entry
                path_trace_list = self.__trace_db[prev_state_index]
                for path_trace in path_trace_list:
                    entry.doors_accept(FromStateIndex=prev_state_index, PathTraceList=path_trace.acceptance_trace)
            else:
                # Postpone:
                #
                # Here, storing Acceptance cannot be deferred to subsequent states, because
                # the first state that restores acceptance is the acceptance state itself.
                #
                # (1) Restore only happens if there is non-uniform acceptance. See 
                #     function 'configure_drop_out(...)'. 
                # (2) Non-uniform acceptance only happens, if there are multiple paths
                #     to the same state with different trailing acceptances.
                # (3) If there was an absolute acceptance, then all previous trailing 
                #     acceptance were deleted (longest match). This contradicts (2).
                #
                # (4) => Thus, there are only pre-contexted acceptances in such a state.
                assert pre_context_id != E_PreContextIDs.NONE 
                postponed_db[accepting_state_index].add((pre_context_id, pattern_id))

        # Postponed: Collected acceptances to be stored in the acceptance states itself.
        #
        # It is possible that a deferred acceptance are already present in the doors. But, 
        # since they all come from trailing acceptances, we know that the acceptance of
        # this state preceeds (longest match). Thus, all the acceptances we add here 
        # preceed the already mentioned ones. Since they all trigger on lexemes of the
        # same length, the only precendence criteria is the pattern_id.
        # 
        for state_index, info_set in postponed_db.iteritems():
            entry = self.__state_db[state_index].entry
            for pre_context_id, pattern_id in sorted(list(info_set), key=itemgetter(1), reverse=True):
                entry.doors_accepter_add_front(pre_context_id, pattern_id)

    def implement_required_position_storage(self):
        """
        Store Input Position / Postpone as much as possible.

        Before we do not reach a state that actually restores the position, it
        does make little sense to store the input position. 

                         Critical Point: Loops and Forks

        If a loop is reached then the input position can no longer be determined
        by the transition number. The good news is that during 'configure_drop_out'
        any state that has undetermined positioning restores the input position.
        Thus 'restore_position_f(register)' is enough to catch this case.
        """
        def get_positioning_state_iterable(from_state_index, path):
            if from_state_index in self.__dangerous_positioning_state_set:
                for to_state_index in self.__to_db[from_state_index]:
                    yield to_state_index
            else:
                yield path[1]

        for state_index, pattern_id, info in self.__require_position_storage_list:
            # state_index  --> state that restores the input position
            # pattern_id   --> pattern which is concerned
            for path in info.path_list_since_positioning:
                # Never store the input position in the state itself. The input position
                # is reached after the entries have been passed.
                from_state_index = path[0]
                for to_state_index in get_positioning_state_iterable(from_state_index, path):
                    state = self.__state_db[to_state_index]
                    # Never store the input position in the state itself. The input position
                    # is reached after the entries have been passed.
                    state.entry.doors_store(FromStateIndex   = from_state_index, 
                                            PreContextID     = info.pre_context_id, 
                                            PositionRegister = pattern_id, 
                                            Offset           = 0)
                # offset           = -1
                # for state_index in islice(path, 1, None):
                    # offset += 1
                    # state = self.__state_db[state_index]
                    # if not state.drop_out.restore_position_f(pattern_id): 
                        # prev_state_index = state_index
                        # continue
                    # state.entry.doors_store(FromStateIndex   = prev_state_index, 
                                            # PreContextID     = info.pre_context_id, 
                                            # PositionRegister = pattern_id, 
                                            # Offset           = offset)
                    # break
                
    def multi_path_positioning_analysis(self, ThePathTraceList):
        """
        This function draws conclusions on the input positioning behavior at
        drop-out based on different paths through the same state.  Basis for
        the analysis are the PathTrace objects of a state specified as
        'ThePathTraceList'.

        RETURNS: For a given state's PathTrace list a dictionary that maps:

                            pattern_id --> PositioningInfo

        --------------------------------------------------------------------
        
        There are the following alternatives for setting the input position:
        
           (1) 'lexeme_start_p + 1' in case of failure.

           (2) 'input_p + offset' if the number of transitions between
               any storing state and the current state is does not differ 
               dependent on the path taken (and does not contain loops).
        
           (3) 'input_p = position_register[i]' if (1) and (2) are not
               not the case.

        The detection of loops has been accomplished during the construction
        of the PathTrace objects for each state. This function focusses on
        the possibility to have different paths to the same state with
        different positioning behaviors.
        """
        class PositioningInfo(object):
            __slots__ = ("transition_n_since_positioning", 
                         "pre_context_id", 
                         "path_list_since_positioning")
            def __init__(self, PathTraceElement):
                self.transition_n_since_positioning = PathTraceElement.transition_n_since_positioning
                self.path_list_since_positioning    = [ PathTraceElement.path_since_positioning ]
                self.pre_context_id                 = PathTraceElement.pre_context_id

            @property
            def positioning_state_index_set(self):
                return set(path[0] for path in self.path_list_since_positioning)

            def add(self, PathTraceElement):
                self.path_list_since_positioning.append(PathTraceElement.path_since_positioning)

                if self.transition_n_since_positioning != PathTraceElement.transition_n_since_positioning:
                    self.transition_n_since_positioning = E_TransitionN.VOID

            def __repr__(self):
                txt  = ".transition_n_since_positioning = %s\n" % repr(self.transition_n_since_positioning)
                txt += ".positioning_state_index_set    = %s\n" % repr(self.positioning_state_index_set) 
                txt += ".pre_context_id                 = %s\n" % repr(self.pre_context_id) 
                return txt

        positioning_info_by_pattern_id = {}
        # -- If the positioning differs for one element in the trace list, or 
        # -- one element has undetermined positioning, 
        # => then the acceptance relates to undetermined positioning.
        for trace in ThePathTraceList:
            for element in trace.acceptance_trace:
                assert element.pattern_id != E_AcceptanceIDs.VOID

                prototype = positioning_info_by_pattern_id.get(element.pattern_id)
                if prototype is None:
                    positioning_info_by_pattern_id[element.pattern_id] = PositioningInfo(element)
                else:
                    prototype.add(element)

        return positioning_info_by_pattern_id

    def multi_path_acceptance_analysis(self, ThePathTraceList):
        """
        This function draws conclusions on the input positioning behavior at
        drop-out based on different paths through the same state.  Basis for
        the analysis are the PathTrace objects of a state specified as
        'ThePathTraceList'.

        Acceptance Uniformity:

            For any possible path to 'this' state the acceptance pattern is
            the same. That is, it accepts exactly the same pattern under the
            same pre contexts and in the same sequence of precedence.

        The very nice thing is that the 'acceptance_trace' of a PathTrace
        object reflects the precedence of acceptance. Thus, one can simply
        compare the acceptance trace objects of each PathTrace.

        RETURNS: True  - uniform acceptance pattern.
                 False - acceptance pattern is not uniform.
        """
        prototype = ThePathTraceList[0].acceptance_trace

        # Check (1) and (2)
        for path_trace in islice(ThePathTraceList, 1, None):
            acceptance_trace = path_trace.acceptance_trace
            if len(prototype) != len(acceptance_trace):    return False
            for x, y in izip(prototype, acceptance_trace):
                if   x.pre_context_id != y.pre_context_id: return False
                elif x.pattern_id     != y.pattern_id:     return False

        return True

    def __iter__(self):
        for x in self.__state_db.values():
            yield x

    def __repr__(self):
        # Provide some type of order that is oriented towards the content of the states.
        # This helps to compare analyzers where the state identifiers differ, but the
        # states should be the same.
        def order(X):
            side_info = 0
            if len(X.transition_map) != 0: side_info = max(trigger_set.size() for trigger_set, t in X.transition_map)
            return (len(X.transition_map), side_info, X.index)

        txt = [ repr(state) for state in sorted(self.__state_db.itervalues(), key=order) ]
        return "".join(txt)

