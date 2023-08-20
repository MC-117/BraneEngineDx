import os
import glob
from Core.BuildObject import Module, TargetConfig, PlaformType
from Core.Foundation import BuildParam

class Python37Module(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
    
    def setup(self, config : TargetConfig):
        if config.plaform == PlaformType.Win64:
            libPath = f'{self.getPath()}/libs'
            self.libPaths = [libPath]
            self.libFiles = [os.path.basename(file) for file in glob.glob(libPath + '/*.lib')]
            self.dllFiles = [file for file in glob.glob(self.getPath() + '/*.dll')]
        else:
            raise NotImplementedError('Only support Win64')
