from Core.BuildObject import Module, TargetConfig, PlaformType, TargetType
from Core.Foundation import BuildParam, FileSearcher

class LaunchModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}']
        self.fileSearcher.addPath('.', True)

        self.moduleDependencies = [
            'Engine',
        ]
