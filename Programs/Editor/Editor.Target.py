from Core.BuildObject import Target, TargetConfig, PlaformType
from Core.Foundation import Log, BuildParam

class EditorTarget(Target):
    def __init__(self, param : BuildParam):
        Target.__init__(self, param)
        releaseConfig = TargetConfig(self, 'Release', PlaformType.Win64)
        debugConfig = TargetConfig(self, 'Debug', PlaformType.Win64)
        debugConfig.enableDebug = True
        
        self.enableDpiAwareness = True

        self.configs = [releaseConfig, debugConfig]
        self.moduleDependencies = ['Editor']
        self.iconPath = '../../Engine/Icon/Logo.ico'
