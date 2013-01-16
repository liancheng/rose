from quex.engine.misc.tree_walker import TreeWalker
from quex.blackboard              import E_Count
from copy import copy

class CharacterCounter(TreeWalker):
    """Recursive Algorithm to count the number of newlines, characters, or spaces
       for each state in the state machine. It is done for each state, so that 
       path walking can be aborted as soon as a known state is hit.

       -- A loop makes a count either (1) void if the counted character appears, 
          or (2) is unimportant. If (1) happens, then the counter is globally
          void. In case of (2) no change happend so any analysis starting from
          the loop's knot point is still valid and does not have to be made 
          again.

       -- A node it met through another path. Exactly the same consideration as
          for loops holds again. The break-up here is also essential to avoid
          exponential time (The total number of paths multiplies with the number
          of branches through each knot on the path).

       When this algorithm is done, still the accepting and storing states need
       to be investigated whether they store the same counts.

       ONLY PATTERNS WITHOUT PRE- AND POST-CONTEXT ARE HANDLED HERE!
    """   
    def __init__(self):  
        self.path     = []
        self.depth    = 0
        self.current  = Count()
        self.result   = Count()
        self.known_db = {}  # state_index --> count
        TreeWalker.__init__(self)

    def on_enter(self, Info):  
        """Info = (state_index of what is entered, character set that triggers to it)"""
        StateIndex, CharacterSet = Info

        if not self.current.compute(CharacterSet):
            self.abort_f = True
            return None

        known = self.known_db.get(StateIndex)
        if known is not None:
            if known.column_n != self.current.column_n: known.column_n = E_Count.VOID
            if known.line_n   != self.current.line_n:   known.line_n   = E_Count.VOID
            if self.result.line_n is None and self.result.column_n is None: 
                self.abort_f   = True
            # Rest of paths starting from this state has been walked along before
            return None
        else:
            known = copy(self.current)
            self.known_db[StateIndex] = known

        state = self.sm.states[StateIndex]
        if state.is_acceptance():
            if   self.result.column_n == E_Count.VIRGIN: self.result.column_n = known.column_n
            elif self.result.column_n != known.column_n: self.result.column_n = E_Count.VOID
            if   self.result.line_n == E_Count.VIRGIN:   self.result.line_n = known.line_n
            elif self.result.line_n != known.line_n:     self.result.line_n = E_Count.VOID

        return state.transitions().get_map().items()

    def on_finished(self, node):   
        pass

class Count(object):
    __slots__ = ('column_n', 'line_n', 'newline_char_db', 'grid_char_db', 'special_char_db')

    newline_char_db = None
    grid_char_db    = None
    special_char_db = None

    @staticmethod
    def init(NewlineCharDB, GridCharDB, SpecialCharDB):
        Count.newline_char_db = NewlineCharDB
        Count.grid_char_db    = GridCharDB
        Count.special_char_db = SpecialCharDB

    def __init__(self):
        self.column_n = E_Count.VIRGIN
        self.line_n   = E_Count.VIRGIN

    def compute(self, CharacterSet):
        """Compute the increase of line and column numbers due to the 
           given character set. If both are void due to the character
           set then the 'abort_f' is raised. 
        """
        def check(CmpSet):
            """Compare 'CmpSet' with 'CharacterSet'
            
               RETURNS: True  -- if all characters in CharacterSet are in 
                                 CmpSet and CharacterSet does not contain
                                 any character beyond.
                        False -- if CharacterSet and CmpSet have no common
                                 characters whatsoever.
                        None  -- if CharacterSet has some characters from 
                                 CmpSet but also others beyond.
            """
            if   CmpSet.is_superset(CharacterSet):      return True
            elif CmpSet.has_intersection(CharacterSet): return None
            else:                                       return False

        for line_n, character_set in Count.newline_char_db.iteritems():
            x = check(character_set)
            if x == True:
                self.line_n   += line_n
                self.column_n  = 0
                return
            elif x == False:
                continue
            else:
                # If a newline character appears together with a space parameter, then
                # both column number and line number are void. Analysis can abort.
                self.line_n   = E_Count.VOID
                self.column_n = E_Count.VOID
                return False # Abort

            return

        for grid_size, character_set in Count.grid_char_db.iteritems():
            x = check(character_set)
            if x == True:
                self.column_n += (self.column_n // grid_size + 1) * grid_size
                return
            elif x == False:
                continue
            else:
                # If a grid character appears together with a space
                # parameter, then column number is void. 
                self.column_n = E_Count.VOID
                return self.line_n is not E_Count.VOID # Abort, if line_n is also void.

            return


        for space_n, character_set in Count.special_char_db.iteritems():
            x = check(character_set)
            if x == True:
                self.column_n += space_n
                return
            elif x == False:
                continue
            else:
                # If a grid character appears together with a space
                # parameter, then column number is void. 
                self.column_n = E_Count.VOID
                return self.line_n is not E_Count.VOID # Abort, if line_n is also void.

            return

        self.column_n += 1
        return True # Do not abort, yet

