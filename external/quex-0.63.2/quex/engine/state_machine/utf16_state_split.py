# (C) 2009 Frank-Rene Schaefer
"""
ABSTRACT:

    !! UTF16 state split is similar to UTF8 state split as shown in file !!
    !! "uf8_state_split.py". Please, read the documentation thera about  !!
    !! the details of the basic idea.                                    !!

    Due to the fact that utf16 conversion has only two possible byte sequence
    lengths, 2 and 4 bytes, the state split process is significantly easier
    than the utf8 stae split.

    The principle idea remains: A single transition from state A to state B is
    translated (sometimes) into an intermediate state transition to reflect
    that the unicode point is represent by a value sequence.

    The special case utf16 again is easier, since, for any unicode point <=
    0xFFFF the representation value remains exactly the same, thus those
    intervals do not have to be adapted at all!
    
    Further, the identification of 'contigous' intervals where the last value
    runs repeatedly from min to max is restricted to the consideration of a
    single word. UTF16 character codes can contain at max two values (a
    'surrogate pair') coded in two 'words' (1 word = 2 bytes). The overun
    happens every 2*10 code points.  Since such intervals are pretty large and
    the probability that a range runs over multiple such ranges is low, it does
    not make sense to try to combine them. The later Hopcroft Minimization will
    not be overwhelmed by a little extra work.

"""
import os
import sys
sys.path.append(os.environ["QUEX_PATH"])

from   quex.engine.utf16                    import utf16_to_unicode, unicode_to_utf16
from   quex.engine.interval_handling        import Interval
import quex.engine.state_machine.algorithm.beautifier as beautifier

ForbiddenRange = Interval(0xD800, 0xE000)

def do(sm):
    state_list = sm.states.items()
    for state_index, state in state_list:
        # Get the 'transition_list', i.e. a list of pairs (TargetState, NumberSet)
        # which indicates what target state is reached via what number set.
        transition_list = state.transitions().get_map().items()
        # Clear the state's transitions, now. This way it can absorb new
        # transitions to intermediate states.
        state.transitions().clear()
        # Loop over all transitions
        for target_state_index, number_set in transition_list:
            # We take the intervals with 'PromiseToTreatWellF' even though they
            # are changed. This is because the intervals would be lost anyway
            # after the state split, so we use the same memory and do not 
            # cause a time consuming memory copy and constructor calls.
            interval_list = number_set.get_intervals(PromiseToTreatWellF=True)

            # 1st check whether a modification is necessary
            modification_required_f = False
            for interval in interval_list:
                if interval.begin >= 0x10000: modification_required_f = True; break

            if modification_required_f == False:
                sm.states[state_index].add_transition(number_set, target_state_index)
                continue

            # Now, intermediate states may be added
            for interval in interval_list:
                create_intermediate_states(sm, state_index, target_state_index, interval)
    
    result = beautifier.do(sm)
    return result

def do_set(NSet):
    """Unicode values > 0xFFFF are translated into byte sequences, thus, only number
       sets below that value can be transformed into number sets. They, actually
       remain the same.
    """
    for interval in NSet.get_intervals(PromiseToTreatWellF=True):
        if interval.end > 0x10000: return None
    return NSet

def create_intermediate_states(sm, StartStateIdx, EndStateIdx, X):
    # Split the interval into a range below and above 0xFFFF. This corresponds
    # unicode values that are represented in utf16 via 2 and 4 bytes (1 and 2 words).
    interval_1word, intervals_2word = get_contigous_intervals(X)

    if interval_1word is not None:
        sm.add_transition(StartStateIdx, interval_1word, EndStateIdx)

    if intervals_2word is not None:
        for interval in intervals_2word:
            # Introduce intermediate state
            trigger_seq = get_trigger_sequence_for_interval(interval)
            s_idx = sm.add_transition(StartStateIdx, trigger_seq[0])
            sm.add_transition(s_idx, trigger_seq[1], EndStateIdx)

def get_contigous_intervals(X):
    """Split Unicode interval into intervals where all values
       have the same utf16-byte sequence length. This is fairly 
       simple in comparison with utf8-byte sequence length: There
       are only two lengths: 2 bytes and 2 x 2 bytes.

       RETURNS:  [X0, List1]  

                 X0   = the sub-interval where all values are 1 word (2 byte)
                        utf16 encoded. 
                         
                        None => No such interval
                
                List1 = list of contigous sub-intervals where coded as 2 words.

                        None => No such intervals
    """
    global ForbiddenRange
    if X.begin == -sys.maxint: X.begin = 0
    if X.end   == sys.maxint:  X.end   = 0x110000
    assert X.end <= 0x110000                   # Interval must lie in unicode range
    assert not X.check_overlap(ForbiddenRange) # The 'forbidden range' is not to be covered.

    if X.end <= 0x10000:     return [X, None]
    elif X.begin >= 0x10000: return [None, split_contigous_intervals_for_surrogates(X.begin, X.end)]
    else:                    return [Interval(X.begin, 0x10000), split_contigous_intervals_for_surrogates(0x10000, X.end)]

def split_contigous_intervals_for_surrogates(Begin, End):
    """Splits the interval X into sub interval so that no interval runs over a 'surrogate'
       border of the last word. For that, it is simply checked if the End falls into the
       same 'surrogate' domain of 'front' (start value of front = Begin). If it does not
       an interval [front, end_of_domain) is split up and front is set to end of domain.
       This procedure repeats until front and End lie in the same domain.
    """
    assert Begin >= 0x10000
    assert End   <= 0x110000

    front_seq = unicode_to_utf16(Begin)
    back_seq  = unicode_to_utf16(End - 1)

    if front_seq[0] == back_seq[0]:
        return [Interval(Begin, End)]

    # Separate into three domains:
    #
    # (1) interval from Begin until second surrogate hits border 0xE000
    # (2) interval where the first surrogate inreases while second 
    #     surrogate iterates over [0xDC00, 0xDFFF]
    # (3) interval from begin of last surrogate border to End
    result = []
    end    = utf16_to_unicode([front_seq[0], 0xDFFF]) + 1
    # The following **must** hold according to entry condition about front and back sequence
    assert End > end
    result.append(Interval(Begin, end))
    if front_seq[0] + 1 != back_seq[0]: 
        mid_end = utf16_to_unicode([back_seq[0] - 1, 0xDFFF]) + 1
        result.append(Interval(end, mid_end)) 
        end = mid_end
    result.append(Interval(end, End)) 

    return result
    
def get_trigger_sequence_for_interval(X):
    # The interval either lies entirely >= 0x10000 or entirely < 0x10000
    assert X.begin >= 0x10000 or X.end < 0x10000

    # An interval below < 0x10000 remains the same
    if X.end < 0x10000: return [ X ]
    
    # In case that the interval >= 0x10000 it the value is split up into
    # two values.
    front_seq = unicode_to_utf16(X.begin)
    back_seq  = unicode_to_utf16(X.end - 1)

    return [ Interval(front_seq[0], back_seq[0] + 1), 
             Interval(front_seq[1], back_seq[1] + 1) ]
    


