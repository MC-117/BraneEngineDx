from Core.BuildObject import Module, TargetConfig, PlaformType
from Core.Foundation import Log, BuildParam

class SpineModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
    
    def setup(self, config : TargetConfig):
        if config.plaform == PlaformType.Win64:
            self.libPaths = [f'{self.getPath()}/lib/x64/{"Debug" if config.enableDebug else "Release"}']
            self.libFiles = ['spine-cpp.lib']
        else:
            raise NotImplementedError('Only support Win64')
