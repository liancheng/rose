#! /usr/bin/env python
#
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
# (C) 2005-2012 Frank-Rene Schaefer
#
################################################################################
import sys
import os
import quex.exception_checker as exception_checker

if sys.version_info[0] >= 3: 
    print("error: This version of quex was not implemented for Python >= 3.0")
    print("error: Please, use Python versions 2.x.")
    sys.exit(-1)

if os.environ.has_key("QUEX_PATH") == False:
    print("Environment variable QUEX_PATH has not been defined.")
else:
    sys.path.insert(0, os.environ["QUEX_PATH"])

try:
    exception_checker.do_on_import(sys.argv)
    import quex.DEFINITIONS
    import quex.input.command_line.core  as command_line
    import quex.input.command_line.query as query
    import quex.core                     as core

except BaseException as instance:
    exception_checker.handle(instance)
    
try:
    pass
    # import psyco
    # psyco.full()
except:
    pass

if __name__ == "__main__":
    try:
        quex.DEFINITIONS.check()

        # (*) Test Exceptions __________________________________________________
        if   exception_checker.do(sys.argv):
            # Done: Tests about exceptions have been performed
            pass

        # (*) Query ____________________________________________________________
        elif query.do(sys.argv):    
            # Done: Queries about unicode sets and regular expressions
            pass

        # (*) The Real Job _____________________________________________________
        elif command_line.do(sys.argv):
            # To do: Interpret input files and generate code or drawings.
            core.do() 

    except BaseException as instance:
        exception_checker.handle(instance)


