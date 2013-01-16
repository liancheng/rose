# (C) 2012 Frank-Rene Schaefer
import quex.engine.analyzer.mega_state.template.core    as     template_analyzer
import quex.engine.analyzer.mega_state.path_walker.core as     path_analyzer
from   quex.engine.analyzer.mega_state.core             import AbsorbedState
from   quex.blackboard                                  import setup as Setup, \
                                                               E_Compression

def do(TheAnalyzer):
    """MegaState Analysis _____________________________________________________

    Normal states are potentially absorbed by MegaState-s which represent more
    than one single state at once.

    The setting 'Setup.compression_type_list' defines what type of algorithms
    have to be executed in to construct MegaStates (if any at all). Consider
    'core.py' in this directory for further reading.
    ___________________________________________________________________________
    """
    mega_state_db = {}

    # The 'remainder' keeps track of states which have not yet been
    # absorbed into a MegaState.
    remainder = set(TheAnalyzer.state_db.keys())
    remainder.remove(TheAnalyzer.init_state_index)

    for ctype in Setup.compression_type_list:
        # -- MegaState-s by Path-Compression
        if ctype in (E_Compression.PATH, E_Compression.PATH_UNIFORM):
            absorbance_db = path_analyzer.do(TheAnalyzer, ctype, remainder)
    
        # -- MegaState-s by Template-Compression
        elif ctype in (E_Compression.TEMPLATE, E_Compression.TEMPLATE_UNIFORM):
            absorbance_db = template_analyzer.do(TheAnalyzer, 
                                                 Setup.compression_template_min_gain, ctype, 
                                                 remainder)
        else:
            assert False

        # -- Post-process the absorption of AnalyzerState-s into MegaState-s
        for state_index, mega_state in absorbance_db.iteritems():
            # Replace the absorbed AnalyzerState by its dummy.
            TheAnalyzer.state_db[state_index] = \
                        AbsorbedState(TheAnalyzer.state_db[state_index], mega_state)

            # Track the remaining not-yet-absorbed states
            assert state_index in remainder
            remainder.remove(state_index)

            # Track MegaStates. A 'absorbance_db.itervalues()' may contain 
            # the same MegaState twice. Use a dictionary to keep them unique.
            if mega_state.index not in mega_state_db:
                assert mega_state.index not in mega_state_db
                mega_state_db[mega_state.index] = mega_state 

    # Let the analyzer know about the MegaState-s and what states they left
    # unabsorbed. 
    TheAnalyzer.non_mega_state_index_set = remainder
    TheAnalyzer.mega_state_list          = mega_state_db.values()

    for mega_state in TheAnalyzer.mega_state_list:
        mega_state.finalize_transition_map(TheAnalyzer.state_db)

    # Only now: We enter the MegaState-s into the 'state_db'. If it was done before,
    # the MegaStates might try to absorb each other.
    TheAnalyzer.state_db.update(mega_state_db)

