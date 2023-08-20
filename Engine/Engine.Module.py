from Core.BuildObject import Module
from Core.Foundation import BuildParam, FileSearcher

class EngineModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}']
        self.fileSearcher.addPath('.', True)
        self.fileSearcher.addPath('../Resource/Engine/Shaders', True)
        self.fileSearcher.excludePath('ThirdParty')
        
        self.otherFileSearchPattern = '^.*\.(py|shadapter|mat|hmat)'
        
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
