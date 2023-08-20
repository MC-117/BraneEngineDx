from Core.BuildObject import Module, TargetConfig, PlaformType
from Core.Foundation import BuildParam

class OpenAlSoftModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
    
    def setup(self, config : TargetConfig):
        if config.plaform == PlaformType.Win64:
            self.libPaths = [f'{self.getPath()}/lib']
            self.libFiles = ['OpenAL32.lib']
            self.dllFiles = [f'{self.getPath()}/dll/OpenAL32.dll']
        else:
            raise NotImplementedError('Only support Win64')
