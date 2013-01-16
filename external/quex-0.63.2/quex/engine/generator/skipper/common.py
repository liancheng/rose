import quex.engine.utf8                   as utf8
import quex.output.cpp.action_preparation as action_preparation
from   quex.engine.interval_handling      import NumberSet
from   quex.engine.misc.file_in           import error_msg
from   quex.blackboard import setup as Setup

__line_counter_in_loop = """
    __QUEX_IF_COUNT_LINES_IF( input == (QUEX_TYPE_CHARACTER)%s ) { 
        __QUEX_IF_COUNT_LINES_ADD((size_t)1);
    }
"""

def line_counter_in_loop():
    TrafoInfo = Setup.buffer_codec_transformation_info
    if TrafoInfo is None: return __line_counter_in_loop % "'\\n'"

    newline_code = get_newline_in_codec(TrafoInfo)
    if newline_code is None: return "" # Codec does not have newline
    else:                    return __line_counter_in_loop % newline_code


__line_column_counter_in_loop = """
    __QUEX_IF_COUNT_IF( input == (QUEX_TYPE_CHARACTER)%s ) { 
        __QUEX_IF_COUNT_LINES_ADD((size_t)1);
        __QUEX_IF_COUNT_COLUMNS_SET((size_t)0);
        __QUEX_IF_COUNT_COLUMNS(reference_p = QUEX_NAME(Buffer_tell_memory_adr)(&me->buffer));
    }
"""

def line_column_counter_in_loop():
    TrafoInfo = Setup.buffer_codec_transformation_info
    if TrafoInfo is None: return __line_column_counter_in_loop % "'\\n'"

    newline_code = get_newline_in_codec(TrafoInfo)
    if newline_code is None: return "" # Codec does not have newline
    else:                    return __line_column_counter_in_loop % newline_code

def get_newline_in_codec(TrafoInfo):
    """Translate the code for the newline character into the given codec by 'TrafoInfo'.

       RETURNS: None if the transformation is not possible.
    """
    tmp = NumberSet(ord('\n'))
    if isinstance(TrafoInfo, (str, unicode)):
        if   TrafoInfo == "utf8-state-split":  pass
        elif TrafoInfo == "utf16-state-split": pass
        else:                                  
            error_msg("Character encoding '%s' unknown to skipper.\n" % TrafoInfo + \
                      "For line number counting assume code of newline character code to be '0x%02X'." % ord('\n'),
                      DontExitF=True)
        return ord('\n')

    tmp.transform(TrafoInfo)
    return tmp.get_the_only_element() # Returns 'None' if there is none


def get_character_sequence(Sequence):
    txt         = ""
    comment_txt = ""
    for letter in Sequence:
        comment_txt += "%s, " % utf8.unicode_to_pretty_utf8(letter)
        txt += "0x%X, " % letter

    return txt, comment_txt

def end_delimiter_is_subset_of_indentation_counter_newline(Mode, EndSequence):
    if Mode is None: return False

    indentation_setup = Mode.options.get("indentation")
    if indentation_setup is None: return False

    return indentation_setup.newline_state_machine.get().does_sequence_match(EndSequence)

def get_on_skip_range_open(Mode, CloserSequence):
    """For unit tests 'Mode' may actually be a string, so that we do not
       have to generate a whole mode just to get the 'on_skip_range_open' 
       code fragment.
    """
    if Mode is None: return ""

    txt = ""
    if type(Mode) in [str, unicode]:
        txt += Mode

    elif not Mode.has_code_fragment_list("on_skip_range_open"):
        txt += 'QUEX_ERROR_EXIT("\\nLexical analyzer mode \'%s\':\\n"\n' % Mode.name + \
               '                "End of file occurred before closing skip range delimiter!\\n"' + \
               '                "The \'on_skip_range_open\' handler has not been specified.");'
    else:
        closer_string = ""
        for letter in CloserSequence:
            closer_string += utf8.unicode_to_pretty_utf8(letter).replace("'", "")

        code, eol_f = action_preparation.get_code(Mode.get_code_fragment_list("on_skip_range_open"))
        txt += "#define Closer \"%s\"\n" % closer_string
        txt += code
        txt += "#undef  Closer\n"
        txt += "RETURN;\n"

    return txt


