from Core.BuildObject import Module
from Core.Foundation import BuildParam, FileSearcher

class ShaderCompilerModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}']        
        self.fileSearcher.addPath('.', True)

        self.moduleDependencies = [
            'Launch',
        ]
