import quex.engine.generator.state.transition.solution  as solution

def get_index(TriggerMap):
    """Returns the best considered index in TriggerMap where the trigger map
       is to be split. This includes the considerations about elegant switch 
       cases. It is tried to avoid cuts in between switch-case regions.
    """
    L = len(TriggerMap)

    # Make sure that no cut in the middle of a switch case
    preferred_section_index = int(L / 2)
    return preferred_section_index

    best_section_index      = -1
    best_dist               = L
    switch_case_range_list  = __get_switch_cases_info(TriggerMap)
    for candidate in xrange(L):
        for p, q in switch_case_range_list:
            if candidate >= p and candidate <= q: 
                break
        else:
            # No intersection happened, so index may be used
            if abs(candidate - preferred_section_index) >= best_dist: continue
            best_section_index = candidate
            best_dist          = abs(candidate - preferred_section_index)

    if best_section_index not in [-1, 0, L-1]: return best_section_index
    else:                                      return preferred_section_index; 

def __get_switch_cases_info(TriggerMap):
    L = len(TriggerMap)
    sum_interval_size          = [0] * (L+1)
    sum_drop_out_interval_size = [0] * (L+1)
    i = 0
    for interval, target in TriggerMap:
        i += 1
        sum_interval_size[i]          = sum_interval_size[i-1]
        sum_drop_out_interval_size[i] = sum_drop_out_interval_size[i-1]
        if target.drop_out_f: sum_drop_out_interval_size[i] += interval.size()
        else:                 sum_interval_size[i]          += interval.size()

    switch_case_range_list = []
    p = 0
    while p < L:
        # Count from the back, so the longest is treated first.
        # Thus, if there is a 'p' for a given 'q' where the criteria
        # holds for a switch case, then the 'p' is the best one, in the
        # sense that it is the largest interval.
        q_found = None
        for q in xrange(L-1, p, -1):
            if solution.get(TriggerMap[p:q+1], 
                            size_all_intervals          = sum_interval_size[q]          - sum_interval_size[p],
                            size_all_drop_out_intervals = sum_drop_out_interval_size[q] - sum_drop_out_interval_size[p]) \
               == solution.E_Type.SWITCH_CASE:
                switch_case_range_list.append((p, q))
                q_found = q
                break
        # If there was a switch case range, that step over it to the next
        if q_found: p = q_found
        p += 1
    return switch_case_range_list
