#include "UTF16Lex"
#include <quex/code_base/analyzer/C-adaptions.h>
QUEX_NAMESPACE_MAIN_OPEN
/* Global */QUEX_NAME(Mode)  QUEX_NAME(X);
#ifndef __QUEX_INDICATOR_DUMPED_TOKEN_ID_DEFINED
    static QUEX_TYPE_TOKEN_ID    QUEX_NAME_TOKEN(DumpedTokenIdObject);
#endif
#define self  (*(QUEX_TYPE_DERIVED_ANALYZER*)me)
#define __self_result_token_id    QUEX_NAME_TOKEN(DumpedTokenIdObject)

void
QUEX_NAME(X_on_entry)(QUEX_TYPE_ANALYZER* me, const QUEX_NAME(Mode)* FromMode) {
    (void)me;
    (void)FromMode;
#   ifdef QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
    QUEX_NAME(X).has_entry_from(FromMode);
#   endif

}

void
QUEX_NAME(X_on_exit)(QUEX_TYPE_ANALYZER* me, const QUEX_NAME(Mode)* ToMode)  {
    (void)me;
    (void)ToMode;
#   ifdef QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
    QUEX_NAME(X).has_exit_to(ToMode);
#   endif

}

#if defined(QUEX_OPTION_INDENTATION_TRIGGER) 
void
QUEX_NAME(X_on_indentation)(QUEX_TYPE_ANALYZER*    me, 
                                        QUEX_TYPE_INDENTATION  Indentation, 
                                        QUEX_TYPE_CHARACTER*   Begin) {
    (void)me;
    (void)Indentation;
    (void)Begin;
    return;
}
#endif

#ifdef QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
bool
QUEX_NAME(X_has_base)(const QUEX_NAME(Mode)* Mode) {
    (void)Mode;
    return false;
}
bool
QUEX_NAME(X_has_entry_from)(const QUEX_NAME(Mode)* Mode) {
    (void)Mode;
    return true; /* default */
}
bool
QUEX_NAME(X_has_exit_to)(const QUEX_NAME(Mode)* Mode) {
    (void)Mode;
    return false;
}
#endif    
#undef self
#undef __self_result_token_id
QUEX_NAMESPACE_MAIN_CLOSE

/* #include "UTF16Lex"*/
QUEX_NAMESPACE_MAIN_OPEN
QUEX_TYPE_CHARACTER  QUEX_LEXEME_NULL_IN_ITS_NAMESPACE = (QUEX_TYPE_CHARACTER)0;

#include <quex/code_base/analyzer/member/basic>
#include <quex/code_base/buffer/Buffer>
#ifdef QUEX_OPTION_TOKEN_POLICY_QUEUE
#   include <quex/code_base/token/TokenQueue>
#endif

#ifdef    CONTINUE
#   undef CONTINUE
#endif
#define   CONTINUE goto __REENTRY_PREPARATION; 

#ifdef    RETURN
#   undef RETURN
#endif

#if defined(QUEX_OPTION_TOKEN_POLICY_QUEUE)
#   define RETURN   return
#else
#   define RETURN   do { return __self_result_token_id; } while(0)
#endif
#include <quex/code_base/temporary_macros_on>

__QUEX_TYPE_ANALYZER_RETURN_VALUE  
QUEX_NAME(X_analyzer_function)(QUEX_TYPE_ANALYZER* me) 
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
    void*                          position                       = (void*)0x0;
    QUEX_TYPE_GOTO_LABEL           target_state_else_index        = QUEX_GOTO_LABEL_VOID;
    const size_t                   PositionRegisterN              = (size_t)0;
    QUEX_TYPE_CHARACTER            input                          = (QUEX_TYPE_CHARACTER)(0x00);
    QUEX_TYPE_GOTO_LABEL           target_state_index             = QUEX_GOTO_LABEL_VOID;
#   ifndef QUEX_OPTION_COMPUTED_GOTOS
#   endif /* QUEX_OPTION_COMPUTED_GOTOS */
#   define X    (QUEX_NAME(X))

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
#   if    defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE) \
       || defined(QUEX_OPTION_ASSERTS)
    me->DEBUG_analyzer_function_at_entry = me->current_analyzer_function;
#   endif
__REENTRY:
    me->buffer._lexeme_start_p = me->buffer._input_p;
    QUEX_LEXEME_TERMINATING_ZERO_UNDO(&me->buffer);
INIT_STATE_TRANSITION_BLOCK:
    input = *(me->buffer._input_p);
    __quex_debug("Init State\n");
    __quex_debug_state(92);
    if( input < 0x1DC0 ) {
        if( input < 0x385 ) {
            if( input < 0x3B ) {
                switch( input ) {
                    case 0x0: QUEX_GOTO_RELOAD_FORWARD(92, 105);

                    case 0x2: 
                    case 0x3: 
                    case 0x4: 
                    case 0x5: 
                    case 0x6: 
                    case 0x7: 
                    case 0x8: goto _93;

                    case 0x9: goto _97;

                    case 0xA: goto _94;

                    case 0xB: 
                    case 0xC: 
                    case 0xD: 
                    case 0xE: 
                    case 0xF: 
                    case 0x10: 
                    case 0x11: 
                    case 0x12: 
                    case 0x13: 
                    case 0x14: 
                    case 0x15: 
                    case 0x16: 
                    case 0x17: 
                    case 0x18: 
                    case 0x19: 
                    case 0x1A: 
                    case 0x1B: 
                    case 0x1C: 
                    case 0x1D: 
                    case 0x1E: 
                    case 0x1F: goto _93;

                    case 0x20: goto _97;

                    case 0x21: 
                    case 0x22: 
                    case 0x23: 
                    case 0x24: 
                    case 0x25: 
                    case 0x26: 
                    case 0x27: 
                    case 0x28: 
                    case 0x29: 
                    case 0x2A: 
                    case 0x2B: 
                    case 0x2C: 
                    case 0x2D: 
                    case 0x2E: 
                    case 0x2F: goto _93;

                    case 0x30: 
                    case 0x31: 
                    case 0x32: 
                    case 0x33: 
                    case 0x34: 
                    case 0x35: 
                    case 0x36: 
                    case 0x37: 
                    case 0x38: 
                    case 0x39: goto _100;

                    case 0x3A: goto _98;

                }
            } else {
                if( input < 0x375 ) {

                    if( input < 0x41 ) {
                        goto _93;

                    } else if( input < 0x47 ) {
                        goto _100;

                    } else if( input < 0x370 ) {
                        goto _93;

                    } else if( input < 0x374 ) {
                        goto _99;

                    } else {
                        goto _93;

}                } else {
                    switch( input ) {
                        case 0x375: 
                        case 0x376: 
                        case 0x377: goto _99;

                        case 0x378: 
                        case 0x379: goto _93;

                        case 0x37A: 
                        case 0x37B: 
                        case 0x37C: 
                        case 0x37D: goto _99;

                        case 0x37E: 
                        case 0x37F: 
                        case 0x380: 
                        case 0x381: 
                        case 0x382: 
                        case 0x383: goto _93;

                        case 0x384: goto _99;

                    }
                }            }        } else {
            if( input < 0x3E2 ) {
                switch( input ) {
                    case 0x385: goto _93;

                    case 0x386: goto _99;

                    case 0x387: goto _93;

                    case 0x388: 
                    case 0x389: 
                    case 0x38A: goto _99;

                    case 0x38B: goto _93;

                    case 0x38C: goto _99;

                    case 0x38D: goto _93;

                    case 0x38E: 
                    case 0x38F: 
                    case 0x390: 
                    case 0x391: 
                    case 0x392: 
                    case 0x393: 
                    case 0x394: 
                    case 0x395: 
                    case 0x396: 
                    case 0x397: 
                    case 0x398: 
                    case 0x399: 
                    case 0x39A: 
                    case 0x39B: 
                    case 0x39C: 
                    case 0x39D: 
                    case 0x39E: 
                    case 0x39F: 
                    case 0x3A0: 
                    case 0x3A1: goto _99;

                    case 0x3A2: goto _93;

                    case 0x3A3: 
                    case 0x3A4: 
                    case 0x3A5: 
                    case 0x3A6: 
                    case 0x3A7: 
                    case 0x3A8: 
                    case 0x3A9: 
                    case 0x3AA: 
                    case 0x3AB: 
                    case 0x3AC: 
                    case 0x3AD: 
                    case 0x3AE: 
                    case 0x3AF: 
                    case 0x3B0: 
                    case 0x3B1: 
                    case 0x3B2: 
                    case 0x3B3: 
                    case 0x3B4: 
                    case 0x3B5: 
                    case 0x3B6: 
                    case 0x3B7: 
                    case 0x3B8: 
                    case 0x3B9: 
                    case 0x3BA: 
                    case 0x3BB: 
                    case 0x3BC: 
                    case 0x3BD: 
                    case 0x3BE: 
                    case 0x3BF: 
                    case 0x3C0: 
                    case 0x3C1: 
                    case 0x3C2: 
                    case 0x3C3: 
                    case 0x3C4: 
                    case 0x3C5: 
                    case 0x3C6: 
                    case 0x3C7: 
                    case 0x3C8: 
                    case 0x3C9: 
                    case 0x3CA: 
                    case 0x3CB: 
                    case 0x3CC: 
                    case 0x3CD: 
                    case 0x3CE: 
                    case 0x3CF: 
                    case 0x3D0: 
                    case 0x3D1: 
                    case 0x3D2: 
                    case 0x3D3: 
                    case 0x3D4: 
                    case 0x3D5: 
                    case 0x3D6: 
                    case 0x3D7: 
                    case 0x3D8: 
                    case 0x3D9: 
                    case 0x3DA: 
                    case 0x3DB: 
                    case 0x3DC: 
                    case 0x3DD: 
                    case 0x3DE: 
                    case 0x3DF: 
                    case 0x3E0: 
                    case 0x3E1: goto _99;

                }
            } else {
                if( input < 0x1D5D ) {

                    if( input < 0x3F0 ) {
                        goto _93;

                    } else if( input < 0x400 ) {
                        goto _99;

                    } else if( input < 0x1D26 ) {
                        goto _93;

                    } else if( input < 0x1D2B ) {
                        goto _99;

                    } else {
                        goto _93;

}                } else {

                    if( input < 0x1D62 ) {
                        goto _99;

                    } else if( input < 0x1D66 ) {
                        goto _93;

                    } else if( input < 0x1D6B ) {
                        goto _99;

                    } else if( input < 0x1DBF ) {
                        goto _93;

                    } else {
                        goto _99;

}                }            }        }    } else {
        if( input < 0x1FB5 ) {
            if( input < 0x1F58 ) {
                if( input < 0x1F20 ) {

                    if( input < 0x1F00 ) {
                        goto _93;

                    } else if( input < 0x1F16 ) {
                        goto _99;

                    } else if( input < 0x1F18 ) {
                        goto _93;

                    } else if( input < 0x1F1E ) {
                        goto _99;

                    } else {
                        goto _93;

}                } else {
                    switch( input ) {
                        case 0x1F20: 
                        case 0x1F21: 
                        case 0x1F22: 
                        case 0x1F23: 
                        case 0x1F24: 
                        case 0x1F25: 
                        case 0x1F26: 
                        case 0x1F27: 
                        case 0x1F28: 
                        case 0x1F29: 
                        case 0x1F2A: 
                        case 0x1F2B: 
                        case 0x1F2C: 
                        case 0x1F2D: 
                        case 0x1F2E: 
                        case 0x1F2F: 
                        case 0x1F30: 
                        case 0x1F31: 
                        case 0x1F32: 
                        case 0x1F33: 
                        case 0x1F34: 
                        case 0x1F35: 
                        case 0x1F36: 
                        case 0x1F37: 
                        case 0x1F38: 
                        case 0x1F39: 
                        case 0x1F3A: 
                        case 0x1F3B: 
                        case 0x1F3C: 
                        case 0x1F3D: 
                        case 0x1F3E: 
                        case 0x1F3F: 
                        case 0x1F40: 
                        case 0x1F41: 
                        case 0x1F42: 
                        case 0x1F43: 
                        case 0x1F44: 
                        case 0x1F45: goto _99;

                        case 0x1F46: 
                        case 0x1F47: goto _93;

                        case 0x1F48: 
                        case 0x1F49: 
                        case 0x1F4A: 
                        case 0x1F4B: 
                        case 0x1F4C: 
                        case 0x1F4D: goto _99;

                        case 0x1F4E: 
                        case 0x1F4F: goto _93;

                        case 0x1F50: 
                        case 0x1F51: 
                        case 0x1F52: 
                        case 0x1F53: 
                        case 0x1F54: 
                        case 0x1F55: 
                        case 0x1F56: 
                        case 0x1F57: goto _99;

                    }
                }            } else {
                switch( input ) {
                    case 0x1F58: goto _93;

                    case 0x1F59: goto _99;

                    case 0x1F5A: goto _93;

                    case 0x1F5B: goto _99;

                    case 0x1F5C: goto _93;

                    case 0x1F5D: goto _99;

                    case 0x1F5E: goto _93;

                    case 0x1F5F: 
                    case 0x1F60: 
                    case 0x1F61: 
                    case 0x1F62: 
                    case 0x1F63: 
                    case 0x1F64: 
                    case 0x1F65: 
                    case 0x1F66: 
                    case 0x1F67: 
                    case 0x1F68: 
                    case 0x1F69: 
                    case 0x1F6A: 
                    case 0x1F6B: 
                    case 0x1F6C: 
                    case 0x1F6D: 
                    case 0x1F6E: 
                    case 0x1F6F: 
                    case 0x1F70: 
                    case 0x1F71: 
                    case 0x1F72: 
                    case 0x1F73: 
                    case 0x1F74: 
                    case 0x1F75: 
                    case 0x1F76: 
                    case 0x1F77: 
                    case 0x1F78: 
                    case 0x1F79: 
                    case 0x1F7A: 
                    case 0x1F7B: 
                    case 0x1F7C: 
                    case 0x1F7D: goto _99;

                    case 0x1F7E: 
                    case 0x1F7F: goto _93;

                    case 0x1F80: 
                    case 0x1F81: 
                    case 0x1F82: 
                    case 0x1F83: 
                    case 0x1F84: 
                    case 0x1F85: 
                    case 0x1F86: 
                    case 0x1F87: 
                    case 0x1F88: 
                    case 0x1F89: 
                    case 0x1F8A: 
                    case 0x1F8B: 
                    case 0x1F8C: 
                    case 0x1F8D: 
                    case 0x1F8E: 
                    case 0x1F8F: 
                    case 0x1F90: 
                    case 0x1F91: 
                    case 0x1F92: 
                    case 0x1F93: 
                    case 0x1F94: 
                    case 0x1F95: 
                    case 0x1F96: 
                    case 0x1F97: 
                    case 0x1F98: 
                    case 0x1F99: 
                    case 0x1F9A: 
                    case 0x1F9B: 
                    case 0x1F9C: 
                    case 0x1F9D: 
                    case 0x1F9E: 
                    case 0x1F9F: 
                    case 0x1FA0: 
                    case 0x1FA1: 
                    case 0x1FA2: 
                    case 0x1FA3: 
                    case 0x1FA4: 
                    case 0x1FA5: 
                    case 0x1FA6: 
                    case 0x1FA7: 
                    case 0x1FA8: 
                    case 0x1FA9: 
                    case 0x1FAA: 
                    case 0x1FAB: 
                    case 0x1FAC: 
                    case 0x1FAD: 
                    case 0x1FAE: 
                    case 0x1FAF: 
                    case 0x1FB0: 
                    case 0x1FB1: 
                    case 0x1FB2: 
                    case 0x1FB3: 
                    case 0x1FB4: goto _99;

                }
            }        } else {
            if( input < 0x1FF5 ) {
                switch( input ) {
                    case 0x1FB5: goto _93;

                    case 0x1FB6: 
                    case 0x1FB7: 
                    case 0x1FB8: 
                    case 0x1FB9: 
                    case 0x1FBA: 
                    case 0x1FBB: 
                    case 0x1FBC: 
                    case 0x1FBD: 
                    case 0x1FBE: 
                    case 0x1FBF: 
                    case 0x1FC0: 
                    case 0x1FC1: 
                    case 0x1FC2: 
                    case 0x1FC3: 
                    case 0x1FC4: goto _99;

                    case 0x1FC5: goto _93;

                    case 0x1FC6: 
                    case 0x1FC7: 
                    case 0x1FC8: 
                    case 0x1FC9: 
                    case 0x1FCA: 
                    case 0x1FCB: 
                    case 0x1FCC: 
                    case 0x1FCD: 
                    case 0x1FCE: 
                    case 0x1FCF: 
                    case 0x1FD0: 
                    case 0x1FD1: 
                    case 0x1FD2: 
                    case 0x1FD3: goto _99;

                    case 0x1FD4: 
                    case 0x1FD5: goto _93;

                    case 0x1FD6: 
                    case 0x1FD7: 
                    case 0x1FD8: 
                    case 0x1FD9: 
                    case 0x1FDA: 
                    case 0x1FDB: goto _99;

                    case 0x1FDC: goto _93;

                    case 0x1FDD: 
                    case 0x1FDE: 
                    case 0x1FDF: 
                    case 0x1FE0: 
                    case 0x1FE1: 
                    case 0x1FE2: 
                    case 0x1FE3: 
                    case 0x1FE4: 
                    case 0x1FE5: 
                    case 0x1FE6: 
                    case 0x1FE7: 
                    case 0x1FE8: 
                    case 0x1FE9: 
                    case 0x1FEA: 
                    case 0x1FEB: 
                    case 0x1FEC: 
                    case 0x1FED: 
                    case 0x1FEE: 
                    case 0x1FEF: goto _99;

                    case 0x1FF0: 
                    case 0x1FF1: goto _93;

                    case 0x1FF2: 
                    case 0x1FF3: 
                    case 0x1FF4: goto _99;

                }
            } else {
                if( input < 0xD800 ) {

                    if( input == 0x1FF5 ) {
                        goto _93;

                    } else if( input < 0x1FFF ) {
                        goto _99;

                    } else if( input < 0x2126 ) {
                        goto _93;

                    } else if( input == 0x2126 ) {
                        goto _99;

                    } else {
                        goto _93;

}                } else {

                    if( input == 0xD800 ) {
                        goto _96;

                    } else if( input < 0xD834 ) {
                        goto _93;

                    } else if( input == 0xD834 ) {
                        goto _95;

                    } else if( input < 0x110000 ) {
                        goto _93;

                    } else {

}                }            }        }    }    __quex_debug_drop_out(92);
    
    goto _107; /* TERMINAL_FAILURE */


_92: /* (92 from NONE) */


    ++(me->buffer._input_p);
    goto INIT_STATE_TRANSITION_BLOCK;


    __quex_assert_no_passage();
_102: /* (102 from 102) (102 from 99) (102 from 96) (102 from 95) (102 from 104) (102 from 103) */

    ++(me->buffer._input_p);
    input = *(me->buffer._input_p);
    __quex_debug_state(102);
    if( input < 0x1F20 ) {
        switch( input ) {
            case 0x0: QUEX_GOTO_RELOAD_FORWARD(102, 108);

            case 0x370: 
            case 0x371: 
            case 0x372: 
            case 0x373: 
            case 0x375: 
            case 0x376: 
            case 0x377: 
            case 0x37A: 
            case 0x37B: 
            case 0x37C: 
            case 0x37D: 
            case 0x384: 
            case 0x386: 
            case 0x388: 
            case 0x389: 
            case 0x38A: 
            case 0x38C: 
            case 0x38E: 
            case 0x38F: 
            case 0x390: 
            case 0x391: 
            case 0x392: 
            case 0x393: 
            case 0x394: 
            case 0x395: 
            case 0x396: 
            case 0x397: 
            case 0x398: 
            case 0x399: 
            case 0x39A: 
            case 0x39B: 
            case 0x39C: 
            case 0x39D: 
            case 0x39E: 
            case 0x39F: 
            case 0x3A0: 
            case 0x3A1: 
            case 0x3A3: 
            case 0x3A4: 
            case 0x3A5: 
            case 0x3A6: 
            case 0x3A7: 
            case 0x3A8: 
            case 0x3A9: 
            case 0x3AA: 
            case 0x3AB: 
            case 0x3AC: 
            case 0x3AD: 
            case 0x3AE: 
            case 0x3AF: 
            case 0x3B0: 
            case 0x3B1: 
            case 0x3B2: 
            case 0x3B3: 
            case 0x3B4: 
            case 0x3B5: 
            case 0x3B6: 
            case 0x3B7: 
            case 0x3B8: 
            case 0x3B9: 
            case 0x3BA: 
            case 0x3BB: 
            case 0x3BC: 
            case 0x3BD: 
            case 0x3BE: 
            case 0x3BF: 
            case 0x3C0: 
            case 0x3C1: 
            case 0x3C2: 
            case 0x3C3: 
            case 0x3C4: 
            case 0x3C5: 
            case 0x3C6: 
            case 0x3C7: 
            case 0x3C8: 
            case 0x3C9: 
            case 0x3CA: 
            case 0x3CB: 
            case 0x3CC: 
            case 0x3CD: 
            case 0x3CE: 
            case 0x3CF: 
            case 0x3D0: 
            case 0x3D1: 
            case 0x3D2: 
            case 0x3D3: 
            case 0x3D4: 
            case 0x3D5: 
            case 0x3D6: 
            case 0x3D7: 
            case 0x3D8: 
            case 0x3D9: 
            case 0x3DA: 
            case 0x3DB: 
            case 0x3DC: 
            case 0x3DD: 
            case 0x3DE: 
            case 0x3DF: 
            case 0x3E0: 
            case 0x3E1: 
            case 0x3F0: 
            case 0x3F1: 
            case 0x3F2: 
            case 0x3F3: 
            case 0x3F4: 
            case 0x3F5: 
            case 0x3F6: 
            case 0x3F7: 
            case 0x3F8: 
            case 0x3F9: 
            case 0x3FA: 
            case 0x3FB: 
            case 0x3FC: 
            case 0x3FD: 
            case 0x3FE: 
            case 0x3FF: 
            case 0x1D26: 
            case 0x1D27: 
            case 0x1D28: 
            case 0x1D29: 
            case 0x1D2A: 
            case 0x1D5D: 
            case 0x1D5E: 
            case 0x1D5F: 
            case 0x1D60: 
            case 0x1D61: 
            case 0x1D66: 
            case 0x1D67: 
            case 0x1D68: 
            case 0x1D69: 
            case 0x1D6A: 
            case 0x1DBF: 
            case 0x1F00: 
            case 0x1F01: 
            case 0x1F02: 
            case 0x1F03: 
            case 0x1F04: 
            case 0x1F05: 
            case 0x1F06: 
            case 0x1F07: 
            case 0x1F08: 
            case 0x1F09: 
            case 0x1F0A: 
            case 0x1F0B: 
            case 0x1F0C: 
            case 0x1F0D: 
            case 0x1F0E: 
            case 0x1F0F: 
            case 0x1F10: 
            case 0x1F11: 
            case 0x1F12: 
            case 0x1F13: 
            case 0x1F14: 
            case 0x1F15: 
            case 0x1F18: 
            case 0x1F19: 
            case 0x1F1A: 
            case 0x1F1B: 
            case 0x1F1C: 
            case 0x1F1D: goto _102;

        }
    } else {
        if( input < 0x1FC5 ) {
            if( input < 0x1F5B ) {
                switch( input ) {
                    case 0x1F20: 
                    case 0x1F21: 
                    case 0x1F22: 
                    case 0x1F23: 
                    case 0x1F24: 
                    case 0x1F25: 
                    case 0x1F26: 
                    case 0x1F27: 
                    case 0x1F28: 
                    case 0x1F29: 
                    case 0x1F2A: 
                    case 0x1F2B: 
                    case 0x1F2C: 
                    case 0x1F2D: 
                    case 0x1F2E: 
                    case 0x1F2F: 
                    case 0x1F30: 
                    case 0x1F31: 
                    case 0x1F32: 
                    case 0x1F33: 
                    case 0x1F34: 
                    case 0x1F35: 
                    case 0x1F36: 
                    case 0x1F37: 
                    case 0x1F38: 
                    case 0x1F39: 
                    case 0x1F3A: 
                    case 0x1F3B: 
                    case 0x1F3C: 
                    case 0x1F3D: 
                    case 0x1F3E: 
                    case 0x1F3F: 
                    case 0x1F40: 
                    case 0x1F41: 
                    case 0x1F42: 
                    case 0x1F43: 
                    case 0x1F44: 
                    case 0x1F45: 
                    case 0x1F48: 
                    case 0x1F49: 
                    case 0x1F4A: 
                    case 0x1F4B: 
                    case 0x1F4C: 
                    case 0x1F4D: 
                    case 0x1F50: 
                    case 0x1F51: 
                    case 0x1F52: 
                    case 0x1F53: 
                    case 0x1F54: 
                    case 0x1F55: 
                    case 0x1F56: 
                    case 0x1F57: 
                    case 0x1F59: goto _102;

                }
            } else {
                switch( input ) {
                    case 0x1F5B: 
                    case 0x1F5D: 
                    case 0x1F5F: 
                    case 0x1F60: 
                    case 0x1F61: 
                    case 0x1F62: 
                    case 0x1F63: 
                    case 0x1F64: 
                    case 0x1F65: 
                    case 0x1F66: 
                    case 0x1F67: 
                    case 0x1F68: 
                    case 0x1F69: 
                    case 0x1F6A: 
                    case 0x1F6B: 
                    case 0x1F6C: 
                    case 0x1F6D: 
                    case 0x1F6E: 
                    case 0x1F6F: 
                    case 0x1F70: 
                    case 0x1F71: 
                    case 0x1F72: 
                    case 0x1F73: 
                    case 0x1F74: 
                    case 0x1F75: 
                    case 0x1F76: 
                    case 0x1F77: 
                    case 0x1F78: 
                    case 0x1F79: 
                    case 0x1F7A: 
                    case 0x1F7B: 
                    case 0x1F7C: 
                    case 0x1F7D: 
                    case 0x1F80: 
                    case 0x1F81: 
                    case 0x1F82: 
                    case 0x1F83: 
                    case 0x1F84: 
                    case 0x1F85: 
                    case 0x1F86: 
                    case 0x1F87: 
                    case 0x1F88: 
                    case 0x1F89: 
                    case 0x1F8A: 
                    case 0x1F8B: 
                    case 0x1F8C: 
                    case 0x1F8D: 
                    case 0x1F8E: 
                    case 0x1F8F: 
                    case 0x1F90: 
                    case 0x1F91: 
                    case 0x1F92: 
                    case 0x1F93: 
                    case 0x1F94: 
                    case 0x1F95: 
                    case 0x1F96: 
                    case 0x1F97: 
                    case 0x1F98: 
                    case 0x1F99: 
                    case 0x1F9A: 
                    case 0x1F9B: 
                    case 0x1F9C: 
                    case 0x1F9D: 
                    case 0x1F9E: 
                    case 0x1F9F: 
                    case 0x1FA0: 
                    case 0x1FA1: 
                    case 0x1FA2: 
                    case 0x1FA3: 
                    case 0x1FA4: 
                    case 0x1FA5: 
                    case 0x1FA6: 
                    case 0x1FA7: 
                    case 0x1FA8: 
                    case 0x1FA9: 
                    case 0x1FAA: 
                    case 0x1FAB: 
                    case 0x1FAC: 
                    case 0x1FAD: 
                    case 0x1FAE: 
                    case 0x1FAF: 
                    case 0x1FB0: 
                    case 0x1FB1: 
                    case 0x1FB2: 
                    case 0x1FB3: 
                    case 0x1FB4: 
                    case 0x1FB6: 
                    case 0x1FB7: 
                    case 0x1FB8: 
                    case 0x1FB9: 
                    case 0x1FBA: 
                    case 0x1FBB: 
                    case 0x1FBC: 
                    case 0x1FBD: 
                    case 0x1FBE: 
                    case 0x1FBF: 
                    case 0x1FC0: 
                    case 0x1FC1: 
                    case 0x1FC2: 
                    case 0x1FC3: 
                    case 0x1FC4: goto _102;

                }
            }        } else {
            switch( input ) {
                case 0x1FC6: 
                case 0x1FC7: 
                case 0x1FC8: 
                case 0x1FC9: 
                case 0x1FCA: 
                case 0x1FCB: 
                case 0x1FCC: 
                case 0x1FCD: 
                case 0x1FCE: 
                case 0x1FCF: 
                case 0x1FD0: 
                case 0x1FD1: 
                case 0x1FD2: 
                case 0x1FD3: 
                case 0x1FD6: 
                case 0x1FD7: 
                case 0x1FD8: 
                case 0x1FD9: 
                case 0x1FDA: 
                case 0x1FDB: 
                case 0x1FDD: 
                case 0x1FDE: 
                case 0x1FDF: 
                case 0x1FE0: 
                case 0x1FE1: 
                case 0x1FE2: 
                case 0x1FE3: 
                case 0x1FE4: 
                case 0x1FE5: 
                case 0x1FE6: 
                case 0x1FE7: 
                case 0x1FE8: 
                case 0x1FE9: 
                case 0x1FEA: 
                case 0x1FEB: 
                case 0x1FEC: 
                case 0x1FED: 
                case 0x1FEE: 
                case 0x1FEF: 
                case 0x1FF2: 
                case 0x1FF3: 
                case 0x1FF4: 
                case 0x1FF6: 
                case 0x1FF7: 
                case 0x1FF8: 
                case 0x1FF9: 
                case 0x1FFA: 
                case 0x1FFB: 
                case 0x1FFC: 
                case 0x1FFD: 
                case 0x1FFE: 
                case 0x2126: goto _102;

                case 0xD800: goto _103;

                case 0xD834: goto _104;

            }
        }    }_108:
    __quex_debug_drop_out(102);
    goto TERMINAL_15;


    __quex_assert_no_passage();
_101: /* (101 from 101) (101 from 100) */

    ++(me->buffer._input_p);
    input = *(me->buffer._input_p);
    __quex_debug_state(101);
    switch( input ) {
        case 0x0: QUEX_GOTO_RELOAD_FORWARD(101, 109);

        case 0x30: 
        case 0x31: 
        case 0x32: 
        case 0x33: 
        case 0x34: 
        case 0x35: 
        case 0x36: 
        case 0x37: 
        case 0x38: 
        case 0x39: 
        case 0x41: 
        case 0x42: 
        case 0x43: 
        case 0x44: 
        case 0x45: 
        case 0x46: goto _101;

    }
_109:
    __quex_debug_drop_out(101);
    goto TERMINAL_18;


    __quex_assert_no_passage();
_103: /* (103 from 99) (103 from 102) */

    ++(me->buffer._input_p);
    input = *(me->buffer._input_p);
    __quex_debug_state(103);

    if( input >= 0xDD8B ) {

    } else if( input >= 0xDD40 ) {
        goto _102;

    } else if( input >= 0x1 ) {

    } else {
        QUEX_GOTO_RELOAD_FORWARD(103, 110);

}_110:
    __quex_debug_drop_out(103);
    me->buffer._input_p -= 1; 
    goto TERMINAL_15;


    __quex_assert_no_passage();
_104: /* (104 from 99) (104 from 102) */

    ++(me->buffer._input_p);
    input = *(me->buffer._input_p);
    __quex_debug_state(104);

    if( input >= 0xDE46 ) {

    } else if( input >= 0xDE00 ) {
        goto _102;

    } else if( input >= 0x1 ) {

    } else {
        QUEX_GOTO_RELOAD_FORWARD(104, 111);

}_111:
    __quex_debug_drop_out(104);
    me->buffer._input_p -= 1; 
    goto TERMINAL_15;


    __quex_assert_no_passage();
_96: /* (96 from 92) */

    ++(me->buffer._input_p);
    input = *(me->buffer._input_p);
    __quex_debug_state(96);

    if( input >= 0xDD8B ) {

    } else if( input >= 0xDD40 ) {
        goto _102;

    } else if( input >= 0x1 ) {

    } else {
        QUEX_GOTO_RELOAD_FORWARD(96, 112);

}_112:
    __quex_debug_drop_out(96);
    goto TERMINAL_24;


    __quex_assert_no_passage();
_97: /* (97 from 92) */

    ++(me->buffer._input_p);
    __quex_debug_state(97);
    __quex_debug_drop_out(97);
    goto TERMINAL_24;


    __quex_assert_no_passage();
_98: /* (98 from 92) */

    ++(me->buffer._input_p);
    __quex_debug_state(98);
    __quex_debug_drop_out(98);
    goto TERMINAL_21;


    __quex_assert_no_passage();
_99: /* (99 from 92) */

    ++(me->buffer._input_p);
    input = *(me->buffer._input_p);
    __quex_debug_state(99);
    if( input < 0x1F20 ) {
        switch( input ) {
            case 0x0: QUEX_GOTO_RELOAD_FORWARD(99, 115);

            case 0x370: 
            case 0x371: 
            case 0x372: 
            case 0x373: 
            case 0x375: 
            case 0x376: 
            case 0x377: 
            case 0x37A: 
            case 0x37B: 
            case 0x37C: 
            case 0x37D: 
            case 0x384: 
            case 0x386: 
            case 0x388: 
            case 0x389: 
            case 0x38A: 
            case 0x38C: 
            case 0x38E: 
            case 0x38F: 
            case 0x390: 
            case 0x391: 
            case 0x392: 
            case 0x393: 
            case 0x394: 
            case 0x395: 
            case 0x396: 
            case 0x397: 
            case 0x398: 
            case 0x399: 
            case 0x39A: 
            case 0x39B: 
            case 0x39C: 
            case 0x39D: 
            case 0x39E: 
            case 0x39F: 
            case 0x3A0: 
            case 0x3A1: 
            case 0x3A3: 
            case 0x3A4: 
            case 0x3A5: 
            case 0x3A6: 
            case 0x3A7: 
            case 0x3A8: 
            case 0x3A9: 
            case 0x3AA: 
            case 0x3AB: 
            case 0x3AC: 
            case 0x3AD: 
            case 0x3AE: 
            case 0x3AF: 
            case 0x3B0: 
            case 0x3B1: 
            case 0x3B2: 
            case 0x3B3: 
            case 0x3B4: 
            case 0x3B5: 
            case 0x3B6: 
            case 0x3B7: 
            case 0x3B8: 
            case 0x3B9: 
            case 0x3BA: 
            case 0x3BB: 
            case 0x3BC: 
            case 0x3BD: 
            case 0x3BE: 
            case 0x3BF: 
            case 0x3C0: 
            case 0x3C1: 
            case 0x3C2: 
            case 0x3C3: 
            case 0x3C4: 
            case 0x3C5: 
            case 0x3C6: 
            case 0x3C7: 
            case 0x3C8: 
            case 0x3C9: 
            case 0x3CA: 
            case 0x3CB: 
            case 0x3CC: 
            case 0x3CD: 
            case 0x3CE: 
            case 0x3CF: 
            case 0x3D0: 
            case 0x3D1: 
            case 0x3D2: 
            case 0x3D3: 
            case 0x3D4: 
            case 0x3D5: 
            case 0x3D6: 
            case 0x3D7: 
            case 0x3D8: 
            case 0x3D9: 
            case 0x3DA: 
            case 0x3DB: 
            case 0x3DC: 
            case 0x3DD: 
            case 0x3DE: 
            case 0x3DF: 
            case 0x3E0: 
            case 0x3E1: 
            case 0x3F0: 
            case 0x3F1: 
            case 0x3F2: 
            case 0x3F3: 
            case 0x3F4: 
            case 0x3F5: 
            case 0x3F6: 
            case 0x3F7: 
            case 0x3F8: 
            case 0x3F9: 
            case 0x3FA: 
            case 0x3FB: 
            case 0x3FC: 
            case 0x3FD: 
            case 0x3FE: 
            case 0x3FF: 
            case 0x1D26: 
            case 0x1D27: 
            case 0x1D28: 
            case 0x1D29: 
            case 0x1D2A: 
            case 0x1D5D: 
            case 0x1D5E: 
            case 0x1D5F: 
            case 0x1D60: 
            case 0x1D61: 
            case 0x1D66: 
            case 0x1D67: 
            case 0x1D68: 
            case 0x1D69: 
            case 0x1D6A: 
            case 0x1DBF: 
            case 0x1F00: 
            case 0x1F01: 
            case 0x1F02: 
            case 0x1F03: 
            case 0x1F04: 
            case 0x1F05: 
            case 0x1F06: 
            case 0x1F07: 
            case 0x1F08: 
            case 0x1F09: 
            case 0x1F0A: 
            case 0x1F0B: 
            case 0x1F0C: 
            case 0x1F0D: 
            case 0x1F0E: 
            case 0x1F0F: 
            case 0x1F10: 
            case 0x1F11: 
            case 0x1F12: 
            case 0x1F13: 
            case 0x1F14: 
            case 0x1F15: 
            case 0x1F18: 
            case 0x1F19: 
            case 0x1F1A: 
            case 0x1F1B: 
            case 0x1F1C: 
            case 0x1F1D: goto _102;

        }
    } else {
        if( input < 0x1FC5 ) {
            if( input < 0x1F5B ) {
                switch( input ) {
                    case 0x1F20: 
                    case 0x1F21: 
                    case 0x1F22: 
                    case 0x1F23: 
                    case 0x1F24: 
                    case 0x1F25: 
                    case 0x1F26: 
                    case 0x1F27: 
                    case 0x1F28: 
                    case 0x1F29: 
                    case 0x1F2A: 
                    case 0x1F2B: 
                    case 0x1F2C: 
                    case 0x1F2D: 
                    case 0x1F2E: 
                    case 0x1F2F: 
                    case 0x1F30: 
                    case 0x1F31: 
                    case 0x1F32: 
                    case 0x1F33: 
                    case 0x1F34: 
                    case 0x1F35: 
                    case 0x1F36: 
                    case 0x1F37: 
                    case 0x1F38: 
                    case 0x1F39: 
                    case 0x1F3A: 
                    case 0x1F3B: 
                    case 0x1F3C: 
                    case 0x1F3D: 
                    case 0x1F3E: 
                    case 0x1F3F: 
                    case 0x1F40: 
                    case 0x1F41: 
                    case 0x1F42: 
                    case 0x1F43: 
                    case 0x1F44: 
                    case 0x1F45: 
                    case 0x1F48: 
                    case 0x1F49: 
                    case 0x1F4A: 
                    case 0x1F4B: 
                    case 0x1F4C: 
                    case 0x1F4D: 
                    case 0x1F50: 
                    case 0x1F51: 
                    case 0x1F52: 
                    case 0x1F53: 
                    case 0x1F54: 
                    case 0x1F55: 
                    case 0x1F56: 
                    case 0x1F57: 
                    case 0x1F59: goto _102;

                }
            } else {
                switch( input ) {
                    case 0x1F5B: 
                    case 0x1F5D: 
                    case 0x1F5F: 
                    case 0x1F60: 
                    case 0x1F61: 
                    case 0x1F62: 
                    case 0x1F63: 
                    case 0x1F64: 
                    case 0x1F65: 
                    case 0x1F66: 
                    case 0x1F67: 
                    case 0x1F68: 
                    case 0x1F69: 
                    case 0x1F6A: 
                    case 0x1F6B: 
                    case 0x1F6C: 
                    case 0x1F6D: 
                    case 0x1F6E: 
                    case 0x1F6F: 
                    case 0x1F70: 
                    case 0x1F71: 
                    case 0x1F72: 
                    case 0x1F73: 
                    case 0x1F74: 
                    case 0x1F75: 
                    case 0x1F76: 
                    case 0x1F77: 
                    case 0x1F78: 
                    case 0x1F79: 
                    case 0x1F7A: 
                    case 0x1F7B: 
                    case 0x1F7C: 
                    case 0x1F7D: 
                    case 0x1F80: 
                    case 0x1F81: 
                    case 0x1F82: 
                    case 0x1F83: 
                    case 0x1F84: 
                    case 0x1F85: 
                    case 0x1F86: 
                    case 0x1F87: 
                    case 0x1F88: 
                    case 0x1F89: 
                    case 0x1F8A: 
                    case 0x1F8B: 
                    case 0x1F8C: 
                    case 0x1F8D: 
                    case 0x1F8E: 
                    case 0x1F8F: 
                    case 0x1F90: 
                    case 0x1F91: 
                    case 0x1F92: 
                    case 0x1F93: 
                    case 0x1F94: 
                    case 0x1F95: 
                    case 0x1F96: 
                    case 0x1F97: 
                    case 0x1F98: 
                    case 0x1F99: 
                    case 0x1F9A: 
                    case 0x1F9B: 
                    case 0x1F9C: 
                    case 0x1F9D: 
                    case 0x1F9E: 
                    case 0x1F9F: 
                    case 0x1FA0: 
                    case 0x1FA1: 
                    case 0x1FA2: 
                    case 0x1FA3: 
                    case 0x1FA4: 
                    case 0x1FA5: 
                    case 0x1FA6: 
                    case 0x1FA7: 
                    case 0x1FA8: 
                    case 0x1FA9: 
                    case 0x1FAA: 
                    case 0x1FAB: 
                    case 0x1FAC: 
                    case 0x1FAD: 
                    case 0x1FAE: 
                    case 0x1FAF: 
                    case 0x1FB0: 
                    case 0x1FB1: 
                    case 0x1FB2: 
                    case 0x1FB3: 
                    case 0x1FB4: 
                    case 0x1FB6: 
                    case 0x1FB7: 
                    case 0x1FB8: 
                    case 0x1FB9: 
                    case 0x1FBA: 
                    case 0x1FBB: 
                    case 0x1FBC: 
                    case 0x1FBD: 
                    case 0x1FBE: 
                    case 0x1FBF: 
                    case 0x1FC0: 
                    case 0x1FC1: 
                    case 0x1FC2: 
                    case 0x1FC3: 
                    case 0x1FC4: goto _102;

                }
            }        } else {
            switch( input ) {
                case 0x1FC6: 
                case 0x1FC7: 
                case 0x1FC8: 
                case 0x1FC9: 
                case 0x1FCA: 
                case 0x1FCB: 
                case 0x1FCC: 
                case 0x1FCD: 
                case 0x1FCE: 
                case 0x1FCF: 
                case 0x1FD0: 
                case 0x1FD1: 
                case 0x1FD2: 
                case 0x1FD3: 
                case 0x1FD6: 
                case 0x1FD7: 
                case 0x1FD8: 
                case 0x1FD9: 
                case 0x1FDA: 
                case 0x1FDB: 
                case 0x1FDD: 
                case 0x1FDE: 
                case 0x1FDF: 
                case 0x1FE0: 
                case 0x1FE1: 
                case 0x1FE2: 
                case 0x1FE3: 
                case 0x1FE4: 
                case 0x1FE5: 
                case 0x1FE6: 
                case 0x1FE7: 
                case 0x1FE8: 
                case 0x1FE9: 
                case 0x1FEA: 
                case 0x1FEB: 
                case 0x1FEC: 
                case 0x1FED: 
                case 0x1FEE: 
                case 0x1FEF: 
                case 0x1FF2: 
                case 0x1FF3: 
                case 0x1FF4: 
                case 0x1FF6: 
                case 0x1FF7: 
                case 0x1FF8: 
                case 0x1FF9: 
                case 0x1FFA: 
                case 0x1FFB: 
                case 0x1FFC: 
                case 0x1FFD: 
                case 0x1FFE: 
                case 0x2126: goto _102;

                case 0xD800: goto _103;

                case 0xD834: goto _104;

            }
        }    }_115:
    __quex_debug_drop_out(99);
    goto TERMINAL_15;


    __quex_assert_no_passage();
_100: /* (100 from 92) */

    ++(me->buffer._input_p);
    input = *(me->buffer._input_p);
    __quex_debug_state(100);
    switch( input ) {
        case 0x0: QUEX_GOTO_RELOAD_FORWARD(100, 116);

        case 0x30: 
        case 0x31: 
        case 0x32: 
        case 0x33: 
        case 0x34: 
        case 0x35: 
        case 0x36: 
        case 0x37: 
        case 0x38: 
        case 0x39: 
        case 0x41: 
        case 0x42: 
        case 0x43: 
        case 0x44: 
        case 0x45: 
        case 0x46: goto _101;

    }
_116:
    __quex_debug_drop_out(100);
    goto TERMINAL_24;


    __quex_assert_no_passage();
_93: /* (93 from 92) */

    ++(me->buffer._input_p);
    __quex_debug_state(93);
    __quex_debug_drop_out(93);
    goto TERMINAL_24;


    __quex_assert_no_passage();
_94: /* (94 from 92) */

    ++(me->buffer._input_p);
    __quex_debug_state(94);
    __quex_debug_drop_out(94);
    goto TERMINAL_27;


    __quex_assert_no_passage();
_95: /* (95 from 92) */

    ++(me->buffer._input_p);
    input = *(me->buffer._input_p);
    __quex_debug_state(95);

    if( input >= 0xDE46 ) {

    } else if( input >= 0xDE00 ) {
        goto _102;

    } else if( input >= 0x1 ) {

    } else {
        QUEX_GOTO_RELOAD_FORWARD(95, 119);

}_119:
    __quex_debug_drop_out(95);
    goto TERMINAL_24;

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
#   define LexemeL      ((size_t)(me->buffer._input_p - me->buffer._lexeme_start_p))
#   define LexemeEnd    me->buffer._input_p
#endif

#define LexemeNull      (&QUEX_LEXEME_NULL)

TERMINAL_24:
    __quex_debug("* terminal 24:   .\n");
    __QUEX_COUNT_VOID(self.counter);
    {
#   line 20 "greek-other.qx"
    QUEX_NAME_TOKEN(take_text)(self_write_token_p(), &self, self.buffer._lexeme_start_p, self.buffer._input_p);
    self_send(TKN_UNKNOWN);
    QUEX_SETTING_AFTER_SEND_CONTINUE_OR_RETURN();
    
#   line 1732 "UTF16Lex.cpp"

    }
    goto __REENTRY_PREPARATION;

TERMINAL_18:
    __quex_debug("* terminal 18:   {NUMBER}\n");
    __QUEX_COUNT_NEWLINE_N_ZERO_COLUMN_N_FIXED(self.counter, LexemeL);
    {
#   line 18 "greek-other.qx"
    QUEX_NAME_TOKEN(take_text)(self_write_token_p(), &self, self.buffer._lexeme_start_p, self.buffer._input_p);
    self_send(TKN_NUMBER);
    QUEX_SETTING_AFTER_SEND_CONTINUE_OR_RETURN();
    
#   line 1746 "UTF16Lex.cpp"

    }
    goto __REENTRY_PREPARATION;

TERMINAL_27:
    __quex_debug("* terminal 27:   [ \\t\\n]\n");
    __QUEX_COUNT_VOID(self.counter);
    {
#   line 21 "greek-other.qx"
    QUEX_NAME_TOKEN(take_text)(self_write_token_p(), &self, self.buffer._lexeme_start_p, self.buffer._input_p);
    self_send(TKN_WHITE);
    QUEX_SETTING_AFTER_SEND_CONTINUE_OR_RETURN();
    
#   line 1760 "UTF16Lex.cpp"

    }
    goto __REENTRY_PREPARATION;

TERMINAL_21:
    __quex_debug("* terminal 21:   \":\"\n");
    __QUEX_COUNT_NEWLINE_N_ZERO_COLUMN_N_FIXED(self.counter, 1);
    {
#   line 19 "greek-other.qx"
    QUEX_NAME_TOKEN(take_text)(self_write_token_p(), &self, self.buffer._lexeme_start_p, self.buffer._input_p);
    self_send(TKN_COLON);
    QUEX_SETTING_AFTER_SEND_CONTINUE_OR_RETURN();
    
#   line 1774 "UTF16Lex.cpp"

    }
    goto __REENTRY_PREPARATION;

TERMINAL_15:
    __quex_debug("* terminal 15:   {WORD}\n");
    __QUEX_COUNT_NEWLINE_N_ZERO_COLUMN_N_FIXED(self.counter, LexemeL);
    {
#   line 17 "greek-other.qx"
    QUEX_NAME_TOKEN(take_text)(self_write_token_p(), &self, self.buffer._lexeme_start_p, self.buffer._input_p);
    self_send(TKN_WORD);
    QUEX_SETTING_AFTER_SEND_CONTINUE_OR_RETURN();
    
#   line 1788 "UTF16Lex.cpp"

    }
    goto __REENTRY_PREPARATION;

_105: /* TERMINAL: END_OF_STREAM */
    __QUEX_COUNT_END_OF_STREAM_EVENT(self.counter);
    {
#   line 16 "greek-other.qx"
    QUEX_NAME_TOKEN(take_text)(self_write_token_p(), &self, LexemeNull, LexemeNull);
    self_send(TKN_TERMINATION);
    
#   line 1800 "UTF16Lex.cpp"

    }
    /* End of Stream causes a return from the lexical analyzer, so that no
     * tokens can be filled after the termination token.                    */
    RETURN;          

_107: /* TERMINAL: FAILURE */
    if(QUEX_NAME(Buffer_is_end_of_file)(&me->buffer)) {
        /* Init state is going to detect 'input == buffer limit code', and
         * enter the reload procedure, which will decide about 'end of stream'. */
    } else {
        /* In init state 'input = *input_p' and we need to increment
         * in order to avoid getting stalled. Else, input = *(input_p - 1),
         * so 'input_p' points already to the next character.              */
        if( me->buffer._input_p == me->buffer._lexeme_start_p ) {
            /* Step over non-matching character */
            ++(me->buffer._input_p);
        }
    }
    __QUEX_COUNT_VOID(self.counter);
    QUEX_LEXEME_TERMINATING_ZERO_SET(&me->buffer);
    {
QUEX_ERROR_EXIT("\n    Match failure in mode 'X'.\n"
    "    No 'on_failure' section provided for this mode.\n"
    "    Proposal: Define 'on_failure' and analyze 'Lexeme'.\n");

    }
    goto __REENTRY_PREPARATION;

__REENTRY_PREPARATION:
    /* (*) Common point for **restarting** lexical analysis.
     *     at each time when CONTINUE is called at the end of a pattern. */
 

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
#   if    defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE)        || defined(QUEX_OPTION_ASSERTS)
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

    goto __REENTRY;

    __quex_assert_no_passage();
__RELOAD_FORWARD:
    __quex_debug1("__RELOAD_FORWARD");

    __quex_assert(input == QUEX_SETTING_BUFFER_LIMIT_CODE);
    if( me->buffer._memory._end_of_file_p == 0x0 ) {
        __quex_debug_reload_before();
        QUEX_NAME(buffer_reload_forward)(&me->buffer, (QUEX_TYPE_CHARACTER_POSITION*)position, PositionRegisterN);
        __quex_debug_reload_after();
        QUEX_GOTO_STATE(target_state_index);
    }
    __quex_debug("reload impossible\n");
    QUEX_GOTO_STATE(target_state_else_index);
#   ifndef QUEX_OPTION_COMPUTED_GOTOS
    __quex_assert_no_passage();
__STATE_ROUTER:
    switch( target_state_index ) {
        case 92: { goto _92; }
        case 93: { goto _93; }
        case 94: { goto _94; }
        case 95: { goto _95; }
        case 96: { goto _96; }
        case 97: { goto _97; }
        case 98: { goto _98; }
        case 99: { goto _99; }
        case 100: { goto _100; }
        case 101: { goto _101; }
        case 102: { goto _102; }
        case 103: { goto _103; }
        case 104: { goto _104; }
        case 105: { goto _105; }
        case 108: { goto _108; }
        case 109: { goto _109; }
        case 110: { goto _110; }
        case 111: { goto _111; }
        case 112: { goto _112; }
        case 115: { goto _115; }
        case 116: { goto _116; }
        case 119: { goto _119; }

        default:
            __QUEX_STD_fprintf(stderr, "State router: index = %i\n", (int)target_state_index);
            QUEX_ERROR_EXIT("State router: unknown index.");
    }
#   endif /* QUEX_OPTION_COMPUTED_GOTOS */

    /* Prevent compiler warning 'unused variable': use variables once in a part of the code*/
    /* that is never reached (and deleted by the compiler anyway).*/
    (void)QUEX_LEXEME_NULL;
    (void)QUEX_NAME_TOKEN(DumpedTokenIdObject);
    QUEX_ERROR_EXIT("Unreachable code has been reached.\n");
#   undef X
#   undef self
}
#include <quex/code_base/temporary_macros_off>
QUEX_NAMESPACE_MAIN_CLOSE


QUEX_NAMESPACE_TOKEN_OPEN

const char*
QUEX_NAME_TOKEN(map_id_to_name)(const QUEX_TYPE_TOKEN_ID TokenID)
{
   static char  error_string[64];
   static const char  uninitialized_string[] = "<UNINITIALIZED>";
   static const char  termination_string[]   = "<TERMINATION>";
#  if defined(QUEX_OPTION_INDENTATION_TRIGGER)
   static const char  indent_string[]        = "<INDENT>";
   static const char  dedent_string[]        = "<DEDENT>";
   static const char  nodent_string[]        = "<NODENT>";
#  endif
   static const char  token_id_str_COLON[]         = "COLON";
   static const char  token_id_str_NUMBER[]        = "NUMBER";
   static const char  token_id_str_UNKNOWN[]       = "UNKNOWN";
   static const char  token_id_str_WHITE[]         = "WHITE";
   static const char  token_id_str_WORD[]          = "WORD";
       

   /* NOTE: This implementation works only for token id types that are 
    *       some type of integer or enum. In case an alien type is to
    *       used, this function needs to be redefined.                  */
   switch( TokenID ) {
   default: {
       __QUEX_STD_sprintf(error_string, "<UNKNOWN TOKEN-ID: %i>", (int)TokenID);
       return error_string;
   }
   case TKN_TERMINATION:    return termination_string;
   case TKN_UNINITIALIZED:  return uninitialized_string;
#  if defined(QUEX_OPTION_INDENTATION_TRIGGER)
   case TKN_INDENT:         return indent_string;
   case TKN_DEDENT:         return dedent_string;
   case TKN_NODENT:         return nodent_string;
#  endif
   case TKN_COLON:         return token_id_str_COLON;
   case TKN_NUMBER:        return token_id_str_NUMBER;
   case TKN_UNKNOWN:       return token_id_str_UNKNOWN;
   case TKN_WHITE:         return token_id_str_WHITE;
   case TKN_WORD:          return token_id_str_WORD;

   }
}

QUEX_NAMESPACE_TOKEN_CLOSE

