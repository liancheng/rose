from quex.engine.analyzer.state.track_analyzis import AcceptInfo
from quex.blackboard import E_AcceptanceIDs, E_TransitionN

from itertools import islice, izip

class PathsToState(object):
    def __init__(self):
        self.__acceptance_trace_list = []

    def add(self, AcceptancePath):
        self.__acceptance_trace_list.append(AcceptancePath)

    def multi_path_positioning_analysis(self):
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
        positioning_info_by_pattern_id = {}
        # -- If the positioning differs for one element in the trace list, or 
        # -- one element has undetermined positioning, 
        # => then the acceptance relates to undetermined positioning.
        for acceptance_trace in self.__acceptance_trace_list:
            for element in acceptance_trace:
                assert isinstance(element, AcceptInfo)
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

class PositioningInfo(object):
    """PositioningInfo is associated with one PatternID in one State.
    
       transition_n_since_positioning -- transition number since positioning happend for 
                                         a given pattern id.

                                         if VOID => transition number cannot be determined.
                                                    position store/restore required.

       pre_context_id                 -- condition that must hold for acceptance.
       path_list_since_positioning    -- list of all paths that guide from the positioning
                                         state to the current state.
    """
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

