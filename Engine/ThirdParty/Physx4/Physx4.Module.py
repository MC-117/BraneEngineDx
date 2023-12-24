import os
import glob
from Core.BuildObject import Module, TargetConfig, PlaformType
from Core.Foundation import BuildParam, Log

class Physx4Module(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}/include']

        self.preprocessorDefinitions = [
            "PX_PHYSX_STATIC_LIB"
        ]
    
    def setup(self, config : TargetConfig):
        if config.plaform == PlaformType.Win64:
            libPath = f'{self.getPath()}/lib/x64/{"Debug" if config.enableDebug else "Release"}'
            dllPath = f'{self.getPath()}/dll/x64/{"Debug" if config.enableDebug else "Release"}'
            self.libPaths = [libPath]
            self.libFiles = [os.path.basename(file) for file in glob.glob(libPath + '/*.lib')]
            self.dllFiles = [file for file in glob.glob(dllPath + '/*.dll')]
        else:
            raise NotImplementedError('Only support Win64')
