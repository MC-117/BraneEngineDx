from Core.BuildObject import Target, TargetConfig, PlaformType
from Core.Foundation import Log, BuildParam

class ToolTarget(Target):
    def __init__(self, param : BuildParam):
        Target.__init__(self, param)
        releaseConfig = TargetConfig(self, 'Release', PlaformType.Win64, '../../Resource')
        debugConfig = TargetConfig(self, 'Debug', PlaformType.Win64, '../../Resource')
        debugConfig.enableDebug = True
        
        self.enableDpiAwareness = True

        self.configs = [releaseConfig, debugConfig]
        self.moduleDependencies = ['Tool']
        self.iconPath = '../../Engine/Icon/Logo.ico'
