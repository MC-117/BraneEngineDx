import glob
import os
from Core.BuildObject import Module, TargetConfig
from Core.Foundation import Log, BuildParam

class AssimpModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']
    
    def setup(self, config : TargetConfig):
        libPath = f'{self.getPath()}/lib/{config.name}'

        self.libPaths = [libPath]
        self.libFiles = [os.path.basename(file) for file in glob.glob(libPath + '/*.lib')]
