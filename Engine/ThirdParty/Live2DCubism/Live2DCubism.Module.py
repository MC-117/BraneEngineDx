from Core.BuildObject import Module, TargetConfig, Target, PlaformType, TargetType
from Core.Foundation import BuildParam

class Live2DCubismModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
        self.sourceTarget = 'Live2DCubism'
    
    def setup(self, config : TargetConfig):
        if config.plaform != PlaformType.Win64:
            raise NotImplementedError('Only support Win64')
        if config.target.type == TargetType.StaticLib:
            self.fileSearcher.addPath('include', True)
        else:
            self.libPaths = [f'{self.getPath()}/lib/windows/x86_64/142']
            if config.enableDebug:
                self.libFiles = ['Live2DCubismCore_MTd.lib', 'Live2DCubism_MTd.lib']
            else:
                self.libFiles = ['Live2DCubismCore_MT.lib', 'Live2DCubism_MT.lib']
