import quex.engine.state_machine.index         as index

def __nice(SM_ID): 
    assert isinstance(SM_ID, (long, int))
    return repr(SM_ID).replace("L", "").replace("'", "")
    
db = {}

class AddressDB:
    #-----------------------------------------------------------------------
    #  Maps anything, such as a 'terminal' or anything else to an address.
    #  The address of a state machine state is the state's index. New
    #  addresses are generated using 'index.get()' which produces unique
    #  state indices.
    #-----------------------------------------------------------------------
    def __init__(self):
        self.__db = {}
        self.__special = set([
            "__RELOAD_FORWARD", 
            "__RELOAD_BACKWARD", 
            "__STATE_ROUTER", 
            "__TERMINAL_ROUTER",
            "INIT_STATE_TRANSITION_BLOCK",
            "__REENTRY_PREPARATION", 
            "__REENTRY",
        ])
        self.__direct_transition_db = {}

    def get(self, NameOrTerminalID):
        """NameOrTerminalID is something that identifies a position/address 
                            in the code. This function returns a numeric id
                            for this address. 

           Exceptions are labels that are 'unique' inside a state machine 
           as defined by '__address_db_special'. For those the string itself
           is returned.
        """
        # Special addresses are not treated, but returned as string
        if NameOrTerminalID in self.__special:
            return NameOrTerminalID

        # If the thing is known, return its id immediately
        entry = self.__db.get(NameOrTerminalID)
        if entry is not None: return entry

        # Generate unique id for the label: Use unique state index
        entry = index.get()
        self.__db[NameOrTerminalID] = entry
        return entry

    def set_direct_transitions(self, DirectTransitionDB):
        assert type(DirectTransitionDB) == dict
        self.__direct_transition_db = DirectTransitionDB

    def get_real(self, StateIndex):
        if self.__direct_transition_db.has_key(StateIndex):
            result = self.__direct_transition_db[StateIndex]
            assert type(result) in [int, long]
            return result

        return StateIndex

    def get_entry(self, DoorId):
        assert DoorId.__class__.__name__ == "DoorID", "%s\n%s"  % (repr(DoorId), DoorId.__class__.__name__)
        # TODO: door_index == 0 is a special: The entry into the state without any
        #       commands/actions being applied. This is also the entry after reload.
        #       In this case, the the addess be the state index itself.
        if DoorId.door_index == 0: return DoorId.state_index

        return self.get("%i_from_%s" % (DoorId.state_index, DoorId.door_index))

__address_db = AddressDB()

__label_db = {
    # Let's make one thing clear: addresses of labels are aligned with state indices:
    "$entry":                 lambda DoorId:      __address_db.get_entry(DoorId),
    # 
    "$terminal":              lambda TerminalIdx: __address_db.get("TERMINAL_%s"        % __nice(TerminalIdx)),
    "$terminal-router":       lambda NoThing:     __address_db.get("__TERMINAL_ROUTER"),
    "$terminal-direct":       lambda TerminalIdx: __address_db.get("TERMINAL_%s_DIRECT" % __nice(TerminalIdx)),
    "$terminal-general-bw":   lambda NoThing:     __address_db.get("TERMINAL_GENERAL_BACKWARD"),
    "$terminal-EOF":          lambda NoThing:     __address_db.get("TERMINAL_END_OF_STREAM"),
    "$terminal-FAILURE":      lambda NoThing:     __address_db.get("TERMINAL_FAILURE"),
    #
    "$state-router":          lambda NoThing:     __address_db.get("__STATE_ROUTER"),
    #
    "$reload":                lambda StateIdx:    __address_db.get("STATE_%s_RELOAD"    % __nice(StateIdx)),
    "$reload-FORWARD":        lambda StateIdx:    __address_db.get("__RELOAD_FORWARD"),
    "$reload-BACKWARD":       lambda StateIdx:    __address_db.get("__RELOAD_BACKWARD"),
    "$drop-out":              lambda StateIdx:    __address_db.get("STATE_%s_DROP_OUT" % __nice(StateIdx)),
    "$re-start":              lambda NoThing:     __address_db.get("__REENTRY_PREPARATION"),
    "$start":                 lambda NoThing:     __address_db.get("__REENTRY"),
    "$skipper-reload":        lambda StateIdx:    __address_db.get("__SKIPPER_RELOAD_TERMINATED_%s" % __nice(StateIdx)),
    "$bipd-return":           lambda DetectorID:  __address_db.get("BIPD_%i_RETURN" % DetectorID),
    "$bipd-terminal":         lambda DetectorID:  __address_db.get("BIPD_%i_TERMINAL" % DetectorID),
    "$init_state_fw_transition_block": lambda NoThing: "INIT_STATE_TRANSITION_BLOCK",
}

__referenced_label_set = set([])
def __referenced_label_set_add(Label):
    global __referenced_label_set
    __referenced_label_set.add(Label)
    # If a terminal router is used, then a state router is also required.
    if Label == "__TERMINAL_ROUTER": 
        __referenced_label_set.add("__STATE_ROUTER")

def is_label_referenced(Type, Arg=None):
    global __referenced_label_set
    label = get_label(Type, Arg)
    return label in __referenced_label_set

__routed_address_set = set([])

def init_address_handling(DirectTransitionDB):
    __referenced_label_set.clear()
    __routed_address_set.clear()
    __address_db.set_direct_transitions(DirectTransitionDB)

def get_address_set_subject_to_routing():
    return __routed_address_set

def get_address(Type, Arg=None, U=False, R=False):
    """U -- mark as 'used' if True
       R -- mark as subject to 'routing' if True
    
       RETURNS A NUMBER that can be potentially be used for 
       routing (i.e. "switch( index ) { case N: goto _address; ... "
    """
    global __label_db
    result = __label_db[Type](Arg)

    assert type(result) in [int, long], \
           "Label type '%s' is not suited for routing. Found %s" % (Type, result)
    
    if U: __referenced_label_set_add(get_label_of_address(result))
    if R: __routed_address_set.add(__address_db.get_real(result))

    return result

def get_label(LabelType, Arg=None, U=False, R=False):
    """U -- mark as 'used' if True
       R -- mark as subject to 'routing' if True
    
       RETURNS A STRING, that can be used directly for a goto statement.
    """
    global __label_db
    label_id = __label_db[LabelType](Arg)
    if type(label_id) in [int, long]: result = get_label_of_address(label_id)
    else:                             result = label_id

    assert type(result) in [str, unicode]

    if U: 
        __referenced_label_set_add(result)
    if R: 
        assert type(label_id) in [int, long], \
               "Only labels that expand to addresses can be routed."
        __routed_address_set.add(__address_db.get_real(label_id))

    return result

def get_label_of_address(Adr, U=False):
    """U -- mark as 'used' if True
       (labels cannot be routed => no option 'R')
    
       RETURNS A STRING--the label that belongs to a certain (numeric) address.
    """

    result = "_%s" % __nice(Adr)
    if U: __referenced_label_set_add(result)

    return result

class Address:
    def __init__(self, LabelType, LabelTypeArg=None, Code=None):
        """LabelType, LabelTypeArg --> used to access __address_db.

           Code  = Code that is to be generated, supposed that the 
                   label is actually referred.
                   (May be empty, so that that only the label is not printed.)
        """
        self.label = get_label(LabelType, LabelTypeArg)

        if   Code is None:       self.code = [ self.label, ":\n" ]
        elif type(Code) == list: self.code = Code
        else:                    self.code = [ Code ]

def get_plain_strings(txt_list, RoutingInfoF=True):
    """-- Replaces unreferenced 'Address' objects by empty strings.
       -- Replaces integers by indentation, i.e. '1' = 4 spaces.
    """
    global __referenced_label_set

    size = len(txt_list)
    i    = -1
    while i < size - 1:
        i += 1

        elm = txt_list[i]

        if type(elm) in [int, long]:    
            # Indentation: elm = number of indentations
            txt_list[i] = "    " * elm

        elif not isinstance(elm, Address): 
            # Text is left as it is
            continue

        elif elm.label in __referenced_label_set: 
            # If an address is referenced, the correspondent code is inserted.
            txt_list[i:i+1] = elm.code
            # txt_list = txt_list[:i] + elm.code + txt_list[i+1:]
            size += len(elm.code) - 1
            i    -= 1
        else:
            # If an address is not referenced, the it is replaced by an empty string
            txt_list[i] = ""

    return txt_list

