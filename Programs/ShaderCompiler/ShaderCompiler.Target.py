from Core.BuildObject import Target, TargetConfig, PlaformType, SubsystemType
from Core.Foundation import Log, BuildParam

class ShaderCompilerTarget(Target):
    def __init__(self, param : BuildParam):
        Target.__init__(self, param)
        self.subsystem = SubsystemType.Console

        releaseConfig = TargetConfig(self, 'Release', PlaformType.Win64, '../../Resource')
        debugConfig = TargetConfig(self, 'Debug', PlaformType.Win64, '../../Resource')
        debugConfig.enableDebug = True
        
        self.enableDpiAwareness = True

        self.configs = [releaseConfig, debugConfig]
        self.moduleDependencies = ['ShaderCompiler']
        self.iconPath = '../../Engine/Icon/Logo.ico'
