from   quex.engine.generator.languages.address  import Address, get_label, get_address, get_label_of_address
from   operator                                 import itemgetter

def do(StateRouterInfoList):
    """Create code that allows to jump to a state based on an integer value.
    """
    prolog = "#   ifndef QUEX_OPTION_COMPUTED_GOTOS\n" \
             "    __quex_assert_no_passage();\n"       \
             "__STATE_ROUTER:\n"

    # It is conceivable, that 'last_acceptance' is never set to a valid 
    # terminal. Further, there might be solely the init state. In this
    # case the state router is void of states. But, the terminal router
    # requires it to be defined --> define a dummy state router.
    if len(StateRouterInfoList) == 0:
        return Address("$state-router",
                       Code = [  prolog  \
                               + "    QUEX_ERROR_EXIT(\"Entered section of empty state router.\");\n"
                               + "#   endif\n"
                               ])


    txt = ["    switch( target_state_index ) {\n" ]

    done_set = set([])
    for index, code in sorted(StateRouterInfoList, key=itemgetter(0)):
        if index in done_set: continue
        done_set.add(index)
        txt.append("        case %i: { " % index)
        if type(code) == list: txt.extend(code)
        else:                  txt.append(code)
        txt.append("}\n")

    txt.append("\n")
    txt.append("        default:\n")
    txt.append("            __QUEX_STD_fprintf(stderr, \"State router: index = %i\\n\", (int)target_state_index);\n")
    txt.append("            QUEX_ERROR_EXIT(\"State router: unknown index.\");\n")
    txt.append("    }\n")

    epilog = "#   endif /* QUEX_OPTION_COMPUTED_GOTOS */\n"

    return Address("$state-router", Code=[prolog] + txt + [epilog])

def get_info(StateIndexList):
    # In some strange cases, a 'dummy' state router is required so that 
    # 'goto __STATE_ROUTER;' does not reference a non-existing label. Then,
    # we return an empty text array.
    if len(StateIndexList) == 0: return []

    # Make sure, that for every state the 'drop-out' state is also mentioned
    result = [None] * len(StateIndexList)
    for i, index in enumerate(StateIndexList):
        assert type(index) != str
        if index >= 0:
            # Transition to state entry
            code = "goto %s; " % get_label_of_address(index)
            result[i] = (index, code)
        else:
            # Transition to a templates 'drop-out'
            code = "goto " + get_label("$drop-out", - index) + "; "
            result[i] = (get_address("$drop-out", - index), code)
    return result
