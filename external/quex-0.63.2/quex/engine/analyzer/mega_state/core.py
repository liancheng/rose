"""MEGA STATES _________________________________________________________________

A 'MegaState' is a state which absorbs and implements multiple AnalyzerState-s
in a manner that is beneficial in terms of code size, computational speed, or
both. All MegaState-s shall be derived from class MegaState, and thus are
committed to the described interface. The final product of a MegaState is a
piece of code which can act on behalf of its absorbed AnalyzerState-s. 

A 'state_key' indicates for any point in time the AnalyzerState which the
MegaState represents. 

The following scheme displays the general idea of a class hierarchy with a
MegaState involved. At the time of this writing there are two derived classes
'TemplateState' and 'PathWalkerState'--each represent a compression algorith: 

    AnalyzerState <------- MegaState <----+---- TemplateState
                                          |
                                          '---- PathWalkerState


Analogous to the AnalyzerState, a MegaState has special classes to implement
'Entry' and 'DropOut', namely 'MegaState_Entry' and 'MegaState_DropOut'.  Where
an AnalyzerState's transition_map associates a character interval with a target
state index, the MegaState's transition_map associates a character interval
with a 'MegaState_Target'. Given a state_key, the MegaState_Target provides the
target state index for the given character interval.

The following pinpoints the general idea of a MegaState.

    MegaStateEntry:
 
        ... entry doors of absorbed states ...

    /* Some Specific Actions ... */

    tansition_map( input ) {
        in interval_0:  MegaState_Target_0[state_key];  --> Target states
        in interval_1:  MegaState_Target_1[state_key];  --> depending on
        in interval_2:  MegaState_Target_2[state_key];  --> current input
        ...                                             --> character.
        in interval_N:  MegaState_Target_N[state_key]; 
    }

    MegaState_DropOut:

        ... drop-out actions of absorbed states ...
_______________________________________________________________________________

This file provides two special classes for to represent 'normal' 
AnalyzerState-s:

-- PseudoMegaState: represents an AnalyzerState as if it was a 
                    MegaState. This way, it may act homogeneously in 
                    algorithms that work on MegaState-s and AnalyzerState-s
                    at the same time.
 
-- AbsorbedState:   represent an AnalyzerState in the original state database,
                    even though it is absorbed by a MegaState.

_______________________________________________________________________________
(C) 2012 Frank-Rene Schaefer
"""
from quex.engine.analyzer.state.core         import AnalyzerState
from quex.engine.analyzer.state.entry        import Entry
from quex.engine.analyzer.state.drop_out     import DropOut, \
                                                    DropOutBackward, \
                                                    DropOutBackwardInputPositionDetection
from quex.engine.analyzer.state.entry_action import DoorID
from quex.blackboard                         import E_StateIndices

from copy import copy

class MegaState_Entry(Entry):
    """________________________________________________________________________
    
    Implements a common base class for Entry classes of MegaState-s. Entries of
    MegaState-s are special in a sense that they implement transitions to more
    than one state. The '.action_db' of an Entry of an AnalyzerState contains
    only transitions (from_index, to_index) where 'to_index == state_index'. A
    MegaState implements two or more AnalyzerState-s, so the 'to_index' may
    have more than one value in keys of '.action_db'.
    
    PRELIMINARY: Documentation of class 'Entry'.

    ___________________________________________________________________________
    """
    def __init__(self, MegaStateIndex):
        Entry.__init__(self, MegaStateIndex, FromStateIndexList=[])

class MegaState(AnalyzerState):
    """________________________________________________________________________
    
    Interface for all derived MegaState-s:

       .implemented_state_index_list():
       
          List of indices of AnalyzerState-s which have been absorbed by the 
          MegaState.

       .map_state_index_to_state_key(): 
       
          Provides the state_key that the MegaState requires to act on behalf
          of state_index.

       .map_state_key_to_state_index():

          Determines the state_index on whose behalf the MegaState acts, if its
          state_key is as specified.

       '.bad_company'

          Keeps track of indices of AnalyzerState-s which are not good company.
          Algorithms that try to combine multiple MegaState-s into one (e.g.
          'Template Compression') profit from avoiding to combine MegaStates
          where its elements are bad company to each other.

       .finalize_transition_map()

          Adapts the transition_map. When it detects that all elements of a
          'scheme' enter the same state door, it is replaced by the DoorID.  If
          a uniform target state is entered through different doors depending
          on the absorbed state, then it is replaced by a scheme that contains
          the target state multiple times. 

          The transition_map can only be finalized after ALL MegaState-s have
          been generated.
    ___________________________________________________________________________
    """ 
    def __init__(self, TheEntry, TheDropOut, StateIndex):
        # A 'PseudoMegaState' does not implement a 'MegaState_Entry' and 'MegaState_DropOut'.
        # On the long term 'MegaState_DropOut' should be derived from 'DropOut'.
        assert isinstance(TheEntry, Entry), Entry.__class__.__name__
        assert isinstance(TheDropOut, (MegaState_DropOut, DropOut, DropOutBackward, DropOutBackwardInputPositionDetection)) 
        assert isinstance(StateIndex, long)

        self.__entry    = TheEntry
        self.__drop_out = TheDropOut
        AnalyzerState.set_index(self, StateIndex)

        # Maintain a list of states with which the state may not combine well
        self.__bad_company = set()
        
        # State Index Sequence: Implemented States (and may be others) in an 
        # ordered Sequence.
        self.__state_index_sequence = None

    @property
    def entry(self):        return self.__entry

    @property
    def drop_out(self):     return self.__drop_out

    @property
    def init_state_f(self): return False

    def state_index_sequence(self):
        assert False, "This function needs to be overwritten by derived class."

    def implemented_state_index_list(self):
        assert False, "This function needs to be overwritten by derived class."

    def map_state_index_to_state_key(self, StateIndex):
        assert False, "This function needs to be overwritten by derived class."

    def map_state_key_to_state_index(self, StateKey):
        assert False, "This function needs to be overwritten by derived class."

    def bad_company_add(self, StateIndex):
        self.__bad_company.add(StateIndex)

    def bad_company_set(self, StateIndexSet):
        self.__bad_company = StateIndexSet

    def bad_company(self):
        """RETURN: List of state indices with which the MegaState does not 
                   combine well.
        """
        return self.__bad_company

    def finalize_transition_map(self, StateDB):
        """Finalizes the all involved targets in the transition map. Due
        to some mega state analysis door ids may have changed. Thus, some 
        common states may be entered through different doors, etc.

        Call '.finalize()' for each involved MegaState_Target.
        """
        scheme_db = {}
        for i, info in enumerate(self.transition_map):
            interval, target = info
            adapted = target.finalize(self, StateDB, scheme_db)
            if adapted is None: continue
            self.transition_map[i] = (interval, adapted)

MegaState_Target_DROP_OUT_hash = hash(E_StateIndices.DROP_OUT)

class MegaState_Target(object):
    """________________________________________________________________________
    
    Where an AnalyzerState's transition map associates a character interval
    with a target state index, a MegaState's transition map associates a
    character interval with a MegaState_Target.

    A MegaState_Target determines the target state, or target state's entry
    door, by means of a state key. It is very well possible that it is
    homogeneous and independent of the state key. In that case, it contains a
    '.target_state_index' or '.door_id'. If not, the '.scheme' member describes
    the relationship between and target state index. For example, a given
    interval X triggers to MegaState_Target T, i.e. there is an element in the
    transition map:

             ...
             [ X, T ]
             ...

    then 'T.scheme[state key]' tells the 'target state index' for a given state key.
    The door through which it enters is determined by the transition id:

        TransitionID(FromStateIndex = MS.map_state_key_to_state_index(state key), 
                     ToStateIndex   = T.scheme[state key])

    where MS is the MegaState that contains the transition map. The
    TransitionID can be translated into a DoorID by the target state's entry
    database 'transition_db'.
    
    TRACKING SCHEMES: _________________________________________________________

    There might be multiple intervals following the same target scheme. This class
    keeps track of the schemes by means of the '.object_db'. Before handling 
    a transition map the function

              MegaState_Target.init()

    initializes the .object_db. An independent copy of the .object_db can be
    obtained by

              my_copy = MegaState_Target.disconnect_object_db()

    FINALIZATION: _____________________________________________________________

    Once the whole state configuration and the states' entry doors are
    determined, the actual MegaState_Target object can be finalized. That is:
       
       -- A common target may become a scheme, if the DoorIDs differ depending
          on the 'from_state_index' (from .implemented_state_index_list()).

       -- A scheme may become a common target, if the target DoorID 
          is the same for all indices in .implemented_state_index_list().

    Finalization sets the 'scheme_id' if it is a scheme. It set's the
    '.door_id' if the target state's door is the same for all involved states.

    ___________________________________________________________________________
    NOTE: All 'DropOut' MegaState_Target are represented by the single object
          'MegaState_Target_DROP_OUT'. This saves memory.
    """
    __slots__ = ('__drop_out_f', '__scheme', '__scheme_id', '__hash', '__target_state_index', '__door_id')

    __object_db = dict()

    @staticmethod
    def init():
        """Initializes: '__object_db' which keeps track of generated MegaState_Target-s."""
        MegaState_Target.__object_db.clear()
        # The Drop-Out target must be always in there.
        MegaState_Target.__object_db[E_StateIndices.DROP_OUT] = MegaState_Target_DROP_OUT

    @staticmethod
    def disconnect_object_db():
        """Disconnects the '__object_db' so that it may be used without influencing 
           the '__object_db' of MegaState_Target.
        """
        tmp_object_db                = MegaState_Target.__object_db
        MegaState_Target.__object_db = dict()
        return tmp_object_db

    @staticmethod
    def create(Target):
        assert Target is not None 

        result = MegaState_Target.__object_db.get(Target)
        if result is None: 
            result = MegaState_Target(Target)
            MegaState_Target.__object_db[Target] = result

        return result

    def __init__(self, Target):
        global MegaState_Target_DROP_OUT_hash
        if Target is None: # Only to be used by 'self.clone()'
            return 

        self.__target_state_index = None
        self.__scheme             = None
        self.__scheme_id          = None # Only possibly set in 'finalize'
        self.__door_id            = None # Only possibly set in 'finalize'

        if   Target == E_StateIndices.DROP_OUT: 
            self.__drop_out_f = True; 
            self.__hash       = MegaState_Target_DROP_OUT_hash 
            return

        self.__drop_out_f = False
        self.__hash       = None
        if   isinstance(Target, long):   self.__target_state_index = Target 
        elif isinstance(Target, tuple):  self.__scheme             = Target
        elif isinstance(Target, DoorID): self.__door_id            = Target # only by '.finalize()'
        else:                            assert False, Target.__class__.__name__

    def get_hash(self):
        if self.__hash is None: 
            if self.__target_state_index is not None: self.__hash = hash(self.__target_state_index)
            elif self.__scheme is not None:           self.__hash = hash(self.__scheme)
            else:                                     self.__hash = hash(self.__door_id)
        return self.__hash
    @property
    def scheme(self):              return self.__scheme
    @property
    def target_state_index(self):  return self.__target_state_index
    @property
    def target_door_id(self):      return self.__door_id
    @property
    def drop_out_f(self):          return self.__drop_out_f
    @property
    def scheme_id(self):           return self.__scheme_id

    def finalize(self, TheMegaState, StateDB, scheme_db):
        """Once the whole state configuration and the states' entry doors are
        determined, the actual MegaState_Target object can be finalized.
        That is:
           
           -- A common target may become a scheme, if the DoorIDs differ
              depending on the 'from_state_index' (which is one of the
              .implemented_state_index_list()).

           -- A scheme may become a common target, if the target DoorID 
              is the same for all indices in .implemented_state_index_list().
        """
        if self.drop_out_f:
            return

        implemented_state_index_list = TheMegaState.implemented_state_index_list()
        L = len(implemented_state_index_list)
        assert L > 1

        def determine_scheme_id(scheme_db, Scheme):
            scheme_id = scheme_db.get(Scheme)
            if scheme_id is None: 
                scheme_id = len(scheme_db)
                scheme_db[Scheme] = scheme_id
            return scheme_id

        # NOTE: Due to the possible cover-up of parts of the transition map, it
        #       is possible that not all implemented states of a MegaState trigger
        #       to '.target_state_index' or the states mentioned in '.scheme'.
        #
        #       This results in 'target_entry.get_door_id(To, From)' being 'None'
        #       sometimes. This is not an error!
        if self.scheme is not None:
            assert len(self.scheme) == L
            # The targets in a 'scheme' may be implemented by the same MegaState--
            # with the CommandList at state entry. In this case, a target state
            # scheme translates into a common transition to target DoorID.
            prototype = None
            for state_index in implemented_state_index_list:
                state_key = TheMegaState.map_state_index_to_state_key(state_index)
                target_state_index = self.scheme[state_key]

                if target_state_index != E_StateIndices.DROP_OUT:
                    # DROP_OUT cannot be in a scheme, if there was some non-DROP-OUT there.
                    # => Only give it a chance as long as no DROP_OUT target appears.
                    target_entry = StateDB[target_state_index].entry
                    door_id      = target_entry.get_door_id(target_state_index, state_index)
                    if   prototype is None:    prototype = door_id; continue
                    elif prototype == door_id: continue

                # The scheme is indeed not uniform => Stay with the scheme
                self.__scheme_id = determine_scheme_id(scheme_db, self.__scheme)
                return # Nothing to be done
            else:
                # All has been uniform => generate transition through common DoorID
                assert prototype is not None
                return MegaState_Target.create(prototype)

        else:
            assert self.target_state_index is not None
            # The common target state may be entered by different doors
            # depending on the 'from_state' which is currently implemented by
            # the MegaState. Then, a common 'target_state_index' translates into 
            # a target scheme. DoorID's for each element are computed later
            # depending on the '.implemented_state_index_list'.
            target_entry = StateDB[self.target_state_index].entry
            prototype    = None
            for state_index in implemented_state_index_list:
                door_id   = target_entry.get_door_id(self.target_state_index, state_index)
                if prototype is None:      prototype = door_id; continue
                elif prototype == door_id: continue

                # The door_ids are not uniform => generate a scheme
                result = MegaState_Target.create((self.target_state_index,) * L)
                result.__scheme_id = determine_scheme_id(scheme_db, result.scheme)
                return result
            else:
                # All has been uniform => Stay with 'target_state_index'
                assert prototype is not None
                return # Nothing to be done

    def __repr__(self):
        if   self.drop_out_f:                     return "MegaState_Target:DropOut"
        elif self.target_state_index is not None: return "MegaState_Target:(%s)"       % repr(self.__target_state_index).replace("L", "")
        elif self.target_door_id is not None:     return "MegaState_Target:%s"         % repr(self.__door_id).replace("L", "")
        elif self.scheme is not None:             return "MegaState_Target:scheme(%s)" % repr(self.__scheme).replace("L", "")
        else:                                     return "MegaState_Target:<ERROR>"

    def __hash__(self):
        if   self.__drop_out_f:                     return 0
        elif self.__target_state_index is not None: return self.__target_state_index.state_index
        elif self.__scheme is not None:             return hash(self.__scheme)
        else:                                       assert False

    def __eq__(self, Other):
        if   isinstance(Other, MegaState_Target) == False: 
            return False
        elif self.__drop_out_f and Other.__drop_out_f: 
            return True
        elif self.__target_state_index is not None and Other.__target_state_index is not None:
            return self.__target_state_index == Other.__target_state_index
        elif self.__scheme  is not None and Other.__scheme  is not None:
            return self.__scheme == Other.__scheme
        else:
            return False
        ## if self.__scheme_id != Other.__scheme_id: return False
        return self.__scheme == Other.__scheme

# Globally unique object to stand up for all 'drop-outs'.
MegaState_Target_DROP_OUT      = MegaState_Target(E_StateIndices.DROP_OUT)

class MegaState_DropOut(dict):
    """_________________________________________________________________________
    
    Map: 'DropOut' object --> indices of states that implement the 
                              same drop out actions.

    For example, if four states 1, 4, 7, and 9 have the same drop_out behavior
    DropOut_X, then this is stated by an entry in the dictionary as

             { ...     DropOut_X: [1, 4, 7, 9],      ... }

    For this to work, the drop-out objects must support a proper interaction
    with the 'dict'-objects. Namely, they must support:

             __hash__          --> get the right 'bucket'.
             __eq__ or __cmp__ --> compare elements of 'bucket'.
    ____________________________________________________________________________
    """
    def __init__(self, *StateList):
        """Receives a list of states, extracts the drop outs and associates 
        each DropOut with the state indices that implement it.
        """
        for state in StateList:
            self.update_from_state(state)
        return

    @property
    def uniform_f(self):
        """Uniform drop-out means, that for all drop-outs mentioned the same
        actions have to be performed. This is the case, if all states are
        categorized under the same drop-out. Thus the dictionary's size
        will be '1'.
        """
        return len(self) == 1

    def is_uniform_with(self, Other):
        """The given Other drop-out belongs to a 'normal state'. This function
        investigates if it's drop-out behavior is the same as all in others
        in this MegaState_DropOut. 

        If this MegaState_DropOut is not uniform, then of course it cannot
        become uniform with 'Other'.
        """
        if not self.uniform_f: return False

        prototype = self.iterkeys().next()
        return prototype == Other

    def update_from_other(self, MS_DropOut):
        for drop_out, state_index_set in MS_DropOut.iteritems():
            # assert hasattr(drop_out, "__hash__")
            # assert hasattr(drop_out, "__eq__") # PathWalker may enter 'None' in unit test
            x = self.get(drop_out)
            if x is None: self[drop_out] = copy(state_index_set)
            else:         x.update(state_index_set)

    def update_from_state(self, TheState):
        drop_out = TheState.drop_out
        if hasattr(drop_out, "iteritems"): 
            self.update_from_other(drop_out)
            return
        #assert hasattr(drop_out, "__hash__")
        #assert hasattr(drop_out, "__eq__")
        x = self.get(drop_out)
        if x is None: self[drop_out] = set([TheState.index])
        else:         x.add(TheState.index)

class PseudoMegaState(MegaState): 
    """________________________________________________________________________
    
    Represents an AnalyzerState in a way to that it acts homogeneously with
    other MegaState-s. That is, the transition_map is adapted so that it maps
    from a character interval to a MegaState_Target.

              transition_map:  interval --> MegaState_Target

    instead of mapping to a target state index.
    ___________________________________________________________________________
    """
    def __init__(self, Represented_AnalyzerState):
        assert not isinstance(Represented_AnalyzerState, MegaState)
        self.__state = Represented_AnalyzerState

        pseudo_mega_state_drop_out = MegaState_DropOut(Represented_AnalyzerState)

        MegaState.__init__(self, self.__state.entry, 
                           pseudo_mega_state_drop_out,
                           Represented_AnalyzerState.index)

        self.__state_index_sequence = [ Represented_AnalyzerState.index ]

        self.transition_map = self.__transition_map_construct()

    def __transition_map_construct(self):
        """Build a transition map that triggers to MegaState_Target-s rather
        than simply to target states.

        CAVEAT: In general, it is **NOT TRUE** that if two transitions (x,a) and
        (x, b) to a state 'x' share a DoorID in the original state, then they
        share the DoorID in the MegaState. 
        
        The critical case is the recursive transition (x,x). It may trigger the
        same actions as another transition (x,a) in the original state.
        However, when 'x' is implemented in a MegaState it needs to set a
        'state_key' upon entry from 'a'. This is, or may be, necessary to tell
        the MegaState on which's behalf it has to operate. The recursive
        transition from 'x' to 'x', though, does not have to set the state_key,
        since the MegaState still operates on behalf of the same state. While
        (x,x) and (x,a) have the same DoorID in the original state, their DoorID
        differs in the MegaState which implements 'x'.
        
        THUS: A translation 'old DoorID' --> 'new DoorID' is not sufficient to
        adapt transition maps!
        
        Here, the recursive target is implemented as a 'scheme' in order to
        prevent that it may be treated as 'uniform' with other targets.
        """
        return [ (interval, MegaState_Target.create(target)) \
                 for interval, target in self.__state.transition_map]

    def state_index_sequence(self):
        return self.__state_index_sequence

    def implemented_state_index_list(self):
        return self.__state_index_sequence

    def map_state_index_to_state_key(self, StateIndex):
        assert False, "PseudoMegaState-s exist only for analysis. They shall never be implemented."

    def map_state_key_to_state_index(self, StateKey):
        assert False, "PseudoMegaState-s exist only for analysis. They shall never be implemented."

class AbsorbedState_Entry(Entry):
    """________________________________________________________________________

    The information about what transition is implemented by what
    DoorID is stored in this Entry. It is somewhat isolated from the
    AbsorbedState's Entry object.
    ___________________________________________________________________________
    """
    def __init__(self, StateIndex, TransitionDB, DoorDB):
        Entry.__init__(self, StateIndex, FromStateIndexList=[])
        self.set_transition_db(TransitionDB)
        self.set_door_db(DoorDB)

class AbsorbedState(AnalyzerState):
    """________________________________________________________________________
    
    An AbsorbedState object represents an AnalyzerState which has been
    implemented by a MegaState. Its sole purpose is to pinpoint to the
    MegaState which implements it and to translate the transtions into itself
    to DoorIDs of the implementing MegaState.
    ___________________________________________________________________________
    """
    def __init__(self, AbsorbedAnalyzerState, AbsorbingMegaState):
        AnalyzerState.set_index(self, AbsorbedAnalyzerState.index)
        # The absorbing MegaState may, most likely, contain other transitions
        # than the transitions into the AbsorbedAnalyzerState. Those, others
        # do not do any harm, though. Filtering out those out of the hash map
        # does, most likely, not bring any benefit.
        assert AbsorbedAnalyzerState.index in AbsorbingMegaState.implemented_state_index_list()
        #----------------------------------------------------------------------

        self.__entry     = AbsorbedState_Entry(AbsorbedAnalyzerState.index, 
                                               AbsorbingMegaState.entry.transition_db,
                                               AbsorbingMegaState.entry.door_db)
        self.absorbed_by = AbsorbingMegaState
        self.__state     = AbsorbedAnalyzerState

    @property
    def drop_out(self):
        return self.__state.drop_out

    @property
    def entry(self): 
        return self.__entry

