################################################################################
#! /usr/bin/env python
# Quex is  free software;  you can  redistribute it and/or  modify it  under the
# terms  of the  GNU Lesser  General  Public License  as published  by the  Free
# Software Foundation;  either version 2.1 of  the License, or  (at your option)
# any later version.
# 
# This software is  distributed in the hope that it will  be useful, but WITHOUT
# ANY WARRANTY; without even the  implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the  GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License along
# with this  library; if not,  write to the  Free Software Foundation,  Inc., 59
# Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# (C) 2006, 2007 Frank-Rene Schaefer
#
################################################################################

################################################################################
# IMPORTANT: This file shall be import-able by any 'normal' module of Quex.    #
#            For this, it was a designated design goal to make sure that the   #
#            imports are 'flat' and only cause environment or outer modules.   #
################################################################################
from quex.engine.generator.code_fragment_base import CodeFragment
from quex.engine.misc.enum                    import Enum
from quex.engine.interval_handling            import NumberSet, Interval
from quex.input.setup                         import QuexSetup, SETUP_INFO
from copy                                     import deepcopy

#-----------------------------------------------------------------------------------------
# setup: All information of the user's desired setup.
#-----------------------------------------------------------------------------------------
setup = QuexSetup(SETUP_INFO)

#-----------------------------------------------------------------------------------------
# StateIndices: Values to be used as target states for transitions
#-----------------------------------------------------------------------------------------
E_StateIndices = Enum("DROP_OUT", 
                      "RELOAD_PROCEDURE",
                      "INIT_STATE_TRANSITION_BLOCK",
                      "END_OF_PRE_CONTEXT_CHECK",
                      "RECURSIVE",
                      "ALL", 
                      "ANALYZER_REENTRY", 
                      "NONE", 
                      "VOID") 

E_PreContextIDs  = Enum("NONE",    
                        "BEGIN_OF_LINE", 
                        "_DEBUG_NAME_PreContextIDs")

E_AcceptanceIDs  = Enum("FAILURE", 
                        "PRE_CONTEXT_FULFILLED", 
                        "TERMINAL_PRE_CONTEXT_CHECK", 
                        "TERMINAL_BACKWARD_INPUT_POSITION", 
                        "VOID", 
                        "_DEBUG_NAME_E_AcceptanceIDs")

E_PostContextIDs = Enum("NONE", 
                        "IRRELEVANT",
                        "_DEBUG_NAME_E_PostContextIDs")

E_EngineTypes    = Enum("FORWARD", 
                        "BACKWARD_PRE_CONTEXT", 
                        "BACKWARD_INPUT_POSITION",
                        "INDENTATION_COUNTER",
                        "ELSE",                       # skipper, or whatever ...
                        "_DEBUG_E_EngineTypes")

E_TransitionN = Enum("VOID", 
                     "LEXEME_START_PLUS_ONE",
                     "IRRELEVANT",
                     "_DEBUG_NAME_TransitionNs")

E_InputActions = Enum("DEREF", 
                      "INCREMENT", 
                      "INCREMENT_THEN_DEREF", 
                      "DECREMENT",
                      "DECREMENT_THEN_DEREF",
                      "_DEBUG_InputActions")

E_Compression = Enum("PATH", 
                     "PATH_UNIFORM",
                     "TEMPLATE",
                     "TEMPLATE_UNIFORM",
                     "_DEBUG_Compression")

E_Count = Enum("VIRGIN", 
               "VOID",
               "_DEBUG_Count")

E_Commonality = Enum("NONE", "BOTH", "A_IN_B", "B_IN_A")

E_SpecialPatterns = Enum("INDENTATION_NEWLINE", 
                         "SUPPRESSED_INDENTATION_NEWLINE",
                         "SKIP", 
                         "SKIP_RANGE", 
                         "SKIP_NESTED_RANGE", 
                         "_DEBUG_PatternNames")

#-----------------------------------------------------------------------------------------
# mode_db: storing the mode information into a dictionary:
#            key  = mode name
#            item = Mode object
#-----------------------------------------------------------------------------------------
mode_db = {}


#-----------------------------------------------------------------------------------------
# Counter Settings (Default)
# 
Default_NewlineCharDB = {
    1: NumberSet([Interval(0x0A),     # Line Feed 
                  Interval(0x0B),     # Vertical Tab 
                  Interval(0x0C),     # Form Feed 
                  #        0x0D       --> set to '0' newlines, see below
                  Interval(0x85),     # Next Line 
                  Interval(0x2028),   # Line Separator 
                  Interval(0x2029)]), # Paragraph Separator 
    0: NumberSet(Interval(0x0D)),     # Carriage Return
    #                                 # DOS/Windows: 0x0D, 0x0A --> 1 newline
}
Default_GridCharDB = {
    4: NumberSet(ord('\t'))           # Tabulator: Grid of 4 columns
}
Default_SpecialCharDB = { 
    #                                 # Special character sizes are font dependent. 
    #                                 # No assumptions made by default.
}

class CounterDB:
    # (*) Databases:
    # 
    #     Characters may trigger different increments to line numbers and column numbers.
    #     Information about this behavior is stored in the following databases.
    newline = Default_NewlineCharDB  # map: delta line_n   <--> character set
    grid    = Default_GridCharDB     # map: grid  column_n <--> character set
    #                                #      This implements grids for tabulators. The increment
    #                                #      depends on the current value of column_n
    special = Default_SpecialCharDB  # map: delta column_n <--> character set
    #                                #      Normally, a character increments the column by '1'
    #                                #      This database allows to assign different values for 
    #                                #      special characters.
    __enabled_f = True

    @staticmethod
    def reset():
        """Reset the database to default values."""
        CounterDB.newline     = Default_NewlineCharDB
        CounterDB.grid        = Default_GridCharDB
        CounterDB.special     = Default_SpecialCharDB
        CounterDB.__enabled_f = True

    @staticmethod
    def assert_consistency():
        """Character sets associated with countings cannot overlap. A character
           may very well appear as a line counter and a grid, for example. But,
           it should not appear twice in the same database.
        """
        def do(DB):
            combined = NumberSet()
            for number_set in DB.itervalues():
                assert not number_set.has_intersection(combined)
                combined.unite_with(number_set)
        do(CounterDB.newline)
        do(CounterDB.grid)
        do(CounterDB.special)

    @staticmethod
    def disable():
        """Disable character and line counting. Must be enabled with .reset()"""
        CounterDB.__enabled_f = False

    @staticmethod
    def is_enabled(): 
        return CounterDB.__enabled_f


#-----------------------------------------------------------------------------------------
# initial_mode: mode in which the lexcial analyser shall start
#-----------------------------------------------------------------------------------------
initial_mode = CodeFragment()

#-----------------------------------------------------------------------------------------
# header: code fragment that is to be pasted before mode transitions
#         and pattern action pairs (e.g. '#include<something>'
#-----------------------------------------------------------------------------------------
header = CodeFragment()

#-----------------------------------------------------------------------------------------
# class_body_extension: code fragment that is to be pasted inside the class definition
#                       of the lexical analyser class.
#-----------------------------------------------------------------------------------------
class_body_extension = CodeFragment()

#-----------------------------------------------------------------------------------------
# class_constructor_extension: code fragment that is to be pasted inside the lexer class constructor
#-----------------------------------------------------------------------------------------
class_constructor_extension = CodeFragment()

#-----------------------------------------------------------------------------------------
# memento_extension: fragment to be pasted into the memento  class's body.
#-----------------------------------------------------------------------------------------
memento_class_extension = CodeFragment()
#-----------------------------------------------------------------------------------------
# memento_pack_extension: fragment to be pasted into the function that packs the
#                         lexical analyzer state in a memento.
#-----------------------------------------------------------------------------------------
memento_pack_extension = CodeFragment()
#-----------------------------------------------------------------------------------------
# memento_unpack_extension: fragment to be pasted into the function that unpacks the
#                           lexical analyzer state in a memento.
#-----------------------------------------------------------------------------------------
memento_unpack_extension = CodeFragment()

fragment_db = {
        "header":         "header",
        "body":           "class_body_extension",
        "init":           "class_constructor_extension",
        "memento":        "memento_class_extension",
        "memento_pack":   "memento_pack_extension",
        "memento_unpack": "memento_unpack_extension",
}

all_section_title_list = ["start", "define", "token", "mode", "repeated_token", "token_type" ] + fragment_db.keys()

class PatternShorthand:
    def __init__(self, Name="", StateMachine="", Filename="", LineN=-1, RE=""):
        assert StateMachine.__class__.__name__ == "StateMachine"

        self.name               = Name
        self.__state_machine    = StateMachine
        self.filename           = Filename
        self.line_n             = LineN
        self.regular_expression = RE

    def get_state_machine(self):
        return self.__state_machine.clone()

    def get_character_set(self):
        if len(self.__state_machine.states) != 2: return None
        t = self.__state_machine.states[self.__state_machine.init_state_index].transitions()
        db = t.get_map()
        if len(db) != 1: return None
        return deepcopy(db[db.keys()[0]])

#-----------------------------------------------------------------------------------------
# shorthand_db: user defined names for regular expressions.
#-----------------------------------------------------------------------------------------
shorthand_db = {}

#-----------------------------------------------------------------------------------------
# token_id_db: list of all defined token-ids together with the file position
#              where they are defined. See token_ide_maker, class TokenInfo.
#-----------------------------------------------------------------------------------------
token_id_db = {}


#-----------------------------------------------------------------------------------------
# token_id_implicit_list: Keep track of all token identifiers that ware defined 
#                         implicitly, i.e. not in a token section or in a token id file. 
#                         Each list element has three cells:
#                         [ Prefix-less Token ID, Line number in File, File Name]
#-----------------------------------------------------------------------------------------
token_id_implicit_list = []

#-----------------------------------------------------------------------------------------
# token_repetition_support: Quex can be told to return multiple times the same
#                           token before further analyzsis happens. For this,
#                           the engine needs to know how to read and write the
#                           repetition number in the token itself.
# If the 'token_repetition_token_id_list' is None, then the token repetition feature
# is disabled. Otherwise, token repetition in 'token-receiving.i' is enabled
# and the token id that can be repeated is 'token_repetition_token_id'.
#-----------------------------------------------------------------------------------------
token_repetition_token_id_list = ""

#-----------------------------------------------------------------------------------------
# token_type_definition: Object that defines a (user defined) token class.
#
#                        The first token_type section defines the variable as 
#                        a real 'TokenTypeDescriptor'.
#
#                        Default = None is detected by the 'input/file/core.py' and
#                        triggers the parsing of the default token type description. 
#          
#                        The setup_parser.py checks for the specification of a manually
#                        written token class file. If so then an object of type 
#                        'ManualTokenClassSetup' is assigned.
#-----------------------------------------------------------------------------------------
token_type_definition = None

#-----------------------------------------------------------------------------------------
# Helper functions about required features.
#-----------------------------------------------------------------------------------------
def requires_indentation_count(ModeDB):
    """Determine whether the lexical analyser needs indentation counting
       support. if one mode has an indentation handler, than indentation
       support must be provided.                                         
    """
    for mode in ModeDB.itervalues():
        if    mode.has_code_fragment_list("on_indent")      \
           or mode.has_code_fragment_list("on_nodent")      \
           or mode.has_code_fragment_list("on_indentation") \
           or mode.has_code_fragment_list("on_dedent"):
            return True

        if mode.options["indentation"] is not None:
            assert mode.options["indentation"].__class__.__name__ == "IndentationSetup"
            return True

    return False

def requires_begin_of_line_condition_support(ModeDB):
    """If one single pattern in one mode depends on begin of line, then
       the begin of line condition must be supported. Otherwise not.
    """
    for mode in ModeDB.values():
        pattern_action_pair_list = mode.get_pattern_action_pair_list()
        for info in pattern_action_pair_list:
            if info.pattern().pre_context_trivial_begin_of_line_f:
                return True
    return False

def deprecated(*Args):
    """This function is solely to be used as setter/getter property, 
       of member variables that are deprecated. This way misuse
       can be detected. Example usage:

       class X(object):  # Class must be derived from 'object'
           ...
           my_old = property(deprecated, deprecated, "Alarm on 'my_old'")
    """
    assert False
