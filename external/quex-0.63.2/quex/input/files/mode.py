
import quex.blackboard as blackboard

import quex.input.regular_expression.core                  as regular_expression
import quex.input.files.code_fragment                      as code_fragment
import quex.input.files.indentation_setup                  as indentation_setup
import quex.input.files.consistency_check                  as consistency_check
import quex.input.regular_expression.snap_character_string as snap_character_string
from   quex.input.regular_expression.construct             import Pattern
from   quex.blackboard                                     import setup as Setup, \
                                                                  E_SpecialPatterns

from   quex.engine.generator.action_info                   import CodeFragment, UserCodeFragment, GeneratedCode, PatternActionInfo
from   quex.engine.generator.languages.address             import get_label
import quex.engine.generator.skipper.character_set         as     skip_character_set
import quex.engine.generator.skipper.range                 as     skip_range
import quex.engine.generator.skipper.nested_range          as     skip_nested_range
import quex.engine.generator.state.indentation_counter     as     indentation_counter
from   quex.engine.misc.file_in                            import EndOfStreamException, \
                                                                  check, \
                                                                  check_or_die, \
                                                                  copy, \
                                                                  error_msg, \
                                                                  get_current_line_info_number, \
                                                                  read_identifier, \
                                                                  read_option_start, \
                                                                  read_option_value, \
                                                                  read_until_letter, \
                                                                  read_until_whitespace, \
                                                                  skip_whitespace, \
                                                                  verify_word_in_list

from   quex.engine.state_machine.core                      import StateMachine
import quex.engine.state_machine.check.identity            as identity_checker
import quex.engine.state_machine.sequentialize             as sequentialize
import quex.engine.state_machine.repeat                    as repeat
import quex.engine.state_machine.algorithm.beautifier             as beautifier
import quex.engine.state_machine.algorithm.nfa_to_dfa             as nfa_to_dfa
import quex.engine.state_machine.algorithm.hopcroft_minimization  as hopcroft

from   copy import deepcopy

# ModeDescription/Mode Objects:
#
# During parsing 'ModeDescription' objects are generated. Once parsing is over, 
# the descriptions are translated into 'real' mode objects where code can be generated
# from. All matters of inheritance and pattern resolution are handled in the
# transition from description to real mode.
#-----------------------------------------------------------------------------------------
# mode_description_db: storing the mode information into a dictionary:
#                      key  = mode name
#                      item = ModeDescription object
#-----------------------------------------------------------------------------------------
mode_description_db = {}

class OptionInfo:
    """This type is used only in context of a dictionary, the key
       to the dictionary is the option's name."""
    def __init__(self, Type, Domain=None, Default=-1):
        # self.name = Option see comment above
        self.type          = Type
        self.domain        = Domain
        self.default_value = Default

class ModeDescription:
    def __init__(self, Name, Filename, LineN):

        self.filename = Filename
        self.line_n   = LineN

        self.name       = Name
        self.base_modes = []
        # Read pattern information into dictionary object. This allows for the following:
        # (i)   inheritance of pattern behavior in different modes.
        # (ii)  'virtual' patterns in the sense that their behavior can be
        #       overwritten.
        self.__matches = {}            # genuine patterns as specified in the mode declaration

        self.__repriorization_db = {}  # patterns of the base class to be reprioritized
        #                              # map: pattern --> new pattern index
        self.__deletion_db       = {}  # patterns of the base class to be deleted

        # The list of actual pattern action pairs is constructed inside the function
        # '__post_process(...)'. Function 'get_pattern_action_pairs(...) calls it
        # in case that this variable is still [].
        self.__pattern_action_pair_list = []  

        # (*) Default Options
        self.options = {}      
        for name, descr in mode_option_info_db.items():
            # Not only copy the reference, copy the default value object!
            self.options[name] = deepcopy(descr.default_value)

        # (*) Default Event Handler: Empty
        self.events = {}
        for name in event_handler_db.keys():
            self.events[name] = CodeFragment()

        # Register ModeDescription at the mode database
        mode_description_db[Name] = self

    def add_match(self, PatternStr, Action, ThePattern, Comment=""):
        assert    ThePattern.sm.is_DFA_compliant()
        assert    ThePattern.inverse_pre_context_sm is None \
               or ThePattern.inverse_pre_context_sm.is_DFA_compliant()

        if self.__matches.has_key(PatternStr):
            error_msg("Pattern '%s' appeared twice in mode definition.\n" % PatternStr + \
                      "Only the last definition is considered.", 
                      Action.filename, Action.line_n, DontExitF=True)

        if     len(ThePattern.sm.get_orphaned_state_index_list()) != 0 \
           or (    ThePattern.inverse_pre_context_sm is not None \
               and len(ThePattern.inverse_pre_context_sm.get_orphaned_state_index_list()) != 0):
            error_msg("Pattern '%s' resulted in state machine with orphan states.\n" % PatternStr + \
                      "(After Transformation to internal encoding).\n" + \
                      "Please, submit a bug at quex.sourceforge.net.", 
                      DontExitF=True, WarningF=True)

        self.__matches[PatternStr] = PatternActionInfo(ThePattern, Action, PatternStr, 
                                                       ModeName=self.name, Comment=Comment)

    def add_match_priority(self, Pattern, ThePattern, PatternIdx, FileName, LineN):
        if self.__matches.has_key(Pattern):
            error_msg("Pattern '%s' appeared twice in mode definition.\n" % Pattern + \
                      "Only this priority mark is considered.", FileName, LineN)

        self.__repriorization_db[Pattern] = [ThePattern, FileName, LineN, PatternIdx]

    def add_match_deletion(self, Pattern, ThePattern, FileName, LineN):
        if self.__matches.has_key(Pattern):
            error_msg("Deletion of '%s' which appeared before in same mode.\n" % Pattern + \
                      "Deletion of pattern.", FileName, LineN)

        self.__deletion_db[Pattern] = [ThePattern, FileName, LineN]

    def add_option(self, Option, Value):
        """ SANITY CHECK:
                -- which options are concatinated to a list
                -- which ones are replaced
                -- what are the values of the options
        """
        assert mode_option_info_db.has_key(Option)

        option_info = mode_option_info_db[Option]
        if option_info.type == "list":
            self.options.setdefault(Option, []).append(Value)
        else:
            if option_info.domain is not None: assert Value in option_info.domain
            self.options[Option] = Value

    def get_pattern_action_pair(self, PatternStr):
        return self.__matches[PatternStr]

    def get_match_list(self):
        return self.__matches.values()

    def get_repriorization_db(self):
        return self.__repriorization_db

    def get_deletion_db(self):
        return self.__deletion_db

    def has_event_handler(self):
        for fragment in self.events.values():
            if fragment.get_code() != "": return True
        return False

    def has_pattern(self, PatternStr):
        return self.__matches.has_key(PatternStr)

    def has_own_matches(self):
        return len(self.__matches) != 0

    def has_matches(self):
        if self.__matches != {}: return True

        for name in self.base_modes:
           if mode_description_db[name].has_matches(): return True

        return False

class Mode:
    def __init__(self, Other):
        """Translate a ModeDescription into a real Mode. Here is the place were 
           all rules of inheritance mechanisms and pattern precedence are applied.
        """
        assert isinstance(Other, ModeDescription)
        self.name     = Other.name
        self.filename = Other.filename
        self.line_n   = Other.line_n
        self.options  = Other.options

        self.__base_mode_sequence = []
        self.__determine_base_mode_sequence(Other, [])

        # (1) Collect Event Handlers
        self.__event_handler_code_fragment_list = {}
        self.__collect_event_handler()
        
        # (2) Collect Pattern/Action Pairs
        self.__history_repriorization = []
        self.__history_deletion       = []
        self.__pattern_action_pair_list = self.__collect_pattern_action_pairs()

        # (3) Collection Options
        self.__collect_options()

    def insert_code_fragment_at_front(self, EventName, TheCodeFragment):
        assert isinstance(TheCodeFragment, CodeFragment)
        assert EventName == "on_end_of_stream"
        self.__event_handler_code_fragment_list[EventName].insert(0, TheCodeFragment)

    def set_code_fragment_list(self, EventName, TheCodeFragment):
        assert isinstance(TheCodeFragment, CodeFragment)
        assert EventName in ["on_end_of_stream", "on_failure"]
        assert len(self.__event_handler_code_fragment_list[EventName]) == 0
        self.__event_handler_code_fragment_list[EventName] = [TheCodeFragment]

    def has_base_mode(self):
        return len(self.__base_mode_sequence) != 1

    def has_code_fragment_list(self, EventName):
        assert self.__event_handler_code_fragment_list.has_key(EventName)
        return len(self.__event_handler_code_fragment_list[EventName]) != 0

    def get_base_mode_sequence(self):
        return self.__base_mode_sequence

    def get_base_mode_name_list(self):
        return map(lambda mode: mode.name, self.__base_mode_sequence)

    def get_code_fragment_list(self, EventName):
        assert self.__event_handler_code_fragment_list.has_key(EventName)
        return self.__event_handler_code_fragment_list[EventName]

    def get_pattern_action_pair_list(self):
        return self.__pattern_action_pair_list

    def get_indentation_counter_terminal_index(self):
        """Under some circumstances a terminal code need to jump to the indentation
           counter directly. Thus, it must be known in what terminal it is actually 
           located.

            RETURNS: None, if no indentation counter is involved.
                     > 0,  terminal id of the terminal that contains the indentation
                           counter.
        """
        for info in self.__pattern_action_pair_list:
            action = info.action()
            if   action.__class__.__name__ != "GeneratedCode": continue
            elif action.function != indentation_counter.do:    continue
            return info.pattern().sm.get_id()
        return None

    def get_documentation(self):
        L = max(map(lambda mode: len(mode.name), self.__base_mode_sequence))
        txt  = "\nMODE: %s\n" % self.name

        txt += "\n"
        if len(self.__base_mode_sequence) != 1:
            txt += "    BASE MODE SEQUENCE:\n"
            base_mode_name_list = map(lambda mode: mode.name, self.__base_mode_sequence[:-1])
            base_mode_name_list.reverse()
            for name in base_mode_name_list:
                txt += "      %s\n" % name
            txt += "\n"

        if len(self.__history_deletion) != 0:
            txt += "    DELETION ACTIONS:\n"
            for entry in self.__history_deletion:
                txt += "      %s:  %s%s  (from mode %s)\n" % \
                       (entry[0], " " * (L - len(self.name)), entry[1], entry[2])
            txt += "\n"

        if len(self.__history_repriorization) != 0:
            txt += "    PRIORITY-MARK ACTIONS:\n"
            self.__history_repriorization.sort(lambda x, y: cmp(x[4], y[4]))
            for entry in self.__history_repriorization:
                txt += "      %s: %s%s  (from mode %s)  (%i) --> (%i)\n" % \
                       (entry[0], " " * (L - len(self.name)), entry[1], entry[2], entry[3], entry[4])
            txt += "\n"

        if len(self.__pattern_action_pair_list) != 0:
            txt += "    PATTERN-ACTION PAIRS:\n"
            self.__pattern_action_pair_list.sort(lambda x, y:
                            cmp(x.pattern().sm.get_id(),
                                y.pattern().sm.get_id()))
            for pattern_action_pair in self.__pattern_action_pair_list:
                txt += "      (%3i) %s: %s%s\n" % \
                       (pattern_action_pair.pattern().sm.get_id(),
                        pattern_action_pair.mode_name, " " * (L - len(self.name)), 
                        pattern_action_pair.pattern_string())
            txt += "\n"

        return txt

    def default_indentation_handler_sufficient(Mode):
        """If no user defined indentation handler is defined, then the 
           default token handler is sufficient.
        """
        return     not Mode.has_code_fragment_list("on_indentation_error") \
               and not Mode.has_code_fragment_list("on_indentation_bad")   \
               and not Mode.has_code_fragment_list("on_indent")            \
               and not Mode.has_code_fragment_list("on_dedent")            \
               and not Mode.has_code_fragment_list("on_nodent") 
           
    def __determine_base_mode_sequence(self, ModeDescr, InheritancePath):
        """Determine the sequence of base modes. The type of sequencing determines
           also the pattern precedence. The 'deep first' scheme is chosen here. For
           example a mode hierarchie of

                                       A
                                     /   \ 
                                    B     C
                                   / \   / \
                                  D  E  F   G

           results in a sequence: (A, B, D, E, C, F, G).reverse()

           This means, that patterns and event handlers of 'E' have precedence over
           'C' because they are the childs of a preceding base mode.

           This function detects circular inheritance.
        """
        if ModeDescr.name in InheritancePath:
            msg = "mode '%s'\n" % InheritancePath[0]
            for mode_name in InheritancePath[InheritancePath.index(ModeDescr.name) + 1:]:
                msg += "   inherits mode '%s'\n" % mode_name
            msg += "   inherits mode '%s'" % ModeDescr.name

            error_msg("circular inheritance detected:\n" + msg, ModeDescr.filename, ModeDescr.line_n)

        base_mode_name_list_reversed = deepcopy(ModeDescr.base_modes)
        #base_mode_name_list_reversed.reverse()
        for name in base_mode_name_list_reversed:
            # -- does mode exist?
            verify_word_in_list(name, mode_description_db.keys(),
                                "Mode '%s' inherits mode '%s' which does not exist." % (ModeDescr.name, name),
                                ModeDescr.filename, ModeDescr.line_n)

            if name in map(lambda m: m.name, self.__base_mode_sequence): continue

            # -- grab the mode description
            mode_descr = mode_description_db[name]
            self.__determine_base_mode_sequence(mode_descr, InheritancePath + [ModeDescr.name])

        self.__base_mode_sequence.append(ModeDescr)

        return self.__base_mode_sequence

    def __collect_event_handler(self):
        """Collect event handlers from base mode and the current mode.
           Event handlers of the most 'base' mode come first, then the 
           derived event handlers. 

           See '__determine_base_mode_sequence(...) for details about the line-up.
        """
        for event_name in event_handler_db.keys():
            self.__event_handler_code_fragment_list[event_name] = []

        for mode_descr in self.__base_mode_sequence:
            
            for event_name in event_handler_db.keys():
                fragment = mode_descr.events[event_name]
                if fragment is not None and fragment.get_code() != "":
                    self.__event_handler_code_fragment_list[event_name].append(fragment)

        return 

    def __collect_pattern_action_pairs(self):
        """Collect patterns of all inherited modes. Patterns are like virtual functions
           in C++ or other object oriented programming languages. Also, the patterns of the
           uppest mode has the highest priority, i.e. comes first.
        """
        def __ensure_pattern_indeces_follow_precedence(MatchList, RepriorizationDB, PrevMaxPatternIndex):
            """When a derived mode is defined before its base mode, then its pattern ids
               (according to the time they were created) are lower than thos of the base
               mode. This would imply that they have higher precedence, which is against
               our matching rules. Here, pattern ids are adapted to be higher than a certain
               minimum, and follow the same precedence sequence.
            """
            # Patterns of a 'lower precedence mode' **must** have higher pattern ids
            # that patterns of a 'higher precedence mode'. This is to ensure that 
            # base mode patterns precede derived mode patterns.
            min_pattern_index = min(map(lambda match: match.pattern().sm.get_id(),
                                        MatchList))
            if min_pattern_index > PrevMaxPatternIndex:
                return MatchList, RepriorizationDB

            match_list        = deepcopy(MatchList)
            repriorization_db = deepcopy(RepriorizationDB)

            # Determine the offset for each pattern
            offset = PrevMaxPatternIndex + 1 - min_pattern_index
            assert offset >= 1

            # Assign new pattern ids starting from MinPatternID
            for match in match_list:
                current_pattern_id = match.pattern().sm.get_id()
                match.pattern().sm.set_id(current_pattern_id + offset)
            
            # The reprioritizations must also be adapted
            ## for key, info in repriorization_db.items():
            ##    print "##reprio:", key, info[-1], info[-1] + offset
            for info in repriorization_db.items():
                info[-1] += offset

            return match_list, repriorization_db 
                                             
        def __handle_deletion_and_repriorization(CurrentModeName, pattern_action_pair_list, 
                                                 repriorization_db, deletion_db):
            def __validate_marks(DB, DoneDB, CommentStr):
                ok_f = True
                for pattern, info in DB.items():
                    if DoneDB.has_key(pattern): continue
                    ok_f = False
                    error_msg("Pattern '%s' was marked %s but does not\n" % (pattern, CommentStr) + \
                              "exist in any base mode of mode '%s'." % self.name,
                              info[1], info[2], DontExitF=True, WarningF=False)
                return ok_f

            def __is_in_patterns(AllegedIdenticalSM, MyDB):
                for pattern_str, info in MyDB.items():
                    pattern = info[0]
                    if identity_checker.do(AllegedIdenticalSM, pattern): return pattern_str
                return ""

            # DELETION / PRIORITY-MARK 
            deletion_done_db       = {}
            repriorization_done_db = {}
            i    = 0
            size = len(pattern_action_pair_list)
            while i < size:
                match   = pattern_action_pair_list[i]
                pattern = match.pattern()

                found_pattern = __is_in_patterns(pattern, deletion_db)
                if found_pattern != "":
                    # Delete pattern from the list of pattern action pairs
                    del pattern_action_pair_list[i]
                    size -= 1
                    # Mark 'deletion applied'
                    deletion_done_db[found_pattern] = True
                    self.__history_deletion.append([CurrentModeName, match.pattern, match.mode_name])
                    continue

                found_pattern = __is_in_patterns(pattern, repriorization_db)
                if found_pattern != "":
                    # Adapt the pattern index, this automatically adapts the match precedence
                    old_pattern_id = pattern.sm.get_id()
                    new_pattern_id = repriorization_db[found_pattern][-1]
                    new_match = deepcopy(match)
                    new_match.pattern().sm.set_id(new_pattern_id)
                    pattern_action_pair_list[i] = new_match
                    # Mark 'repriorization applied'
                    repriorization_done_db[found_pattern] = True
                    self.__history_repriorization.append([CurrentModeName, match.pattern, match.mode_name,
                                                          old_pattern_id, new_pattern_id]) 
                i += 1

            # Ensure that all mentioned marks really had some effect.
            if    not __validate_marks(deletion_db, deletion_done_db, "for DELETION")  \
               or not __validate_marks(repriorization_db, repriorization_done_db, "with PRIORITY-MARK"):
                error_msg("Abort.")
            return

        def __add_new_pattern_action_pair(pattern_action_pair_list, PatternActionPair):
            # Shallow copy is enough! Later on, there might be actions that 
            # generate source code, and then the source code takes the place of
            # the action. For this to work, inherited actions must be de-antangled.
            pattern_action_pair_list.append(copy(PatternActionPair))

        result                 = []
        prev_max_pattern_index = -1
        # Iterate from the base to the top (include this mode's pattern)
        for mode_descr in self.__base_mode_sequence:

            repriorization_db = {}
            consider_pattern_action_pairs_f = mode_descr.has_own_matches()
            if consider_pattern_action_pairs_f:
                match_list, repriorization_db = \
                        __ensure_pattern_indeces_follow_precedence(mode_descr.get_match_list(),
                                                                   mode_descr.get_repriorization_db(),
                                                                   prev_max_pattern_index)

            # Delete/Repriorize patterns from more basic modes
            __handle_deletion_and_repriorization(mode_descr.name, result, 
                                                 repriorization_db, mode_descr.get_deletion_db())

            if consider_pattern_action_pairs_f:
                # Add the new pattern action pairs
                for pattern_action_pair in match_list:
                    __add_new_pattern_action_pair(result, pattern_action_pair)

                # Determine the max pattern index at this level of inheritance
                prev_max_pattern_index = max([prev_max_pattern_index] + \
                                             map(lambda match: match.pattern().sm.get_id(),
                                             match_list))


        return result

    def __collect_options(self):
        for mode in self.__base_mode_sequence[:-1]:
            for name, option_descr in mode_option_info_db.items():
                if option_descr.type != "list": continue
                # Need to decouple by means of 'deepcopy'
                self.options.setdefault(name, []).extend(mode.options[name])

mode_option_info_db = {
   # -- a mode can be inheritable or not or only inheritable. if a mode
   #    is only inheritable it is not printed on its on, only as a base
   #    mode for another mode. default is 'yes'
   "inheritable":       OptionInfo("single", ["no", "yes", "only"], Default="yes"),
   # -- a mode can restrict the possible modes to exit to. this for the
   #    sake of clarity. if no exit is explicitly mentioned all modes are
   #    possible. if it is tried to transit to a mode which is not in
   #    the list of explicitly stated exits, an error occurs.
   #    entrys work respectively.
   "exit":              OptionInfo("list", Default=[]),
   "entry":             OptionInfo("list", Default=[]),
   # -- a mode can restrict the exits and entrys explicitly mentioned
   #    then, a derived mode cannot add now exits or entrys
   "restrict":          OptionInfo("list", ["exit", "entry"], Default=[]),
   # -- a mode can have 'skippers' that effectivels skip ranges that are out of interest.
   "skip":              OptionInfo("list", Default=[]), # "multiple: RE-character-set
   "skip_range":        OptionInfo("list", Default=[]), # "multiple: RE-character-string RE-character-string
   "skip_nested_range": OptionInfo("list", Default=[]), # "multiple: RE-character-string RE-character-string
   # -- indentation setup information
   "indentation":       OptionInfo("single", Default=None),
}

event_handler_db = {
    "on_entry":                  "On entry of a mode.",
    "on_exit":                   "On exit of a mode.", 
    "on_indent":                 "On opening indentation.",
    "on_nodent":                 "On same indentation.",
    "on_dedent":                 "On closing indentation'.",
    "on_n_dedent":               "On closing indentation'.",
    "on_indentation_error":      "Closing indentation on non-border.",
    "on_indentation_bad":        "On bad character in indentation.",
    "on_indentation":            "General Indentation Handler.",
    "on_match":                  "On each match (before pattern action).",
    "on_after_match":            "On each match (after pattern action).",
    "on_failure":                "In case that no pattern matches.",
    "on_skip_range_open":        "On missing skip range delimiter.",
    "on_end_of_stream":          "On end of file/stream.",
}

def parse(fh):
    """This function parses a mode description and enters it into the 
       'mode_description_db'. Once all modes are parsed
       they can be translated into 'real' modes and are located in
       'blackboard.mode_db'. 
    """

    # NOTE: Catching of EOF happens in caller: parse_section(...)
    skip_whitespace(fh)
    mode_name = read_identifier(fh)
    if mode_name == "":
        error_msg("missing identifier at beginning of mode definition.", fh)

    # NOTE: constructor does register this mode in the mode_db
    new_mode  = ModeDescription(mode_name, fh.name, get_current_line_info_number(fh))

    # (*) inherited modes / options
    skip_whitespace(fh)
    dummy = fh.read(1)
    if dummy not in [":", "{"]:
        error_msg("missing ':' or '{' after mode '%s'" % mode_name, fh)

    if dummy == ":":
        __parse_option_list(new_mode, fh)

    # (*) read in pattern-action pairs and events
    while __parse_element(new_mode, fh): 
        pass

    # (*) check for modes w/o pattern definitions
    if not new_mode.has_event_handler() and not new_mode.has_own_matches():
        if new_mode.options["inheritable"] != "only":
            new_mode.options["inheritable"] = "only"
            error_msg("Mode without pattern and event handlers needs to be 'inheritable only'.\n" + \
                      "<inheritable: only> has been added automatically.", fh,  DontExitF=True)

def finalize():
    """After all modes have been defined, the mode descriptions can now
       be translated into 'real' modes.
    """
    global mode_description_db

    # (*) Translate each mode description int a 'real' mode
    for name, mode_descr in mode_description_db.iteritems():
        blackboard.mode_db[name] = Mode(mode_descr)

    # (*) perform consistency check 
    consistency_check.do(blackboard.mode_db)

def __parse_option_list(new_mode, fh):
    position = fh.tell()
    try:  
        # ':' => inherited modes/options follow
        skip_whitespace(fh)

        __parse_base_mode_list(fh, new_mode)
        
        while __parse_option(fh, new_mode):
            pass

    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while parsing options of mode '%s'." % new_mode.name, fh)

def __parse_base_mode_list(fh, new_mode):
    new_mode.base_modes = []
    trailing_comma_f    = False
    while 1 + 1 == 2:
        if   check(fh, "{"): fh.seek(-1, 1); break
        elif check(fh, "<"): fh.seek(-1, 1); break

        skip_whitespace(fh)
        identifier = read_identifier(fh)
        if identifier == "": break

        new_mode.base_modes.append(identifier)
        trailing_comma_f = False
        if not check(fh, ","): break
        trailing_comma_f = True


    if trailing_comma_f:
        error_msg("Trailing ',' after base mode '%s'." % new_mode.base_modes[-1], fh, 
                  DontExitF=True, WarningF=True)
        
    elif len(new_mode.base_modes) != 0:
        # This check is a 'service' -- for those who follow the old convention
        pos = fh.tell()
        skip_whitespace(fh)
        dummy_identifier = read_identifier(fh)
        if dummy_identifier != "":
            error_msg("Missing separating ',' between base modes '%s' and '%s'.\n" \
                      % (new_mode.base_modes[-1], dummy_identifier) + \
                      "(The comma separator is mandatory since quex 0.53.1)", fh)
        fh.seek(pos)

def __parse_string(fh, Name):
    pos = fh.tell()
    if fh.read(1) != "\"":
        pos = fh.tell()
        msg = fh.read(5)
        fh.seek(pos)
        error_msg("%s can\n" % Name + 
                  "only be a string and must start with a quote like \".\n" +
                  "Found '%s'" % msg, fh)

    sequence = snap_character_string.get_character_code_sequence(fh)
    end_pos  = fh.tell()
    fh.seek(pos)
    msg      = fh.read(end_pos - pos)
    return msg, sequence

def __parse_option(fh, new_mode):
    def get_pattern_object(SM):
        if not SM.is_DFA_compliant(): result = nfa_to_dfa.do(SM)
        else:                         result = SM
        result = hopcroft.do(result, CreateNewStateMachineF=False)
        return Pattern(result, AllowStateMachineTrafoF=True)

    identifier = read_option_start(fh)
    if identifier is None: return False

    verify_word_in_list(identifier, mode_option_info_db.keys(),
                        "mode option", fh.name, get_current_line_info_number(fh))

    if identifier == "skip":
        # A skipper 'eats' characters at the beginning of a pattern that belong
        # to a specified set of characters. A useful application is most probably
        # the whitespace skipper '[ \t\n]'. The skipper definition allows quex to
        # implement a very effective way to skip these regions.
        pattern_str, trigger_set = regular_expression.parse_character_set(fh, PatternStringF=True)
        skip_whitespace(fh)

        if fh.read(1) != ">":
            error_msg("missing closing '>' for mode option '%s'." % identifier, fh)

        if trigger_set.is_empty():
            error_msg("Empty trigger set for skipper." % identifier, fh)

        # TriggerSet skipping is implemented the following way: As soon as one element of the 
        # trigger set appears, the state machine enters the 'trigger set skipper section'.
        # Enter the skipper as if the opener pattern was a normal pattern and the 'skipper' is the action.
        # NOTE: The correspondent CodeFragment for skipping is created in 'implement_skippers(...)'
        pattern_sm  = StateMachine()
        pattern_sm.add_transition(pattern_sm.init_state_index, trigger_set, AcceptanceF=True)

        # Skipper code is to be generated later
        action = GeneratedCode(skip_character_set.do, 
                               FileName = fh.name, 
                               LineN    = get_current_line_info_number(fh))
        action.data["character_set"] = trigger_set

        new_mode.add_match(pattern_str, action, get_pattern_object(pattern_sm), 
                           Comment=E_SpecialPatterns.SKIP)

        return True

    elif identifier in ["skip_range", "skip_nested_range"]:
        # A non-nesting skipper can contain a full fledged regular expression as opener,
        # since it only effects the trigger. Not so the nested range skipper-see below.

        # -- opener
        skip_whitespace(fh)
        if identifier == "skip_nested_range":
            # Nested range state machines only accept 'strings' not state machines
            opener_str, opener_sequence = __parse_string(fh, "Opener pattern for 'skip_nested_range'")
            opener_sm = StateMachine.from_sequence(opener_sequence)
        else:
            opener_str, opener_pattern = regular_expression.parse(fh)
            opener_sm = opener_pattern.sm
            # For 'range skipping' the opener sequence is not needed, only the opener state
            # machine is webbed into the pattern matching state machine.
            opener_sequence       = None

        skip_whitespace(fh)

        # -- closer
        closer_str, closer_sequence = __parse_string(fh, "Closing pattern for 'skip_range' or 'skip_nested_range'")
        skip_whitespace(fh)
        if fh.read(1) != ">":
            error_msg("missing closing '>' for mode option '%s'" % identifier, fh)

        # Skipper code is to be generated later
        generator_function, comment = { 
                "skip_range":        (skip_range.do,        E_SpecialPatterns.SKIP_RANGE),
                "skip_nested_range": (skip_nested_range.do, E_SpecialPatterns.SKIP_NESTED_RANGE),
        }[identifier]
        action = GeneratedCode(generator_function,
                               FileName = fh.name, 
                               LineN    = get_current_line_info_number(fh))

        action.data["opener_sequence"] = opener_sequence
        action.data["closer_sequence"] = closer_sequence
        action.data["mode_name"]       = new_mode.name

        new_mode.add_match(opener_str, action, get_pattern_object(opener_sm), Comment=comment)

        return True
        
    elif identifier == "indentation":
        value = indentation_setup.do(fh)

        # Enter 'Newline' and 'Suppressed Newline' as matches into the engine.
        # Similar to skippers, the indentation count is then triggered by the newline.
        # -- Suppressed Newline = Suppressor followed by Newline,
        #    then newline does not trigger indentation counting.
        suppressed_newline_pattern_str = ""
        if value.newline_suppressor_state_machine.get() is not None:
            suppressed_newline_pattern_str = \
                  "(" + value.newline_suppressor_state_machine.pattern_string() + ")" \
                + "(" + value.newline_state_machine.pattern_string() + ")"
                                           
            suppressed_newline_sm = \
                sequentialize.do([value.newline_suppressor_state_machine.get(),
                                  value.newline_state_machine.get()])
                 
            FileName = value.newline_suppressor_state_machine.file_name
            LineN    = value.newline_suppressor_state_machine.line_n
            # Go back to start.
            code = UserCodeFragment("goto %s;" % get_label("$start", U=True), FileName, LineN)

            new_mode.add_match(suppressed_newline_pattern_str, code, 
                               get_pattern_object(suppressed_newline_sm),
                               Comment=E_SpecialPatterns.SUPPRESSED_INDENTATION_NEWLINE)

        # When there is an empty line, then there shall be no indentation count on it.
        # Here comes the trick: 
        #
        #      Let               newline         
        #      be defined as:    newline ([space]* newline])*
        # 
        # This way empty lines are eating away before the indentation count is activated.

        # -- 'space'
        x0 = StateMachine()
        x0.add_transition(x0.init_state_index, value.indentation_count_character_set(), 
                          AcceptanceF=True)
        # -- '[space]*'
        x1 = repeat.do(x0)
        # -- '[space]* newline'
        x2 = sequentialize.do([x1, value.newline_state_machine.get()])
        # -- '([space]* newline)*'
        x3 = repeat.do(x2)
        # -- 'newline ([space]* newline)*'
        x4 = sequentialize.do([value.newline_state_machine.get(), x3])
        # -- nfa to dfa; hopcroft optimization
        sm = beautifier.do(x4)

        FileName = value.newline_state_machine.file_name
        LineN    = value.newline_state_machine.line_n
        action   = GeneratedCode(indentation_counter.do, FileName, LineN)

        action.data["indentation_setup"] = value

        new_mode.add_match(value.newline_state_machine.pattern_string(), action, 
                           get_pattern_object(sm), 
                           Comment=E_SpecialPatterns.INDENTATION_NEWLINE)

        # Announce the mode to which the setup belongs
        value.set_containing_mode_name(new_mode.name)
    else:
        value = read_option_value(fh)

    # The 'verify_word_in_list()' call must have ensured that the following holds
    assert mode_option_info_db.has_key(identifier)

    # Is the option of the appropriate value?
    option_info = mode_option_info_db[identifier]
    if option_info.domain is not None and value not in option_info.domain:
        error_msg("Tried to set value '%s' for option '%s'. " % (value, identifier) + \
                  "Though, possible for this option are only: %s." % repr(option_info.domain)[1:-1], fh)

    # Finally, set the option
    new_mode.add_option(identifier, value)

    return True

def __parse_element(new_mode, fh):
    """Returns: False, if a closing '}' has been found.
                True, else.
    """
    position = fh.tell()
    try:
        description = "Pattern or event handler name.\n" + \
                      "Missing closing '}' for end of mode"

        skip_whitespace(fh)
        # NOTE: Do not use 'read_word' since we need to continue directly after
        #       whitespace, if a regular expression is to be parsed.
        position = fh.tell()

        word = read_until_whitespace(fh)
        if word == "}": return False

        # -- check for 'on_entry', 'on_exit', ...
        if __parse_event(new_mode, fh, word): return True

        fh.seek(position)
        description = "Start of mode element: regular expression"
        pattern_str, pattern = regular_expression.parse(fh)

        if new_mode.has_pattern(pattern_str):
            previous = new_mode.get_pattern_action_pair(pattern_str)
            error_msg("Pattern has been defined twice.", fh, DontExitF=True)
            error_msg("First defined here.", 
                      previous.action().filename, previous.action().line_n)

        position    = fh.tell()
        description = "Start of mode element: code fragment for '%s'" % pattern_str

        __parse_action(new_mode, fh, pattern_str, pattern)

    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while parsing %s." % description, fh)

    return True

def __parse_action(new_mode, fh, pattern_str, pattern):

    position = fh.tell()
    try:
        skip_whitespace(fh)
        position = fh.tell()
            
        code_obj = code_fragment.parse(fh, "regular expression", ErrorOnFailureF=False) 
        if code_obj is not None:
            new_mode.add_match(pattern_str, code_obj, pattern)
            return

        fh.seek(position)
        word = read_until_letter(fh, [";"])
        if word == "PRIORITY-MARK":
            # This mark 'lowers' the priority of a pattern to the priority of the current
            # pattern index (important for inherited patterns, that have higher precedence).
            # The parser already constructed a state machine for the pattern that is to
            # be assigned a new priority. Since, this machine is not used, let us just
            # use its id.
            fh.seek(-1, 1)
            check_or_die(fh, ";", ". Since quex version 0.33.5 this is required.")
            new_mode.add_match_priority(pattern_str, pattern, pattern.sm.get_id(), 
                                        fh.name, get_current_line_info_number(fh))

        elif word == "DELETION":
            # This mark deletes any pattern that was inherited with the same 'name'
            fh.seek(-1, 1)
            check_or_die(fh, ";", ". Since quex version 0.33.5 this is required.")
            new_mode.add_match_deletion(pattern_str, pattern, fh.name, get_current_line_info_number(fh))
            
        else:
            error_msg("Missing token '{', 'PRIORITY-MARK', 'DELETION', or '=>' after '%s'.\n" % pattern_str + \
                      "found: '%s'. Note, that since quex version 0.33.5 it is required to add a ';'\n" % word + \
                      "to the commands PRIORITY-MARK and DELETION.", fh)


    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while parsing action code for pattern.", fh)

def __parse_event(new_mode, fh, word):
    pos = fh.tell()

    # Allow '<<EOF>>' and '<<FAIL>>' out of respect for classical tools like 'lex'
    if   word == "<<EOF>>":                  word = "on_end_of_stream"
    elif word == "<<FAIL>>":                 word = "on_failure"
    elif word in blackboard.all_section_title_list:
        error_msg("Pattern '%s' is a quex section title. Has the closing '}' of mode %s \n" % (word, new_mode.name) \
                  + "been forgotten? Else use quotes, i.e. \"%s\"." % word, fh)
    elif len(word) < 3 or word[:3] != "on_": return False

    comment = "Unknown event handler '%s'. \n" % word + \
              "Note, that any pattern starting with 'on_' is considered an event handler.\n" + \
              "use double quotes to bracket patterns that start with 'on_'."

    __general_validate(fh, new_mode, word, pos)
    verify_word_in_list(word, event_handler_db.keys(), comment, fh)
    __validate_required_token_policy_queue(word, fh, pos)

    continue_f = True
    if word == "on_end_of_stream":
        # When a termination token is sent, no other token shall follow. 
        # => Enforce return from the analyzer! Do not allow CONTINUE!
        continue_f = False

    new_mode.events[word] = code_fragment.parse(fh, "%s::%s event handler" % (new_mode.name, word),
                                                ContinueF=continue_f)

    return True

def __general_validate(fh, Mode, Name, pos):
    if Name == "on_indentation":
        fh.seek(pos)
        error_msg("Definition of 'on_indentation' is no longer supported since version 0.51.1.\n"
                  "Please, use 'on_indent' for the event of an opening indentation, 'on_dedent'\n"
                  "for closing indentation, and 'on_nodent' for no change in indentation.", fh) 


    def error_dedent_and_ndedent(code, A, B):
        filename = "(unknown)"
        line_n   = "0"
        if hasattr(code, "filename"): filename = code.filename
        if hasattr(code, "line_n"):   line_n   = code.line_n
        error_msg("Indentation event handler '%s' cannot be defined, because\n" % A,
                  fh, DontExitF=True, WarningF=False)
        error_msg("the alternative '%s' has already been defined." % B,
                  filename, line_n)

    if Name == "on_dedent" and Mode.events.has_key("on_n_dedent"):
        fh.seek(pos)
        code = Mode.events["on_n_dedent"]
        if code.get_code() != "":
            error_dedent_and_ndedent(code, "on_dedent", "on_n_dedent")
                      
    if Name == "on_n_dedent" and Mode.events.has_key("on_dedent"):
        fh.seek(pos)
        code = Mode.events["on_dedent"]
        if code.get_code() != "":
            error_dedent_and_ndedent(code, "on_n_dedent", "on_dedent")
                      
def __validate_required_token_policy_queue(Name, fh, pos):
    """Some handlers are better only used with token policy 'queue'."""

    if Name not in ["on_entry", "on_exit", 
                    "on_indent", "on_n_dedent", "on_dedent", "on_nodent", 
                    "on_indentation_bad", "on_indentation_error", 
                    "on_indentation"]: 
        return
    if Setup.token_policy == "queue":
        return
    if Setup.warning_disabled_no_token_queue_f:
        return

    fh.seek(pos)
    error_msg("Using '%s' event handler, while the token queue is disabled.\n" % Name + \
              "Use '--token-policy queue', so then tokens can be sent safer\n" + \
              "from inside this event handler. Disable this warning by command\n"
              "line option '--no-warning-on-no-token-queue'.", fh, DontExitF=True) 
