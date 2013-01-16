from   quex.blackboard          import setup as Setup, \
                                       E_StateIndices
from   quex.engine.misc.file_in import error_msg

from   collections              import defaultdict, namedtuple
from   operator                 import attrgetter, itemgetter
from   copy                     import deepcopy, copy
from   itertools                import combinations, islice, izip

class TransitionAction(object):
    __slots__ = ("transition_id", "command_list")
    def __init__(self, StateIndex, FromStateIndex, TheCommandList=None):
        assert TheCommandList is None or isinstance(TheCommandList, CommandList)

        self.transition_id = TransitionID(StateIndex, FromStateIndex)
        if TheCommandList is not None: self.command_list = TheCommandList
        else:                          self.command_list = CommandList()
 
    def clone(self):
        return TransitionAction(self.transition_id.state_index, 
                                self.transition_id.from_state_index, 
                                self.command_list.clone())

    # Make TransitionAction usable for dictionary and set
    def __hash__(self):      
        return hash(self.transition_id)

    def __eq__(self, Other):
        if self.transition_id != Other.transition_id: return False
        return self.command_list == Other.command_list

    def __repr__(self):
        command_list_str = repr(self.command_list)
        return "(f: %s, a: [%s])" % (self.transition_id, command_list_str)

class TransitionID(object):
    """An 'advanced' implementation of a 'transition_id' that includes
       the state which is entered. Objects of this type can be used whenever
       a 'transition_id' is required with which an command_list is associated.

       Objects of this type are for example used in TemplateState objects.
    """
    __slots__ = ("state_index", "from_state_index")
    def __init__(self, StateIndex, FromStateIndex):
        assert isinstance(StateIndex, (int, long)) or StateIndex in E_StateIndices
        assert isinstance(FromStateIndex, (int, long)) or FromStateIndex in E_StateIndices
        self.state_index      = StateIndex
        self.from_state_index = FromStateIndex
    def precedes(self, Other):
        """This function helps sorting transition ids. It is not considered
           critical in the sense that it changes WHAT is going on. So, 
           the assumption that 'self' precedes 'Other' if both are equal
           does not do any harm. It safes some time, though at the place
           where this function is used (see  get_best_common_command_list()).
        """
        if   self.from_state_index < Other.from_state_index: return True
        elif self.from_state_index > Other.from_state_index: return False
        elif self.state_index < Other.state_index:           return True
        elif self.state_index > Other.state_index:           return False
        else:                                                return True 

    def __hash__(self):
        if isinstance(self.from_state_index, (int, long)): 
            xor_sum = self.from_state_index + 1
        else:         
            xor_sum = 0
        if isinstance(self.state_index, (int, long)): 
            xor_sum ^= self.state_index + 1
        return xor_sum
    def __eq__(self, Other):
        if not isinstance(Other, TransitionID): return False
        return     self.from_state_index == Other.from_state_index \
               and self.state_index      == Other.state_index
    def __repr__(self):
        return "TransitionID(to=%s, from=%s)" % (self.state_index, self.from_state_index)

class CommandList:
    def __init__(self, TheCommandList=None, TheAccepter=None):
        self.accepter = TheAccepter
        self.misc     = set()
        if TheCommandList is not None:
            for action in TheCommandList:
                if isinstance(action, Accepter):
                    assert self.accepter is None
                    self.accepter = action
                else:
                    self.misc.add(action)

    def difference_update(self, CommandList):
        if self.accepter == CommandList.accepter: self.accepter = None
        self.misc.difference_update(CommandList.misc)

    def is_empty(self):
        if self.accepter is not None: return False
        return len(self.misc) == 0

    def has_action(self, X):
        if isinstance(X, Accepter): return self.accepter == X
        return X in self.misc

    def clone(self):
        if self.accepter is None: accepter = None
        else:                     accepter = self.accepter.clone()
        result = CommandList(TheAccepter    = accepter, 
                             TheCommandList = (x.clone() for x in self.misc))
        return result

    def cost(self):
        return sum(x.cost() for x in self)

    def delete_SetPathIterator_commands(self):
        """Delete the 'SetPathIterator' command from the command list. There should
           never be more than ONE such command in a commant list. This is so, because
           the 'SetPathIterator' defines a state that the MegaState shall represent.
           A MegaState can only represent on state at a time.
        """
        for element in self.misc:
            if isinstance(element, SetPathIterator): 
                self.misc.remove(element)
                break
        else:
            return

        # Double check that there was only one SetPathIterator command in the list.
        for element in self.misc:
            assert not isinstance(element, SetPathIterator)

    def __iter__(self):
        """Allow iteration over comand list."""
        if self.accepter is not None: 
            yield self.accepter
        def sort_key(X):
            if   isinstance(X, StoreInputPosition): 
                return (0, X.pre_context_id, X.position_register, X.offset)
            elif isinstance(X, PreConditionOK):   
                return (1, X.pre_context_id)
            elif isinstance(X, SetTemplateStateKey):   
                return (2, X.value)
            elif isinstance(X, SetPathIterator):   
                return (3, X.offset, X.path_id, X.path_walker_id)
            else:
                assert False, "Command '%s' cannot be part of .misc." % X.__class__.__name__

        for action in sorted(self.misc, key=sort_key):
            yield action

    def __hash__(self):
        xor_sum = 0
        for x in self.misc:
            xor_sum ^= hash(x)

        if self.accepter is not None:
            xor_sum ^= hash(self.accepter)

        return xor_sum

    def is_equivalent(self, Other):
        """For 'equivalence' the commands 'MegaState_Control' are unimportant."""
        # Rely on '__eq__' of Accepter
        if not (self.accepter == Other.accepter): return False
        self_misc_pure  = set(x for x in self.misc  if not isinstance(x, MegaState_Control))
        Other_misc_pure = set(x for x in Other.misc if not isinstance(x, MegaState_Control))
        return self_misc_pure == Other_misc_pure

    def __eq__(self, Other):
        # Rely on '__eq__' of Accepter
        if not (self.accepter == Other.accepter): return False
        return self.misc == Other.misc

    def __repr__(self):
        txt = ""
        if self.accepter is not None:
            txt += "a"
            for x in self.accepter:
                txt += "%s," % repr(x.pattern_id)

        for action in self.misc:
            txt += "%s" % action
        return txt

class Command(object):
    @staticmethod
    def type_id():           assert False
    def priority(self):      assert False
    def clone(self):         assert False, "Derived class must implement 'clone()'"
    def cost(self):          assert False, "Derived class must implement 'cost()'"
    def __hash__(self):      assert False, "Derived class must implement '__hash__()'"
    def __eq__(self, Other): assert False, "Derived class must implement '__eq__()'"

class StoreInputPosition(Command):
    """
    StoreInputPosition: 

    Storing the input position is actually dependent on the pre_context_id, if 
    there is one. The pre_context_id is left out for the following reasons:

    -- Testing the pre_context_id is not necessary.
       If a pre-contexted acceptance is reach where the pre-context is required
       two things can happen: 
       (i) Pre-context-id is not fulfilled, then no input position needs to 
           be restored. Storing does no harm.
       (ii) Pre-context-id is fulfilled, then the position is restored. 

    -- Avoiding overhead for pre_context_id test.
       In case (i) cost = test + jump, (ii) cost = test + assign + jump. Without
       test (i) cost = assign, (ii) cost = storage. Assume cost for test <= assign.
       Thus not testing is cheaper.

    -- In the process of register economization, some post contexts may use the
       same position register. The actions which can be combined then can be 
       easily detected, if no pre-context is considered.
    """
    __slots__ = ("pre_context_id", "position_register", "offset")
    def __init__(self, PreContextID, PositionRegister, Offset):
        self.pre_context_id    = PreContextID
        self.position_register = PositionRegister
        self.offset            = Offset

    def clone(self):
        return StoreInputPosition(self.pre_context_id, self.position_register, self.offset)

    # Require 'type_id' and 'priority' for sorting of entry actions.
    @staticmethod
    def type_id():      return 0
    def priority(self): return - self.position_register

    # Estimate the cost of the 'store input position':
    # (Assume, we do not check for pre_context_id, see above)
    # = 1 x Assignment
    def cost(self):     return 1

    # Require '__hash__' and '__eq__' to be element of a set.
    def __hash__(self):
        if isinstance(self.position_register, (int, long)):
            return self.position_register
        else:
            return -1

    def __eq__(self, Other):
        if not isinstance(Other, StoreInputPosition): return False
        return     self.pre_context_id    == Other.pre_context_id \
               and self.position_register == Other.position_register \
               and self.offset            == Other.offset           

    def __cmp__(self, Other):
        if   self.pre_context_id    > Other.pre_context_id:    return 1
        elif self.pre_context_id    < Other.pre_context_id:    return -1
        elif self.position_register > Other.position_register: return 1
        elif self.position_register < Other.position_register: return -1
        elif self.offset            > Other.offset:            return 1
        elif self.offset            < Other.offset:            return -1
        return 0

    def __repr__(self):
        # if self.pre_context_id != E_PreContextIDs.NONE:
        #     if_txt = "if '%s': " % repr_pre_context_id(self.pre_context_id)
        # else:
        #     if_txt = ""
        #
        # if self.offset == 0:
        #     return "%s%s = input_p;\n" % (if_txt, repr_position_register(self.position_register))
        # else:
        #     return "%s%s = input_p - %i;\n" % (if_txt, repr_position_register(self.position_register), 
        #                                        self.offset))
        return "pre(%s) --> store[%i] = input_p - %i;\n" % \
               (self.pre_context_id, self.position_register, self.offset) 

class PreConditionOK(Command):
    __slots__ = ["__pre_context_id"]
    def __init__(self, PreContextID):
        self.__pre_context_id = PreContextID
    def clone(self):
        return PreConditionOK(self.__pre_context_id)
    @property
    def pre_context_id(self): 
        return self.__pre_context_id
    def cost(self):     
        return 1
    def __hash__(self):       
        if isinstance(self.__pre_context_id, (int, long)): return self.__pre_context_id
        else:                                              return -1
    def __eq__(self, Other):  
        if not isinstance(Other, PreConditionOK): return False
        return self.__pre_context_id == Other.__pre_context_id
    def __repr__(self):       
        return "    pre-context-fulfilled = %s;\n" % self.__pre_context_id

# Accepter:
# 
# In this case the pre-context-id is essential. We cannot accept a pattern if
# its pre-context is not fulfilled.
AccepterElement = namedtuple("AccepterElement", ("pre_context_id", "pattern_id"))
class Accepter(Command):
    __slots__ = ["__list"]
    def __init__(self, PathTraceList=None):
        if PathTraceList is None: 
            self.__list = []
        else:
            self.__list = [ AccepterElement(x.pre_context_id, x.pattern_id) for x in PathTraceList ]

    def clone(self):
        result = Accepter()
        result.__list = [ deepcopy(x) for x in self.__list ]
        return result
    
    def add(self, PreContextID, PatternID):
        self.__list.append(AccepterElement(PreContextID, PatternID))

    def insert_front(self, PreContextID, PatternID):
        self.__list.insert(0, AccepterElement(PreContextID, PatternID))

    # Require 'type_id' and 'priority' for sorting of entry actions.
    @staticmethod
    def type_id():      return 1
    def priority(self): return len(self.__list)

    # Estimate cost for the accepter:
    # pre-context check + assign acceptance + conditional jump: 3
    # assign acceptance:                                        1
    def cost(self):
        result = 0
        for action in self.__list:
            if action.pre_context_id: result += 3
            else:                     result += 1
        return result

    # Require '__hash__' and '__eq__' to be element of a set.
    def __hash__(self): 
        xor_sum = 0
        for x in self.__list:
            if isinstance(x.pattern_id, (int, long)): xor_sum ^= x.pattern_id
        return xor_sum

    def __eq__(self, Other):
        if not isinstance(Other, Accepter):      return False
        if len(self.__list) != len(Other.__list):       return False
        for x, y in zip(self.__list, Other.__list):
            if   x.pre_context_id != y.pre_context_id:  return False
            elif x.pattern_id     != y.pattern_id:      return False
        return True

    def __iter__(self):
        for x in self.__list:
            yield x

    def __repr__(self):
        return "".join(["pre(%s) --> accept(%s)\n" % (element.pre_context_id, element.pattern_id) \
                       for element in self.__list])

class MegaState_Control:
    """This class exists for the sole sense to identify 'MegaState_Control' commands
       such as 'SetTemplateStateKey' and 'SetPathIterator'.
    """
    pass

class SetTemplateStateKey(Command, MegaState_Control):
    __slots__ = ("__value")
    def __init__(self, StateKey):
        self.__value = StateKey
    def clone(self):
        return SetTemplateStateKey(self.__value)
    @property
    def value(self): 
        return self.__value
    def set_value(self, Value): 
        assert isinstance(Value, (int, long))
        self.__value = Value
    def cost(self):     return 1 
    def __hash__(self):       
        if isinstance(self.__value, (int, long)): return self.__value
        else:                                     return -1
    def __eq__(self, Other):  
        if not isinstance(Other, SetTemplateStateKey): return False
        return self.__value == Other.__value
    def __repr__(self):       
        return "    state_key = %s;\n" % self.__value

class SetPathIterator(Command, MegaState_Control):
    __slots__ = ("__path_walker_id", "__path_id", "__offset")
    def __init__(self, Offset, PathWalkerID=-1, PathID=-1):
        self.__offset         = Offset
        self.__path_walker_id = PathWalkerID   # Let it be '-1' to be able to be hashed easily
        self.__path_id        = PathID         #                   - " -

    def clone(self):
        return SetPathIterator(self.__offset, self.__path_walker_id, self.__path_id)

    def cost(self):     return 1 

    def set_path_walker_id(self, Value): 
        self.__path_walker_id = Value

    def set_path_id(self, Value):        
        self.__path_id = Value

    @property
    def path_walker_id(self): return self.__path_walker_id
    @property
    def path_id(self):        return self.__path_id
    @property
    def offset(self):         return self.__offset

    def __hash__(self):       
        return self.__path_walker_id ^ self.__path_id ^ self.__offset

    def __eq__(self, Other):  
        if not isinstance(Other, SetPathIterator): return False
        return     self.__path_walker_id == Other.__path_walker_id \
               and self.__path_id        == Other.__path_id        \
               and self.__offset         == Other.__offset

    def __repr__(self):       
        return "    (pw=%s,pid=%s,off=%s)\n" % (self.__path_walker_id, self.__path_id, self.__offset)

class Door:
    """A 'Door' is a node in a door tree. A door tree is constructed from 
       a set of command lists, each command list is to be executed upon 
       entry from another state. For example,


          From State A:                      From State B:

          last_acceptance = 12;              last_acceptance = 12;
          position[0]     = input_p - 2;     position[0] = input_p - 2;
          position[1]     = input_p;         position[1] = input_p - 1;


       In this case, the entry from A and B have the following commands in common:

                          last_acceptance = 12;
                          position[0]     = input_p - 2;

       The following door tree can be constructed:


          Door 2:                        Door 1:     
          position[1] = input_p;         position[1] = input_p - 1;
          goto Door 0;                   goto Door 0;


                           Door 0:
                           last_acceptance = 12;
                           position[0]     = input_p - 2;
                           (...  to transition map ... )

       The entry from state 'A' happens through door 2 and the entry from state
       'B' happens through door 1. Door 1 and 2 store door 0 as their parent.
       By means of the the '.parent' and '.child_list' members the tree
       structure is maintained. The relation from which state one comes and
       through which door one enters is stored in the global variable
       'Door.transition_id_to_door_id_db', it maps:

                  transition_id ---> door_identifier
    """
    state_index                      = None
    id_counter                       = 0
    transition_id_to_door_id_db      = {} # map: TransitionID --> DoorID
    door_id_to_transition_id_list_db = {} # map: DoorID       --> list of TransitionID

    def __init__(self, Parent, CommonCommandList, TransitionActionList):
        self.door_id             = DoorID(Door.state_index, Door.id_counter)
        self.parent              = Parent  
        self.child_list          = []
        self.common_command_list = CommonCommandList

        Door.register(self.door_id, TransitionActionList)

    @staticmethod
    def register(DoorIdentifier, TransitionActionList):
        transition_id_list = [x.transition_id for x in TransitionActionList]
        Door.transition_id_to_door_id_db.update((i, DoorIdentifier) for i in transition_id_list)
        # There might be a door with zero transition actions related to it: The root door,
        # if there are no common actions at all between the command lists.
        # Thus: It is OK that a door might be assigned an empty transition_id_list.
        Door.door_id_to_transition_id_list_db[DoorIdentifier] = transition_id_list
        Door.id_counter += 1

    @staticmethod
    def init(StateIndex):
        Door.state_index = StateIndex
        Door.id_counter = 0
        Door.transition_id_to_door_id_db.clear()
        Door.door_id_to_transition_id_list_db.clear()

    def is_equivalent(self, Other):
        """If -- the 'Other' door tree node does exactly the same common actions,
              (-- has the same door_id,)
              -- and all its childs are equivalent,
           then the node is equivalent to the other.
        """
        # 'is_equivalent' does not care about 'MegaState_Control' commands
        if not self.common_command_list.is_equivalent(Other.common_command_list): 
            return False

        if len(self.child_list) != len(Other.child_list): return False
        for child, other_child in izip(sorted(self.child_list, key=attrgetter("door_id")),
                                       sorted(Other.child_list,key=attrgetter("door_id"))):
            if not child.is_equivalent(other_child): return False
        return True

    def has_commands_other_than_MegaState_Control(self):
        # __dive --> here we have recursion
        for found in (x for x in self.common_command_list if not isinstance(x, MegaState_Control)):
            return True

        for child in self.child_list:
            if child.has_commands_other_than_MegaState_Control(): return True

        return False

    def __eq__(self, Other):
        return id(self) == id(Other)

    def __repr__(self):
        assert False, "Use 'get_string()'"

    def get_string(self, DoorID_to_TransitionID_DB, OnlyFromStateIndexF=False):
        """DoorID_to_TransitionID_DB can be received, for example from the 
           'entry.transition_db' object.
        """
        txt = []
        for child in sorted(self.child_list, key=attrgetter("door_id")):
            txt.append("%s\n" % child.get_string(DoorID_to_TransitionID_DB))

        txt.append("[%i]: " % self.door_id.door_index)
        transition_id_list = DoorID_to_TransitionID_DB[self.door_id]
        for transition_id in sorted(transition_id_list, key=attrgetter("state_index", "from_state_index")):
            if OnlyFromStateIndexF:
                txt.append("(%s) " % transition_id.from_state_index)
            else:
                txt.append("(%s<-%s) " % (transition_id.state_index, transition_id.from_state_index))
        txt.append("\n")
        for action in self.common_command_list:
            action_str = repr(action)
            action_str = "    " + action_str[:-1].replace("\n", "\n    ") + action_str[-1]
            txt.append(action_str)

        if self.parent is None: txt.append("parent: [None]\n")
        else:                   txt.append("parent: [%s]\n" % repr(self.parent.door_id.door_index))
        return "".join(txt)

    @staticmethod
    def replace_node_by_parent(Node):
        parent = Node.parent
        assert parent is not None

        # Remove node as child of parent
        del parent.child_list[parent.child_list.index(Node)]

        # Connect childs of node to its parents
        # -- The parent of node becomes the parent of all childs.
        for child in Node.child_list:
            child.parent = parent
        # -- The childs of node become childs of the node's parent.
        parent.child_list.extend(Node.child_list)

        # Relate the node's transition_ids to the parent door:
        #  -- update door_id_to_transition_id_list_db
        #  -- update transition_id_to_door_id_db
        transition_id_list = Door.door_id_to_transition_id_list_db[Node.door_id]
        Door.transition_id_to_door_id_db.update((transition_id, parent.door_id) \
                                                for transition_id in transition_id_list)
        Door.door_id_to_transition_id_list_db[parent.door_id].extend(transition_id_list)
        del Door.door_id_to_transition_id_list_db[Node.door_id]

class DoorID(object):
    __slots__ = ("__state_index", "__door_index")
    def __init__(self, StateIndex, DoorIndex):
        assert isinstance(StateIndex, (int, long)) or StateIndex in E_StateIndices
        # 'DoorIndex is None' --> right after the entry commands (targetted after reload).
        assert isinstance(DoorIndex, (int, long))  or DoorIndex is None
        self.__state_index = StateIndex
        self.__door_index  = DoorIndex
    @property
    def state_index(self): return self.__state_index
    @property
    def door_index(self): return self.__door_index

    def set(self, Other):
        self.__state_index = Other.__state_index
        self.__door_index  = Other.__door_index

    def clone(self):
        return DoorID(self.__state_index, self.__door_index)
    def __hash__(self):
        if isinstance(self.__state_index, (int, long)): xor_sum = self.__state_index + 1
        else:                                           xor_sum = 0
        xor_sum ^= self.__door_index 
        return xor_sum
    def __eq__(self, Other):
        if not isinstance(Other, DoorID): return False
        return     self.__state_index == Other.__state_index \
               and self.__door_index  == Other.__door_index
    def __cmp__(self, Other):
        if not isinstance(Other, DoorID): return -1
        result = cmp(self.__state_index, Other.__state_index)
        if result != 0: return result
        return cmp(self.__door_index, Other.__door_index)
    def __repr__(self):
        return "DoorID(s=%s, d=%s)" % (self.__state_index, self.__door_index)

def categorize_command_lists(StateIndex, TransitionActionList):
    """Better clone the TransitionActionList before calling this function, 
       if the content as it is is till required later.

       Note: 'Door 0' is supposed to be the node without any commands.
             This is used for 'after reload', where the state is entered
             a second time with freshly loaded data.
    """
    Door.init(StateIndex)
    root         = Door(None, CommandList(), [])
    parent       = root

    ## empties, non_sharing, pending_list = pre_investigate(TransitionActionList)
    pending_list = TransitionActionList

    # All empty transition actions are associated with the root node.
    ##for TA in empties:
    ##    Door.door_id_to_transition_id_list_db[root.door_id].append(TA.transition_id)
    ##    Door.transition_id_to_door_id_db[TA.transition_id] = root.door_id

    # All non-sharing actions get their own door.
    ##for TA in non_sharing:
    ##    door = Door(parent, TA.command_list, [TA])
    ##    parent.child_list.append(door)

    # All pending are subject to investigation
    work_list    = [ (parent, pending_list) ]
    while len(work_list) != 0:
        parent, pending_list = work_list.pop()

        # (*) Find Best Common Command List
        #
        # Consider the command list that is the biggest and shared by the 
        # most transition actions. 
        shared_command_list, \
        sharing_transition_action_list = get_best_common_command_list(pending_list)

        # (*) Configure Node in Door Tree (if possible)
        if len(sharing_transition_action_list) != 0:
            # -- Some common commands have been found
            # 
            #    done:    The 'shared_command_lists' are placed in a Door object.
            #    pending: The 'sharing_transition_action_list' may have remaining 
            #             commands which are not in 'shared_command_lists'. 
            #             -- The 'shared_command_lists' are cut out of their command lists.
            #             -- They are put on the 'work_list' as pending 
            #             -- Their parent is the sharing door.
            done    = []
            pending = []
            for transition_action in sharing_transition_action_list:
                transition_action.command_list.difference_update(shared_command_list)
                if transition_action.command_list.is_empty(): done.append(transition_action)
                else:                                         pending.append(transition_action)

            sharing_door = Door(parent, shared_command_list, done)
            parent.child_list.append(sharing_door)
            if len(pending) != 0:
                work_list.append((sharing_door, pending))

        elif shared_command_list.is_empty():
            # -- No common action between any of the comand lists 
            #    => every transition_action that is pending gets its own Door
            def sort_key(TA):
                return (TA.transition_id.from_state_index, TA.transition_id.state_index)
            parent.child_list.extend(Door(parent, x.command_list, TransitionActionList=[x]) \
                                     for x in sorted(pending_list, key=sort_key))
            continue

        # (*) Handle Transition Actions that do not share the 'shared_command_list'
        #     => All of them are pending, 
        #        the parent is the current one.
        if len(sharing_transition_action_list) != len(pending_list):
            non_sharing_transition_action_list = [ x for x in pending_list \
                                                     if x not in sharing_transition_action_list ]
            work_list.append((parent, non_sharing_transition_action_list))

        # Sort by the number of involved doors
        work_list.sort(key=itemgetter(1))

    assert root is not None
    remove_empty_nodes(root)
    assert root is not None

    return copy(Door.transition_id_to_door_id_db), \
           copy(Door.door_id_to_transition_id_list_db), \
           root

def pre_investigate(TransitionActionList):
    """Categorize the TransitionActions into one of three kinds:

       -- 'empties' where there is no action whatsoever.

       -- 'non_sharing' which are CommandList-s that do not share
          any command with any other.

       -- 'remainder' which do not fall into 'empties' or 'non_sharing'.
    """
    empties     = []
    non_sharing = []
    remainder   = set()
    done_set    = set()
    L           = len(TransitionActionList)
    for i, x in enumerate(TransitionActionList):
        if x.command_list.is_empty(): 
            empties.append(x)
            continue
        elif i in done_set: 
            continue

        shared_f = False
        for k in xrange(i+1, L):
            y = TransitionActionList[k]
            if y.command_list.is_empty(): 
                empties.append(y)
                done_set.add(k)     
                continue
            for y_action in y.command_list:
                if not x.command_list.has_action(y_action): continue
                shared_f = True
                remainder.add(i)
                remainder.add(k)
                done_set.add(k) # 'k' has proven to share
                #               # 'i' won't appear anyway a second time
                break
            if shared_f: break
        if not shared_f: 
            non_sharing.append(x)

    return empties, non_sharing, [TransitionActionList[i] for i in remainder]

def get_best_common_command_list(TransitionActionList):
    """
    RETURNS: [0] --> command list of the best transition action combination.
             [1] --> best transition action combination

    Avoid the complexity of iterating over all combinations. The 
    exponentiality of its nature could literally explode the computation
    time. Approach: Loop over all combinations of two doors, then 
    collect all doors that may match a certain command list, i.e.

         count_db:  command_list -->  doors that have all its actions
                                      (and may be other actions).

    If no common command list is determined, then the transition actions
    with no commands are the 'sharing_transition_action_list' of an empty 
    'shared_command_list'.
    """
    def get_common_command_list(Al, Bl):
        return CommandList(action for action in Al if Bl.has_action(action))

    cmdlist_db = defaultdict(set)
    # Map: CommandList --> TransitionAction-s that do exactly that CommandList
    for x in TransitionActionList:
        cmdlist_db[x.command_list].add(x)

    # Shortcut: If all transitions require exactly the same CommandList,
    #           then the result can be computed quickly: best = all common.
    if len(cmdlist_db) == 1:
        return TransitionActionList[0].command_list.clone(), [x for x in TransitionActionList]

    if len(cmdlist_db) > Setup.state_entry_analysis_complexity_limit:
        # PROBLEM:  Computation time propotional to square of size!
        #           ONLY A PROBLEM IN VERY EXTREME CASES WHERE size >> 1000.
        # In such cases, live with a possibly sub-optimal solution. Reduce
        # the set of candidates to what may be a good guess: The 1000 
        # command_lists with the most doors related to them.
        error_msg("The limit of state entry action complexity has been exceeded. To be \n"
                  "able to handle the analysis, Quex decided to reduce the complexity with\n"     
                  "the risk that the result may not be the absolute optimum. Currently,\n" 
                  "this value is set to %i. It can be modified with the command line\n"      
                  "option '--state-entry-analysis-complexity-limit'." \
                  % Setup.state_entry_analysis_complexity_limit,
                  DontExitF=True)
        iterable = sorted(cmdlist_db.iteritems(), key=lambda x: len(x[1]))
        iterable = islice(iterable, Setup.state_entry_analysis_complexity_limit)
        cmdlist_db = dict( iterable )

    count_db = defaultdict(set) # map: command_list --> 'cost' of comand list
    cost_db  = {}               # map: command_list --> set of doors that share it.
    for command_list, ta_list in cmdlist_db.iteritems():
        if len(ta_list) > 1: 
            count_db[command_list].update(ta_list)
            cost_db[command_list] = command_list.cost()
    
    for x, y in combinations(cmdlist_db.iteritems(), 2):
        x_command_list, x_ta_list = x
        y_command_list, y_ta_list = y
        common_command_list = get_common_command_list(x_command_list, y_command_list)
        action_cost         = cost_db.get(common_command_list)
        if action_cost is None: 
            action_cost                  = common_command_list.cost() 
            cost_db[common_command_list] = action_cost
            # TODO:
            # The current combination can achieve at least a 'cost' of action_cost * 2.
            # A door in a combination can at best achieve a cost of cost(all actions) * N
            # if all other doors have the same comand list. Therefore, any door where 
            # cost(all action_cost) * N < best_action_cost * 2 can be dropped.
        entry = count_db[common_command_list]
        entry.update(x_ta_list)
        entry.update(y_ta_list)

    # Determine the best combination: max(cost * door number)
    def __precedence(BestCombination, Combination):
        """Purpose of this function is to define a certain 'order' where 
           TransitionAction combination may be preferred according to their
           transition_id. This has the sole advantage that the result becomes
           more human readable. It does not influence the quality of the 
           optimization result.

           Determine precedence based on the numeric transition_id. Take
           the 'most preceding' transition id of each TransitionAction list.
        """
        if BestCombination is None: return False
        # Take just the highest precedence of each TransitionID mentioned
        def __best(TAList):
            assert len(TAList) != 0
            result = None
            for transition_action in TAList:
                if result is None or transition_action.transition_id.precedes(result):
                    result = transition_action.transition_id
            return result

        return __best(BestCombination).precedes(__best(Combination))

    best_cost         = 0
    best_command_list = None   # instance of CommandList 
    best_combination  = None   # list of instances of TransitionAction
    for command_list, combination in count_db.iteritems():
        cost = cost_db[command_list] * len(combination)
        if   cost < best_cost: 
            continue
        elif cost == best_cost and __precedence(best_combination, combination):
            # Allow 'preceding' transition ids to win, even if the cost does
            # not improve, i.e. 'cost == best_cost'. This allows for some sort order
            # of child nodes, where preceding transition ids are always listed
            # first. 
            continue # no improvement, no precedence -> skip

        # Here: either cost > best_cost, or the aforementioned holds.
        best_cost         = cost
        best_command_list = command_list
        best_combination  = combination

    if best_combination is None or best_command_list.is_empty():
        return CommandList(), set(x for x in TransitionActionList if x.command_list.is_empty())
    else:
        return best_command_list.clone(), best_combination
            
def remove_empty_nodes(RootNode):
    """Remove nodes without any command lists from the door tree.
    """
    work_list = [ child for child in RootNode.child_list ]

    while len(work_list) != 0:
        node = work_list.pop()
        work_list.extend(node.child_list)
        if node.common_command_list.is_empty():
            Door.replace_node_by_parent(node)

#def clear_door_tree(RootNode):
#    """Cleaning the door tree considers only one case:
#
#       There is only one child which has no common_commands. Then this 
#       child can be replaced by the root node. This basically means, 
#       that all transitions assigned to it are now assigned to the
#       root node. 
#       
#       Historical Note: 
#
#       Previously a more sophisticated analyzis has been applied. But, the
#       result was only effecting the 'root' node. We need, that there is an
#       entry without any action directly to the place where we access the next
#       input character. Thus this function was abolished, but left in comment
#       below in this file.
#    """
#    assert RootNode.common_command_list.is_empty()
#    if len(RootNode.child_list) != 1: return RootNode
#
#    child = RootNode.child_list[0]
#    if not child.common_command_list.is_empty(): return RootNode
#
#    # If the command list is empty there should be no
#    assert len(child.child_list) == 0
#
#    del RootNode.child_list[0]
#
#    door_id            = child.door_id
#    transition_id_list = Door.door_id_to_transition_id_list_db[door_id] 
#
#    # The only 'transition_id'-s  in the database are the ones in the child node
#    Door.door_id_to_transition_id_list_db.clear()
#    Door.transition_id_to_door_id_db.clear()
#
#    # Assign all transitions to the root node
#    Door.door_id_to_transition_id_list_db[RootNode.door_id] = transition_id_list
#
#    Door.transition_id_to_door_id_db.update((x, RootNode.door_id) \
#                                            for x in transition_id_list)
#    return RootNode
#
#def clear_door_tree(Node):
#    """In the door tree, there might be nodes that have
#       (Was originally applied on the Door.root object, not necessary)
#
#       (1) only one single child
#           (if there is more than one child it is still a necessary 'target'
#            of a goto from one of the childs)
#       (2) no transition action directly related to it.
#           (transition_id_list for door_id is empty)
#       (3) no common commands
#
#       In this case, 
#
#       -- the single child can inherit the node door_id, and
#       -- inherit the comand list of 'Node'
#
#       NOTE: This case is IMPOSSIBLE for any node but the root node!
#
#       (Nevertheless, the recursive solution is left in place commented below,
#       for the case, that circumstances may change and it may become
#       necessary.)
#
#       PROOF: 
#       
#       Let X be a set of TransitionAction-s belonging to a node which is
#       not the root node. The fact that is is a node implies that X have
#       a 'shared_command_list'. If the 'shared_command_list' is not empty,
#       then (3) does not hold. Thus, let us assume that 'shared_command_list'
#       is empty. For the 'sharing_transition_action_list' of an empty
#       'shared_command_list' the door node will be directly related to
#       the transition actions, so (2) cannot hold.
#    """
#    if len(Node.child_list) != 1: return Node
#
#    # Single Child
#    transition_id_list = Door.door_id_to_transition_id_list_db[Node.door_id]
#    if len(transition_id_list) == 0 and Node.common_command_list.is_empty(): 
#        return Door.remove_node(Node, None)
#    return Node
#
# For the above function, the member function below must be added to class Door:
#
#    @staticmethod
#    def remove_node(Node, ChildIndex):
#        """A node may be replaced by its single child, if it does nothing.
#           See function clear_door_tree(...)
#
#           Node       = The node of concern.
#           ChildIndex = Index of the node in the parent's child_list.
#        """
#        assert len(Node.child_list) == 1
#        # Single Child inherits Door door_id and command list. 
#        # Node itself is taken out.
#        inheritor              = Node.child_list[0]
#        old_door_id            = inheritor.door_id
#        old_transition_id_list = Door.door_id_to_transition_id_list_db[old_door_id] 
#        # Take node out of tree by adapting
#        #    -- .parent of the inheritor
#        #    -- .child_list of the .parent
#        inheritor.parent          = Node.parent
#        inheritor.door_id      = Node.door_id
#        if Node.parent is not None:
#            Node.parent.child_list[ChildIndex] = inheritor
#        # Adapt database entries:
#        #    global Door.transition_id_to_door_id_db
#        #    global Door.door_id_to_transition_id_list_db
#        for transition_id, door_id in Door.transition_id_to_door_id_db.iteritems():
#            if door_id != old_door_id: continue
#            Door.transition_id_to_door_id_db[transition_id] = Node.door_id
#        del Door.door_id_to_transition_id_list_db[old_door_id]
#        Door.door_id_to_transition_id_list_db[inheritor.door_id] = old_transition_id_list
#
#        return inheritor
#def RECURIVE_WHICH_IS_CURRENTLY_NOT_REQUIRED_clear_door_tree(Node):
#    """According to PROOF in the aforementioned implementation, no recursive
#       implementation is necessary. This function only exists, so that 
#       in case things change, it may be used again.
#    """
#    def __dive(Node, ChildIndex):
#        inheritor = None
#        for i, child in enumerate(Node.child_list):
#            __dive(child, i)
#
#        if  len(Node.child_list) == 1:
#            # Single Child
#            transition_id_list = Door.door_id_to_transition_id_list_db[Node.door_id]
#            if len(transition_id_list) == 0 and Node.common_command_list.is_empty(): 
#                inheritor = Door.remove_node(Node, ChildIndex)
#
#        return inheritor
#
#    # "ChildIndex is None" will not be considered, since Node.parent is None
#    # for root.
#    result = __dive(Node, None)
#
#    if result is not None: return result
#    else:                  return Node

