from Core.BuildObject import Module, TargetConfig, Target, PlaformType, TargetType
from Core.Foundation import BuildParam

class Live2DCubismModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
        self.buildTarget = 'Live2DCubism'
    
    def setup(self, config : TargetConfig):
        if config.plaform != PlaformType.Win64:
            raise NotImplementedError('Only support Win64')
        if config.target.type == TargetType.ModuleLib:
            self.fileSearcher.addPath('include', True)
        else:
            self.libPaths = [self.buildTargetAbsOutputPath]
            self.libFiles = [self.buildTargetOutputName + '.lib']
            if config.enableDebug:
                self.libFiles.append('Live2DCubismCore_MTd.lib')
            else:
                self.libFiles.append('Live2DCubismCore_MT.lib')
