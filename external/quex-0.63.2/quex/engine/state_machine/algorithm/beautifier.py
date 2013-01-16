import quex.engine.state_machine.algorithm.nfa_to_dfa            as nfa_to_dfa
import quex.engine.state_machine.algorithm.hopcroft_minimization as hopcroft

def do(SM):
    result = nfa_to_dfa.do(SM)
    hopcroft.do(result, CreateNewStateMachineF=False)
    return result
