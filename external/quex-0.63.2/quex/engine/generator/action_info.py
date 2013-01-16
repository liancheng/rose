from   quex.engine.misc.file_in import \
                                       open_file_or_die, \
                                       write_safely_and_close, \
                                       get_current_line_info_number, \
                                       error_msg
from   quex.engine.generator.code_fragment_base import CodeFragment
from   quex.blackboard import setup as Setup

UserCodeFragment_OpenLinePragma = {
#___________________________________________________________________________________
# Line pragmas allow to direct the 'virtual position' of a program in a file
# that was generated to its origin. That is, if an error occurs in line in a 
# C-program which is actually is the pasted code from a certain line in a .qx 
# file, then the compiler would print out the line number from the .qx file.
# 
# This mechanism relies on line pragmas of the form '#line xx "filename"' telling
# the compiler to count the lines from xx and consider them as being from filename.
#
# During code generation, the pasted code segments need to be followed by line
# pragmas resetting the original C-file as the origin, so that errors that occur
# in the 'real' code are not considered as coming from the .qx files.
# Therefore, the code generator introduces placeholders that are to be replaced
# once the whole code is generated.
#
#  ...[Language][0]   = the placeholder until the whole code is generated
#  ...[Language][1]   = template containing 'NUMBER' and 'FILENAME' that
#                       are to replaced in order to get the resetting line pragma.
#___________________________________________________________________________________
        "C": 
        [
            [ '/* POST-ADAPTION: FILL IN APPROPRIATE LINE PRAGMA */',
              '#   line NUMBER "FILENAME"' ],
            ['/* POST-ADAPTION: FILL IN APPROPRIATE LINE PRAGMA CppTemplate.txt */',
             '#   line NUMBER "CppTemplate.txt"' ]
        ],
   }

class UserCodeFragment(CodeFragment):
    def __init__(self, Code, Filename, LineN, LanguageDB=None):
        assert isinstance(Code, (str, unicode))
        assert isinstance(LanguageDB, dict) or LanguageDB is None
        assert isinstance(Filename, (str, unicode))
        assert isinstance(LineN, (int, long, float))

        self.filename = Filename
        self.line_n   = LineN

        CodeFragment.__init__(self, Code)

    def get_code(self):
        return self.adorn_with_source_reference(self.get_pure_code())

    def adorn_with_source_reference(self, Code, ReturnToSourceF=True):
        if len(Code.strip()) == 0: return ""

        # Even under Windows (tm), the '/' is accepted. Thus do not rely on 'normpath'
        norm_filename = Setup.get_file_reference(self.filename) 
        txt  = '\n#   line %i "%s"\n' % (self.line_n, norm_filename)
        txt += Code
        if ReturnToSourceF:
            if txt[-1] != "\n": txt = txt + "\n"
            txt += get_return_to_source_reference()
        return txt

def get_return_to_source_reference():
    return "\n" + UserCodeFragment_OpenLinePragma["C"][0][0] + "\n"

def UserCodeFragment_straighten_open_line_pragmas(filename, Language):
    if Language not in UserCodeFragment_OpenLinePragma.keys():
        return

    fh = open_file_or_die(filename)
    norm_filename = Setup.get_file_reference(filename)

    new_content = []
    line_n      = 0
    LinePragmaInfoList = UserCodeFragment_OpenLinePragma[Language]
    for line in fh.readlines():
        line_n += 1
        if Language == "C":
            for info in LinePragmaInfoList:
                if line.find(info[0]) == -1: continue
                line = info[1]
                # Since by some definition, line number pragmas < 32768; let us avoid
                # compiler warnings by setting line_n = min(line_n, 32768)
                line = line.replace("NUMBER", repr(int(min(line_n + 1, 32767))))
                # Even under Windows (tm), the '/' is accepted. Thus do not rely on 'normpath'
                line = line.replace("FILENAME", norm_filename)
                if len(line) == 0 or line[-1] != "\n":
                    line = line + "\n"
        new_content.append(line)

    fh.close()

    write_safely_and_close(filename, "".join(new_content))

class GeneratedCode(UserCodeFragment):
    def __init__(self, GeneratorFunction, FileName=-1, LineN=None):
        self.function = GeneratorFunction
        self.data     = { "indentation_counter_terminal_id": None, }
        UserCodeFragment.__init__(self, "", FileName, LineN)

    def get_code(self):
        return self.function(self.data)

class PatternActionInfo:
    def __init__(self, ThePattern, Action, PatternStr="", IL = None, ModeName="", Comment=""):

        assert Action is None or \
               issubclass(Action.__class__, CodeFragment) or \
               type(Action) in [str, unicode]
        assert (ThePattern.__class__.__name__ == "Pattern") or (ThePattern is None)


        self.__pattern_state_machine = ThePattern
        if type(Action) in [str, unicode]: self.__action = CodeFragment(Action)
        else:                              self.__action = Action

        self.__pattern_str = PatternStr
        self.mode_name     = ModeName
        self.comment       = Comment

    def pattern(self):
        return self.__pattern_state_machine

    def pattern_string(self):
        return self.__pattern_str

    @property
    def line_n(self): return self.action().line_n
    @property
    def file_name(self): return self.action().filename

    def action(self):
        return self.__action

    def set_action(self, Action):
        assert Action is None or \
               issubclass(Action.__class__, CodeFragment) or \
               type(Action) in [str, unicode]
        self.__action = Action

    def get_action_location(self):
        """RETURNS:  FileName, LineN   in case that it can be specified.
                     -1, None          in case it cannot be specified.

           This corresponds to the required input for 'error_msg'.
        """
        if hasattr(self.__action, "filename") and hasattr(self.__action, "line_n"):
            return self.__action.filename, self.__action.line_n
        else:
            return -1, None

    def pattern_index(self):
        return self.pattern_state_machine().get_id()

    def __repr__(self):         
        txt  = ""
        txt += "self.mode_name      = " + repr(self.mode_name) + "\n"
        txt += "self.pattern_string = " + repr(self.pattern_string()) + "\n"
        txt += "self.pattern        = \n" + repr(self.pattern()).replace("\n", "\n      ")
        txt += "self.action         = " + repr(self.action().get_code()) + "\n"
        if self.action().__class__ == UserCodeFragment:
            txt += "self.filename   = " + repr(self.action().filename) + "\n"
            txt += "self.line_n     = " + repr(self.action().line_n) + "\n"
        txt += "self.pattern_index  = " + repr(self.pattern().sm.get_id()) + "\n"
        return txt

class LocalizedParameter:
    def __init__(self, Name, Default, FH=-1):
        self.name      = Name
        self.__default = Default
        if FH == -1:
            self.__value   = None
            self.file_name = ""
            self.line_n    = -1
        else:
            self.__value   = Default
            self.file_name = FH.name
            self.line_n    = get_current_line_info_number(FH)
        self.__pattern_string = None

    def set(self, Value, fh):
        if self.__value is not None:
            error_msg("%s has been defined more than once.\n" % self.name, fh, DontExitF=True)
            error_msg("previous definition has been here.\n", self.file_name, self.line_n)
                      
        self.__value   = Value
        self.file_name = fh.name
        self.line_n    = get_current_line_info_number(fh)

    def get(self):
        if self.__value is not None: return self.__value
        return self.__default

    def set_pattern_string(self, Value):
        self.__pattern_string = Value

    def pattern_string(self):
        return self.__pattern_string

    def get_action_location(self):
        """RETURNS:  FileName, LineN   in case that it can be specified.
                     -1, None          in case it cannot be specified.

           This corresponds to the required input for 'error_msg'.
        """
        return self.file_name, self.line_n

    @property
    def comment(self):
        return self.name

