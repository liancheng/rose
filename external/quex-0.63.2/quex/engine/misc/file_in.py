#! /usr/bin/env python
# Quex is  free software;  you can  redistribute it and/or  modify it  under the
# terms  of the  GNU Lesser  General  Public License  as published  by the  Free
# Software Foundation;  either version 2.1 of  the License, or  (at your option)
# any later version.
# 
# This software is  distributed in the hope that it will  be useful, but WITHOUT
# ANY WARRANTY; without even the  implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the  GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License along
# with this  library; if not,  write to the  Free Software Foundation,  Inc., 59
# Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
################################################################################
from   copy    import copy
from   string  import split
from   StringIO import StringIO
import codecs
import os
import sys

import quex.engine.misc.similarity  as similarity

class EndOfStreamException(Exception):
    pass

temporary_files  = []

def skip_whitespace(fh):
    def __skip_until_newline(fh):
        tmp = ""
        while tmp != '\n':
            tmp = fh.read(1)
            if tmp == "": raise EndOfStreamException()

    while 1 + 1 == 2:
        tmp = fh.read(1)

        if   tmp.isspace(): continue
        elif tmp == "": raise EndOfStreamException()

        # -- character was not a whitespace character
        #    => is there a '//' or a '/*' -comment ?
        tmp2 = fh.read(1)
        if tmp2 == "": fh.seek(-1, 1); return

        tmp += tmp2
        if tmp == "//":   
            __skip_until_newline(fh)

        elif tmp == "/*":
            # skip until '*/'
            previous = " "
            while tmp != "":
                tmp = fh.read(1)
                if tmp == "": raise EndOfStreamException()
                if previous + tmp == "*/":
                    break
                previous = tmp
        else:
            # no whitespace, no comment --> real character
            fh.seek(-2, 1)
            return                

def is_identifier_start(character):
    if len(character) != 1:
        # It is theoretically possible that a character > 0x10000 arrives on a python
        # narrow build.
        error_msg("The underlying python build cannot handle character '%s'." % character)

    char_value = ord(character)
    return    character == "_" \
           or (char_value >= ord('a') and char_value <= ord('z')) \
           or (char_value >= ord('A') and char_value <= ord('Z'))

def is_identifier_continue(character):
    if len(character) != 1:
        # It is theoretically possible that a character > 0x10000 arrives on a python
        # narrow build.
        error_msg("The underlying python build cannot handle character '%s'." % character)

    char_value = ord(character)
    return    is_identifier_start(character) \
           or (char_value >= ord('0') and char_value <= ord('9'))

def is_identifier(identifier, TolerantF=False):
    if identifier == "": return False

    if TolerantF:
        if not is_identifier_continue(identifier[0]): return False
    else:
        if not is_identifier_start(identifier[0]): return False

    if len(identifier) == 1: return True

    for letter in identifier[1:]:
        if is_identifier_continue(letter) == False: return False

    return True

def read_identifier(fh, TolerantF=False):
    txt = fh.read(1)
    if txt == "": return ""

    if TolerantF:
        if is_identifier_continue(txt) == False: fh.seek(-1, 1); return ""
    else:
        if is_identifier_start(txt) == False: fh.seek(-1, 1); return ""

    while 1 + 1 == 2:
        tmp = fh.read(1)
        if tmp == "": return txt

        if is_identifier_continue(tmp): txt += tmp
        else:                           fh.seek(-1, 1); return txt

def find_end_of_identifier(Txt, StartIdx, L):
    for i in range(StartIdx, L):
        if not is_identifier_continue(Txt[i]): return i
    else:
        return L

def check_or_die(fh, What, Comment = "."):
    if not check(fh, What):
        error_msg("Missing '%s'" % What + Comment, fh)

def parse_assignment(fh, Comment=""):
    check_or_die(fh, "=", " for assignment.")

    skip_whitespace(fh)
    identifier = read_until_letter(fh, [";"])

    return identifier.strip()

def parse_identifier_assignment(fh):
    # NOTE: Catching of EOF happens in caller
    check_or_die(fh, "=", " for assignment")

    skip_whitespace(fh)
    identifier = read_identifier(fh)

    check_or_die(fh, ";", " for assignment. Since quex version 0.33.5 this is required.")

    return identifier.strip()

def read_namespaced_name(FileHandle_or_String, Meaning, AllowEmptyF=False):
    string_f = False
    if type(FileHandle_or_String) in [str, unicode]:
        fh = StringIO(FileHandle_or_String)
        string_f = True
    else:
        fh = FileHandle_or_String

    # Overstep a starting '::' if present.
    if fh.read(2) != "::": fh.seek(-2, 1)

    # Parsing the namespace definition
    try:
        name_list  = [""]   # Signalize Empty by <<name_list[-1] == "">>
        while 1 + 1 == 2:
            skip_whitespace(fh)
            name = read_identifier(fh)
            if name == "": break

            name_list[-1] = name
            if not check(fh, "::"): break
            name_list.append("")
    except:
        pass

    # Error Check
    if name_list[-1] == "": # Empty, or last name missing?
        if AllowEmptyF: 
            return "", [], ""
        else:
            if string_f: fh = -1
            error_msg("Missing identifier in %s name specification." % Meaning, fh)

    if string_f: 
        trailing_chars = fh.read()
        if trailing_chars != "":
            error_msg("Trailing characters '%s' in '%s'\nfor %s name specification." % \
                      (trailing_chars, FileHandle_or_String, Meaning))

    return name_list[-1], name_list[:-1], reduce(lambda x, y: x + "_" + y, name_list)  

def get_text_line_n(Txt, Pos):
    line_n = 1
    for i in range(0, Pos):
        if Txt[i] == "\n": line_n += 1
    return line_n

def delete_comment(Content, Opener, Closer, LeaveNewlineDelimiter=False):
    # delete comment lines in C++ form
    new_content = ""
    prev_i      = 0
    while 1 + 1 == 2:
        i = Content.find(Opener, prev_i)
        if i == -1: 
            new_content += Content[prev_i:]
            break
        new_content += Content[prev_i:i]
        prev_i = i + len(Opener)
        i = Content.find(Closer, prev_i)
        if i == -1: break
        if LeaveNewlineDelimiter:
            new_content += "\n" * Content[prev_i:i+len(Closer)].count("\n")
        prev_i = i + len(Closer)

    return new_content

def read_integer(fh):
    pos = fh.tell()

    first = fh.read(1)
    if first == "" or first.isdigit() == False: 
        fh.seek(pos)
        return None

    txt  = ""
    base = None
    if first == "0":
        second = fh.read(1)
        if   second == "":  return 0
        elif second == "x": base = 16;       digit_list = "0123456789abcdefABCDEF."
        elif second == "o": base = 8;        digit_list = "01234567."
        elif second == "b": base = 2;        digit_list = "01."
        elif second == "r": base = "roman";  digit_list = "MCDXLIVmcdxliv"
        elif second.isdigit(): txt = second; # base = None --> base = 10, see below
        elif second == ".":
            error_msg("Decimal integer number cannot contain '.'.", fh)
        else:
            error_msg("Number format '0%s' is not supported by quex.\n" % second + \
                      "Use prefix '0x' for hexadecimal numbers.\n" + \
                      "           '0o' for octal numbers.\n"       + \
                      "           '0b' for binary numbers.\n"      + \
                      "           '0r' for roman numbers.\n"      + \
                      "           and no prefix for decimal numbers.", fh)
    else:
        txt = first

    if base is None:        base = 10;       digit_list = "0123456789"

    while 1 + 1 == 2:
        tmp = fh.read(1)
        if   tmp == "": break
        elif tmp not in digit_list: fh.seek(-1, 1); break
        txt += tmp

    # If we drop out on a digit, then let us assume that the user just missed a point
    if tmp.isdigit() or (tmp in list("ABCDEFabcdef")):
        error_msg("Digit '%s' cannot be part of an expression of base %s." % (tmp, base), fh)

    txt = txt.replace(".", "")
    if len(txt) == 0:
        if base in [2, 8, 16, "roman"]:
            error_msg("Missing digits after '0%s' base %s, found '%s'." % (second, repr(base), tmp), fh)
        fh.seek(pos)
        return None

    # Octal, decimal, and hexadecimal numbers
    if base in [2, 8, 10, 16]:
        return int(txt, base)

    elif base == "roman":
        return __roman_number(txt, fh)

    # Binary numbers
    result = 0
    for letter in txt:
        result <<= 1
        if letter == "1": result |= 1
    return result

def __roman_number(Text, fh):
    """Source: http://code.activestate.com -- Code Recipes 
               Recipe 81611 by Paul Winkler.
    """
    input = Text.upper() 
    # map of (numeral, value, maxcount) tuples
    roman_numeral_map = (('M',  1000, None), ('CM', 900, 1),
                         ('D',  500, 1),     ('CD', 400, 1),
                         ('C',  100, 3),     ('XC', 90, 1),
                         ('L',  50, 1),      ('XL', 40, 1),
                         ('X',  10, 3),      ('IX', 9, 1),
                         ('V',  5, 1),       ('IV', 4, 1), 
                         ('I',  1, 3))

    result, index = 0, 0
    for numeral, value, maxcount in roman_numeral_map:
        count = 0
        while input[index: index + len(numeral)] == numeral:
            count += 1 # how many of this numeral we have
            if maxcount is not None and count > maxcount:
                error_msg("input 0r%s is not a valid roman numeral." % Text, fh)
            result += value
            index  += len(numeral)

    if index < len(input): # There are characters unaccounted for.
        error_msg("input 0r%s is not a valid roman numeral." % Text, fh)

    return result

def extract_identifiers_with_specific_sub_string(Content, SubString):
    L = len(Content)
    i = 0
    finding_list = []
    while 1 + 1 == 2:
        i = Content.find(SubString, i)
        # not found?
        if i == -1: break
        # is it inside an identifier?

        # go back to the begin of the identifier
        while i > 0 and (is_identifier_continue(Content[i]) or is_identifier_continue(Content[i])): 
            i -= 1

        if i != 0 and is_identifier_start(Content[i-1]): i += 1; continue
        end_i = find_end_of_identifier(Content, i, L)
        finding_list.append([Content[i:end_i], get_text_line_n(Content, i)])
        i = end_i
    return finding_list

def extract_identifiers_with_specific_prefix(Content, Prefix):
    L = len(Content)
    i = 0
    finding_list = []
    while 1 + 1 == 2:
        i = Content.find(Prefix, i)
        # not found?
        if i == -1: break
        # is it inside an identifier?
        if i != 0 and is_identifier_start(Content[i-1]): i += 1; continue
        end_i = find_end_of_identifier(Content, i, L)
        finding_list.append([Content[i:end_i], get_text_line_n(Content, i)])
        i = end_i
    return finding_list

def read_until_whitespace(fh):
    txt = ""
    previous_tmp = ""
    while 1 + 1 == 2:
        tmp = fh.read(1)
        if   tmp == "": 
            if txt == "": raise EndOfStreamException()
            else:         return txt

        elif tmp.isspace():                                fh.seek(-1, 1); return txt
        elif previous_tmp == "/" and (tmp in ["*",  "/"]): fh.seek(-2, 1); return txt[:-1]
        txt += tmp
        previous_tmp = tmp

def read_until_closing_bracket(fh, Opener, Closer,
                               IgnoreRegions = [ ['"', '"'],      # strings
                                                 ['\'', '\''],    # characters
                                                 ["//", "\n"],    # c++ comments
                                                 ["/*", "*/"] ],  # c comments
                               SkipClosingDelimiterF = True):                    
    """This function does not eat the closing bracket from the stream.
    """                                                             
    # print "# # read_until_closing_bracket: ", Opener, ", ", Closer, ", ", IgnoreRegions

    open_brackets_n = 1
    backslash_f     = False
    txt     = ""
    CacheSz = max(len(Opener), len(Closer))
    if len(IgnoreRegions) != 0: 
        # check for correct type, because this can cause terrible errors
        assert type(IgnoreRegions) == list

        for element in IgnoreRegions:                                    
            assert type(element) == list
                                                 
        CacheSz = max(map(lambda delimiter: len(delimiter[0]), IgnoreRegions) + [ CacheSz ])

    cache = ["\0"] * CacheSz

    def match_against_cache(Delimiter):
        """Determine whether the string 'Delimiter' is flood into the cache or not."""
        assert len(Delimiter) <= len(cache), \
               "error: read_until_closing_bracket() cache smaller than delimiter"

        # delimiter == "" means that it is, in fact, not a delimiter (disabled)    
        if Delimiter == "": return False
        L = len(Delimiter)
        i = -1
        for letter in Delimiter:
            i += 1
            if letter != cache[L-i-1]: return False
        return True

    # ignore_start_triggers = map(lamda x: x[0], IgnoreRegions)
    while 1 + 1 == 2:
        tmp = fh.read(1)
        txt += tmp
        cache.insert(0, tmp)  # element 0 last element flood into cache (current)
        cache.pop(-1)         # element N first element                 (oldest)

        if tmp == "":         
            raise EndOfStreamException()

        elif tmp == "\\":       
            backslash_f = not backslash_f   # every second backslash switches to 'non-escape char'
            continue

        if not backslash_f:
            if   match_against_cache(Opener):
                open_brackets_n += 1
            elif match_against_cache(Closer):
                open_brackets_n -= 1
                if open_brackets_n == 0: 
                    # stop accumulating text when the closing delimiter is reached. do not 
                    # append the closing delimiter to the text. 
                    txt = txt[:-len(Closer)]
                    break

        backslash_f = False

        for delimiter in IgnoreRegions:
            # If the start delimiter for ignored regions matches the strings recently in flooded into
            # the cache, then read until the end of the region that is to be ignored.
            if match_against_cache(delimiter[0]): 
                position = fh.tell()
                try:
                    txt += read_until_closing_bracket(fh, "", delimiter[1], IgnoreRegions=[]) 
                except:
                    fh.seek(position)
                           
                    error_msg("Unbalanced '%s', reached end of file before closing '%s' was found." % \
                              (delimiter[0].replace("\n", "\\n"), delimiter[1].replace("\n", "\\n")), 
                              fh)

                txt += delimiter[1]
                # the 'ignore region info' may contain information about with what the
                # closing delimiter is to be replaced
                # flush the cache
                cache = ["\0"] * CacheSz
                break
                
    return txt

def read_until_character(fh, Character):
    """Backslash may disable terminating character."""
    backslash_n = 0
    txt         = ""

    # ignore_start_triggers = map(lamda x: x[0], IgnoreRegions)
    # TODO: incorporate "comment ignoring"
    while 1 + 1 == 2:
        tmp = fh.read(1)
        if   tmp == "": raise EndOfStreamException()
        elif tmp == "\\": backslash_n += 1
        else:
            backslash_n = 0
            if backslash_n % 2 != 1:
                if tmp == Character:
                    return txt
        txt += tmp

    return txt

def read_until_letter(fh, EndMarkers, Verbose=False):
    txt = ""
    while 1 + 1 == 2:
        tmp = fh.read(1)
        if tmp == "":   
            if Verbose: return "", -1
            else:       return ""

        if tmp in EndMarkers:
            if Verbose: return txt, EndMarkers.index(tmp)
            else:       return txt
        txt += tmp

def read_until_line_contains(in_fh, LineContent):
    line = in_fh.readline()
    if line == "": return ""

    collector = ""
    while line.find(LineContent) == -1:
        collector += line
        line = in_fh.readline()
        if line == "":  break

    return collector

def get_file_content_or_die(FileName, Mode="rb"):
    fh = open_file_or_die(FileName, Mode)
    txt = fh.read()
    fh.close()
    return txt

def get_current_line_info_number(fh):
    position = fh.tell()
    fh.seek(0)
    # When reading 'position' number of characters from '0', then we are
    # at position 'position' at the end of the read. That means, we are where
    # we started.
    passed_text = fh.read(position)
    return passed_text.count("\n")

def clean_up():
    # -- delete temporary files
    for file in temporary_files:
        os.system("rm %s" % file)

def open_file_or_die(FileName, Mode="rb", Env=None, CodecCheckF=True):
    file_name = FileName.replace("//","/")
    file_name = os.path.normpath(file_name)
    try:
        fh = open(file_name, Mode)
    except:
        if Env is not None:
            error_msg("Is environment variable '%s' set propperly?" % Env, DontExitF=True)
        error_msg("Cannot open file '%s'" % FileName)

    if CodecCheckF and Mode.find("w") == -1: 
        __check_codec(fh, FileName)

    return fh

def write_safely_and_close(FileName, txt):
    fh = open_file_or_die(FileName, Mode="wb", CodecCheckF=False)
    if os.linesep != "\n": txt = txt.replace("\n", os.linesep)
    # NOTE: According to bug 2813381, maybe due to an error in python,
    #       there appeared two "\r" instead of one "\r\r".
    while txt.find("\r\r") != -1:
        txt = txt.replace("\r\r", "\r")
    fh.write(txt)
    fh.close()

def indented_open(Filename, Indentation = 3, Reference="quex"):
    """Opens a file but indents all the lines in it. In fact, a temporary
    file is created with all lines of the original file indented. The filehandle
    returned points to the temporary file."""
    
    IndentString = " " * Indentation
    
    file_name = Filename.replace("//","/")
    try:
        fh = open(file_name, "rb")
    except:
        error_msg("%s:error: failed to open file '%s' " % (Reference, Filename))
    new_content = ""
    for line in fh.readlines():
        new_content += IndentString + line
    fh.close()

    tmp_filename = Filename + ".tmp"

    if tmp_filename not in temporary_files:
        temporary_files.append(copy(tmp_filename))

    fh = open(tmp_filename, "wb")
    fh.write(new_content)
    fh.close()

    fh = open(tmp_filename)

    return fh

def read_next_word(fh):
    skip_whitespace(fh)
    word = read_until_whitespace(fh)

    if word == "": raise EndOfStreamException()
    return word

def read_word_list(fh, EndMarkers, Verbose=False):
    """Reads whitespace separated words until the arrivel
    of a string mentioned in the array 'EndMarkers'. If
    the Verbose flag is set not only the list of found words
    is returned. Moreover the index of the end marker which
    triggered is given as a second return value."""
    word_list = []
    while 1 + 1 == 2:
        skip_whitespace(fh)
        word = read_next_word(fh)        
        if word == "": raise EndOfStreamException()
        if word in EndMarkers:
            if Verbose: return word_list, EndMarkers.index(word)
            else:       return word_list
        word_list.append(word)

def error_msg(ErrMsg, fh=-1, LineN=None, DontExitF=False, Prefix="", WarningF=True):
    # fh        = filehandle [1] or filename [2]
    # LineN     = line_number of error
    # DontExitF = True then no exit from program
    #           = False then total exit from program
    # WarningF  = Asked only if DontExitF is set. 
    #
    # count line numbers (this is a kind of 'dirty' solution for not
    # counting line numbers on the fly. it does not harm at all and
    # is much more direct to be programmed.)

    if fh == -1:
        if Prefix == "": prefix = "command line"
        else:            prefix = Prefix

    elif fh == "assert":
        if type(LineN) != str: 
            error_msg("3rd argument needs to be a string,\n" + \
                      "if message type == 'assert'", "assert", 
                      "file_in.error_msg()")
        file_name = LineN    
        prefix = "internal assert:" + file_name + ":"   
    else:
        if type(fh) == str:
            line_n = LineN
            Filename = fh 
        else:
            if fh is not None:
                line_n   = get_current_line_info_number(fh)
                if hasattr(fh, "name"): Filename = fh.name
                else:                   Filename = "command line"
            else:
                line_n = -1
                Filename = ""
        if DontExitF and WarningF: 
            prefix = "%s:%i:warning" % (Filename, line_n + 1)   
        else:
            prefix = "%s:%i:error" % (Filename, line_n + 1)   
        
    for line in split(ErrMsg, "\n"):
        print prefix + ": %s" % line

    if not DontExitF: sys.exit(-1)  # Here, sys.exit(-1) is accepted

def make_safe_identifier(String, NoCodeF=True):
    txt = ""
    for letter in String:
        if len(letter) != 1:
            # It is theoretically possible that a character > 0x10000 arrives on a python
            # narrow build.
            error_msg("The underlying python build cannot handle character '%s'." % letter)

        if letter.isalpha() or letter.isdigit() or letter == "_": txt += letter.upper()
        elif letter == ":":                                       txt += "_"
        elif NoCodeF:                                             txt += "_" 
        else:                                                     txt += "_x%x_" % ord(letter)
    return txt

def get_include_guard_extension(Filename):
    """Transforms the letters of a filename, so that they can appear in a C-macro."""
    return make_safe_identifier(Filename, NoCodeF=False)

def read_option_start(fh):
    skip_whitespace(fh)

    # (*) base modes 
    if fh.read(1) != "<": 
        ##fh.seek(-1, 1) 
        return None

    skip_whitespace(fh)
    identifier = read_identifier(fh).strip()

    if identifier == "":  error_msg("missing identifer after start of mode option '<'", fh)
    skip_whitespace(fh)
    if fh.read(1) != ":": error_msg("missing ':' after option name '%s'" % identifier, fh)
    skip_whitespace(fh)

    return identifier

def read_option_value(fh):

    position = fh.tell()

    value = ""
    depth = 1
    while 1 + 1 == 2:
        try: 
            letter = fh.read(1)
        except EndOfStreamException:
            fh.seek(position)
            error_msg("missing closing '>' for mode option.", fh)

        if letter == "<": 
            depth += 1
        if letter == ">": 
            depth -= 1
            if depth == 0: break
        value += letter

    return value.strip()

def check(fh, Word):
    position = fh.tell()
    try:
        skip_whitespace(fh)
        dummy = fh.read(len(Word))
        if dummy == Word: return True
    except:
        pass
    fh.seek(position)
    return False

def check_letter_from_list(fh, LetterList):
    position = fh.tell()
    if fh.read(1) in LetterList: return True
    fh.seek(position)
    return False

def verify_word_in_list(Word, WordList, Comment, FH=-1, LineN=None, ExitF=True):
    """FH, and LineN work exactly the same as for error_msg(...)"""
    if len(WordList) == 0:
        error_msg(Comment + "\n'%s' is not addmissible here." % Word, FH, LineN, DontExitF=False)
        return

    position_known_f = False
    if type(WordList) == dict:
        word_list = WordList.keys()
    elif WordList[0].__class__.__name__ == "UserCodeFragment":
        word_list        = map(lambda x: x.get_code(), WordList)
        position_known_f = True
    else:
        word_list        = WordList

    if Word in word_list: return True

    # Word was not in list
    similar_index = similarity.get(Word, word_list)

    if similar_index == -1:
        txt = "Acceptable: "
        length = len(txt)
        for word in WordList:
            L = len(word)
            if length + L > 80: 
                txt += "\n"; length = 0
            txt += word + ", "
            length += L

        if txt != "": txt = txt[:-2] + "."
        error_msg(Comment + "\n" + txt, FH, LineN, DontExitF=False)

    else:
        similar_word = word_list[similar_index]
        if position_known_f:
            error_msg(Comment, FH, LineN, DontExitF=True)
            error_msg("Did you mean '%s'?" % similar_word,
                      WordList[similar_index].filename, 
                      WordList[similar_index].line_n, 
                      DontExitF=not ExitF)
        else:
            error_msg(Comment + "\n" + "Did you mean '%s'?" % similar_word,
                      FH, LineN, DontExitF=not ExitF)

    return False

def error_msg_file_not_found(FileName, Comment="", FH=-1, LineN=None):
    """FH and LineN follow format of 'error_msg(...)'"""
    directory = os.path.dirname(FileName)
    if directory == "": directory = os.path.normpath("./"); suffix = "."
    else:               suffix = "in directory\n'%s'." % directory
    comment = ""
    if Comment != "": comment = "(%s) " % Comment
    try:
        files_in_directory = os.listdir(directory)
    except:
        error_msg("File '%s' (%s) not found." % (FileName, comment), FH, LineN)
    verify_word_in_list(FileName, files_in_directory, 
                        "File '%s' %snot found%s" % (FileName, comment, suffix), FH, LineN)
    
def get_propperly_slash_based_file_name(PathName):
    """Replaces backslashes '\\' by '/' and replaces adjacent
       slashes by single slashes.
    """
    path = PathName.replace("\\", "/")
    while path.find("//") != -1:
        path = path.replace("//", "/")
    return path

# 'Bad' byte order marks (we only treat UTF8)
Bad_BOM_list = [ codecs.BOM_BE, codecs.BOM_LE, codecs.BOM_UTF16, codecs.BOM_UTF16_BE, codecs.BOM_UTF16_LE, codecs.BOM_UTF32, codecs.BOM_UTF32_BE, codecs.BOM_UTF32_LE]
def __check_codec(fh, FileName):
    global Bad_BOM_list

    begin = fh.read(4)
    for bom in Bad_BOM_list:
        if begin.startswith(bom):
            error_msg("Quex requires ASCII or UTF8 input character format.\n" \
                      "File '%s' contains Non-UTF8 Byte Order Mark." % FileName)

    start_pos = 0
    if begin.startswith(codecs.BOM_UTF8):
        start_pos = len(codecs.BOM_UTF8) # Step over the initial BOM

    content = fh.read()
    if content.count('\0') > 1:
        error_msg("Quex requires ASCII or UTF8 input character format.\n" \
                  "File '%s' contains many '0' characters. Is it UTF16/UTF32 encoded?." % FileName)

    try: 
        content.decode("utf8")
    except:
        error_msg("Quex requires ASCII or UTF8 input character format.\n" \
                  "File '%s' is not ASCII or UTF8 encoded. Or, it contains encoding errors." % FileName)
    fh.seek(start_pos)
    return fh
