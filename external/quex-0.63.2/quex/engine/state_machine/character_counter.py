# (C) 2012 Frank-Rene Schaefer
from quex.engine.misc.tree_walker  import TreeWalker
from quex.blackboard               import E_Count, CounterDB

def do(SM):
    """Counts line and column number, if possible, from the structure
       of the state machine that represents the pattern.

       State machine shall not contain pre- or post-contexts.
       
       DEPENDS ON: CounterDB in quex.blackboard. In this namespace the
                   three databases 'newline', 'grid', and 'special' are
                   defined.

       RETURN: newline_n, column_n

               Each one may be 'E_Count.VOID' if the value can only
               be determined at run time.

       SHORTCOMING OF THE ALGO:

       The current approach does consider the column count to be void as soon
       as a state is reached with two different column counts. This is too rigid
       in a sense that a newline may clear the column count later in the pattern.
       If the column counts to the acceptance state are then equal from there on,
       the column count could be a numeric constant.

       Practically, this means that Quex will implement a column counter in some
       special cases where a pattern contains a newline, where a fixed constant
       could be added instead. Multi-line patterns are considered to be rare and
       the overhead of counting from the end of the lexeme to the last newline 
       is considered to be minimal. There is no significant performance decrease
       expected from this shortcoming.

       To fix this, another approach would have to be implemented where the 
       state machine is inverted and then the column counts starts from rear
       to front until the first newline. This tremendous computation time overhead
       is shied away from, because of the aforementioned low expected value add.
    """
    if not CounterDB.is_enabled(): 
        return E_Count.VOID, E_Count.VOID

    Count.init()

    counter = CharacterCounter(SM)
    state   = SM.get_init_state()
    count   = Count(0, 0)
    # Next Node: [0] state index of target state
    #            [1] character set that triggers to it
    #            [2] count information
    initial = [ (state_index, character_set, count.clone()) \
                 for state_index, character_set in state.transitions().get_map().iteritems() ]
    counter.do(initial)

    return counter.result.line_n, counter.result.column_n #, counter.column_increment_per_character, counter.contains_newline_f

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

       ONLY PATTERNS WITHOUT PRE- AND POST-CONTEXT ARE HANDLED HERE!
    """   
    def __init__(self, SM):  
        self.sm       = SM
        self.depth    = 0
        self.result   = Count(E_Count.VIRGIN, E_Count.VIRGIN)
        self.known_db = {}  # state_index --> count
        TreeWalker.__init__(self)

    def on_enter(self, Info):  
        """Info = (state_index of what is entered, character set that triggers to it)"""
        StateIndex, CharacterSet, count = Info

        if not count.compute(CharacterSet):
            self.result.line_n   = E_Count.VOID
            self.result.column_n = E_Count.VOID
            self.abort_f = True
            return None

        state = self.sm.states[StateIndex]
        known = self.known_db.get(StateIndex)
        if known is not None:
            if known.column_n != count.column_n: self.result.column_n = E_Count.VOID
            if known.line_n   != count.line_n:   self.result.line_n   = E_Count.VOID

            if self.result.line_n == E_Count.VOID and self.result.column_n == E_Count.VOID: 
                self.abort_f = True

            # Rest of paths starting from this state has been walked along before
            subsequent = None
        else:
            known = Count(count.column_n, count.line_n)
            self.known_db[StateIndex] = known

            subsequent = [ (state_index, character_set, count.clone()) \
                           for state_index, character_set in state.transitions().get_map().iteritems() ]

        if state.is_acceptance():
            if   self.result.column_n == E_Count.VIRGIN: self.result.column_n = known.column_n
            elif self.result.column_n != known.column_n: self.result.column_n = E_Count.VOID
            if   self.result.line_n == E_Count.VIRGIN:   self.result.line_n = known.line_n
            elif self.result.line_n != known.line_n:     self.result.line_n = E_Count.VOID

            if self.result.line_n == E_Count.VOID and self.result.column_n == E_Count.VOID: 
                self.abort_f = True

        return subsequent

    def on_finished(self, node):   
        pass

class Count(object):
    __slots__ = ('column_n', 'line_n')

    # (*) Increment per step:
    #
    #     If the increment per step is the same 'C' for any character that appears 
    #     in the pattern, then the length of the pattern can be computed at run-
    #     time by a simple subtration:
    # 
    #               length = (LexemeEnd - LexemeBegin) * C
    #
    #     provided that there is no newline in the pattern this is at the same 
    #     time the column increment. Same holds for line number increments.
    column_increment_per_step = E_Count.VIRGIN
    # Just for info, in unicode there are the following candidates which may possibly
    # have assigned a separate line number increment: Line Feed, 0x0A; Vertical Tab, 0x0B; 
    # Form Feed, 0x0C; Carriage Return, 0x0D; Next Line, 0x85; Line Separator, 0x28; 
    # Paragraph Separator, 0x2029; 
    line_increment_per_step   = E_Count.VIRGIN

    # (*) Contains newline?
    contains_newline_f        = False
    
    @staticmethod
    def init():
        """Initialize global objects in namespace 'Count'."""
        Count.column_increment_per_step = E_Count.VIRGIN
        Count.line_increment_per_step   = E_Count.VIRGIN
        Count.contains_newline_f        = False

    def __init__(self, ColumnN, LineN):
        self.column_n = ColumnN
        self.line_n   = LineN

    def clone(self):
        return Count(self.column_n, self.line_n)

    def compute(self, CharacterSet):
        """Compute the increase of line and column numbers due to the given
           character set. If both are void due to the character set then the
           'abort_f' is raised.
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

        for delta_line_n, character_set in CounterDB.newline.iteritems():
            x = check(character_set)
            if x == False: continue
            Count.announce_line_n_per_step(delta_line_n)
            Count.contains_newline_f = True

            if x == True:
                self.line_n   += delta_line_n
                self.column_n  = 0
                return True  # 'CharacterSet' does not contain anything beyond 'character_set'
            else:
                self.line_n   = E_Count.VOID  # Newline together with other characters in one
                self.column_n = E_Count.VOID  # transition. => delta line, delta column = void.
                return False # Abort

        for grid_size, character_set in CounterDB.grid.iteritems():
            x = check(character_set)
            if x == False: continue
            Count.announce_column_n_per_step(E_Count.VOID)

            if x == True:
                self.column_n = (self.column_n // grid_size + 1) * grid_size
                return True
            else:
                # Same transition with characters of different horizonzal size.
                # => delta column_n = VOID
                self.column_n = E_Count.VOID
                return self.line_n is not E_Count.VOID # Abort, if line_n is also void.

        for delta_column_n, character_set in CounterDB.special.iteritems():
            x = check(character_set)
            if x == False: continue
            Count.announce_column_n_per_step(delta_column_n)

            if x == True:
                self.column_n += delta_column_n
                return True
            else:
                # Same transition with characters of different horizonzal size.
                # => delta column_n = VOID
                self.column_n = E_Count.VOID
                return self.line_n is not E_Count.VOID # Abort, if line_n is also void.

        if   self.column_n == E_Count.VIRGIN: 
            self.column_n = 1
        elif self.column_n != E_Count.VOID:                               
            self.column_n += 1
            Count.announce_column_n_per_step(1)
        
        return True # Do not abort, yet

    @staticmethod
    def announce_line_n_per_step(DeltaLineN):
        if Count.line_increment_per_step == E_Count.VIRGIN: Count.line_increment_per_step = DeltaLineN
        elif Count.line_increment_per_step != DeltaLineN:   Count.line_increment_per_step = E_Count.VOID

    @staticmethod
    def announce_column_n_per_step(DeltaLineN):
        if Count.column_increment_per_step == E_Count.VIRGIN: Count.column_increment_per_step = DeltaLineN
        elif Count.column_increment_per_step != DeltaLineN:   Count.column_increment_per_step = E_Count.VOID
