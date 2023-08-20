import os
from Foundation import *
from pathlib import Path
import traceback
import importlib
import enum

class TargetType(enum.Enum):
    Executable = 0
    StaticLib = 1
    SharedDll = 2

class CharSetType(enum.Enum):
    Utf8 = 0
    Unicode = 1

class PlaformType(enum.Enum):
    Win32 = 0
    Win64 = 1

class CppStandard(enum.Enum):
    CppLatest = 0
    Cpp14 = 1
    Cpp17 = 2
    Cpp20 = 3

class SubsystemType(enum.Enum):
    Console = 0
    Windows = 1

class TargetConfig:
    pass

class Target(BuildObject):
    pass

class Module(BuildObject):
    def __init__(self, param : BuildParam):
        BuildObject.__init__(self, param)
        self.sourceTarget = ''
        self.includePaths : list[str] = []
        self.sourcePaths : list[str] = []
        self.libPaths : list[str] = []
        self.libFiles : list[str] = []
        self.dllFiles : list[str] = []
        self.moduleDependencies : list[str] = []
        self.preprocessorDefinitions : list[str] = []

        self.fileSearcher : FileSearcher = FileSearcher()
        self.headFileSearchPattern : str = '^.*\.(h|hh|hpp|hxx|hm|inl|inc|ipp|xsd)$'
        self.sourceFileSearchPattern : str = '^.*\.(cpp|c|cc|cxx|cppm|ixx|def|ams|asmx)$'
        self.otherFileSearchPattern : str = '^.*\.(py)'
    
    def instance(self):
        param = BuildParam()
        param.entry = self.entry
        param.name = self.name
        param.defFile = self.defFile
        instance = self.__class__(param)
        return instance
    
    def searchIncludeFiles(self, collector : BuildObjectCollector) -> list:
        if self.headFileSearchPattern:
            return self.fileSearcher.search(
                self.entry.basePath,
                self.headFileSearchPattern,
                collector.absPaths)
        return []
    
    def searchSourceFiles(self, collector : BuildObjectCollector) -> list:
        if self.sourceFileSearchPattern:
            return self.fileSearcher.search(
                self.entry.basePath,
                self.sourceFileSearchPattern,
                collector.absPaths)
        return []
    
    def searchOtherFiles(self, collector : BuildObjectCollector) -> list:
        if self.otherFileSearchPattern:
            return self.fileSearcher.search(
                self.entry.basePath,
                self.otherFileSearchPattern,
                collector.absPaths)
        return []
    
    def setup(self, config : TargetConfig):
        pass

class TargetConfig:        
    def __init__(self, target : Target, name : str, plaform : PlaformType):
        self.target : Target = target
        self.name : str = name
        self.plaform : PlaformType = plaform
        self.optimize : bool = True
        self.enableDebug : bool  = False
        self.needDebugInfo : bool  = True
        self.intermediatePath : str = ''
        self.outputPath : str = ''
        self.outputName : str = ''
        self.postBuildCommand : str = ''
        self.debugPath : str = ''
        self.debugArguments : str = ''
        self.modules : list[Module] = []
        self.includeFiles : list[str] = []
        self.sourceFiles : list[str] = []
        self.otherFiles : list[str] = []
    
    def __eq__(self, __value : object) -> bool:
            return self.name == __value.name and self.plaform == __value.plaform
        
    def __hash__(self) -> int:
        return hash((self.name, self.plaform))

class Target(BuildObject):
    def __init__(self, param : BuildParam):
        BuildObject.__init__(self, param)
        self.type : TargetType = TargetType.Executable
        self.charSet : CharSetType = CharSetType.Utf8
        self.cppStandard : CppStandard = CppStandard.Cpp17
        self.subsystem : SubsystemType = SubsystemType.Windows
        self.enableDpiAwareness : bool = False
        self.iconPath : str = ''
        self.preprocessorDefinitions : list[str] = []
        self.libraries : list[str] = []
        self.configs : set[TargetConfig] = set()

        self.moduleDependencies : list[str] = []
        self.modules : list[Module] = []

        self.targetDependencies : set[str] = []
    
    def resolveModules(self, collector : BuildObjectCollector[Module]):
        pendingNames = self.moduleDependencies.copy()
        loadedNames = set()
        workingNames = []
        workingModules = []

        while len(pendingNames) > 0:
            moduleName = pendingNames[-1]
            if len(workingNames) > 0 and moduleName == workingNames[-1]:
                pendingNames.pop()
                workingNames.pop()
                loadedNames.add(moduleName)
                self.modules.append(workingModules.pop())
                Log.log(f'Module "{moduleName}" resolved')
                continue
            if moduleName in loadedNames:
                pendingNames.pop()
                continue
            if moduleName in workingNames:
                errStr = f'Circular dependency is found on module "{moduleName}"'
                Log.err(errStr)
                raise RecursionError(errStr)
            
            module = collector.get(moduleName)
            if module is None:
                errStr = f'Module "{moduleName}" not found'
                Log.err(errStr)
                raise LookupError(errStr)
            
            if len(module.moduleDependencies) == 0:
                pendingNames.pop()
                loadedNames.add(moduleName)
                self.modules.append(module)
                Log.log(f'Module "{moduleName}" resolved')
            else:
                pendingNames.extend(module.moduleDependencies)
                workingNames.append(moduleName)
                workingModules.append(module)
                Log.log(f'Resolving dependencies of Module "{moduleName}"')
        
        targetDependencies = [name for name in self.targetDependencies]
        targetDependencies.extend([module.sourceTarget for module in self.modules
            if module.sourceTarget and module.sourceTarget != self.name])
        self.targetDependencies = set(targetDependencies)
    
    def convertToRelatedPath(self, path : str):
        if os.path.isabs(path):
            relPath = os.path.relpath(path, self.getPath())
            return Path(relPath if relPath else path).as_posix()
        else:
            return Path(path).as_posix()
    
    def convertToRelatedPaths(self, paths : list):
        for i in range(len(paths)):
            paths[i] = self.convertToRelatedPath(paths[i])
    
    def setupModules(self, collector : BuildObjectCollector[Module]):
        for config in self.configs:
            for module in self.modules:
                instance = module.instance()
                config.modules.append(instance)
            for module in config.modules:
                module.setup(config)
                includeFiles = module.searchIncludeFiles(collector)
                sourceFiles = module.searchSourceFiles(collector)
                otherFiles = module.searchOtherFiles(collector)
                otherFiles.append(module.defFile)
                otherFiles.append(self.defFile)
                self.convertToRelatedPaths(includeFiles)
                self.convertToRelatedPaths(sourceFiles)
                self.convertToRelatedPaths(otherFiles)
                config.includeFiles.extend(includeFiles)
                config.sourceFiles.extend(sourceFiles)
                config.otherFiles.extend(otherFiles)
                self.convertToRelatedPaths(module.includePaths)
                self.convertToRelatedPaths(module.libPaths)
                self.convertToRelatedPaths(module.sourcePaths)
                self.convertToRelatedPaths(module.dllFiles)

                if config.intermediatePath:
                    config.intermediatePath = self.convertToRelatedPath(config.intermediatePath)
                if config.outputPath:
                    config.outputPath = self.convertToRelatedPath(config.outputPath)
                if config.debugPath:
                    config.debugPath = self.convertToRelatedPath(config.debugPath)
