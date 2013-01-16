from   quex.engine.analyzer.state.entry  import repr_acceptance_id, repr_pre_context_id, repr_positioning
from   quex.blackboard                   import E_PreContextIDs, E_AcceptanceIDs, E_TransitionN
from   itertools                         import ifilter, imap

class DropOut(object):
    """The general drop-out of a state has the following two 'sub-tasks'

                /* (1) Check pre-contexts to determine acceptance */
                if     ( pre_context_4_f ) acceptance = 26;
                else if( pre_context_3_f ) acceptance = 45;
                else if( pre_context_8_f ) acceptance = 2;
                else                       acceptance = last_acceptance;

                /* (2) Route to terminal / position input pointer. */
                switch( acceptance ) {
                case 2:  input_p -= 10; goto TERMINAL_2;
                case 15: input_p  = post_context_position[4]; goto TERMINAL_15;
                case 26: input_p  = post_context_position[3]; goto TERMINAL_26;
                case 45: input_p  = last_acceptance_position; goto TERMINAL_45;
                }

       The first sub-task is described by the member '.acceptance_checker' which is a list
       of objects of class 'AcceptanceCheckerElement'. An empty list means that
       there is no check and the acceptance has to be restored from 'last_acceptance'.
       
       The second sub-task is described by member '.terminal_router' which is a list of 
       objects of class 'TerminalRouterElement'.

       The exact content of both lists is determined by analysis of the acceptance
       trances.

       NOTE: This type supports being a dictionary key by '__hash__' and '__eq__'.
             Required for the optional 'template compression'.
    """
    __slots__ = ("__acceptance_checker", "__terminal_router", "__hash")

    def __init__(self):
        self.__acceptance_checker = []
        self.__terminal_router    = []
        self.__hash               = None

    @property
    def restore_acceptance_f(self):
        for element in self.__acceptance_checker:
            if element.acceptance_id == E_AcceptanceIDs.VOID: return True
        return False

    def set_acceptance_checker(self, AC):
        assert isinstance(AC, list)
        self.__acceptance_checker = AC
        self.__hash               = None

    def get_acceptance_checker(self):
        return self.__acceptance_checker

    def set_terminal_router(self, TR):
        assert isinstance(TR, list)
        self.__terminal_router = TR
        self.__hash            = None

    def get_terminal_router(self):
        return self.__terminal_router

    def restore_position_f(self, RegisterIndex):
        for element in [x for x in self.__terminal_router if x.position_register == RegisterIndex]:
            return element.positioning == E_TransitionN.VOID
        return False

    def accept(self, PreContextID, PatternID):
        self.__acceptance_checker.append(
             AcceptanceCheckerElement(PreContextID, PatternID))
        self.__hash = None

    def route_to_terminal(self, PatternID, TransitionNSincePositioning):
        self.__terminal_router.append(
             TerminalRouterElement(PatternID, TransitionNSincePositioning))
        self.__hash = None

    def __hash__(self):
        if self.__hash is None:
            h = 0x5A5A5A5A
            for x in self.__acceptance_checker:
                h ^= hash(x.pre_context_id) ^ hash(x.acceptance_id)
            for x in self.__terminal_router:
                h ^= hash(x.positioning) ^ hash(x.acceptance_id)
            self.__hash = h
        return self.__hash

    def __neq__(self, Other):
        assert False, "Use __eq__()"

    def __eq__(self, Other):
        if   self.__acceptance_checker != Other.__acceptance_checker: return False
        elif self.__terminal_router    != Other.__terminal_router:    return False
        return True

    def is_equal(self, Other):
        return self.__eq__(Other)

    def finish(self, PositionRegisterMap):
        for element in self.__terminal_router:
            if element.positioning is not E_TransitionN.VOID: continue
            element.position_register = PositionRegisterMap[element.position_register]

    def trivialize(self):
        """If there is only one acceptance involved and no pre-context,
           then the drop-out action can be trivialized.

           RETURNS: None                          -- if the drop out is not trivial
                    TerminalRouterElement -- if the drop-out is trivial
        """
        if E_AcceptanceIDs.TERMINAL_PRE_CONTEXT_CHECK in imap(lambda x: x.acceptance_id, self.__terminal_router):
            assert len(self.__acceptance_checker) == 1
            assert self.__acceptance_checker[0].pre_context_id == E_PreContextIDs.NONE
            assert self.__acceptance_checker[0].acceptance_id  == E_AcceptanceIDs.VOID
            assert len(self.__terminal_router) == 1
            return [None, self.__terminal_router[0]]

        for dummy in ifilter(lambda x: x.acceptance_id == E_AcceptanceIDs.VOID, self.__acceptance_checker):
            # There is a stored acceptance involved, thus need acceptance_checker + terminal_router.
            return None

        result = []
        for check in self.__acceptance_checker:
            for route in self.__terminal_router:
                if route.acceptance_id == check.acceptance_id: break
            else:
                assert False, \
                       "Acceptance ID '%s' not found in terminal_router.\nFound: %s" % \
                       (check.acceptance_id, map(lambda x: x.acceptance_id, self.__terminal_router))
            result.append((check, route))
            # NOTE: "if check.pre_context_id is None: break"
            #       is not necessary since get_drop_out_object() makes sure that the acceptance_checker
            #       stops after the first non-pre-context drop-out.

        return result

    def __repr__(self):
        if len(self.__acceptance_checker) == 0 and len(self.__terminal_router) == 0:
            return "    <unreachable code>"
        info = self.trivialize()
        if info is not None:
            if len(info) == 2 and info[0] is None:
                return "    goto PreContextCheckTerminated;"
            else:
                txt = []
                if_str = "if"
                for easy in info:
                    if easy[0].pre_context_id == E_PreContextIDs.NONE:
                        txt.append("    %s goto %s;\n" % \
                                   (repr_positioning(easy[1].positioning, easy[1].position_register),
                                    repr_acceptance_id(easy[1].acceptance_id)))
                    else:
                        txt.append("    %s %s: %s goto %s;\n" % \
                                   (if_str,
                                    repr_pre_context_id(easy[0].pre_context_id),
                                    repr_positioning(easy[1].positioning, easy[1].position_register),
                                    repr_acceptance_id(easy[1].acceptance_id)))
                        if_str = "else if"
                return "".join(txt)

        txt = ["    Checker:\n"]
        if_str = "if     "
        for element in self.__acceptance_checker:
            if element.pre_context_id != E_PreContextIDs.NONE:
                txt.append("        %s %s\n" % (if_str, repr(element)))
            else:
                txt.append("        accept = %s\n" % repr_acceptance_id(element.acceptance_id))
                # No check after the unconditional acceptance
                break

            if_str = "else if"

        txt.append("    Router:\n")
        for element in self.__terminal_router:
            txt.append("        %s\n" % repr(element))

        return "".join(txt)

class DropOutBackward(DropOut):
    def __init__(self):
        DropOut.__init__(self)

        DropOut.set_acceptance_checker(self, [AcceptanceCheckerElement(E_PreContextIDs.NONE, 
                                                                       E_AcceptanceIDs.VOID)])
        DropOut.set_terminal_router(self, [TerminalRouterElement(E_AcceptanceIDs.TERMINAL_PRE_CONTEXT_CHECK, 
                                                                 E_TransitionN.IRRELEVANT)])
    def finish(self, PositionRegisterMap):
        pass

class DropOutBackwardInputPositionDetection(object):
    __slots__ = ("__reachable_f")
    def __init__(self, AcceptanceF):
        """A non-acceptance drop-out can never be reached, because we walk a 
           path backward, that we walked forward before.
        """
        self.__reachable_f = AcceptanceF

    def finish(self, PositionRegisterMap):
        pass

    @property
    def reachable_f(self):         return self.__reachable_f

    def __hash__(self):            return self.__reachable_f
    def __eq__(self, Other):       return self.__reachable_f == Other.__reachable_f
    def __repr__(self):
        if not self.__reachable_f: return "<unreachable>"
        else:                      return "<backward input position detected>"

class AcceptanceCheckerElement(object):
    """Objects of this class shall describe a check sequence such as

            if     ( pre_condition_5_f ) last_acceptance = 34;
            else if( pre_condition_7_f ) last_acceptance = 67;
            else if( pre_condition_9_f ) last_acceptance = 31;
            else                         last_acceptance = 11;

       by a list such as [(5, 34), (7, 67), (9, 31), (None, 11)]. Note, that
       the prioritization is not necessarily by pattern_id. This is so, since
       the whole trace is considered and length precedes pattern_id.
    
       The values for .pre_context_id and .acceptance_id are carry the 
       following meaning:

       .pre_context_id   PreContextID of concern. 

                         == None --> no pre-context (normal pattern)
                         == -1   --> pre-context 'begin-of-line'
                         >= 0    --> id of the pre-context state machine/flag

       .acceptance_id    Terminal to be targeted (what was accepted).

                         == None --> acceptance determined by stored value in 
                                     'last_acceptance', thus "goto *last_acceptance;"
                         == -1   --> goto terminal 'failure', nothing matched.
                         >= 0    --> goto terminal given by '.terminal_id'

    """
    __slots__ = ("pre_context_id", "acceptance_id") 

    def __init__(self, PreContextID, AcceptanceID):
        assert    isinstance(AcceptanceID, (int, long)) \
               or AcceptanceID in E_AcceptanceIDs
        self.pre_context_id = PreContextID
        self.acceptance_id  = AcceptanceID

    def __eq__(self, Other):
        return     self.pre_context_id == Other.pre_context_id \
               and self.acceptance_id  == Other.acceptance_id

    def __repr__(self):
        txt = []
        txt.append("%s: accept = %s" % (repr_pre_context_id(self.pre_context_id),
                                        repr_acceptance_id(self.acceptance_id)))
        return "".join(txt)

class TerminalRouterElement(object):
    """Objects of this class shall be elements to build a router to the terminal
       based on the setting 'last_acceptance', i.e.

            switch( last_acceptance ) {
                case  45: input_p -= 3;                   goto TERMINAL_45;
                case  43:                                 goto TERMINAL_43;
                case  41: input_p -= 2;                   goto TERMINAL_41;
                case  33: input_p = lexeme_start_p - 1;   goto TERMINAL_33;
                case  22: input_p = position_register[2]; goto TERMINAL_22;
            }

       That means, the 'router' actually only tells how the positioning has to happen
       dependent on the acceptance. Then it goes to the action of the matching pattern.
       Following elements are provided:

        .acceptance_id    Terminal to be targeted (what was accepted).

                         == -1   --> goto terminal 'failure', nothing matched.
                         >= 0    --> goto terminal given by '.terminal_id'

        .positioning     Adaption of the input pointer, before the terminal is entered.

                         >= 0    
                                   input_p -= .positioning 

                            This is only possible if the number of transitions
                            since acceptance is determined before run time.

                         == E_TransitionN.VOID 
                         
                                   input_p = position[.position_register]

                            Restore a stored input position from its register.
                            A loop appeared on the path from 'store input
                            position' to here.

                         == E_TransitionN.LEXEME_START_PLUS_ONE 
                         
                                   input_p = lexeme_start_p + 1

                            Case of failure (actually redundant information).
    """
    __slots__ = ("acceptance_id", "positioning", "position_register")

    def __init__(self, AcceptanceID, TransitionNSincePositioning):
        assert    TransitionNSincePositioning == E_TransitionN.VOID \
               or TransitionNSincePositioning == E_TransitionN.LEXEME_START_PLUS_ONE \
               or TransitionNSincePositioning == E_TransitionN.IRRELEVANT \
               or TransitionNSincePositioning >= 0
        assert    AcceptanceID in E_AcceptanceIDs \
               or AcceptanceID >= 0

        self.acceptance_id     = AcceptanceID
        self.positioning       = TransitionNSincePositioning
        self.position_register = AcceptanceID                 # May later be adapted.

    def __eq__(self, Other):
        return     self.acceptance_id     == Other.acceptance_id   \
               and self.positioning       == Other.positioning     \
               and self.position_register == Other.position_register

    def __repr__(self):
        if self.acceptance_id == E_AcceptanceIDs.FAILURE: assert self.positioning == E_TransitionN.LEXEME_START_PLUS_ONE
        else:                                             assert self.positioning != E_TransitionN.LEXEME_START_PLUS_ONE

        if self.positioning != 0:
            return "case %s: %s goto %s;" % (repr_acceptance_id(self.acceptance_id, PatternStrF=False),
                                             repr_positioning(self.positioning, self.position_register), 
                                             repr_acceptance_id(self.acceptance_id))
        else:
            return "case %s: goto %s;" % (repr_acceptance_id(self.acceptance_id, PatternStrF=False),
                                          repr_acceptance_id(self.acceptance_id))
        
