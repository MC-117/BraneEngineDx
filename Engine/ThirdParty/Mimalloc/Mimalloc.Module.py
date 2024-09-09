from Core.BuildObject import Module, TargetConfig, PlaformType, TargetType
from Core.Foundation import BuildParam

class MimallocModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
        self.buildTarget = 'Mimalloc'

    def setup(self, config : TargetConfig):
        if config.plaform != PlaformType.Win64:
            raise NotImplementedError('Only support Win64')
        if config.target.type == TargetType.ModuleLib:
            self.fileSearcher.addPath('include', True)
            config.sourceFiles.append('src/static.c')
        else:
            self.libPaths = [self.buildTargetAbsOutputPath]
            self.libFiles.append(self.buildTargetOutputName + '.lib')
