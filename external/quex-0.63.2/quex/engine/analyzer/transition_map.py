from   quex.engine.interval_handling import Interval
from   quex.blackboard import E_StateIndices
import sys
from   copy import deepcopy
from   itertools import imap, izip
from   operator  import itemgetter

def zipped_iterable(TransitionMapA, TransitionMapB):
    """Produces an iterable over two transition maps at once. The borders in the
    zipped transition map consist of a superset of all borders of transition
    map 'A' and 'B'. Whenever a border hits a new interval is notified. 

            YIELDS:  begin, end, a_target, b_target

    Interval [begin, end) is homogenous in the zipped transition map, i.e.
    inside this interval 'A' triggers to 'a_target' and 'B' to 'b_target'.  
    """
    LenA             = len(TransitionMapA)
    LenB             = len(TransitionMapB)
    i                = 0 # iterator over TransitionMapA
    k                = 0 # iterator over TransitionMapB
    i_itvl, i_target = TransitionMapA[i]
    k_itvl, k_target = TransitionMapB[k]
    prev_end         = - sys.maxint
    # Intervals in trigger map are always adjacent, so the '.begin' member is
    # not accessed.
    while not (i == LenA - 1 and k == LenB - 1):
        end    = min(i_itvl.end, k_itvl.end)

        yield prev_end, end, i_target, k_target

        prev_end  = end

        if   i_itvl.end == k_itvl.end: 
            i += 1; i_itvl, i_target = TransitionMapA[i]
            k += 1; k_itvl, k_target = TransitionMapB[k]
        elif i_itvl.end <  k_itvl.end: 
            i += 1; i_itvl, i_target = TransitionMapA[i]
        else:                          
            k += 1; k_itvl, k_target = TransitionMapB[k]

    # Treat the last trigger interval
    yield prev_end, sys.maxint, TransitionMapA[-1][1], TransitionMapB[-1][1]
    return

def clone(transition_map):
    return [ (interval.clone(), deepcopy(target)) for interval, target in transition_map ]

def relate_to_door_ids(transition_map, TheAnalyzer, StateIndex):
    """Creates a transition_map that triggers to DoorIDs instead of target states.
    """
    def adapt(Target):
        if Target == E_StateIndices.DROP_OUT:
            return Target
        else:
            result = TheAnalyzer.state_db[Target].entry.get_door_id(StateIndex=Target, FromStateIndex=StateIndex)
            return result

    return [(interval, adapt(target)) for interval, target in transition_map]

def get_string(transition_map, Option="utf8"):
    assert Option in ("hex", "dec", "utf8")
    if len(transition_map) == 0:
        return "   <empty>"
    L = max(len(x[0].get_string(Option)) for x in transition_map)
    txt = []
    for interval, target in transition_map:
        interval_str = interval.get_string(Option)
        txt.append("   %s%s %s\n" % (interval_str, " " * (L - len(interval_str)), target))
    return "".join(txt)


def bisect_begin(transition_map, Value, lower=0):
    """Find entry 'i' by bisectioning so that it holds:

          -- transition_map[i][0].begin <= Value
          -- transition_map[k][0].begin > Value for all k > i
    """
    upper = len(transition_map)
    delta = upper - lower
    while delta > 1:
        i       = lower + delta / 2
        current = transition_map[i][0].begin
        if   current > Value: upper = i 
        elif current < Value: lower = i
        else:                 return i
        delta   = upper - lower

    return lower if transition_map[lower][0].begin == Value else -1

def set(transition_map, Character, NewTarget):
    # (bisectioning would certainly be more elegant ...)
    for i, entry in enumerate(transition_map):
        interval, target = entry
        if interval.contains(Character):
            break
    else:
        print "TM: {\n%s}" % get_string(transition_map, "dec")
        print "Character:", Character
        assert False

    # Found the interval that contains the Character
    assert interval.size() > 0
    L = len(transition_map)

    # Check whether an adjacent interval has the same target, so that
    # the new interval can directly be docked to it.
    if target == NewTarget:
        return # Nothing to be done

    if interval.size() == 1:
        if i > 0 and interval.begin == Character and transition_map[i-1][1] == NewTarget:
            # Interval before fits
            if i < L-1 and interval.end == Character + 1 and transition_map[i+1][1] == NewTarget:
                # Interval before and after fits
                transition_map[i-1][0].end = transition_map[i+1][0].end
                del transition_map[i:i+2]
            else:
                # Only interval before fits
                transition_map[i-1][0].end = transition_map[i][0].end
                del transition_map[i]

        elif i < L-1 and interval.end == Character + 1 and transition_map[i+1][1] == NewTarget:
            # Only interval after fits, Interval before does not fit
            transition_map[i+1][0].begin = transition_map[i][0].begin
            del transition_map[i]

        else:
            transition_map[i] = (interval, NewTarget)

        assert_adjacency(transition_map, TotalRangeF=False)
        return

    if i > 0 and interval.begin == Character and transition_map[i-1][1] == NewTarget:
        # Interval before fits, Interval after cannot fit, because size() > 1
        transition_map[i-1][0].end = Character + 1
        transition_map[i][0].begin = Character + 1

    elif i < L-1 and interval.end == Character + 1 and transition_map[i+1][1] == NewTarget:
        # Interval after fits, Interval before cannot fit, because size() > 1
        transition_map[i+1][0].begin = Character 
        transition_map[i][0].end     = Character

    elif interval.begin == Character:
        transition_map[i][0].begin = Character + 1
        transition_map.insert(i, (Interval(Character), NewTarget))

    elif interval.end == Character + 1:
        transition_map.insert(i+1, (Interval(Character), NewTarget))
        transition_map[i][0].end = Character

    else:
        # Character lies in the middle of a non-fitting interval
        transition_map.insert(i+1, (Interval(Character), NewTarget))
        transition_map.insert(i+2, (Interval(Character+1, interval.end), target))
        transition_map[i][0].end = Character 

    assert_adjacency(transition_map, TotalRangeF=False)
    return

def assert_adjacency(transition_map, TotalRangeF=False):
    """Check that the trigger map consist of sorted adjacent intervals 
       This assumption is critical because it is assumed that for any isolated
       interval the bordering intervals have bracketed the remaining cases!
    """
    if len(transition_map) == 0: return
    iterable = transition_map.__iter__()
    if TotalRangeF: previous_end = - sys.maxint
    else:           previous_end = iterable.next()[0].end 
    for interval in imap(itemgetter(0), iterable):
        assert interval.begin == previous_end # Intervals are adjacent!
        assert interval.end > interval.begin  # Interval size > 0! 
        previous_end = interval.end

    assert (not TotalRangeF) or transition_map[-1][0].end == sys.maxint

def index(transition_map, Character):
    # TODO: Bisectioning
    for i, info in enumerate(transition_map):
        interval, target = info
        if   interval.end   <  Character + 1: continue
        elif interval.begin <= Character: return i
    return None

def smoothen(transition_map, Character):
    """Replaces a single character transition by a transition of its adjacent 
    intervals.
    """
    i = index(transition_map, Character)
    assert transition_map[i][0].size() == 1

    L = len(transition_map)
    if i > 0: 
        if i < L - 1 and transition_map[i+1][1] == transition_map[i-1][1]:
            transition_map[i-1][0].end = transition_map[i+1][0].end
            del transition_map[i:i+2]
        else:
            transition_map[i-1][0].end = Character
            del transition_map[i]

    elif i < L:
        transition_map[i+1][0].end = Character + 1
        del transition_map[i]
    else:
        assert False

def is_equal(One, Other):
    if len(One) != len(Other): return False
    for x, y in izip(One, Other):
        if   x[0] != y[0]: return False  # Interval
        elif x[1] != y[1]: return False  # Target
    return True


def get_target(transition_map, Character):
    # TODO: Bisectioning
    for interval, target in transition_map:
        if   interval.end   <  Character + 1: continue
        elif interval.begin <= Character: return target
    return None
