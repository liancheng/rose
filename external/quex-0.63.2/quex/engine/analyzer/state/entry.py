import quex.engine.analyzer.state.entry_action as entry_action
from   quex.engine.analyzer.state.entry_action import TransitionID, TransitionAction, DoorID
from   quex.blackboard import E_PreContextIDs,  \
                              E_AcceptanceIDs, E_PostContextIDs, \
                              E_TransitionN, E_StateIndices
from   operator        import attrgetter

class Entry(object):
    """________________________________________________________________________

    An Entry object stores commands to be executed at entry into a state
    depending on a particular source state.
    
    BASICS _________________________________________________________________

    To keep track of entry actions, CommandList-s need to 
    be associated with a TransitionID-s, i.e. pairs of (state_index,
    from_state_index). This happens in the member '.action_db', i.e.
    
       .action_db:    (state_index, from_state_index) --> actions

    precisely in terms of classes:

       .action_db:    TransitionID --> TransitionAction

    where a TransitionID stores a pair of (StateIndex, FromStateIndex) and
    a TransitionAction stores a TransitionID along with a CommandList of
    commands that are executed for the given transition.

    The actions associated with transitions may be equal or have many
    commonalities. In order to avoid multiple definitions of the same
    action, a 'door tree' is constructed. Now, transitions need to be
    associated with doors into the entry. After '.door_tree_configure()'
    has been called the two databases:

        .transition_db: DoorID --> list of TransitionID-s

        .door_db:       TransitionID --> DoorID

    inform about the relation between TransitionID-s and the DoorID-s
    through which they enter.

    DOOR TREE ______________________________________________________________

    For code generation, the TransitionActions are organized more efficiently.
    Many transitions may share some commands, so that they could actually enter
    the state through the same or similar doors. Example:

        (4, from 1) --> accept 4711; store input position;
        (4, from 2) --> accept 4711; 
        (4, from 2) --> nothing;

    Then, the entry could look like this:

        Door2: store input position; 
               goto Door1;
        Door1: accept 4711; 
               goto Door0;
        Door0: /* nothing */

        ... the transition map ...

    This configuration is done in function 'door_tree_configure()'. As a result
    the following mappings are available:

       transition_db:  DoorID --> list of TransitionID

                       Tells for every door what transitions it implements.

       door_db:        TransitionID --> DoorID

                       Tells for every transition what door implements a given 
                       transition.

       door_tree_root: The hierarchically organized tree of doors as shown in
                       the example above.

    BRIEF REMARK: _________________________________________________________

    Before '.door_tree_configure()' is called no assumptions about doors
    can be made. Then, only information about what transition causes what
    action can be provided.
    ___________________________________________________________________________
    """
    __slots__ = ("__state_index", "__uniform_doors_f", "__action_db", "__door_db", "__transition_db", "__door_tree_root")
    tmp_transition_id = TransitionID(E_StateIndices.VOID, E_StateIndices.VOID)

    def __init__(self, StateIndex, FromStateIndexList, PreContextFulfilledID_List=None):
        # map:  (from_state_index) --> list of actions to be taken if state is entered 
        #                              'from_state_index' for a given pre-context.
        # if len(FromStateIndexList) == 0: FromStateIndexList = [ E_StateIndices.NONE ]
        self.__state_index = StateIndex
        self.__action_db   = dict((TransitionID(StateIndex, i),     \
                                   TransitionAction(StateIndex, i)) \
                                   for i in FromStateIndexList)

        # Are the actions for all doors the same?
        self.__uniform_doors_f = None 

        # Function 'categorize_command_lists()' fills the following members
        self.__door_db        = None # map: transition_id --> door_id
        self.__transition_db  = None # map: door_id       --> transition_id
        self.__door_tree_root = None # The root of the door tree.

        # Only for 'Backward Detecting Pre-Contexts'.
        if PreContextFulfilledID_List is not None:
            pre_context_ok_command_list = [ entry_action.PreConditionOK(pre_context_id) \
                                           for pre_context_id in PreContextFulfilledID_List ]
            for entry in self.__action_db.itervalues():
                entry.command_list.misc.update(pre_context_ok_command_list)

    def doors_accept(self, FromStateIndex, PathTraceList):
        """At entry via 'FromStateIndex' implement an acceptance pattern that 
           is determined via 'PathTraceList'. This function is called upon the
           detection of a state that restores acceptance. The previous state 
           must be of uniform acceptance (does not restore). At the entry of 
           this function we implement the acceptance pattern of the previous
           state.
        """
        # Construct the Accepter from PathTraceList
        accepter = entry_action.Accepter(PathTraceList)

        Entry.tmp_transition_id.state_index      = self.__state_index
        Entry.tmp_transition_id.from_state_index = FromStateIndex
        self.__action_db[Entry.tmp_transition_id].command_list.accepter = accepter

    def doors_accepter_add_front(self, PreContextID, PatternID):
        """Add an acceptance at the top of each accepter at every door. If there
           is no accepter in a door it is created.
        """
        for door in self.__action_db.itervalues():
            # Catch the accepter, if there is already one, of not create one.
            if door.command_list.accepter is None: 
                door.command_list.accepter = entry_action.Accepter()
            door.command_list.accepter.insert_front(PreContextID, PatternID)

    def doors_store(self, FromStateIndex, PreContextID, PositionRegister, Offset):
        # Add 'store input position' to specific door. See 'entry_action.StoreInputPosition'
        # comment for the reason why we do not store pre-context-id.
        entry = entry_action.StoreInputPosition(PreContextID, PositionRegister, Offset)
        Entry.tmp_transition_id.state_index      = self.__state_index
        Entry.tmp_transition_id.from_state_index = FromStateIndex
        self.__action_db[Entry.tmp_transition_id].command_list.misc.add(entry)

    @property
    def transition_db(self):
        """Map:  door_id --> transition_id list
           The door_db is determined by 'categorize_command_lists()'
        """
        assert self.__transition_db is not None
        return self.__transition_db

    def set_transition_db(self, TransitionDB):
        assert isinstance(TransitionDB, dict)

        # Assert that we do not receive nonsense
        if len(TransitionDB) != 0:
            # DoorDB: door_id --> [ transition_id ] +
            door_id, transition_id_list = TransitionDB.iteritems().next()
            isinstance(transition_id_list, list)
            if len(transition_id_list) != 0: isinstance(transition_id_list[0], TransitionID)
            isinstance(door_id, DoorID)

        self.__transition_db = TransitionDB

    @property
    def door_tree_root(self): 
        """The door_tree_root is determined by 'categorize_command_lists()'"""
        assert self.__door_tree_root is not None
        return self.__door_tree_root

    def set_door_tree_root(self, DoorTreeRoot):
        self.__door_tree_root = DoorTreeRoot

    @property
    def action_db(self):
        return self.__action_db

    def action_db_get_command_list(self, StateIndex, FromStateIndex):
        for transition_id, command_list in self.__action_db.iteritems():
            if     transition_id.state_index      == StateIndex    \
               and transition_id.from_state_index == FromStateIndex:
                return command_list
        return None

    def action_db_delete_transition(self, StateIndex, FromStateIndex):
        for transition_id, command_list in self.__action_db.iteritems():
            if     transition_id.state_index      == StateIndex    \
               and transition_id.from_state_index == FromStateIndex:
                del self.__action_db[transition_id]
                return
        return

    def set_action_db(self, ActionDB):
        self.__action_db = ActionDB

    @property
    def door_db(self):
        """Map:  transition_id --> door_id 
           The door_db is determined by 'categorize_command_lists()'
        """
        assert self.__door_db is not None
        return self.__door_db

    def set_door_db(self, DoorDB):
        assert isinstance(DoorDB, dict)

        # Assert that we do not receive nonsense
        if len(DoorDB) != 0:
            # DoorDB: transition_id --> door_id
            transition_id, door_id = DoorDB.iteritems().next()
            isinstance(transition_id, TransitionID)
            isinstance(door_id, DoorID)

        self.__door_db = DoorDB

    def get_door_id(self, StateIndex, FromStateIndex):
        """RETURN: DoorID of the door which implements the transition 
                          (FromStateIndex, StateIndex).
                   None,  if the transition is not implemented in this
                          state.
        """
        Entry.tmp_transition_id.state_index      = StateIndex
        Entry.tmp_transition_id.from_state_index = FromStateIndex
        
        return self.__door_db.get(Entry.tmp_transition_id)

    def get_door_id_by_command_list(self, TheCommandList):
        """Finds the DoorID of the door that implements TheCommandList.
           RETURNS: None if no such door exists that implements TheCommandList.
        """
        assert self.__door_tree_root is not None, "door_tree_configure() has not yet been called!"

        if TheCommandList.is_empty():
            return DoorID(self.__state_index, 0) # 'Door 0' is sure to not do anything!

        for transition_id, action in self.action_db.iteritems():
            if action.command_list == TheCommandList:
                break
        else:
            return None

        return self.door_db.get(transition_id) # Returns 'None' on missing transition_id

    def door_find(self, DoorId):
        """Find the Door object that belongs to DoorId"""
        assert self.__door_tree_root is not None
        assert isinstance(DoorId, DoorID)

        def __dive(node):
             if node.door_id == DoorId: return node
             for child in node.child_list:
                 result = __dive(child)
                 if result is not None: return result
             return None
        return __dive(self.__door_tree_root)

    def door_has_commands(self, StateIndex, FromStateIndex):
        """Assume that 'door_tree_root' has been built alread."""

        # (1) Find the door for 'StateIndex' from 'FromStateIndex'
        Entry.tmp_transition_id.state_index      = StateIndex
        Entry.tmp_transition_id.from_state_index = FromStateIndex
        door_id = self.__door_db.get(Entry.tmp_transition_id)
        # A transition that does not exist, does not have commands related to it.
        if door_id is None: return False
        door          = self.door_find(door_id)
        assert door is not None

        # (2) Go all the way back from the entry door to the 'over parent'.
        #     If there is a node on the way with a non-empty command list, then
        #     there are actions associated with the door; and vice versa.
        while 1 + 1 == 2:
            if not door.common_command_list.is_empty(): return True
            # If we reach the 'over parent' without having any commands being
            # detected, then there are no commands associated with the given door.
            if door.parent is None: return False
            door = door.parent

    def has_accepter(self):
        for door in self.__action_db.itervalues():
            for action in door.command_list:
                if isinstance(action, entry_action.Accepter): return True
        return False

    def __hash__(self):
        xor_sum = 0
        for door in self.__action_db.itervalues():
            xor_sum ^= hash(door.command_list)
        return xor_sum

    def __eq__(self, Other):
        assert self.__door_tree_root is not None
        return self.__door_tree_root.is_equivalent(Other.__door_tree_root)

    def is_uniform(self, Other):
        assert self.__door_tree_root is not None
        return self.__door_tree_root.is_equivalent(Other.__door_tree_root)

    def is_equal(self, Other):
        # Maybe, we can delete this ...
        return self.__eq__(self, Other)

    def uniform_doors_f(self): 
        return self.__uniform_doors_f

    def has_special_door_from_state(self, StateIndex):
        """Determines whether the state has a special entry from state 'StateIndex'.
           RETURNS: False -- if entry is not at all source state dependent.
                          -- if there is no single door for StateIndex to this entry.
                          -- there is one or less door for the given StateIndex.
                    True  -- If there is an entry that depends on StateIndex in exclusion
                             of others.
        """
        if   self.__uniform_doors_f:    return False
        elif len(self.__action_db) <= 1: return False
        return self.__action_db.has_key(StateIndex)

    def finish(self, PositionRegisterMap):
        """Once the whole state machine is analyzed and positioner and accepters
           are set, the entry can be 'finished'. That means that some simplifications
           may be accomplished:

           -- The PositionRegisterMap shows how condition register indices must be 
              replaced. This is a result from analysis about what registers can
              actually be shared.

           -- If a position for a post-context is stored in the unconditional
              case, then all pre-contexted position storages of the same post-
              context are superfluous.

           -- If the entry into the state behaves the same for all 'from'
              states then the entry is independent_of_source_state.
        
           -- Call to 'categorize_command_lists()' where action lists are grouped
              and hierarchically ordered.

           At state entry the positioning might differ dependent on the the
           state from which it is entered. If the positioning is the same for
           each source state, then the positioning can be unified.

           A unified entry is coded as 'ALL' --> common positioning.
        """
        # (*) Adapt position registers (if necessary)
        if PositionRegisterMap is not None and len(PositionRegisterMap) != 0: 
            # (*) Some post-contexts may use the same position register. Those have
            #     been identified in PositionRegisterMap. Do the replacement.
            for from_state_index, door in self.__action_db.items():
                if door.command_list.is_empty(): continue
                change_f = False
                for action in door.command_list.misc:
                    if isinstance(action, entry_action.StoreInputPosition):
                        # Replace position register according to 'PositionRegisterMap'
                        action.position_register = PositionRegisterMap[action.position_register]
                        change_f = True
                # If there was a replacement, ensure that each action appears only once
                if change_f:
                    # Adding one by one ensures that double entries are avoided
                    door.command_list.misc = set(x for x in door.command_list.misc)

        # (*) If a door stores the input position in register unconditionally,
        #     then all other conditions concerning the storage in that register
        #     are nonessential.
        for door in self.__action_db.itervalues():
            for action in list(x for x in door.command_list \
                               if     isinstance(x, entry_action.StoreInputPosition) \
                                  and x.pre_context_id == E_PreContextIDs.NONE):
                for x in list(x for x in door.command_list.misc \
                              if isinstance(x, entry_action.StoreInputPosition)):
                    if x.position_register == action.position_register and x.pre_context_id != E_PreContextIDs.NONE:
                        door.command_list.misc.remove(x)

        # (*) Configure the entry door tree
        self.door_tree_configure() # May be this is not necessary here (?).

    def door_tree_configure(self):
        """Configure the 'door tree' (see module 'entry_action).

           BEFORE: An 'action_db' maps 

                   TransitionID  ---> CommandList

           That is, the action_db tells for a given transition (state_index,
           from_state_index) what commands are to be executed. Based on 
           action_db a 'door tree' is configured. The door tree profits from
           the fact, that some commands may be the same for more than one 
           transition into the state. Now, the state can be entered via these
           doors.
           
           AFTER: There are two more databases:

              (1)  door_db:       TransitionID --> DoorID

              which tells for a given transition (state, from_state) what the
              door is into the door tree which has been configured.
           
              (2)  transition_db: DoorID ---> list of TransitionID-s

              which tells what transitions are implemented by a given door.
        """
        # (*) Categorize action lists
        transition_action_list = [ transition_action.clone() for transition_action in self.__action_db.itervalues() ]
        door_db,       \
        transition_db, \
        self.__door_tree_root = entry_action.categorize_command_lists(self.__state_index, transition_action_list)

        self.set_door_db(door_db)               # use set_door_db() 'assert' on content
        self.set_transition_db(transition_db)   # use set_transition_db() 'assert' on content
        assert self.__door_tree_root is not None

    def __repr__(self):
        def get_accepters(AccepterList):
            if len(AccepterList) == 0: return []

            prototype = AccepterList[0]
            txt    = []
            if_str = "if     "
            for action in prototype:
                if action.pre_context_id != E_PreContextIDs.NONE:
                    txt.append("%s %s: " % (if_str, repr_pre_context_id(action.pre_context_id)))
                else:
                    if if_str == "else if": txt.append("else: ")
                txt.append("last_acceptance = %s\n" % repr_acceptance_id(action.pattern_id))
                if_str = "else if"
            return txt

        def get_storers(StorerList):
            txt = []
            for action in sorted(StorerList, key=attrgetter("pre_context_id", "position_register")):
                if action.pre_context_id != E_PreContextIDs.NONE:
                    txt.append("if '%s': " % repr_pre_context_id(action.pre_context_id))
                if action.offset == 0:
                    txt.append("%s = input_p;\n" % repr_position_register(action.position_register))
                else:
                    txt.append("%s = input_p - %i;\n" % (repr_position_register(action.position_register), 
                                                          action.offset))
            return txt

        def get_pre_context_oks(PCOKList):
            txt = []
            for action in sorted(PCOKList, key=attrgetter("pre_context_id")):
                txt.append("%s" % repr(action))
            return txt

        def get_set_template_state_keys(SetTemplateStateKeyList):
            txt = []
            for action in sorted(SetTemplateStateKeyList, key=attrgetter("value")):
                txt.append("%s" % repr(action))
            return txt

        def get_set_path_iterator_keys(SetPathIteratorKeyList):
            txt = []
            for action in sorted(SetPathIteratorKeyList, key=attrgetter("path_walker_id", "path_id", "offset")):
                txt.append("%s" % repr(action))
            return txt

        result = []
        for transition_id, door in sorted(self.__action_db.iteritems(),key=lambda x: x[0].from_state_index):
            accept_command_list = []
            store_command_list  = []
            pcok_command_list   = []
            ssk_command_list    = []
            spi_command_list    = []
            for action in door.command_list:
                if isinstance(action, entry_action.Accepter): 
                    accept_command_list.append(action)
                elif isinstance(action, entry_action.PreConditionOK):
                    pcok_command_list.append(action)
                elif isinstance(action, entry_action.SetTemplateStateKey):
                    ssk_command_list.append(action)
                elif isinstance(action, entry_action.SetPathIterator):
                    spi_command_list.append(action)
                else:
                    store_command_list.append(action)

            result.append("    .from %s:" % repr(transition_id.from_state_index).replace("L", ""))
            a_txt  = get_accepters(accept_command_list)
            s_txt  = get_storers(store_command_list)
            p_txt  = get_pre_context_oks(pcok_command_list)
            sk_txt = get_set_template_state_keys(ssk_command_list)
            pi_txt = get_set_path_iterator_keys(spi_command_list)
            content = "".join(a_txt + s_txt + p_txt + sk_txt + pi_txt)
            if len(content) == 0:
                # Simply new line
                content = "\n"
            elif content.count("\n") == 1:
                # Append to same line
                content = " " + content
            else:
                # Indent properly
                content = "\n        " + content[:-1].replace("\n", "\n        ") + content[-1]
            result.append(content)

        if len(result) == 0: return ""
        return "".join(result)

def repr_acceptance_id(Value, PatternStrF=True):
    if   Value == E_AcceptanceIDs.VOID:                       return "last_acceptance"
    elif Value == E_AcceptanceIDs.FAILURE:                    return "Failure"
    elif Value == E_AcceptanceIDs.TERMINAL_PRE_CONTEXT_CHECK: return "PreContextCheckTerminated"
    elif Value >= 0:                                    
        if PatternStrF: return "Pattern%i" % Value
        else:           return "%i" % Value
    else:                                               assert False

def repr_position_register(Register):
    if Register == E_PostContextIDs.NONE: return "position[Acceptance]"
    else:                                 return "position[PostContext_%i] " % Register

def repr_pre_context_id(Value):
    if   Value == E_PreContextIDs.NONE:          return "Always"
    elif Value == E_PreContextIDs.BEGIN_OF_LINE: return "BeginOfLine"
    elif Value >= 0:                             return "PreContext_%i" % Value
    else:                                        assert False

def repr_positioning(Positioning, PositionRegisterID):
    if   Positioning == E_TransitionN.VOID: 
        return "pos = %s;" % repr_position_register(PositionRegisterID)
    elif Positioning == E_TransitionN.LEXEME_START_PLUS_ONE: 
        return "pos = lexeme_start_p + 1; "
    elif Positioning > 0:   
        return "pos -= %i; " % Positioning
    elif Positioning == 0:  
        return ""
    else: 
        assert False

