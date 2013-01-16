from quex.engine.misc.file_in   import write_safely_and_close
from quex.engine.generator.base import GeneratorBase
from quex.blackboard            import setup as Setup

class Generator(GeneratorBase):
    def __init__(self, PatternActionPairList, StateMachineName):
        assert map(lambda elm: elm.__class__.__name__, PatternActionPairList) \
               == [ "PatternActionInfo" ] * len(PatternActionPairList)

        GeneratorBase.__init__(self, PatternActionPairList, StateMachineName)

    def do(self, Option="utf8"):
        """Prepare output in the 'dot' language, that graphviz uses."""
        assert Option in ["utf8", "hex"]

        self.__do(self.sm, self.state_machine_name + ".dot", Option)

        if self.pre_context_sm is not None:
            file_name = "%s-pre-condition.dot" % self.state_machine_name
            self.pre_context_file_name = file_name
            self.__do(self.pre_context_sm, file_name, Option)

        if len(self.papc_backward_detector_state_machine_list) != 0:
            self.backward_detector_file_name = []
            for sm in self.papc_backward_detector_state_machine_list:
                file_name = "%s_%i.dot" % (self.state_machine_name, sm.get_id())
                self.backward_detector_file_name.append(file_name)
                self.__do(sm, file_name, Option)

    def __do(self, state_machine, FileName, Option="utf8"):
        dot_code = state_machine.get_graphviz_string(NormalizeF=Setup.normalize_f, Option=Option)
        write_safely_and_close(FileName, dot_code)
    

