from   quex.engine.generator.languages.address   import Address
from   quex.blackboard                           import E_EngineTypes, E_AcceptanceIDs,  E_StateIndices, \
                                                        E_TransitionN, E_PostContextIDs, E_PreContextIDs, \
                                                        setup as Setup

def do(txt, TheState, TheAnalyzer, DefineLabelF=True, MentionStateIndexF=True):
    LanguageDB          = Setup.language_db

    if DefineLabelF:
        txt.append(Address("$drop-out", TheState.index))

    if MentionStateIndexF:
        txt.append("    __quex_debug_drop_out(%i);\n" % TheState.index)

    if TheAnalyzer.engine_type == E_EngineTypes.BACKWARD_PRE_CONTEXT:
        txt.append("    %s\n" % LanguageDB.GOTO(E_StateIndices.END_OF_PRE_CONTEXT_CHECK))
        return

    elif TheAnalyzer.engine_type == E_EngineTypes.BACKWARD_INPUT_POSITION:
        if TheState.drop_out.reachable_f:
            # Backward input position detectors are always isolated state machines.
            # => TheAnalyzer.state_machine_id = id of the backward input position detector.
            txt.append('    __quex_debug("backward input position %i detected\\n");\n' % \
                       TheAnalyzer.state_machine_id)
            txt.append("    %s\n\n" % LanguageDB.INPUT_P_INCREMENT())
            txt.append("    goto %s;\n" \
                       % LanguageDB.LABEL_NAME_BACKWARD_INPUT_POSITION_RETURN(TheAnalyzer.state_machine_id))
        return

    info = TheState.drop_out.trivialize()
    # (1) Trivial Solution
    if info is not None:
        for i, easy in enumerate(info):
            positioning_str = ""
            if easy[1].positioning != 0:
                if easy[1].positioning == E_TransitionN.VOID: register = easy[1].position_register
                else:                                         register = E_PostContextIDs.NONE
                positioning_str = "%s\n" % LanguageDB.POSITIONING(easy[1].positioning, register)

            goto_terminal_str = "%s" % LanguageDB.GOTO_TERMINAL(easy[1].acceptance_id)
            txt.append(LanguageDB.IF_PRE_CONTEXT(i == 0, easy[0].pre_context_id, 
                                                 "%s%s" % (positioning_str, goto_terminal_str)))
        return

    # (2) Separate: Pre-Context Check and Routing to Terminal
    # (2.1) Pre-Context Check
    for i, element in enumerate(TheState.drop_out.get_acceptance_checker()):
        if     element.pre_context_id == E_PreContextIDs.NONE \
           and element.acceptance_id  == E_AcceptanceIDs.VOID: 
               break
        txt.append(
            LanguageDB.IF_PRE_CONTEXT(i == 0, element.pre_context_id, 
                                      LanguageDB.ASSIGN("last_acceptance", 
                                                        LanguageDB.ACCEPTANCE(element.acceptance_id)))
        )
        if element.pre_context_id == E_PreContextIDs.NONE: 
            break # No check after the unconditional acceptance

    # (2.2) Routing to Terminal
    # (2.2.1) If the positioning is the same for all entries (except the FAILURE)
    #         then, again, the routing may be simplified:
    #router    = TheState.drop_out.router
    #prototype = (router[0].positioning, router[0].position_register)
    #simple_f  = True
    #for element in islice(router, 1, None):
    #    if element.acceptance_id == E_AcceptanceIDs.FAILURE: continue
    #    if prototype != (element.positioning, element.position_register): 
    #        simple_f = False
    #        break

    #if simple_f:
    #    txt.append("    %s\n    %s\n" % 
    #               (LanguageDB.POSITIONING(element.positioning, element.position_register), 
    #                LanguageDB.GOTO_TERMINAL(E_AcceptanceIDs.VOID)))
    #else:
    case_list = []
    for element in TheState.drop_out.get_terminal_router():
        if element.positioning == E_TransitionN.VOID: register = element.position_register
        else:                                         register = None
        case_list.append((LanguageDB.ACCEPTANCE(element.acceptance_id), 
                          "%s %s" % \
                          (LanguageDB.POSITIONING(element.positioning, register),
                           LanguageDB.GOTO_TERMINAL(element.acceptance_id))))

    txt.extend(LanguageDB.SELECTION("last_acceptance", case_list))

