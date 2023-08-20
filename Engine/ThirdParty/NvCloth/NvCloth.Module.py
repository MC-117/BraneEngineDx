from Core.BuildObject import Module, TargetConfig, PlaformType
from Core.Foundation import BuildParam

class NvClothModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
        self.sourcePaths = [f'{self.getPath()}/src']
    
    def setup(self, config : TargetConfig):
        if config.plaform == PlaformType.Win64:
            self.libPaths = [f'{self.getPath()}/lib']
            self.libFiles = ['NvCloth_x64.lib']
            self.dllFiles = [f'{self.getPath()}/dll/NvCloth_x64.dll']
        else:
            raise NotImplementedError('Only support Win64')
