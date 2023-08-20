from Core.BuildObject import Target, TargetConfig, PlaformType, TargetType
from Core.Foundation import Log, BuildParam

class Live2DCubismTarget(Target):
    def __init__(self, param : BuildParam):
        Target.__init__(self, param)
        self.type = TargetType.StaticLib
        releaseConfig = TargetConfig(self, 'Release', PlaformType.Win64)
        debugConfig = TargetConfig(self, 'Debug', PlaformType.Win64)
        debugConfig.enableDebug = True
        
        outputPath = 'lib/windows/x86_64/142'
        releaseConfig.outputPath = outputPath
        debugConfig.outputPath = outputPath
        
        releaseConfig.intermediatePath = f'intermediate/{releaseConfig.plaform.name}/{releaseConfig.name}'
        debugConfig.intermediatePath = f'intermediate/{debugConfig.plaform.name}/{debugConfig.name}'
        
        releaseConfig.outputName = 'Live2DCubism_MT'
        debugConfig.outputName = 'Live2DCubism_MTd'

        self.configs = [releaseConfig, debugConfig]
        self.moduleDependencies = ['Live2DCubism']
