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
    ModuleLib = 3
    ModuleDll = 4

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
        self.buildTarget : str = ''
        self.buildTargetAbsOutputPath : str = ''
        self.buildTargetOutputName : str = ''
        self.includePaths : list[str] = []
        self.sourcePaths : list[str] = []
        self.libPaths : list[str] = []
        self.libFiles : list[str] = []
        self.dllFiles : list[str] = []
        self.moduleDependencies : list[str] = []
        self.preprocessorDefinitions : list[str] = []

        self.fileSearcher : FileSearcher = FileSearcher()
        self.headFileSearchPattern : str = f'^.*\.(h|hh|hpp|hxx|hm|inl|inc|ipp|xsd)$'
        self.sourceFileSearchPattern : str = f'^.*\.(cpp|c|cc|cxx|cppm|ixx|def|ams|asmx)$'
        self.otherFileSearchPattern : str = f'^.*\.(py)'
    
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
    
    def resolveBuildTarget(self, config : TargetConfig, target : Target):
        compatibleConfig = target.getCompatibleConfig(config)
        if compatibleConfig is None:
            Log.wrn(f'Target "{target.name}" is not compatible with config({config.name} | {config.plaform.name})')
        self.buildTargetAbsOutputPath = target.convertToAbsPath(compatibleConfig.outputPath)
        self.buildTargetOutputName = compatibleConfig.outputName
    
    def setup(self, config : TargetConfig):
        pass

class TargetConfig:        
    def __init__(self, target : Target, name : str, plaform : PlaformType, debugArguments : str = ''):
        self.target : Target = target
        self.name : str = name
        self.plaform : PlaformType = plaform
        self.optimize : bool = True
        self.enableDebug : bool  = False
        self.needDebugInfo : bool  = True
        self.additionalOptions : list[str] = []
        self.intermediatePath : str = ''
        self.outputPath : str = ''
        self.outputName : str = ''
        self.postBuildCommand : str = ''
        self.debugPath : str = ''
        self.debugArguments : str = debugArguments
        self.modules : list[Module] = []
        self.includeFiles : list[str] = []
        self.sourceFiles : list[str] = []
        self.otherFiles : list[str] = []
    
    def __eq__(self, __value : object) -> bool:
            return self.name == __value.name and self.plaform == __value.plaform
        
    def __hash__(self) -> int:
        return hash((self.name, self.plaform))
    
    def isCompatible(self, name : str, plaform : PlaformType):
        return self.name == name and self.plaform == plaform
    
    def isCompatible(self, other : TargetConfig):
        return self == other
    
    def autoFill(self):
        workPathByProjPath = Path(os.path.relpath(os.curdir, self.target.getPath())).as_posix()
        specificPath = f'{self.plaform.name}/{self.name}/{self.target.name}'
        if not self.intermediatePath:
            self.intermediatePath = f'{workPathByProjPath}/Intermediate/{specificPath}'
        if not self.outputPath:
            self.outputPath = f'{workPathByProjPath}/Binaries/{specificPath}'
        if not self.outputName:
            self.outputName = self.target.name

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
        targetDependencies.extend([module.buildTarget for module in self.modules
            if module.buildTarget and module.buildTarget != self.name])
        self.targetDependencies = set(targetDependencies)

        if len(self.targetDependencies) > 0:
            Log.log(f'Target "{self.name}" depends on Targets {" ".join([targetName for targetName in self.targetDependencies])}')
    
    def getCompatibleConfig(self, name : str, plaform : PlaformType):
        for config in self.configs:
            if config.isCompatible(name, plaform):
                return config
    
    def getCompatibleConfig(self, config : TargetConfig):
        for _config in self.configs:
            if _config.isCompatible(config):
                return _config
    
    def convertToRelatedPath(self, path : str):
        if os.path.isabs(path):
            relPath = os.path.relpath(path, self.getPath())
            return Path(relPath if relPath else path).as_posix()
        else:
            return Path(path).as_posix()
    
    def convertToRelatedPaths(self, paths : list):
        for i in range(len(paths)):
            paths[i] = self.convertToRelatedPath(paths[i])
    
    def convertToAbsPath(self, path : str):
        if os.path.isabs(path):
            return Path(path).as_posix()
        else:
            absPath = os.path.abspath(f'{self.getPath()}/{path}')
            return Path(absPath if absPath else path).as_posix()
    
    def setupModules(self, moduleCollector : BuildObjectCollector[Module], targetCollector : BuildObjectCollector[Target]):
        for config in self.configs:
            for module in self.modules:
                instance = module.instance()
                config.modules.append(instance)
            config.autoFill()
            for module in config.modules:
                if module.buildTarget != '':
                    buildTarget = targetCollector.get(module.buildTarget)
                    if buildTarget is None:
                        errStr = f'Build Target "{module.buildTarget}" not found in Module "{module.name}"'
                        Log.err(errStr)
                        raise LookupError(errStr)
                    else:
                        module.resolveBuildTarget(config, buildTarget)
                module.setup(config)
                includeFiles = module.searchIncludeFiles(moduleCollector)
                sourceFiles = module.searchSourceFiles(moduleCollector)
                otherFiles = module.searchOtherFiles(moduleCollector)
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
