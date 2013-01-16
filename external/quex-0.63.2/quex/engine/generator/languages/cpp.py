from   quex.engine.misc.string_handling        import blue_print

from   quex.engine.generator.languages.address import get_label, \
                                                      get_address, \
                                                      Address
from   quex.engine.interval_handling           import NumberSet
from   operator                                import itemgetter
#

def __nice(SM_ID): 
    return repr(SM_ID).replace("L", "")

#________________________________________________________________________________
# C++
#

__header_definitions_txt = """
#include <quex/code_base/analyzer/member/basic>
#include <quex/code_base/buffer/Buffer>
#ifdef QUEX_OPTION_TOKEN_POLICY_QUEUE
#   include <quex/code_base/token/TokenQueue>
#endif

#ifdef    CONTINUE
#   undef CONTINUE
#endif
#define   CONTINUE goto $$GOTO_START_PREPARATION$$; 

#ifdef    RETURN
#   undef RETURN
#endif
"""

__return_with_on_after_match = """
#   define RETURN   goto __RETURN_REPARATION;
"""
__return_without_on_after_match = """
#if defined(QUEX_OPTION_TOKEN_POLICY_QUEUE)
#   define RETURN   return
#else
#   define RETURN   do { return __self_result_token_id; } while(0)
#endif
"""

def __header_definitions(LanguageDB, OnAfterMatchStr):

    txt = __header_definitions_txt
    txt = txt.replace("$$GOTO_START_PREPARATION$$", get_label("$re-start", U=True))

    if len(OnAfterMatchStr) != 0: txt += __return_with_on_after_match
    else:                         txt += __return_without_on_after_match
    return txt

def _local_variable_definitions(VariableDB):
    if len(VariableDB) == 0: return ""

    def __group_by_condition(VariableDB):
        result = {}
        for variable in VariableDB.itervalues():

            variable_list = result.get(variable.condition)
            if variable_list is None: 
                variable_list              = [[], []]
                result[variable.condition] = variable_list

            if not variable.condition_negated_f: variable_list[0].append(variable)
            else:                                variable_list[1].append(variable)

        return result

    def __code(txt, variable):
        variable_type = variable.variable_type
        variable_init = variable.initial_value
        variable_name = variable.name

        if variable.element_n is not None: 
            if variable.element_n != 0:
                variable_name += "[%s]" % repr(variable.element_n)
                if variable_type.find("QUEX_TYPE_GOTO_LABEL") != -1: 
                    variable_name = "(" + variable_name + ")"
            else:
                variable_type += "*"
                variable_init  = ["0x0"]

        if variable_init is None: 
            value = ["/* un-initilized */"]
        else:
            if type(variable_init) != list: variable_init = [ variable_init ]
            value = [" = "] + variable_init

        txt.append("    %s%s %s%s" % \
                   (variable_type, 
                    " " * (30-len(variable_type)), variable_name, 
                    " " * (30 - len(variable_name))))
        txt.extend(value)
        txt.append(";\n")

    # L   = max(map(lambda info: len(info[0]), VariableDB.keys()))
    txt       = []
    for raw_name, variable in sorted(VariableDB.items()):
        if variable.priority_f == False: continue

        if variable.condition is not None:
            if variable.condition_negated_f == False: 
                txt.append("#   ifdef %s\n"  % variable.condition)
            else:
                txt.append("#   ifndef %s\n" %  variable.condition)

        __code(txt, variable)

        if variable.condition is not None:
            txt.append("#   endif /* %s */\n" % variable.condition)

        del VariableDB[variable.name]

    grouped_variable_list = __group_by_condition(VariableDB)
    unconditioned_name_set = set([])
    for condition, groups in sorted(grouped_variable_list.iteritems()):
        if condition is not None: continue
        for variable in groups[0]:
            unconditioned_name_set.add(variable.name)

    for condition, groups in sorted(grouped_variable_list.iteritems()):

        condition_group, negated_condition_group = groups

        if condition is None:
            for variable in condition_group:
                __code(txt, variable)
        else:
            if len(condition_group) != 0:
                txt.append("#   ifdef %s\n"  % condition)

                for variable in condition_group:
                    if variable.name in unconditioned_name_set: continue
                    __code(txt, variable)

            if len(negated_condition_group) != 0:
                if len(condition_group) != 0: txt.append("#   else /* not %s */\n" % condition)
                else:                         txt.append("#   ifndef %s\n"         % condition)

                for variable in negated_condition_group:
                    if variable.name in unconditioned_name_set: continue
                    __code(txt, variable)

            txt.append("#   endif /* %s */\n" % condition)
            
    return txt
         
__function_signature = """
__QUEX_TYPE_ANALYZER_RETURN_VALUE  
QUEX_NAME($$STATE_MACHINE_NAME$$_analyzer_function)(QUEX_TYPE_ANALYZER* me) 
{
    /* NOTE: Different modes correspond to different analyzer functions. The analyzer  
             functions are all located inside the main class as static functions. That  
             means, they are something like 'globals'. They receive a pointer to the   
             lexical analyzer, since static member do not have access to the 'this' pointer.
     */
#   if defined(QUEX_OPTION_TOKEN_POLICY_SINGLE)
    register QUEX_TYPE_TOKEN_ID __self_result_token_id 
           = (QUEX_TYPE_TOKEN_ID)__QUEX_SETTING_TOKEN_ID_UNINITIALIZED;
#   endif
#   ifdef     self
#       undef self
#   endif
#   define self (*((QUEX_TYPE_ANALYZER*)me))
"""

comment_on_post_context_position_init_str = """
    /* Post context positions do not have to be reset or initialized. If a state
     * is reached which is associated with 'end of post context' it is clear what
     * post context is meant. This results from the ways the state machine is 
     * constructed. Post context position's live cycle:
     *
     * (1)   unitialized (don't care)
     * (1.b) on buffer reload it may, or may not be adapted (don't care)
     * (2)   when a post context begin state is passed, then it is **SET** (now: take care)
     * (2.b) on buffer reload it **is adapted**.
     * (3)   when a terminal state of the post context is reached (which can only be reached
     *       for that particular post context), then the post context position is used
     *       to reset the input position.                                              */
"""

def __analyzer_function(StateMachineName, Setup,
                        variable_definitions, function_body, ModeNameList=[]):
    """EngineClassName = name of the structure that contains the engine state.
                         if a mode of a complete quex environment is created, this
                         is the mode name. otherwise, any name can be chosen. 
       SingleModeAnalyzerF = False if a mode for a quex engine is to be created. True
                           if a stand-alone lexical engine is required (without the
                           complete mode-handling framework of quex).
    """              
    LanguageDB          = Setup.language_db
    SingleModeAnalyzerF = Setup.single_mode_analyzer_f

    txt = [
            "#include <quex/code_base/temporary_macros_on>\n",
            __function_signature.replace("$$STATE_MACHINE_NAME$$", StateMachineName),
    ]

    txt.extend(variable_definitions)

    if len(ModeNameList) != 0 and not SingleModeAnalyzerF: 
        L = max(map(lambda name: len(name), ModeNameList))
        for name in ModeNameList:
            txt.append("#   define %s%s    (QUEX_NAME(%s))\n" % (name, " " * (L- len(name)), name))

    txt.extend([
        comment_on_post_context_position_init_str,
        "#   if    defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE) \\\n",
        "       || defined(QUEX_OPTION_ASSERTS)\n",
        "    me->DEBUG_analyzer_function_at_entry = me->current_analyzer_function;\n",
        "#   endif\n",
    ])

    txt.append(get_label("$start") + ":\n")

    # -- entry to the actual function body
    txt.append("    %s\n" % LanguageDB.LEXEME_START_SET())
    txt.append("    QUEX_LEXEME_TERMINATING_ZERO_UNDO(&me->buffer);\n")
    
    txt.extend(function_body)

    # -- prevent the warning 'unused variable'
    txt.append( 
        "\n"                                                                                              \
        "    /* Prevent compiler warning 'unused variable': use variables once in a part of the code*/\n" \
        "    /* that is never reached (and deleted by the compiler anyway).*/\n")

    # Mode Names are defined as macros, so the following is not necessary.
    # for mode_name in ModeNameList:
    #    txt.append("    (void)%s;\n" % mode_name)
    txt.append(                                                             \
        "    (void)QUEX_LEXEME_NULL;\n"                                     \
        "    (void)QUEX_NAME_TOKEN(DumpedTokenIdObject);\n"                 \
        "    QUEX_ERROR_EXIT(\"Unreachable code has been reached.\\n\");\n") 

    ## This was once we did not know ... if there was a goto to the initial state or not.
    ## txt += "        goto %s;\n" % label.get(StateMachineName, InitialStateIndex)
    if len(ModeNameList) != 0 and not SingleModeAnalyzerF: 
        L = max(map(lambda name: len(name), ModeNameList))
        for name in ModeNameList:
            txt.append("#   undef %s\n" % name)

    txt.append("#   undef self\n")
    txt.append("}\n")

    txt.append("#include <quex/code_base/temporary_macros_off>\n")
    return txt

__terminal_router_prolog_str = """
#   if defined(QUEX_OPTION_COMPUTED_GOTOS)
    __quex_assert_no_passage();
    /* Scenario: -- QUEX_GOTO_TERMINAL(last_acceptance) defined
     *              => required __TERMINAL_ROUTER
     *           -- all last_acceptance are 'failure' 
     *              => no routing.
     *           -- Compilation with 'QUEX_OPTION_COMPUTED_GOTOS'.
     *              => no state router required.                     
     * Then: There is no 'goto' to __TERMINAL_ROUTER and the compiler
     *       would complain about an unreferenced label. Avoid this by
     *       putting an explicit, never reached 'goto' here.               */
    goto __TERMINAL_ROUTER;
#   endif
    __quex_assert_no_passage();
__TERMINAL_ROUTER:
    __quex_debug("terminal router");
    /*  if last_acceptance => goto correspondent acceptance terminal state */
    /*  else               => execute default action                       */
    if( last_acceptance == $$TERMINAL_FAILURE-REF$$ ) {
        goto $$TERMINAL_FAILURE$$; /* TERMINAL: FAILURE */
    }
#   ifdef  QUEX_OPTION_COMPUTED_GOTOS
    goto *last_acceptance;
#   else
    target_state_index = last_acceptance;
    goto """

__terminal_router_epilog_str = """
#   endif /* QUEX_OPTION_COMPUTED_GOTOS */
"""
__terminal_state_prolog  = """
    /* (*) Terminal states _______________________________________________________
     *
     * States that implement actions of the 'winner patterns.                     */

    /* Lexeme setup: 
     *
     * There is a temporary zero stored at the end of each lexeme, if the action 
     * references to the 'Lexeme'. 'LexemeNull' provides a reference to an empty
     * zero terminated string.                                                    */
#if defined(QUEX_OPTION_ASSERTS)
#   define Lexeme       QUEX_NAME(access_Lexeme)((const char*)__FILE__, (size_t)__LINE__, &me->buffer)
#   define LexemeBegin  QUEX_NAME(access_LexemeBegin)((const char*)__FILE__, (size_t)__LINE__, &me->buffer)
#   define LexemeL      QUEX_NAME(access_LexemeL)((const char*)__FILE__, (size_t)__LINE__, &me->buffer)
#   define LexemeEnd    QUEX_NAME(access_LexemeEnd)((const char*)__FILE__, (size_t)__LINE__, &me->buffer)
#else
#   define Lexeme       (me->buffer._lexeme_start_p)
#   define LexemeBegin  Lexeme
#   define LexemeL      $$LEXEME_LENGTH$$
#   define LexemeEnd    $$INPUT_P$$
#endif

#define LexemeNull      (&QUEX_LEXEME_NULL)
"""

__terminal_state_epilog = """
$$TERMINAL_END_OF_STREAM-DEF$$: /* TERMINAL: END_OF_STREAM */
$$END_OF_STREAM_ACTION$$
    /* End of Stream causes a return from the lexical analyzer, so that no
     * tokens can be filled after the termination token.                    */
    RETURN;          

$$TERMINAL_FAILURE-DEF$$: /* TERMINAL: FAILURE */
$$FAILURE_ACTION$$
    goto $$GOTO_START_PREPARATION$$;
"""

__return_preparation_str = """
__RETURN_REPARATION:
$$ON_AFTER_MATCH$$
#   if defined(QUEX_OPTION_TOKEN_POLICY_QUEUE)
    return;
#   else
    return __self_result_token_id;
#   endif
"""

__reentry_preparation_str = """
$$REENTRY_PREPARATION$$:
    /* (*) Common point for **restarting** lexical analysis.
     *     at each time when CONTINUE is called at the end of a pattern. */
$$ON_AFTER_MATCH$$ 

#   undef Lexeme
#   undef LexemeBegin
#   undef LexemeEnd
#   undef LexemeNull
#   undef LexemeL

#   ifndef __QUEX_OPTION_PLAIN_ANALYZER_OBJECT
#   ifdef  QUEX_OPTION_TOKEN_POLICY_QUEUE
    if( QUEX_NAME(TokenQueue_is_full)(&self._token_queue) ) RETURN;
#   else
    if( self_token_get_id() != __QUEX_SETTING_TOKEN_ID_UNINITIALIZED) RETURN;
#   endif
#   endif
    $$RESET_LAST_ACCEPTANCE$$
$$DELETE_PRE_CONDITION_FULLFILLED_FLAGS$$
$$COMMENT_ON_POST_CONTEXT_INITIALIZATION$$
    /*  If a mode change happened, then the function must first return and
     *  indicate that another mode function is to be called. At this point, 
     *  we to force a 'return' on a mode change. 
     *
     *  Pseudo Code: if( previous_mode != current_mode ) {
     *                   return 0;
     *               }
     *
     *  When the analyzer returns, the caller function has to watch if a mode change
     *  occurred. If not it can call this function again.                               */
#   if    defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE) \
       || defined(QUEX_OPTION_ASSERTS)
    if( me->DEBUG_analyzer_function_at_entry != me->current_analyzer_function ) 
#   endif
    { 
#       if defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE)
        self_token_set_id(__QUEX_SETTING_TOKEN_ID_UNINITIALIZED);
        RETURN;
#       elif defined(QUEX_OPTION_ASSERTS)
        QUEX_ERROR_EXIT("Mode change without immediate return from the lexical analyzer.");
#       endif
    }

    goto $$GOTO_START$$;
"""
def get_terminal_code(AcceptanceID, pattern_action_info, LanguageDB):
    pattern     = pattern_action_info.pattern()
    #
    action_code = pattern_action_info.action().get_code()
        
    # (*) The 'normal' terminal state can also be reached by the terminal
    #     router and, thus, **must** restore the acceptance input position. This is so, 
    #     because when the 'goto last_acceptance' is triggered the 'last_acceptance'
    #     may lay backwards and needs to be restored.
    result      = []
    for letter in pattern_action_info.pattern_string():
        if letter in ['\\', '"', '\n', '\t', '\r', '\a', '\v']:
            result.append("\\")
        result.append(letter)

    safe_pattern = "".join(result)

    input_position_search_backward_str = ""
    if pattern.input_position_search_backward_sm is not None:
        # Pseudo Ambiguous Post Contexts:
        # (Retrieving the input position for the next run)
        # -- Requires that the end of the core pattern is to be searched! One 
        #    cannot simply restore some stored input position.
        # -- The pseudo-ambiguous post condition is translated into a 'normal'
        #    pattern. However, after a match a backward detection of the end
        #    of the core pattern is done. Here, we first need to go to the point
        #    where the 'normal' pattern ended, then we can do a backward detection.

        bipd_id   = pattern.input_position_search_backward_sm.get_id()
        bipd_str  = "    goto %s;\n" % LanguageDB.LABEL_NAME_BACKWARD_INPUT_POSITION_DETECTOR(bipd_id)
        # After having finished the analyzis, enter the terminal code, here.
        bipd_str += "%s:\n" % LanguageDB.LABEL_NAME_BACKWARD_INPUT_POSITION_RETURN(bipd_id) 
        input_position_search_backward_str = bipd_str


    txt = [
            "\nTERMINAL_%i:\n" % AcceptanceID,
            input_position_search_backward_str,
            "    __quex_debug(\"* terminal %i:   %s\\n\");\n" % (AcceptanceID, safe_pattern),
            action_code, "\n",
            "    goto %s;\n" % get_label("$re-start", U=True)
    ]

    return txt

def __terminal_on_failure_prolog(LanguageDB):
    return [
        "    if(QUEX_NAME(Buffer_is_end_of_file)(&me->buffer)) {\n",
        "        /* Init state is going to detect 'input == buffer limit code', and\n"
        "         * enter the reload procedure, which will decide about 'end of stream'. */\n",
        "    } else {\n",
        "        /* In init state 'input = *input_p' and we need to increment\n",
        "         * in order to avoid getting stalled. Else, input = *(input_p - 1),\n",
        "         * so 'input_p' points already to the next character.              */\n",
        "        if( me->buffer._input_p == me->buffer._lexeme_start_p ) {\n",
        "            /* Step over non-matching character */\n",
        "            %s\n" % LanguageDB.INPUT_P_INCREMENT(),
        "        }\n",
        "    }\n",
    ]

def __terminal_states(StateMachineName, action_db, OnFailureAction, EndOfStreamAction, 
                      PreConditionIDList, LanguageDB, VariableDB, OnAfterMatchStr, LexemeNullObjectName):
    """NOTE: During backward-lexing, for a pre-condition, there is not need for terminal
             states, since only the flag 'pre-condition fulfilled is raised.
    """      

    # (*) specific terminal states of patterns (entered from acceptance states)
    specific_terminal_states = []
    for pattern_id, pattern_action_info in action_db.items():
        if pattern_id in PreConditionIDList: continue
        code = get_terminal_code(pattern_id, pattern_action_info, LanguageDB)
        specific_terminal_states.extend(code)

    delete_pre_context_flags = []
    for pre_context_sm_id in PreConditionIDList:
        delete_pre_context_flags.append("    ")
        delete_pre_context_flags.append(LanguageDB.ASSIGN("pre_context_%s_fulfilled_f" % __nice(pre_context_sm_id), 0))

    # If there is at least a single terminal, the the 're-entry' preparation must be accomplished
    if len(action_db) != 0: get_label("$re-start", U=True)

    #  -- execute 'on_failure' pattern action 
    #  -- goto initial state    
    end_of_stream_code_action_str = EndOfStreamAction.action().get_code()

    # -- FAILURE ACTION: Under 'normal' circumstances the on_failure action is simply to be executed
    #                    since the 'get_forward()' incremented the 'current' pointer.
    #                    HOWEVER, when end of file has been reached the 'current' pointer has to
    #                    be reset so that the initial state can drop out on the buffer limit code
    #                    and then transit to the end of file action.
    # NOTE: It is possible that 'miss' happens after a chain of characters appeared. In any case the input
    #       pointer must be setup right after the lexeme start. This way, the lexer becomes a new chance as
    #       soon as possible.
    on_failure = __terminal_on_failure_prolog(LanguageDB)
    msg        = OnFailureAction.action().get_code()

    on_failure.append(msg)

    prolog = blue_print(__terminal_state_prolog,
                        [
                          ["$$LEXEME_LENGTH$$",      LanguageDB.LEXEME_LENGTH()],
                          ["$$INPUT_P$$",            LanguageDB.INPUT_P()],
                          ["$$LEXEME_NULL_OBJECT$$", LexemeNullObjectName],
                        ]
                       )

    router = Address("$terminal-router", None,
                  [
                      blue_print(__terminal_router_prolog_str,
                      [
                       ["$$TERMINAL_FAILURE-REF$$",         "QUEX_LABEL(%i)" % get_address("$terminal-FAILURE")],
                       ["$$TERMINAL_FAILURE$$",             get_label("$terminal-FAILURE")],
                      ]),
                      # DO NOT 'U=True' for the state router. This is done automatically if 
                      # 'goto reload' is used. 
                      get_label("$state-router"), ";",
                      __terminal_router_epilog_str, 
                  ])
                     
    epilog = blue_print(__terminal_state_epilog, 
             [
              ["$$FAILURE_ACTION$$",             "".join(on_failure)],
              ["$$END_OF_STREAM_ACTION$$",       end_of_stream_code_action_str],
              ["$$TERMINAL_END_OF_STREAM-DEF$$", get_label("$terminal-EOF")],
              ["$$TERMINAL_FAILURE-DEF$$",       get_label("$terminal-FAILURE")],
              ["$$STATE_MACHINE_NAME$$",         StateMachineName],
              ["$$GOTO_START_PREPARATION$$",     get_label("$re-start", U=True)],
             ])


    reset_last_acceptance_str = ""
    if VariableDB.has_key("last_acceptance"):
        reset_last_acceptance_str = "last_acceptance = $$TERMINAL_FAILURE-REF$$; /* TERMINAL: FAILURE */"

    return_preparation = ""
    if OnAfterMatchStr != "":
        return_preparation = blue_print(__return_preparation_str,
                                        [["$$ON_AFTER_MATCH$$",  OnAfterMatchStr]])

    reentry_preparation = blue_print(__reentry_preparation_str,
                          [["$$REENTRY_PREPARATION$$",                    get_label("$re-start")],
                           ["$$DELETE_PRE_CONDITION_FULLFILLED_FLAGS$$",  "".join(delete_pre_context_flags)],
                           ["$$GOTO_START$$",                             get_label("$start", U=True)],
                           ["$$ON_AFTER_MATCH$$",                         OnAfterMatchStr],
                           ["$$RESET_LAST_ACCEPTANCE$$",                  reset_last_acceptance_str],
                           ["$$COMMENT_ON_POST_CONTEXT_INITIALIZATION$$", comment_on_post_context_position_init_str],
                           ["$$TERMINAL_FAILURE-REF$$",                   "QUEX_LABEL(%i)" % get_address("$terminal-FAILURE")],
                          ])

    txt = []
    txt.append(router)
    txt.append(prolog)
    txt.extend(specific_terminal_states)
    txt.append(epilog)
    txt.append(return_preparation)
    txt.append(reentry_preparation)

    return txt
    
def __frame_of_all(Code, Setup):
    # namespace_ref   = LanguageDB.NAMESPACE_REFERENCE(Setup.analyzer_name_space)
    # if len(namespace_ref) > 2 and namespace_ref[:2] == "::":  namespace_ref = namespace_ref[2:]
    # if len(namespace_ref) > 2 and namespace_ref[-2:] == "::": namespace_ref = namespace_ref[:-2]
    # "using namespace " + namespace_ref + ";\n"       + \

    implementation_header_str = ""
    if Setup.language == "C":
        implementation_header_str += "#if defined(__QUEX_OPTION_CONVERTER_HELPER)\n"
        implementation_header_str += "#   include \"%s\"\n" % Setup.get_file_reference(Setup.output_buffer_codec_header_i)
        implementation_header_str += "#endif\n"
        implementation_header_str += "#include <quex/code_base/analyzer/headers.i>\n"
        implementation_header_str += "#include <quex/code_base/analyzer/C-adaptions.h>\n"

    lexeme_null_definition = ""
    if Setup.external_lexeme_null_object == "":
        # LexemeNull has been defined elsewhere.
        lexeme_null_definition = "QUEX_TYPE_CHARACTER  QUEX_LEXEME_NULL_IN_ITS_NAMESPACE = (QUEX_TYPE_CHARACTER)0;\n"

    return "".join(["/* #include \"%s\"*/\n" % Setup.get_file_reference(Setup.output_header_file),
                    implementation_header_str,
                    "QUEX_NAMESPACE_MAIN_OPEN\n",
                    lexeme_null_definition,
                    Code,
                    "QUEX_NAMESPACE_MAIN_CLOSE\n"])                     

def __get_if_in_character_set(ValueList):
    assert type(ValueList) == list
    assert len(ValueList) > 0
    txt = "if( "
    for value in ValueList:
        txt += "input == %i || " % value

    txt = txt[:-3] + ") {\n"
    return txt

def __get_if_in_interval(TriggerSet):
    assert TriggerSet.__class__.__name__ == "Interval"
    assert TriggerSet.size() >= 2

    if TriggerSet.size() == 2:
        return "if( input == %i || input == %i ) {\n" % (TriggerSet.begin, TriggerSet.end - 1)
    else:
        return "if( input >= %i && input < %i ) {\n" % (TriggerSet.begin, TriggerSet.end)

def __condition(txt, CharSet):
    first_f = True
    for interval in CharSet.get_intervals(PromiseToTreatWellF=True):
        if first_f: first_f = False
        else:       txt.append(" || ")

        if interval.end - interval.begin == 1:
            txt.append("(C) == 0x%X"                % interval.begin)
        elif interval.end - interval.begin == 2:
            txt.append("(C) == 0x%X || (C) == 0x%X" % (interval.begin, interval.end - 1))
        else:
            txt.append("(C) <= 0x%X && (C) < 0x%X" % (interval.begin, interval.end))

def __indentation_add(Info):
    # (0) If all involved counts are single spaces, the 'counting' can be done
    #     easily by subtracting 'end - begin', no adaption.
    indent_txt = " " * 16
    if Info.has_only_single_spaces():
        return ""

    def __do(txt, CharSet, Operation):
        txt.append(indent_txt + "if( ")
        __condition(txt, CharSet)
        txt.append(" ) { ")
        txt.append(Operation)
        txt.append(" }\\\n")

    txt       = []
    spaces_db = {} # Sort same space counts together
    grid_db   = {} # Sort same grid counts together
    for name, count_parameter in Info.count_db.items():
        count         = count_parameter.get()
        character_set = Info.character_set_db[name].get()
        if count == "bad": continue
        # grid counts are indicated by negative integer for count.
        if count >= 0:
            spaces_db.setdefault(count, NumberSet()).unite_with(character_set)
        else:
            grid_db.setdefault(count, NumberSet()).unite_with(character_set)

    for count, character_set in spaces_db.items():
        __do(txt, character_set, "(I) += %i;" % count)

    for count, character_set in grid_db.items():
        __do(txt, character_set, "(I) += (%i - ((I) %% %i));" % (abs(count), abs(count)))

    return "".join(txt)

def __indentation_check_whitespace(Info):
    all_character_list = map(lambda x: x.get(), Info.character_set_db.values())
    assert len(all_character_list) != 0

    number_set = all_character_list[0]
    for character_set in all_character_list[1:]:
        number_set.unite_with(character_set)

    txt = []
    __condition(txt, number_set)
    return "".join(txt)

def __get_switch_block(VariableName, CaseCodePairList):
    txt = [0, "switch( %s ) {\n" % VariableName]
    next_i = 0
    L = len(CaseCodePairList)
    CaseCodePairList.sort(key=itemgetter(0))
    for case, code in CaseCodePairList: 
        next_i += 1
        txt.append(1)
        case_label = "0x%X" % case
        if next_i != L and CaseCodePairList[next_i][1] == code:
            txt.append("case %s: %s\n" % (case_label, " " * (7 - len(case_label))))
        else:
            txt.append("case %s: %s" % (case_label, " " * (7 - len(case_label))))
            if type(code) == list: txt.extend(code)
            else:                  txt.append(code)
            txt.append("\n")
            
    txt.append(0)
    txt.append("}\n")
    return txt


