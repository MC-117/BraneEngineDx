from Core.BuildObject import Module, TargetConfig, PlaformType
from Core.Foundation import Log, BuildParam

class RenderDocModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
    
    def setup(self, config : TargetConfig):
        if config.plaform == PlaformType.Win64:
            self.dllFiles = [f'{self.getPath()}/dll/renderdoc.dll']
        else:
            raise NotImplementedError('Only support Win64')
