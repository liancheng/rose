import quex.engine.state_machine.index             as     sm_index
from   quex.engine.generator.skipper.common        import line_counter_in_loop, \
                                                          end_delimiter_is_subset_of_indentation_counter_newline, \
                                                          get_character_sequence, \
                                                          get_on_skip_range_open, \
                                                          line_column_counter_in_loop
from   quex.engine.generator.languages.address     import __nice, get_label
import quex.engine.generator.languages.variable_db as     variable_db
from   quex.blackboard                             import setup as Setup
from   quex.engine.misc.string_handling            import blue_print
import quex.blackboard                             as     blackboard
from   quex.blackboard                             import E_StateIndices
from   copy                                        import copy

def do(Data):
    ClosingSequence = Data["closer_sequence"]
    ModeName        = Data["mode_name"]

    assert type(ModeName) in [str, unicode]
    assert Data.has_key("indentation_counter_terminal_id")
    
    indentation_counter_terminal_id = Data["indentation_counter_terminal_id"]

    Mode = None
    if ModeName != "":
        Mode = blackboard.mode_db[ModeName]

    code_str, db = get_skipper(ClosingSequence, Mode, indentation_counter_terminal_id) 

    return code_str, db

template_str = """
    $$DELIMITER_COMMENT$$
    text_end = QUEX_NAME(Buffer_text_end)(&me->buffer);
$$LC_COUNT_COLUMN_N_POINTER_DEFINITION$$

$$ENTRY$$
    QUEX_BUFFER_ASSERT_CONSISTENCY(&me->buffer);
    __quex_assert(QUEX_NAME(Buffer_content_size)(&me->buffer) >= Skipper$$SKIPPER_INDEX$$L );

    /* NOTE: If _input_p == end of buffer, then it will drop out immediately out of the
     *       loop below and drop into the buffer reload procedure.                      */

    /* Loop eating characters: Break-out as soon as the First Character of the Delimiter
     * (FCD) is reached. Thus, the FCD plays also the role of the Buffer Limit Code. There
     * are two reasons for break-out:
     *    (1) we reached a limit (end-of-file or buffer-limit)
     *    (2) there was really the FCD in the character stream
     * This must be distinguished after the loop was exited. But, during the 'swallowing' we
     * are very fast, because we do not have to check for two different characters.        */
    *text_end = Skipper$$SKIPPER_INDEX$$[0]; /* Overwrite BufferLimitCode (BLC).  */
_$$SKIPPER_INDEX$$_LOOP:
        $$INPUT_GET$$ 
        $$IF_INPUT_EQUAL_DELIMITER_0$$
            goto _$$SKIPPER_INDEX$$_LOOP_EXIT;
        $$ENDIF$$
$$LC_COUNT_IN_LOOP$$
        $$INPUT_P_INCREMENT$$ /* Now, BLC cannot occur. See above. */
    goto _$$SKIPPER_INDEX$$_LOOP;
_$$SKIPPER_INDEX$$_LOOP_EXIT:

    *text_end = QUEX_SETTING_BUFFER_LIMIT_CODE; /* Reset BLC. */

    /* Case (1) and (2) from above can be distinguished easily: 
     *
     *   (1) Distance to text end == 0: 
     *         End-of-File or Buffer-Limit. 
     *         => goto to drop-out handling
     *
     *   (2) Else:                      
     *         First character of delimit reached. 
     *         => For the verification of the tail of the delimiter it is 
     *            essential that it is loaded completely into the buffer. 
     *            For this, it must be required:
     *
     *                Distance to text end >= Delimiter length 
     *
     *                _input_p    end
     *                    |        |           end - _input_p >= 3
     *                [ ][R][E][M][#]          
     * 
     *         The case of reload should be seldom and is costy anyway. 
     *         Thus let's say, that in this case we simply enter the drop 
     *         out and start the search for the delimiter all over again.
     *
     *         (2.1) Distance to text end < Delimiter length
     *                => goto to drop-out handling
     *         (2.2) Start detection of tail of delimiter
     *
     */
    if( QUEX_NAME(Buffer_distance_input_to_text_end)(&me->buffer) < (ptrdiff_t)Skipper$$SKIPPER_INDEX$$L ) {
        /* (2.1) Reload required. */
        goto $$GOTO_RELOAD$$;
    }
    $$LC_ON_FIRST_DELIMITER$$
    /* (2.2) Test the remaining delimiter, but note, that the check must restart at '_input_p + 1'
     *       if any later check fails. */
    $$INPUT_P_INCREMENT$$
    /* Example: Delimiter = '*', '/'; if we get ...[*][*][/]... then the the first "*" causes 
     *          a drop out out of the 'swallowing loop' and the second "*" will mismatch 
     *          the required "/". But, then the second "*" must be presented to the
     *          swallowing loop and the letter after it completes the 'match'.
     * (The whole discussion, of course, is superflous if the range delimiter has length 1.)  */
$$DELIMITER_REMAINDER_TEST$$            
    {
        /* NOTE: The initial state does not increment the input_p. When it detects that
         * it is located on a buffer border, it automatically triggers a reload. No 
         * need here to reload the buffer. */
$$LC_COUNT_END_PROCEDURE$$
        /* No need for re-entry preparation. Acceptance flags and modes are untouched after skipping. */
        $$GOTO_AFTER_END_OF_SKIPPING$$ /* End of range reached. */
    }

$$RELOAD$$:
    QUEX_BUFFER_ASSERT_CONSISTENCY_LIGHT(&me->buffer);
    /* -- When loading new content it is checked that the beginning of the lexeme
     *    is not 'shifted' out of the buffer. In the case of skipping, we do not care about
     *    the lexeme at all, so do not restrict the load procedure and set the lexeme start
     *    to the actual input position.                                                    */
    $$MARK_LEXEME_START$$

$$LC_COUNT_BEFORE_RELOAD$$
    /* -- According to case (2.1) is is possible that the _input_p does not point to the end
     *    of the buffer, thus we record the current position in the lexeme start pointer and
     *    recover it after the loading. */
    me->buffer._input_p = text_end;
    if( QUEX_NAME(Buffer_is_end_of_file)(&me->buffer) == false ) {
        QUEX_NAME(buffer_reload_forward)(&me->buffer, (QUEX_TYPE_CHARACTER_POSITION*)position, PositionRegisterN);
        /* Recover '_input_p' from lexeme start 
         * (inverse of what we just did before the loading) */
        $$INPUT_P_TO_LEXEME_START$$
        /* After reload, we need to increment _input_p. That's how the game is supposed to be played. 
         * But, we recovered from lexeme start pointer, and this one does not need to be incremented. */
        text_end = QUEX_NAME(Buffer_text_end)(&me->buffer);
$$LC_COUNT_AFTER_RELOAD$$
        QUEX_BUFFER_ASSERT_CONSISTENCY(&me->buffer);
        $$GOTO_ENTRY$$
    }
    /* Here, either the loading failed or it is not enough space to carry a closing delimiter */
    $$INPUT_P_TO_LEXEME_START$$
    $$ON_SKIP_RANGE_OPEN$$
"""

def get_skipper(EndSequence, Mode=None, IndentationCounterTerminalID=None, OnSkipRangeOpenStr=""):
    assert type(EndSequence) == list
    assert len(EndSequence) >= 1
    assert map(type, EndSequence) == [int] * len(EndSequence)

    local_variable_db = {}

    global template_str

    LanguageDB   = Setup.language_db

    # Name the $$SKIPPER$$
    skipper_index = sm_index.get()

    # Determine the $$DELIMITER$$
    delimiter_str, delimiter_comment_str = get_character_sequence(EndSequence)
    delimiter_length                     = len(EndSequence)

    tmp = []
    LanguageDB.COMMENT(tmp, "                         Delimiter: %s" % delimiter_comment_str)
    delimiter_comment_str = "".join(tmp)
    # Determine the check for the tail of the delimiter
    delimiter_remainder_test_str = ""
    if len(EndSequence) != 1: 
        txt = ""
        i = 0
        for letter in EndSequence[1:]:
            i += 1
            txt += "    %s\n"    % LanguageDB.ASSIGN("input", LanguageDB.INPUT_P_DEREFERENCE(i-1))
            txt += "    %s"      % LanguageDB.IF_INPUT("!=", "Skipper$$SKIPPER_INDEX$$[%i]" % i)
            txt += "         %s" % LanguageDB.GOTO(skipper_index)
            txt += "    %s"      % LanguageDB.END_IF()
        delimiter_remainder_test_str = txt

    if not end_delimiter_is_subset_of_indentation_counter_newline(Mode, EndSequence):
        goto_after_end_of_skipping_str = LanguageDB.GOTO(E_StateIndices.ANALYZER_REENTRY)
    else:
        # If there is indentation counting involved, then the counter's terminal id must
        # be determined at this place.
        assert IndentationCounterTerminalID is not None
        # If the ending delimiter is a subset of what the 'newline' pattern triggers 
        # in indentation counting => move on to the indentation counter.
        goto_after_end_of_skipping_str = LanguageDB.GOTO_TERMINAL(IndentationCounterTerminalID)

    if OnSkipRangeOpenStr != "": on_skip_range_open_str = OnSkipRangeOpenStr
    else:                        on_skip_range_open_str = get_on_skip_range_open(Mode, EndSequence)

    # The main part
    code_str = blue_print(template_str,
                          [
                           ["$$DELIMITER_COMMENT$$",              delimiter_comment_str],
                           ["$$INPUT_P_INCREMENT$$",              LanguageDB.INPUT_P_INCREMENT()],
                           ["$$INPUT_P_DECREMENT$$",              LanguageDB.INPUT_P_DECREMENT()],
                           ["$$INPUT_GET$$",                      LanguageDB.ACCESS_INPUT()],
                           ["$$IF_INPUT_EQUAL_DELIMITER_0$$",     LanguageDB.IF_INPUT("==", "Skipper$$SKIPPER_INDEX$$[0]")],
                           ["$$ENDIF$$",                          LanguageDB.END_IF()],
                           ["$$ENTRY$$",                          LanguageDB.LABEL(skipper_index)],
                           ["$$RELOAD$$",                         get_label("$reload", skipper_index)],
                           ["$$GOTO_ENTRY$$",                     LanguageDB.GOTO(skipper_index)],
                           ["$$INPUT_P_TO_LEXEME_START$$",        LanguageDB.INPUT_P_TO_LEXEME_START()],
                           # When things were skipped, no change to acceptance flags or modes has
                           # happend. One can jump immediately to the start without re-entry preparation.
                           ["$$GOTO_AFTER_END_OF_SKIPPING$$",     goto_after_end_of_skipping_str], 
                           ["$$MARK_LEXEME_START$$",              LanguageDB.LEXEME_START_SET()],
                           ["$$DELIMITER_REMAINDER_TEST$$",       delimiter_remainder_test_str],
                           ["$$ON_SKIP_RANGE_OPEN$$",             on_skip_range_open_str],
                          ])

    # Line and column number counting
    code_str, reference_p_f = __lc_counting_replacements(code_str, EndSequence)

    # The finishing touch
    code_str = blue_print(code_str,
                          [["$$SKIPPER_INDEX$$", __nice(skipper_index)],
                           ["$$GOTO_RELOAD$$",   get_label("$reload", skipper_index)]])

    if reference_p_f:
        variable_db.enter(local_variable_db, "reference_p", Condition="QUEX_OPTION_COLUMN_NUMBER_COUNTING")

    variable_db.enter(local_variable_db, "Skipper%i", "{ %s }" % delimiter_str, delimiter_length, Index=skipper_index)
    variable_db.enter(local_variable_db, "Skipper%iL", "%i" % delimiter_length,                   Index=skipper_index)
    variable_db.enter(local_variable_db, "text_end")
    return code_str, local_variable_db

def __lc_counting_replacements(code_str, EndSequence):
    """Line and Column Number Counting(Range Skipper):
     
         -- in loop if there appears a newline, then do:
            increment line_n
            set position from where to count column_n
         -- at end of skipping do one of the following:
            if end delimiter contains newline:
               column_n = number of letters since last new line in end delimiter
               increment line_n by number of newlines in end delimiter.
               (NOTE: in this case the setting of the position from where to count
                      the column_n can be omitted.)
            else:
               column_n = current_position - position from where to count column number.

       NOTE: On reload we do count the column numbers and reset the column_p.
    """
    LanguageDB = Setup.language_db


    def get_character_n_after_last_newline(Sequence):
        tmp = copy(Sequence)
        tmp.reverse()
        try:    return tmp.index(ord('\n'))
        except: return -1

    char_n_after_last_newline = get_character_n_after_last_newline(EndSequence)

    reference_p_def = ""

    in_loop         = ""
    end_procedure   = ""
    before_reload   = ""
    after_reload    = ""
    on_first_delimiter = ""

    reference_p_required_f = False

    # Line/Column Counting:
    newline_number_in_delimiter = EndSequence.count(ord('\n'))

    if EndSequence == map(ord, "\n") or EndSequence == map(ord, "\r\n"):
        #  (1) If the end-delimiter is a newline 
        #      => there cannot appear a newline inside the comment
        #      => IN LOOP: no line number increment
        #                  no reference pointer required for column counting
        end_procedure += "        __QUEX_IF_COUNT_COLUMNS_SET((size_t)1);\n" 
        end_procedure += "        __QUEX_IF_COUNT_LINES_ADD((size_t)1);\n"

    else:
        #  (2) If end-delimiter is NOT newline
        #      => there can appear a newline inside the comment
        if newline_number_in_delimiter == 0:
            # -- no newlines in delimiter => line and column number 
            #                                must be counted.
            in_loop       = line_column_counter_in_loop()
            end_procedure = "        __QUEX_IF_COUNT_COLUMNS_ADD((size_t)(QUEX_NAME(Buffer_tell_memory_adr)(&me->buffer)\n" + \
                            "                                    - reference_p));\n" 
            reference_p_required_f = True
        else:
            # -- newline inside delimiter => line number must be counted
            #                                column number is fixed.
            end_procedure = "        __QUEX_IF_COUNT_COLUMNS_SET((size_t)%i);\n" \
                            % (char_n_after_last_newline + 1)

            if    EndSequence[0] == ord('\n') \
               or len(EndSequence) > 1 and EndSequence[0:2] == [ord('\r'), ord('\n')]: 
                # If the first character in the sequence is newline, then the line counting
                # may is prevented by the loop exit. Now, we need to count.
                on_first_delimiter = "/* First delimiter char was a newline */\n" + \
                                     "    __QUEX_IF_COUNT_LINES_ADD((size_t)1);\n" 
                end_procedure += "        __QUEX_IF_COUNT_LINES_ADD((size_t)%i);\n" % (newline_number_in_delimiter - 1)
            else:
                in_loop        = line_counter_in_loop()
                end_procedure += "        __QUEX_IF_COUNT_LINES_ADD((size_t)%i);\n" % newline_number_in_delimiter

        
    if reference_p_required_f:
        reference_p_def = "    __QUEX_IF_COUNT_COLUMNS(reference_p = QUEX_NAME(Buffer_tell_memory_adr)(&me->buffer));\n"
        before_reload   = "    __QUEX_IF_COUNT_COLUMNS_ADD((size_t)(QUEX_NAME(Buffer_tell_memory_adr)(&me->buffer)\n" + \
                          "                                - reference_p));\n" 
        after_reload    = "        __QUEX_IF_COUNT_COLUMNS(reference_p = QUEX_NAME(Buffer_tell_memory_adr)(&me->buffer));\n"

    if len(EndSequence) > 1:
        end_procedure = "%s\n%s" % (LanguageDB.INPUT_P_ADD(len(EndSequence)-1), end_procedure)

    return blue_print(code_str,
                     [["$$LC_COUNT_COLUMN_N_POINTER_DEFINITION$$", reference_p_def],
                      ["$$LC_COUNT_IN_LOOP$$",                     in_loop],
                      ["$$LC_COUNT_END_PROCEDURE$$",               end_procedure],
                      ["$$LC_COUNT_BEFORE_RELOAD$$",               before_reload],
                      ["$$LC_COUNT_AFTER_RELOAD$$",                after_reload],
                      ["$$LC_ON_FIRST_DELIMITER$$",                on_first_delimiter],
                      ]), \
           reference_p_required_f


