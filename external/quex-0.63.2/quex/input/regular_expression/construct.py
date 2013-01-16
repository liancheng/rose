from   quex.engine.interval_handling                import UnicodeInterval, Interval
from   quex.engine.state_machine.utf16_state_split  import ForbiddenRange
import quex.engine.state_machine.character_counter  as character_counter
import quex.engine.state_machine.setup_post_context as setup_post_context
import quex.engine.state_machine.setup_pre_context  as setup_pre_context
import quex.engine.state_machine.transformation     as transformation
#                                                         
from   quex.blackboard           import setup       as Setup, deprecated
from   quex.engine.misc.file_in  import error_msg

class Pattern(object):
    """Let's start as a mimiker ... """
    def __init__(self, CoreSM, PreContextSM=None, PostContextSM=None, 
                 BeginOfLineF=False, EndOfLineF=False, 
                 AllowStateMachineTrafoF=None, fh=-1):
        assert AllowStateMachineTrafoF is not None
        pre_context  = PreContextSM
        core_sm      = CoreSM
        post_context = PostContextSM

        # (1) Character/Newline counting
        #
        #     !! BEFORE Transformation !!
        #
        #     Currently 'transition number' is equal to 'character number'. After 
        #     transformation a transition may represent a byte or whatever the codec 
        #     does to the state machine.
        self.__newline_n, self.__character_n = character_counter.do(CoreSM)

        # (2) [Optional] Transformation according to Codec Information
        #
        #     !! BEFORE Pre- and Post-Context Setup !!
        #
        #     Because pre-context state machines and pseudo-ambiguous state machines
        #     are inverted. They need to be inverted according to the transformed codec.
        #     !! AVOID Double Transformation !!
        #     The transformation should actually only be allowed during the pattern-action
        #     definition inside a mode --> flag 'AllowStateMachineTrafoF' 
        if     AllowStateMachineTrafoF \
           and Setup.buffer_codec_transformation_info is not None:
            sm = transformation.try_this(pre_context, fh)
            if sm is not None: pre_context = sm
            sm = transformation.try_this(core_sm, fh)
            if sm is not None: core_sm = sm
            sm = transformation.try_this(post_context, fh)
            if sm is not None: post_context = sm

        self.__sm = core_sm

        # (3) Pre- and Post-Context Setup
        self.__post_context_f = (post_context is not None)

        self.__sm,                               \
        self.__input_position_search_backward_sm = setup_post_context.do(self.__sm, post_context, EndOfLineF, fh=fh) 

        self.__pre_context_sm_inverse = setup_pre_context.do(self.__sm, pre_context, BeginOfLineF)

        self.__pre_context_trivial_begin_of_line_f = (BeginOfLineF and self.__pre_context_sm_inverse is None)
        
        self.__validate(fh)

    @property
    def newline_n(self):                           return self.__newline_n
    @property
    def character_n(self):                         return self.__character_n
    @property
    def sm(self):                                  return self.__sm
    @property
    def inverse_pre_context_sm(self):              return self.__pre_context_sm_inverse
    @property
    def input_position_search_backward_sm(self):   return self.__input_position_search_backward_sm
    @property
    def pre_context_trivial_begin_of_line_f(self): return self.__pre_context_trivial_begin_of_line_f
    @property
    def post_context_f(self):                      return self.__post_context_f
    def has_pre_or_post_context(self):
        if   self.__pre_context_trivial_begin_of_line_f: return True
        elif self.__pre_context_sm_inverse is not None:  return True
        elif self.__post_context_f:                      return True
        return False
    def has_pre_context(self): 
        return    self.__pre_context_trivial_begin_of_line_f \
               or self.__pre_context_sm_inverse is not None

    def __validate(self, fh):
        # (*) It is essential that state machines defined as patterns do not 
        #     have origins.
        if self.__sm.has_origins():
            error_msg("Regular expression parsing resulted in state machine with origins.\n" + \
                      "Please, log a defect at the projects website quex.sourceforge.net.\n", fh)

        # (*) Acceptance states shall not store the input position when they are 'normally'
        #     post-conditioned. Post-conditioning via the backward search is a different 
        #     ball-game.
        acceptance_f = False
        for state in self.__sm.states.values():
            if state.is_acceptance(): acceptance_f = True
            if     state.input_position_store_f() \
               and state.is_acceptance():
                error_msg("Pattern with post-context: An irregularity occurred.\n" + \
                          "(end of normal post-contexted core pattern is an acceptance state)\n" 
                          "Please, log a defect at the projects website quex.sourceforge.net.", fh)

        if acceptance_f == False:
            error_msg("Pattern has no acceptance state and can never match.\n" + \
                      "Aborting generation process.", fh)

        # All state machines must be DFAs
        assert    self.__sm.is_DFA_compliant(), \
                  repr(self.__sm)
        assert    self.__pre_context_sm_inverse is None \
               or self.__pre_context_sm_inverse.is_DFA_compliant()
        assert    self.__input_position_search_backward_sm is None \
               or self.__input_position_search_backward_sm.is_DFA_compliant()

    def __repr__(self):
        return self.get_string(self)

    def get_string(self, NormalizeF=False, Option="utf8"):
        assert Option in ["utf8", "hex"]

        msg = self.__sm.get_string(NormalizeF, Option)
            
        if self.__pre_context_sm_inverse is not None:
            msg += "pre-condition inverted = "
            msg += self.__pre_context_sm_inverse.get_string(NormalizeF, Option)           

        if self.__input_position_search_backward_sm is not None:
            msg += "post context backward input position detector inverted = "
            msg += self.__input_position_search_backward_sm.get_string(NormalizeF, Option)           

        return msg

    side_info = property(deprecated, deprecated, deprecated, "Member 'side_info' deprecated!")

def do(core_sm, 
       begin_of_line_f=False, pre_context=None, 
       end_of_line_f=False,   post_context=None, 
       fh=-1, 
       AllowNothingIsNecessaryF=False,
       AllowStateMachineTrafoF=True):

    assert type(begin_of_line_f) == bool
    assert type(end_of_line_f) == bool
    assert type(AllowNothingIsNecessaryF) == bool

    # Detect orphan states in the 'raw' state machines --> error in sm-building
    for sm in [pre_context, core_sm, post_context]:
        __detect_initial_orphaned_states(sm, fh)

    # Delete state transitions on signal characters (such as 'buffer limit code')
    # and cut ranges outside the unicode interval.
    for sm in [pre_context, core_sm, post_context]:
        __delete_forbidden_transitions(sm, fh)

    # Now, possibly new orphan states evolved. This must be, at least, a warning.
    # But, in any case they can be cut out without loss.
    for sm in [pre_context, core_sm, post_context]:
        __delete_orphaned_states(sm, fh)

    if core_sm.is_empty():
        error_msg("Deletion of signal characters resulted in empty core pattern.\n" + \
                  "Consider changing the value of the buffer limit code or path delimiter.", fh)

    # Detect the 'Nothing is Necessary' error in a pattern.
    # (*) 'Nothing is necessary' cannot be accepted. See the discussion in the 
    #     module "quex.output.cpp.core"          
    if not AllowNothingIsNecessaryF:
        post_context_f = (post_context is not None)
        __detect_path_of_nothing_is_necessary(pre_context,  "pre context",  post_context_f, fh)
        __detect_path_of_nothing_is_necessary(core_sm,      "core pattern", post_context_f, fh)
        __detect_path_of_nothing_is_necessary(post_context, "post context", post_context_f, fh)

    return Pattern(core_sm, pre_context, post_context, 
                   begin_of_line_f, end_of_line_f, 
                   AllowStateMachineTrafoF, fh)

def __detect_initial_orphaned_states(sm, fh):

    if sm is None: return

    if sm.has_orphaned_states() == False: return

    error_msg("Orphaned state(s) detected in regular expression (optimization lack).\n" + \
              "Please, log a defect at the projects website quex.sourceforge.net.\n"    + \
              "Orphan state(s) = " + repr(sm.get_orphaned_state_index_list()), 
              fh, DontExitF=True)

def __detect_path_of_nothing_is_necessary(sm, Name, PostContextPresentF, fh):
    assert Name in ["core pattern", "pre context", "post context"]

    if sm is None: return

    msg = "The %s contains in a 'nothing is necessary' path in the state machine.\n"   \
          % Name                                                                     + \
          "This means, that without reading a character the analyzer drops into\n"   + \
          "an acceptance state. "

    init_state = sm.get_init_state()

    if not init_state.is_acceptance(): return

    msg += { 
        "core pattern":
            "The analyzer would then stall.",

        "pre context":
            "E.g., pattern 'x*/y/' means that zero or more 'x' are a pre-\n"             + \
            "condition for 'y'. If zero appearances of 'x' are enough, then obviously\n" + \
            "there is no pre-condition for 'y'! Most likely the author intended 'x+/y/'.",

        "post context":
            "A post context where nothing is necessary is superfluous.",
    }[Name]

    if Name != "post context" and PostContextPresentF:
        msg += "\n"                                                          \
               "Note: A post context does not change anything to that fact." 

    error_msg(msg, fh)

def __delete_forbidden_transitions(sm, fh):
    # !! Let the orphaned state check NOT happen before this, because states
    # !! may become orphan in the frame of the following procedure.
    if sm is None: return

    # (*) The buffer limit code has to appear absolutely nowhere!
    if Setup.buffer_limit_code != -1: __delete_forbidden_character(sm, Setup.buffer_limit_code)
    if Setup.path_limit_code != -1:   __delete_forbidden_character(sm, Setup.path_limit_code)

    # (*) Delete transitions that make practically no sense
    __delete_forbidden_ranges(sm, fh)

def __delete_forbidden_character(sm, BLC):
    """The buffer limit code is something that **needs** to cause a drop out.
       In the drop out handling, the buffer is reloaded.

       Critical character is allowed at end of post context.

       NOTE: This operation might result in orphaned states that have to 
             be deleted.
    """
    for state in sm.states.values():
        for target_state_index, trigger_set in state.transitions().get_map().items():

            if trigger_set.contains(BLC):
                trigger_set.cut_interval(Interval(BLC, BLC+1))

            # If the operation resulted in cutting the path to the target state, then delete it.
            if trigger_set.is_empty():
                state.transitions().delete_transitions_to_target(target_state_index)

def __delete_forbidden_ranges(sm, fh):
    """Unicode does define all code points >= 0. Thus there can be no code points
       below zero as it might result from some number set operations.

       NOTE: This operation might result in orphaned states that have to 
             be deleted.
    """
    global Setup

    character_value_limit = Setup.get_character_value_limit()
    for state in sm.states.values():

        for target_state_index, trigger_set in state.transitions().get_map().items():

            # Make sure, all transitions lie inside the unicode code range 
            if trigger_set.minimum() < UnicodeInterval.begin or trigger_set.supremum() >= UnicodeInterval.end:
                trigger_set.intersect_with(UnicodeInterval)

            if trigger_set.supremum() > character_value_limit:
                error_msg("Pattern contains character beyond the scope of the buffer element size (%s)\n" \
                          % Setup.get_character_value_limit_str() + \
                          "Please, cut the character range of the regular expression,\n"
                          "adapt \"--buffer-element-size\" or \"--buffer-element-type\",\n"       + \
                          "or specify '--buffer-element-size-irrelevant' to ignore the issue.", fh)

            if Setup.buffer_codec in ["utf16-le", "utf16-be"]:
                # Delete the forbidden interval: D800-DFFF
                if trigger_set.has_intersection(ForbiddenRange):
                    error_msg("Pattern contains characters in unicode range 0xD800-0xDFFF.\n"
                              "This range is not covered by UTF16. Cutting Interval.", fh, DontExitF=True)
                    trigger_set.cut_interval(ForbiddenRange)
            
            # If the operation resulted in cutting the path to the target state, then delete it.
            if trigger_set.is_empty():
                state.transitions().delete_transitions_to_target(target_state_index)

def __delete_orphaned_states(sm, fh):
    if sm is None: return

    new_orhpan_state_list = sm.get_orphaned_state_index_list()
    if len(new_orhpan_state_list) == 0: return

    error_msg("Pattern contained solely forbidden characters in a state transition.\n"
              "The resulting target state is deleted since no other state triggered to it.", 
              fh, DontExitF=True, WarningF=True)

    for state_index in new_orhpan_state_list:
        del sm.states[state_index]


