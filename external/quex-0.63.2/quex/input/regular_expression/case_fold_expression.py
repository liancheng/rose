from   quex.engine.misc.file_in                import check, error_msg, read_until_character, skip_whitespace
from   quex.engine.interval_handling import Interval
import quex.engine.unicode_db.case_fold_parser as     ucs_case_fold

def do(sh, PatternDict, snap_expression=None, snap_set_expression=None):
    """Parse a case fold expression of the form \C(..){ R } or \C{ R }.
       Assume that '\C' has been snapped already from the stream.

       See function ucs_case_fold_parser.get_fold_set() for details
       about case folding.

       snap_expression is not None, then snap_expression is the function 
                                to parse a RE and the caller
                                expects a state machine.

       snap_set_expression is not None, then snap_set_expression is the
                                    function to parse a character 
                                    set and caller expects a 
                                    NumberSet object.
    """

    pos = sh.tell()
    skip_whitespace(sh)
    # -- parse the optional options in '(' ')' brackets
    if not check(sh, "("):
        # By default 'single' and 'multi' character case folds are active
        if snap_set_expression is not None: flag_txt = "s"
        else:                           flag_txt = "sm"

    else:
        flag_txt = read_until_character(sh, ")")

        if flag_txt == "":
            sh.seek(pos)
            error_msg("Missing closing ')' in case fold expression.", sh)

        flag_txt = flag_txt.replace(" ", "").replace("\t", "").replace("\n", "")

        for letter in flag_txt:
            if letter not in "smt":
                sh.seek(pos)
                error_msg("Letter '%s' not permitted as case fold option.\n" % letter + \
                          "Options are:  's' for simple case fold.\n" + \
                          "              'm' for multi character sequence case fold.\n" + \
                          "              't' for special turkish case fold rules.", sh)

            if snap_set_expression is not None and letter == "m":
                sh.seek(pos)
                error_msg("Option 'm' not permitted as case fold option in set expression.\n" + \
                          "Set expressions cannot absorb multi character sequences.", sh)

        skip_whitespace(sh)

    # -- parse the expression in '{' '}' which is subject to case folding
    if not check(sh, "{"):
        sh.seek(pos)
        error_msg("Missing '{' for case fold expression.", sh)

    skip_whitespace(sh)
    if snap_set_expression is not None:
        trigger_set = snap_set_expression(sh, PatternDict)
        if trigger_set is None:
            error_msg("Missing character set for case fold in set expression.\n" + 
                      "The content in '\\C{content}' should start with '[' or '[:'.", 
                      sh)

        # -- perform the case fold for Sets!
        for interval in trigger_set.get_intervals(PromiseToTreatWellF=True):
            for i in range(interval.begin, interval.end):
                fold = ucs_case_fold.get_fold_set(i, flag_txt)
                for x in fold:
                    assert type(x) != list
                    trigger_set.add_interval(Interval(x, x+1))

        result = trigger_set

    else:
        sm = snap_expression(sh, PatternDict)
        if sm is None:
            error_msg("Missing expression for case fold '\C'.\n" + 
                      "The content in '\\C{content}' should start with '[' or '[:'.", 
                      sh)

        # -- perform the case fold for State Machines!
        for state_idx, state in sm.states.items():
            transitions = state.transitions()
            for target_state_idx, trigger_set in transitions.get_map().items():
                __add_case_fold(sm, flag_txt, trigger_set, state_idx, target_state_idx)

        result = sm

    if not check(sh, "}"):
        sh.seek(pos)
        error_msg("Missing '}' for case fold expression.", sh)

    return result

def __add_intermediate_states(sm, character_list, start_state_idx, target_state_idx):
    next_idx = start_state_idx
    for letter in character_list[:-1]:
        next_idx = sm.add_transition(next_idx, letter)
    sm.add_transition(next_idx, character_list[-1], target_state_idx)

def __add_case_fold(sm, Flags, trigger_set, start_state_idx, target_state_idx):
    for interval in trigger_set.get_intervals(PromiseToTreatWellF=True):
        for i in range(interval.begin, interval.end):
            fold = ucs_case_fold.get_fold_set(i, Flags)
            for x in fold:
                if type(x) == list:
                    __add_intermediate_states(sm, x, start_state_idx, target_state_idx)
                else:
                    trigger_set.add_interval(Interval(x, x+1))


