from Core.BuildObject import Target, TargetConfig, PlaformType, TargetType
from Core.Foundation import Log, BuildParam

class EngineTarget(Target):
    def __init__(self, param : BuildParam):
        Target.__init__(self, param)
        self.type = TargetType.ModuleDll
        releaseConfig = TargetConfig(self, 'Release', PlaformType.Win64)
        debugConfig = TargetConfig(self, 'Debug', PlaformType.Win64)
        debugConfig.enableDebug = True
        debugConfig.additionalOptions = ['bigobj']
        
        self.configs = [releaseConfig, debugConfig]
        self.moduleDependencies = ['Engine']
