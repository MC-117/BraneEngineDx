from Core.BuildObject import Module, TargetConfig, PlaformType, TargetType
from Core.Foundation import BuildParam

class ImGuiModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}']
        self.buildTarget = 'ImGui'

    def setup(self, config : TargetConfig):
        if config.plaform != PlaformType.Win64:
            raise NotImplementedError('Only support Win64')
        if config.target.type == TargetType.ModuleLib or config.target.type == TargetType.ModuleDll:
            self.fileSearcher.addPath('.', True)
        else:
            self.libPaths = [self.buildTargetAbsOutputPath]
            self.libFiles.append(self.buildTargetOutputName + '.lib')
