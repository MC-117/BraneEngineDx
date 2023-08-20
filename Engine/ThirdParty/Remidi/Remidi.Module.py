from Core.BuildObject import Module, TargetConfig, PlaformType
from Core.Foundation import Log, BuildParam

class RemidiModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
    
    def setup(self, config : TargetConfig):
        if config.plaform == PlaformType.Win64:
            self.libPaths = [f'{self.getPath()}/lib/x64']
            self.libFiles = ['libremidi.lib', 'winmm.lib']
        else:
            raise NotImplementedError('Only support Win64')
