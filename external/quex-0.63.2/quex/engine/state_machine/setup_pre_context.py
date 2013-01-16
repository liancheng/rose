# (C) Frank-Rene Schaefer
#     ABSOLUTELY NO WARRANTY
import quex.engine.state_machine.algorithm.beautifier         as beautifier
import quex.engine.state_machine.algorithm.acceptance_pruning as acceptance_pruning
from   quex.blackboard                                        import E_PreContextIDs, setup as Setup

def do(the_state_machine, pre_context_sm, BeginOfLinePreContextF):
    """Sets up a pre-condition to the given state machine. This process
       is entirely different from any sequentializing or parallelization
       of state machines. Here, the state machine representing the pre-
       condition is **not** webbed into the original state machine!

       Instead, the following happens:

          -- the pre-condition state machine is inverted, because
             it is to be walked through backwards.
          -- the inverted state machine is marked with the state machine id
             of the_state_machine.        
          -- the original state machine will refer to the inverse
             state machine of the pre-condition.
          -- the initial state origins and the origins of the acceptance
             states are marked as 'pre-conditioned' indicating the id
             of the inverted state machine of the pre-condition.             
    """
    #___________________________________________________________________________________________
    # (*) do some consistency checking   
    # -- state machines with no states are senseless here. 
    assert not the_state_machine.is_empty() 
    assert pre_context_sm is None or not pre_context_sm.is_empty()
    # -- trivial pre-conditions should be added last, for simplicity

    #___________________________________________________________________________________________
        
    if pre_context_sm is  None:
        if BeginOfLinePreContextF:
            # Mark all acceptance states with the 'trivial pre-context BeginOfLine' flag
            for state in the_state_machine.get_acceptance_state_list():
                state.set_pre_context_id(E_PreContextIDs.BEGIN_OF_LINE)
        return None

    # (*) invert the state machine of the pre-condition 
    inverse_pre_context = pre_context_sm.get_inverse()
        
    if BeginOfLinePreContextF:
        # Extend the existing pre-context with a preceeding 'begin-of-line'.
        inverse_pre_context.mount_newline_to_acceptance_states(Setup.dos_carriage_return_newline_f, InverseF=True)

    # (*) Clean up what has been done by inversion (and optionally 'BeginOfLinePreContextF')
    inverse_pre_context = beautifier.do(inverse_pre_context)

    # (*) Once an acceptance state is reached no further analysis is necessary.
    acceptance_pruning.do(inverse_pre_context)

    # (*) let the state machine refer to it 
    #     [Is this necessary? Is it not enough that the acceptance origins point to it? <fschaef>]
    pre_context_sm_id = inverse_pre_context.get_id()

    # (*) create origin data, in case where there is none yet create new one.
    #     (do not delete, otherwise existing information gets lost)
    for state in the_state_machine.states.values():
        if not state.is_acceptance(): continue
        state.set_pre_context_id(pre_context_sm_id)
    
    return inverse_pre_context

            
