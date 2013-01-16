from   quex.engine.misc.file_in                    import error_msg, verify_word_in_list
import quex.blackboard                             as     blackboard
from   quex.blackboard                             import setup as Setup, E_SpecialPatterns
from   quex.engine.generator.action_info           import CodeFragment
import quex.engine.state_machine.check.outrun      as     outrun_checker
import quex.engine.state_machine.check.superset    as     superset_check
import quex.engine.state_machine.check.same        as     same_check
from   itertools import islice


def do(ModeDB):
    """Consistency check of mode database

       -- Are there applicable modes?
       -- Start mode:
          -- specified (more than one applicable mode req. explicit specification)?
          -- is defined as mode?
          -- start mode is not inheritable only?
       -- Entry/Exit transitions are allows?
    """
    if Setup.token_class_only_f:
        if len(ModeDB) != 0:
            error_msg("Modes found in input files. However, only a token class is generated.", DontExitF=True)
        return

    if len(ModeDB) == 0:
        error_msg("No single mode defined - bailing out", Prefix="consistency check")

    mode_name_list            = map(lambda mode:        mode.name, ModeDB.values())
    mode_name_list.sort()
    applicable_mode_name_list = map(lambda mode:        mode.name, 
                                    filter(lambda mode: mode.options["inheritable"] != "only",
                                           ModeDB.values()))
    applicable_mode_name_list.sort()

    # (*) Is there a mode that is applicable?
    #     That is: is there a more that is not only inheritable?
    if len(applicable_mode_name_list) == 0:
        error_msg("There is no mode that can be applied---all existing modes are 'inheritable only'.\n" + \
                  "modes are = " + repr(mode_name_list)[1:-1],
                  Prefix="consistency check")

    # (*) Start mode specified?
    __start_mode(applicable_mode_name_list, mode_name_list)

    # (*) Modes that are inherited must allow to be inherited
    for mode in ModeDB.values():
        for base_mode in mode.get_base_mode_sequence()[:-1]:
            if base_mode.options["inheritable"] == "no":
                error_msg("mode '%s' inherits mode '%s' which is not inheritable." % \
                          (mode.name, base_mode.name), mode.filename, mode.line_n)

    # (*) A mode that is instantiable (to be implemented) needs finally contain matches!
    for mode in ModeDB.values():
        if mode.name in applicable_mode_name_list and len(mode.get_pattern_action_pair_list()) == 0:
            error_msg("Mode '%s' was defined without the option <inheritable: only>.\n" % mode.name + \
                      "However, it contains no matches--only event handlers. Without pattern\n"     + \
                      "matches it cannot act as a pattern detecting state machine, and thus\n"      + \
                      "cannot be an independent lexical analyzer mode. Define the option\n"         + \
                      "<inheritable: only>.", \
                      mode.filename, mode.line_n)

    # (*) Entry/Exit Transitions
    for mode in ModeDB.values():
        __entry_exit_transitions(mode, mode_name_list)

    # (*) Indentation: Newline Pattern and Newline Suppressor Pattern
    #     shall never trigger on common lexemes.
    for mode in ModeDB.values():
        __indentation_setup_check(mode)

    # (*) [Optional] Warnings on Outrun
    #     Warn when low priority patterns may outrun high priority patterns.
    if Setup.warning_on_outrun_f:
        for mode in ModeDB.values():
             __outrun_investigation(mode)

    # (*) Special Patterns shall not match on same lexemes
    if Setup.error_on_special_pattern_same_f:
        for mode in ModeDB.values():
            for pattern_action_pair in mode.get_pattern_action_pair_list():
                if pattern_action_pair.comment not in E_SpecialPatterns: continue
                __match_same_check(mode, pattern_action_pair)

    # (*) Special Patterns (skip, indentation, etc.) 
    #     shall not be outrun by another pattern.
    if Setup.error_on_special_pattern_outrun_f:
        for mode in ModeDB.values():
            for pattern_action_pair in mode.get_pattern_action_pair_list():
                if pattern_action_pair.comment not in E_SpecialPatterns: continue
                __outrun_check(mode, pattern_action_pair)

    # (*) Special Patterns shall not have common matches with patterns
    #     of higher precedence.
    if Setup.error_on_special_pattern_subset_f:
        for mode in ModeDB.values():
            for pattern_action_pair in mode.get_pattern_action_pair_list():
                if pattern_action_pair.comment not in E_SpecialPatterns: continue
                __subset_check(mode, pattern_action_pair)

    # (*) Check for dominated patterns
    if Setup.error_on_dominated_pattern_f:
        for mode in ModeDB.values():
            for pattern_action_pair in mode.get_pattern_action_pair_list():
                __dominated_pattern_check(mode, pattern_action_pair)


def __indentation_setup_check(mode):
    indentation_setup = mode.options["indentation"]
    if indentation_setup is None: return

    # The newline pattern shall not have intersections with other patterns!
    newline_info            = indentation_setup.newline_state_machine
    newline_suppressor_info = indentation_setup.newline_suppressor_state_machine
    assert newline_info is not None
    if newline_suppressor_info.get() is None: return

    # Newline and newline suppressor should never have a superset/subset relation
    if    superset_check.do(newline_info.get(), newline_suppressor_info.get()) \
       or superset_check.do(newline_suppressor_info.get(), newline_info.get()):

        __error_message(newline_info, newline_suppressor_info,
                        ThisComment="matches on some common lexemes as",
                        ThatComment="") 

def __outrun_investigation(mode):
    pattern_action_pair_list = mode.get_pattern_action_pair_list()
    # Sort by pattern_id
    pattern_action_pair_list.sort(key=lambda x: x.pattern().sm.get_id())
    for i, pap_i in enumerate(pattern_action_pair_list):
        sm_high = pap_i.pattern().sm
        for pap_k in islice(pattern_action_pair_list, i+1, None):
            # 'pap_k' has a higher id than 'pap_i'. Thus, it has a lower
            # priority. Check for outrun.
            sm_low = pap_k.pattern().sm
            if outrun_checker.do(sm_high, sm_low):
                file_name, line_n = pap_i.get_action_location()
                __error_message(pap_k, pap_i, ExitF=False, ThisComment="may outrun")

def __outrun_check(mode, PAP):
    ReferenceSM = PAP.pattern().sm 

    for other_pap in mode.get_pattern_action_pair_list():
        sm = other_pap.pattern().sm
        # No 'commonalities' between self and self shall be checked
        if ReferenceSM.get_id() >= sm.get_id(): continue

        if outrun_checker.do(ReferenceSM, sm):
            __error_message(other_pap, PAP, ExitF=True, 
                            ThisComment="has lower priority but",
                            ThatComment="may outrun")
                             
def __subset_check(mode, PAP):
    """Checks whether a higher prioritized pattern matches a common subset
       of the ReferenceSM. For special patterns of skipper, etc. this would
       be highly confusing.
    """
    global special_pattern_list
    ReferenceSM = PAP.pattern().sm

    for other_pap in mode.get_pattern_action_pair_list():
        sm = other_pap.pattern().sm
        if   ReferenceSM.get_id() <= sm.get_id(): continue
        elif not superset_check.do(ReferenceSM, sm): continue

        __error_message(other_pap, PAP, ExitF=True, 
                        ThisComment="has higher priority and",
                        ThatComment="matches a subset of")

def __match_same_check(mode, PAP):
    """Special patterns shall never match on some common lexemes."""
    A = PAP.pattern().sm
    for other_pap in mode.get_pattern_action_pair_list():

        B  = other_pap.pattern().sm
        if   other_pap.comment not in E_SpecialPatterns: continue
        elif A.get_id() == B.get_id(): continue

        # A superset of B, or B superset of A => there are common matches.
        if same_check.do(A, B):
            __error_message(other_pap, PAP, 
                            ThisComment="matches on some common lexemes as",
                            ThatComment="", 
                            ExitF=True)

def __dominated_pattern_check(mode, PAP):
    pattern_id  = PAP.pattern().sm.get_id()

    for other_pap in mode.get_pattern_action_pair_list():
        if other_pap.pattern().sm.get_id() >= pattern_id: continue
        if superset_check.do(other_pap.pattern(), PAP.pattern()):
            file_name, line_n = other_pap.get_action_location()
            __error_message(other_pap, PAP, 
                            ThisComment = "matches a superset of what is matched by",
                            EndComment  = "The former has precedence and the latter can never match.",
                            ExitF       = True)

def __error_message(This, That, ThisComment, ThatComment="", EndComment="", ExitF=True):

    def get_name(PAP, AddSpaceF=True):
        if PAP.comment in E_SpecialPatterns: 
            result = repr(PAP.comment).replace("_", " ").lower()
        elif isinstance(PAP.comment, (str, unicode)):
            result = PAP.comment
        else:
            return ""

        if AddSpaceF and len(result) != 0: result += " "
        return result

    file_name, line_n = This.get_action_location()
    error_msg("The %spattern '%s' %s" % (get_name(This), This.pattern_string(), ThisComment), 
              file_name, line_n, 
              DontExitF=True, WarningF=not ExitF)

    FileName, LineN   = That.get_action_location()
    if len(ThatComment) != 0: Space = " "
    else:                     Space = ""
    error_msg("%s%s%spattern '%s'." % (ThatComment, Space, get_name(That, AddSpaceF=True), That.pattern_string()), 
              FileName, LineN,
              DontExitF=not (len(EndComment) == 0 and ExitF), 
              WarningF=not ExitF)

    if len(EndComment) != 0:
        error_msg(EndComment, FileName, LineN, DontExitF=not ExitF, WarningF=not ExitF)

def __start_mode(applicable_mode_name_list, mode_name_list):
    """If more then one mode is defined, then that requires an explicit 
       definition 'start = mode'.
    """
    assert len(applicable_mode_name_list) != 0

    start_mode = blackboard.initial_mode.get_pure_code()
    if start_mode == "":
        # Choose an applicable mode as start mode
        start_mode              = applicable_mode_name_list[0]
        blackboard.initial_mode = CodeFragment(start_mode)
        if len(applicable_mode_name_list) > 1:
            error_msg("No initial mode defined via 'start' while more than one applicable mode exists.\n" + \
                      "Use for example 'start = %s;' in the quex source file to define an initial mode." \
                      % start_mode)
        # This Branch: start mode is applicable and present

    else: 
        FileName = blackboard.initial_mode.filename
        LineN    = blackboard.initial_mode.line_n
        # Start mode present and applicable?
        verify_word_in_list(start_mode, mode_name_list,
                            "Start mode '%s' is not defined." % start_mode,
                            FileName, LineN)
        verify_word_in_list(start_mode, applicable_mode_name_list,
                            "Start mode '%s' is inheritable only and cannot be instantiated." % start_mode,
                            FileName, LineN)

def __entry_exit_transitions(mode, mode_name_list):
    FileName = mode.filename
    LineN    = mode.line_n
    for mode_name in mode.options["exit"]:

        verify_word_in_list(mode_name, mode_name_list,
                            "Mode '%s' allows entry from\nmode '%s' but no such mode exists." % \
                            (mode.name, mode_name), FileName, LineN)

        that_mode = blackboard.mode_db[mode_name]

        # Other mode allows all entries => don't worry.
        if len(that_mode.options["entry"]) == 0: continue

        # Other mode restricts the entries from other modes
        # => check if this mode or one of the base modes can enter
        for base_mode in mode.get_base_mode_sequence():
            if base_mode.name in that_mode.options["entry"]: break
        else:
            error_msg("Mode '%s' has an exit to mode '%s' but" % (mode.name, mode_name),
                      FileName, LineN, DontExitF=True, WarningF=False)
            error_msg("mode '%s' has no entry for mode '%s'\n" % (mode_name, mode.name) + \
                      "or any of its base modes.",
                      that_mode.filename, that_mode.line_n)

    for mode_name in mode.options["entry"]:
        # Does that mode exist?
        verify_word_in_list(mode_name, mode_name_list,
                            "Mode '%s' allows entry from\nmode '%s' but no such mode exists." % \
                            (mode.name, mode_name), FileName, LineN)

        that_mode = blackboard.mode_db[mode_name]
        # Other mode allows all exits => don't worry.
        if len(that_mode.options["exit"]) == 0: continue

        # Other mode restricts the exits to other modes
        # => check if this mode or one of the base modes can be reached
        for base_mode in mode.get_base_mode_sequence():
            if base_mode.name in that_mode.options["exit"]: break
        else:
            error_msg("Mode '%s' has an entry for mode '%s' but" % (mode.name, mode_name),
                      FileName, LineN, DontExitF=True, WarningF=False)
            error_msg("mode '%s' has no exit to mode '%s'\n" % (mode_name, mode.name) + \
                      "or any of its base modes.",
                      that_mode.filename, that_mode.line_n)
            
