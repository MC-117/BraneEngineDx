from Core.BuildObject import Module, TargetConfig, PlaformType
from Core.Foundation import BuildParam

class OneTBBModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
    
    def setup(self, config : TargetConfig):
        if config.plaform == PlaformType.Win64:
            self.libPaths = [f'{self.getPath()}/lib/x64']
            if config.enableDebug:
                self.libFiles = ['tbb_debug.lib', 'tbb12_debug.lib']
                self.dllFiles = [f'{self.getPath()}/bin/x64/tbb12_debug.dll']
            else:
                self.libFiles = ['tbb.lib', 'tbb12.lib']
                self.dllFiles = [f'{self.getPath()}/bin/x64/tbb12.dll']
        else:
            raise NotImplementedError('Only support Win64')
