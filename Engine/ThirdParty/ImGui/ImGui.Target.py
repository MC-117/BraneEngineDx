from Core.BuildObject import Target, TargetConfig, PlaformType, TargetType
from Core.Foundation import Log, BuildParam

class ImGuiTarget(Target):
    def __init__(self, param : BuildParam):
        Target.__init__(self, param)
        self.type = TargetType.ModuleLib
        releaseConfig = TargetConfig(self, 'Release', PlaformType.Win64)
        debugConfig = TargetConfig(self, 'Debug', PlaformType.Win64)
        debugConfig.enableDebug = True

        self.configs = [releaseConfig, debugConfig]
        self.moduleDependencies = ['ImGui']
