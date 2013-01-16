#! /usr/bin/env python
import time
import os
import re

from   quex.engine.misc.file_in  import open_file_or_die, \
                                        delete_comment, \
                                        extract_identifiers_with_specific_prefix, \
                                        get_include_guard_extension, \
                                        error_msg

import quex.blackboard                  as blackboard
from   quex.blackboard                  import token_id_db
from   quex.engine.misc.string_handling import blue_print
from   quex.blackboard                  import setup as Setup

def do(setup):
    """Creates a file of token-ids from a given set of names.
       Creates also a function:

       const string& $$token$$::map_id_to_name().
    """
    global file_str
    LanguageDB = Setup.language_db

    __propose_implicit_token_definitions()

    for standard_token_id in standard_token_id_list:
        assert token_id_db.has_key(standard_token_id)

    assert blackboard.token_type_definition is not None, \
           "Token type has not been defined yet, see $QUEX_PATH/quex/core.py how to\n" + \
           "handle this."

    # (*) Token ID File ________________________________________________________________
    #
    #     The token id file can either be specified as database of
    #     token-id names, or as a file that directly assigns the token-ids
    #     to variables. If the flag '--user-token-id-file' is defined, then
    #     then the token-id file is provided by the user. Otherwise, the
    #     token id file is created by the token-id maker.
    #
    #     The token id maker considers the file passed by the option '-t'
    #     as the database file and creates a C++ file with the output filestem
    #     plus the suffix "--token-ids". Note, that the token id file is a
    #     header file.
    #
    if len(token_id_db.keys()) == len(standard_token_id_list):
        token_id_str = "%sTERMINATION and %sUNINITIALIZED" % \
                       (setup.token_id_prefix_plain, setup.token_id_prefix_plain) 
        # TERMINATION + UNINITIALIZED = 2 token ids. If they are the only ones nothing can be done.
        error_msg("Only token ids %s are defined.\n" % token_id_str + \
                  "Quex refuses to proceed. Please, use the 'token { ... }' section to\n" + \
                  "specify at least one other token id.")

    #______________________________________________________________________________________
    L = max(map(lambda name: len(name), token_id_db.keys()))
    def space(Name):
        return " " * (L - len(Name))

    # -- define values for the token ids
    def define_this(txt, token):
        if setup.language == "C":
            txt.append("#define %s%s %s((QUEX_TYPE_TOKEN_ID)%i)\n" \
                       % (setup.token_id_prefix_plain, token.name, space(token.name), token.number))
        else:
            txt.append("const QUEX_TYPE_TOKEN_ID %s%s%s = ((QUEX_TYPE_TOKEN_ID)%i);\n" \
                       % (setup.token_id_prefix_plain, token.name, space(token.name), token.number))

    if setup.token_id_foreign_definition_file != "":
        token_id_txt = ["#include \"%s\"\n" % Setup.get_file_reference(setup.token_id_foreign_definition_file)]

    else:
        if setup.language == "C": 
            prolog = ""
            epilog = ""
        else:
            prolog = LanguageDB.NAMESPACE_OPEN(setup.token_id_prefix_name_space)
            epilog = LanguageDB.NAMESPACE_CLOSE(setup.token_id_prefix_name_space)

        token_id_txt = [prolog]

        # Assign values to tokens with no numeric identifier
        # NOTE: This has not to happen if token's are defined by the user's provided file.
        i = setup.token_id_counter_offset
        # Take the 'dummy_name' only to have the list sorted by name. The key 'dummy_name' 
        # may contain '--' to indicate a unicode value, so do not use it as name.
        for dummy_name, token in sorted(token_id_db.items()):
            if token.number is None: 
                while __is_token_id_occupied(i):
                    i += 1
                token.number = i; 

            define_this(token_id_txt, token)

        # Double check that no token id appears twice
        # Again, this can only happen, if quex itself produced the numeric values for the token
        token_list = token_id_db.values()
        for i, x in enumerate(token_list):
            for y in token_list[i+1:]:
                if x.number != y.number: continue
                error_msg("Token id '%s'" % x.name, x.file_name, x.line_n, DontExitF=True)
                error_msg("and token id '%s' have same numeric value '%s'." \
                          % (y.name, x.number), y.file_name, y.line_n, DontExitF=True)
                          
        token_id_txt.append(epilog)

    content = blue_print(file_str,
                         [["$$TOKEN_ID_DEFINITIONS$$",        "".join(token_id_txt)],
                          ["$$DATE$$",                        time.asctime()],
                          ["$$TOKEN_CLASS_DEFINITION_FILE$$", Setup.get_file_reference(blackboard.token_type_definition.get_file_name())],
                          ["$$TOKEN_PREFIX$$",                setup.token_id_prefix], 
                          ["$$INCLUDE_GUARD_EXT$$",           get_include_guard_extension(         \
                                                                  Setup.analyzer_name_safe.upper() \
                                                                + "__"                             \
                                                                + Setup.token_class_name_safe.upper())], 
                         ])

    return content

standard_token_id_list = ["TERMINATION", "UNINITIALIZED", "INDENT", "NODENT", "DEDENT"]

def prepare_default_standard_token_ids():
    global standard_token_id_list

    token_id_db["TERMINATION"]   = TokenInfo("TERMINATION",   ID=0)
    token_id_db["UNINITIALIZED"] = TokenInfo("UNINITIALIZED", ID=1)
    # Indentation Tokens
    token_id_db["INDENT"]        = TokenInfo("INDENT",        ID=2)
    token_id_db["DEDENT"]        = TokenInfo("DEDENT",        ID=3)
    token_id_db["NODENT"]        = TokenInfo("NODENT",        ID=4)

    # Assert that every standard token id is in the database
    for name in standard_token_id_list:
        assert token_id_db.has_key(name)

class TokenInfo:
    def __init__(self, Name, ID, TypeName=None, Filename="", LineN=-1):
        self.name         = Name
        self.number       = ID
        self.related_type = TypeName
        self.file_name    = Filename
        self.line_n       = LineN
        self.id           = None

file_str = \
"""/* -*- C++ -*- vim: set syntax=cpp:
 * PURPOSE: File containing definition of token-identifier and
 *          a function that maps token identifiers to a string
 *          name.
 *
 * NOTE: This file has been created automatically by Quex.
 *       Visit quex.org for further info.
 *
 * DATE: $$DATE$$
 *
 * (C) 2005-2010 Frank-Rene Schaefer
 * ABSOLUTELY NO WARRANTY                                           */
#ifndef __QUEX_INCLUDE_GUARD__AUTO_TOKEN_IDS_$$INCLUDE_GUARD_EXT$$__
#define __QUEX_INCLUDE_GUARD__AUTO_TOKEN_IDS_$$INCLUDE_GUARD_EXT$$__

#ifndef __QUEX_OPTION_PLAIN_C
#   include<cstdio> 
#else
#   include<stdio.h> 
#endif

/* The token class definition file can only be included after 
 * the definition on TERMINATION and UNINITIALIZED.          
 * (fschaef 12y03m24d: "I do not rememember why I wrote this.
 *  Just leave it there until I am clear if it can be deleted.")   */
#include "$$TOKEN_CLASS_DEFINITION_FILE$$"

$$TOKEN_ID_DEFINITIONS$$

QUEX_NAMESPACE_TOKEN_OPEN
extern const char* QUEX_NAME_TOKEN(map_id_to_name)(const QUEX_TYPE_TOKEN_ID TokenID);
QUEX_NAMESPACE_TOKEN_CLOSE

#endif /* __QUEX_INCLUDE_GUARD__AUTO_TOKEN_IDS_$$INCLUDE_GUARD_EXT$$__ */
"""

func_str = \
"""
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
$$TOKEN_NAMES$$       

   /* NOTE: This implementation works only for token id types that are 
    *       some type of integer or enum. In case an alien type is to
    *       used, this function needs to be redefined.                  */
   switch( TokenID ) {
   default: {
       __QUEX_STD_sprintf(error_string, "<UNKNOWN TOKEN-ID: %i>", (int)TokenID);
       return error_string;
   }
   case $$TOKEN_PREFIX$$TERMINATION:    return termination_string;
   case $$TOKEN_PREFIX$$UNINITIALIZED:  return uninitialized_string;
#  if defined(QUEX_OPTION_INDENTATION_TRIGGER)
   case $$TOKEN_PREFIX$$INDENT:         return indent_string;
   case $$TOKEN_PREFIX$$DEDENT:         return dedent_string;
   case $$TOKEN_PREFIX$$NODENT:         return nodent_string;
#  endif
$$TOKEN_ID_CASES$$
   }
}

QUEX_NAMESPACE_TOKEN_CLOSE
"""

def __is_token_id_occupied(TokenID):
    return TokenID in map(lambda x: x.number, token_id_db.values())

def __propose_implicit_token_definitions():
    if len(blackboard.token_id_implicit_list) == 0: return

    file_name = blackboard.token_id_implicit_list[0][1]
    line_n    = blackboard.token_id_implicit_list[0][2]
    error_msg("Detected implicit token identifier definitions. Proposal:\n"
              "   token {" , file_name, line_n, 
              DontExitF=True, WarningF=True)

    for token_name, file_name, line_n in blackboard.token_id_implicit_list:
        error_msg("     %s;" % token_name, file_name, line_n, DontExitF=True, WarningF=True)
    error_msg("   }", file_name, line_n, DontExitF=True, WarningF=True)

def do_map_id_to_name_function():
    L = max(map(lambda name: len(name), token_id_db.keys()))
    def space(Name):
        return " " * (L - len(Name))

    # -- define the function for token names
    switch_cases = []
    token_names  = []
    for token_name in sorted(token_id_db.keys()):
        if token_name in standard_token_id_list: continue

        # UCS codepoints are coded directly as pure numbers
        if len(token_name) > 2 and token_name[:2] == "--":
            token = token_id_db[token_name]
            switch_cases.append("   case 0x%06X: return token_id_str_%s;\n" % \
                                (token.number, token.name))
            token_names.append("   static const char  token_id_str_%s[]%s = \"%s\";\n" % \
                               (token.name, space(token.name), token.name))
        else:
            switch_cases.append("   case %s%s:%s return token_id_str_%s;\n" % \
                                (Setup.token_id_prefix, token_name, space(token_name), token_name))
            token_names.append("   static const char  token_id_str_%s[]%s = \"%s\";\n" % \
                               (token_name, space(token_name), token_name))

    return blue_print(func_str,
                      [["$$TOKEN_ID_CASES$$", "".join(switch_cases)],
                       ["$$TOKEN_PREFIX$$",   Setup.token_id_prefix], 
                       ["$$TOKEN_NAMES$$",    "".join(token_names)], ])

def parse_token_id_file(ForeignTokenIdFile, TokenPrefix, CommentDelimiterList, IncludeRE):
    """This function somehow interprets the user defined token id file--if there is
       one. It does this in order to find the names of defined token ids. It does
       some basic interpretation and include file following, but: **it is in no
       way perfect**. Since its only purpose is to avoid warnings about token ids
       that are not defined it is not essential that it may fail sometimes.

       It is more like a nice feature that quex tries to find definitions on its own.
       
       Nevertheless, it should work in the large majority of cases.
    """
    include_re_obj = re.compile(IncludeRE)

    # validate(...) ensured, that the file exists.
    work_list    = [ ForeignTokenIdFile ] 
    done_list    = []
    while len(work_list) != 0:
        fh = open_file_or_die(work_list.pop(), Mode="rb")
        content = fh.read()

        # delete any comment inside the file
        for opener, closer in CommentDelimiterList:
            content = delete_comment(content, opener, closer, LeaveNewlineDelimiter=True)

        # add any found token id to the list
        token_id_finding_list = extract_identifiers_with_specific_prefix(content, TokenPrefix)
        for token_name, line_n in token_id_finding_list:
            prefix_less_token_name = token_name[len(TokenPrefix):]
            # NOTE: The line number might be wrong, because of the comment deletion
            # NOTE: The actual token value is not important, since the token's numeric
            #       identifier is defined in the user's header. We do not care.
            token_id_db[prefix_less_token_name] = \
                    TokenInfo(prefix_less_token_name, None, None, fh.name, line_n) 
        
        # find "#include" statements
        include_file_list = include_re_obj.findall(content)
        include_file_list = filter(lambda file: file not in done_list,    include_file_list)
        include_file_list = filter(lambda file: os.access(file, os.F_OK), include_file_list)
        work_list.extend(include_file_list)

        fh.close()

