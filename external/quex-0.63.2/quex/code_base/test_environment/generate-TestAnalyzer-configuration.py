import os
import sys

sys.path.insert(0, os.environ["QUEX_PATH"])

import quex.output.cpp.configuration   as configuration
from   quex.blackboard                 import setup as Setup
import quex.input.command_line.core    as command_line
from   quex.engine.misc.file_in        import open_file_or_die
import quex.input.files.core           as quex_file_parser


command_line.do(["-i", "nothing", "-o", "TestAnalyzer", "--token-policy", "single", "--no-include-stack"])

# Parse default token file
fh = open_file_or_die(os.environ["QUEX_PATH"] 
                      + Setup.language_db["$code_base"] 
                      + Setup.language_db["$token-default-file"])
quex_file_parser.parse_section(fh)
fh.close()

BeginOfLineSupportF = True
IndentationSupportF = False     

txt = configuration.do({})

open("TestAnalyzer-configuration", "w").write(txt)
