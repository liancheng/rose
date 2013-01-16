#! /usr/bin/env python
from   quex.engine.misc.file_in                         import error_msg
from   quex.engine.state_machine.core                   import StateMachine
import quex.engine.state_machine.algorithm.nfa_to_dfa             as     nfa_to_dfa
import quex.engine.state_machine.algorithm.hopcroft_minimization  as     hopcroft
import quex.engine.state_machine.algorithm.beautifier             as     beautifier
import quex.engine.state_machine.ambiguous_post_context as     ambiguous_post_context
from   quex.blackboard                                  import E_PreContextIDs, setup as Setup


def do(the_state_machine, post_context_sm, EndOfLinePostContextF, fh=-1):
    """Appends a post context to the given state machine and changes 
       state infos as required. 

       NOTE: 

           In case that:    post_context_sm is not None 
                         or EndOfLinePostContextF  

           The function appends something to the state machine and
           it is therefore required to pass 'NFA to DFA'--better
           also Hopcroft Minimization.
       
       ________________________________________________________________________
       This process is very similar to sequentialization. 
       There is a major difference, though:
       
       Given a state machine (e.g. a pattern) X with a post context Y, 
       a match is only valid if X is followed by Y. Let Xn be an acceptance
       state of X and Ym an acceptance state of Y: 

              ---(Xn-1)---->(Xn)---->(Y0)----> ... ---->((Ym))
                            store                       acceptance
                            input
                            position
       
       That is, it holds:

          -- The next input position is stored the position of Xn, even though
             it is 'officially' not an acceptance state.

          -- Ym will be an acceptance state, but it will not store 
             the input position!       

       The analysis of the next pattern will start at the position where
       X stopped, even though Ym is required to state acceptance.    
       
    """
    # State machines with no states are senseless here. 
    assert not the_state_machine.is_empty(), \
           "empty state machine can have no post context."
    assert post_context_sm is None or not post_context_sm.is_empty(), \
           "empty state machine cannot be a post-context."

    # State machines involved with post condition building are part of a pattern, 
    # but not configured out of multiple patterns. Thus there should be no origins.
    assert the_state_machine.has_origins() == False
    assert post_context_sm is None or not post_context_sm.has_origins()
    for state in the_state_machine.get_acceptance_state_list():
        for origin in state.origins(): 
            assert origin.pre_context_id() == E_PreContextIDs.NONE, \
                   "Post Contexts MUST be mounted BEFORE pre-contexts."

    if post_context_sm is None:
        if not EndOfLinePostContextF:
            return the_state_machine, None
        # Generate a new post context that just contains the 'newline'
        post_context_sm = StateMachine(AcceptanceF=True)
        post_context_sm.mount_newline_to_acceptance_states(Setup.dos_carriage_return_newline_f)

    elif EndOfLinePostContextF: 
        # Mount 'newline' to existing post context
        post_context_sm.mount_newline_to_acceptance_states(Setup.dos_carriage_return_newline_f)

    # A post context with an initial state that is acceptance is not really a
    # 'context' since it accepts anything. The state machine remains un-post context.
    if post_context_sm.get_init_state().is_acceptance():
        error_msg("Post context accepts anything---replaced by no post context.", fh, 
                  DontExitF=True)
        return the_state_machine, None
    
    # (*) Two ways of handling post-contexts:
    #
    #     -- Seldom Exception: 
    #        Pseudo-Ambiguous Post Conditions (x+/x) -- detecting the end of the 
    #        core pattern after the end of the post context
    #        has been reached.
    #
    if ambiguous_post_context.detect_forward(the_state_machine, post_context_sm):
        if ambiguous_post_context.detect_backward(the_state_machine, post_context_sm):
            # -- for post contexts that are forward and backward ambiguous
            #    a philosophical cut is necessary.
            error_msg("Post context requires philosophical cut--handle with care!\n"
                      "Proposal: Isolate pattern and ensure results are as expected!", fh, 
                      DontExitF=True)
            post_context_sm = ambiguous_post_context.philosophical_cut(the_state_machine, post_context_sm)
        
        # NOTE: May be, the_state_machine does contain now an epsilon transition. See
        #       comment at entry of this function.
        ipsb_sm = ambiguous_post_context.mount(the_state_machine, post_context_sm)
        the_state_machine = beautifier.do(the_state_machine)
        ipsb_sm           = beautifier.do(ipsb_sm)
        return the_state_machine, ipsb_sm 

    # -- The 'normal' way: storing the input position at the end of the core
    #    pattern.
    #
    # (*) Need to clone the state machines, i.e. provide their internal
    #     states with new ids, but the 'behavior' remains. This allows
    #     state machines to appear twice, or being used in 'larger'
    #     conglomerates.
    post_clone = post_context_sm.clone() 

    # -- Once an acceptance state is reached no further analysis is necessary.
    ## NO: acceptance_pruning.do(post_clone)
    ## BECAUSE: it may have to compete with a pseudo-ambiguous post context

    # (*) collect all transitions from both state machines into a single one
    #
    #     NOTE: The start index is unique. Therefore, one can assume that each
    #           clone_list '.states' dictionary has different keys. One can simply
    #           take over all transitions of a start index into the result without
    #           considering interferences (see below)
    #
    orig_acceptance_state_id_list = the_state_machine.get_acceptance_state_index_list()

    # -- mount on every acceptance state the initial state of the following state
    #    machine via epsilon transition
    the_state_machine.mount_to_acceptance_states(post_clone.init_state_index, 
                                                 CancelStartAcceptanceStateF=True)
    for start_state_index, state in post_clone.states.iteritems():        
        the_state_machine.states[start_state_index] = state # states are already cloned

    # -- raise at each old acceptance state the 'store input position flag'
    # -- set the post context flag for all acceptance states
    for state_idx in orig_acceptance_state_id_list:
        state = the_state_machine.states[state_idx]
        state.set_input_position_store_f(True)
    
    # -- no acceptance state shall store the input position
    # -- set the post context flag for all acceptance states
    for state in the_state_machine.get_acceptance_state_list():
        state.set_input_position_store_f(False)
        state.set_input_position_restore_f(True)

    # No input position backward search required
    the_state_machine = nfa_to_dfa.do(the_state_machine)
    hopcroft.do(the_state_machine, CreateNewStateMachineF=False)
    return the_state_machine, None

