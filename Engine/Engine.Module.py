from Core.BuildObject import Module, TargetConfig, PlaformType, TargetType
from Core.Foundation import BuildParam, FileSearcher

class EngineModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}']
        self.buildTarget = 'Engine'
        
        self.libFiles = [
            'dxguid.lib',
            'dinput8.lib',
            'd3d11.lib',
            'd3d12.lib',
            'd3dcompiler.lib',
            'dxgi.lib',
            'dwmapi.lib',
        ]

        self.preprocessorDefinitions = [
            '_HAS_STD_BYTE=0',
            'AUDIO_USE_OPENAL',
        ]

        self.moduleDependencies = [
            'Assimp',
            'ImGui',
            'Live2DCubism',
            'MemTracer',
            'NvCloth',
            'OpenAlSoft',
            'Physx4',
            'Python37',
            'Remidi',
            'RenderDoc',
            'Spine',
            'STB',
        ]
    
    def setup(self, config : TargetConfig):
        if config.plaform != PlaformType.Win64:
            raise NotImplementedError('Only support Win64')
        if config.target.type == TargetType.ModuleLib or config.target.type == TargetType.ModuleDll:
            self.fileSearcher.addPath('.', True)
            self.fileSearcher.addPath('../Resource/Engine/Shaders', True)
            self.fileSearcher.excludePath('ThirdParty')
        
            self.otherFileSearchPattern = '^.*\.(py|shadapter|mat|hmat)'
        else:
            self.libPaths = [self.buildTargetAbsOutputPath]
            self.libFiles.append(self.buildTargetOutputName + '.lib')
            self.dllFiles = [f'{self.buildTargetAbsOutputPath}/{self.buildTargetOutputName}.dll']
