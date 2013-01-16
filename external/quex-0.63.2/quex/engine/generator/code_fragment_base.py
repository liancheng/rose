
class CodeFragment:
    def __init__(self, Code="", RequireTerminatingZeroF=False):
        self.__code = Code
        self.__related_generator = None

    def set_code(self, Code):
        self.__code = Code

    def get_code(self):
        return self.__code

    def get_pure_code(self):
        return self.__code

    def set_related_generator(self, GeneratorRef):
        self.__related_generator = GeneratorRef

    def get_related_generator(self):
        return self.__related_generator

