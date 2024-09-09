from Core.ProjectGenerator import ProjectGenerator
from Core.Foundation import BuildObjectCollector
from Core.BuildObject import *
import hashlib

class VSTargetConfig:
    def __init__(self, config : TargetConfig):
        
        def getPlaformName(plaformType : PlaformType):
            if plaformType == PlaformType.Win32:
                return 'win32'
            elif plaformType == PlaformType.Win64:
                return 'x64'
            raise TypeError(plaformType)
        
        def getDebugMacro(enableDebug : bool):
            return '_DEBUG' if enableDebug else 'NDEBUG'
        
        def getAdditionalOption(option : str):
            if option == 'bigobj':
                return '/bigobj'
        
        self.name : str = config.name
        self.plaform : PlaformType = config.plaform
        self.plaformName : str = getPlaformName(config.plaform)
        self.optimize : bool = config.optimize
        self.enableDebug : bool = config.enableDebug
        self.needDebugInfo : bool = config.needDebugInfo

        self.intermediatePath = config.intermediatePath
        self.outputPath = config.outputPath
        self.outputName = config.outputName

        self.conditionStr : str = self.name + '|' + self.plaformName

        self.includePathStr : str = ''.join([''.join(item+";" for item in module.includePaths) for module in config.modules])
        self.libPathStr : str = ''.join([''.join(item+";" for item in module.libPaths) for module in config.modules])
        self.sourcePathStr : str = ''.join([''.join(item+";" for item in module.sourcePaths) for module in config.modules])
        self.libraryStr : str = ''.join([''.join(item+";" for item in module.libFiles) for module in config.modules])

        apiDefine = ''
        if config.target.type == TargetType.ModuleDll:
            apiDefine = '__declspec(dllexport)'
        elif config.target.type == TargetType.Executable or config.target.type == TargetType.SharedDll:
            apiDefine = '__declspec(dllimport)'
        moduleDefines = [getDebugMacro(config.enableDebug)]
        for module in config.modules:
            moduleDefines.extend(module.preprocessorDefinitions)
            moduleDefines.append(f'{module.name.upper()}_API={apiDefine}')
        self.preprocessorDefinitionStr = ';'.join(moduleDefines) + ';'
        additionalOptions = []
        for option in config.additionalOptions:
            option = getAdditionalOption(option)
            if option:
                additionalOptions.append(option)
        additionalOptions.append('%(AdditionalOptions)')
        self.additionalOptionStr = ' '.join(additionalOptions)

        self.includeFiles : list[str] = config.includeFiles
        self.sourceFiles : list[str] = config.sourceFiles
        self.otherFiles : list[str] = config.otherFiles
        self.dllFiles : set[str] = set()
        for module in config.modules:
            for dllFile in module.dllFiles:
                self.dllFiles.add(dllFile)

class VSProjFileWriter:
    def __init__(self, writable, indentSymbol = '  '):
        self.writable = writable
        self.indent = ''
        self.indentSymbol = indentSymbol
    
    def wl(self, line : str):
        '''write line'''
        self.writable.write(self.indent)
        self.writable.write(line)
        self.writable.write('\n')
    
    def ss(self, line : str):
        '''scope start'''
        self.wl(line)
        self.indent += self.indentSymbol
    
    def se(self, line : str):
        '''scope end'''
        self.indent = self.indent[0:-len(self.indentSymbol)]
        self.wl(line)

class VisualStudioProjectGenerator(ProjectGenerator, name = "VS"):
    class VSProjectInfo:
        def __init__(self):
            self.name : str = ''
            self.path : str = ''
            self.guid : str = ''
            self.configStrs : list[str] = []
            self.dependencies : set[str] = set()

    def __init__(self):
        ProjectGenerator.__init__(self)
        self.platformToolSet = 'v142'
        self.sdlCheck = True
        self.warningLevel = 'Level3'
        self.conformanceMode = False
        self.folderEntryGuid = '{2150E333-8FDC-42A3-9474-1A3956D46DE8}'
        self.cppProjEntryGuid = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'
        self.csProjEntryGuid = '{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}'

    @staticmethod
    def genGUID(filter : str):
        hexStr = hashlib.md5(filter.encode(encoding="utf-8")).hexdigest().upper()
        return hexStr[0:8] + '-' + hexStr[8:12] + '-' + hexStr[12:16] + '-' + hexStr[16:20] + '-' + hexStr[20:32]
    
    def generateProjectFile(self, target : Target) -> VSProjectInfo:

        def getTargetTypeName(targetType : TargetType):
            if targetType == TargetType.Executable:
                return 'Application'
            elif targetType == TargetType.StaticLib or targetType == TargetType.ModuleLib:
                return 'StaticLibrary'
            elif targetType == TargetType.SharedDll or targetType == TargetType.ModuleDll:
                return 'DynamicLibrary'
            raise TypeError(targetType)
        
        def getCppStandardName(cppStd : CppStandard):
            if cppStd == CppStandard.CppLatest:
                return 'stdcpplatest'
            elif cppStd == CppStandard.Cpp14:
                return 'stdcpp14'
            elif cppStd == CppStandard.Cpp17:
                return 'stdcpp17'
            elif cppStd == CppStandard.Cpp20:
                return 'stdcpp20'
            raise TypeError(cppStd)
        
        def getCharSetName(charSet : CharSetType):
            if charSet == CharSetType.Utf8:
                return 'MultiByte'
            elif charSet == CharSetType.Unicode:
                return 'Unicode'
            raise TypeError(charSet)
        
        def getSubsystemName(subsystem : SubsystemType):
            if subsystem == SubsystemType.Console:
                return 'Console'
            elif subsystem == SubsystemType.Windows:
                return 'Windows'
            raise TypeError(subsystem)
        
        def getSubsystemMacro(subsystem : SubsystemType):
            if subsystem == SubsystemType.Console:
                return '_CONSOLE'
            elif subsystem == SubsystemType.Windows:
                return '_WINDOWS'
            raise TypeError(subsystem)
        
        iconAbsPath = os.path.normpath(target.entry.basePath + '/' + target.iconPath) if target.iconPath else ''
        iconWinPath = Path(target.iconPath).as_posix().replace('/', '\\\\')
        needResourceFiles = os.path.exists(iconAbsPath)
        if needResourceFiles:
            rcFilePath = f'{target.entry.basePath}/{target.name}.rc'
            Log.log(f'[VS]Generate RC file {rcFilePath}')
            with open(rcFilePath, 'w') as rcFile:
                rcFile.write(
fr'''// Microsoft Visual C++ generated resource script.
//

#include "resource.h"

#define APSTUDIO_READONLY_SYMBOLS
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 2 resource.
//
#include "winres.h"

/////////////////////////////////////////////////////////////////////////////
#undef APSTUDIO_READONLY_SYMBOLS

#ifdef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// TEXTINCLUDE
//

1 TEXTINCLUDE 
BEGIN
    "resource.h\0"
END

2 TEXTINCLUDE 
BEGIN
    "#include ""winres.h""\r\n"
    "\0"
END

3 TEXTINCLUDE 
BEGIN
    "\r\n"
    "\0"
END

#endif    // APSTUDIO_INVOKED

/////////////////////////////////////////////////////////////////////////////
//
// Icon
//

// Icon with lowest ID value placed first to ensure application icon
// remains consistent on all systems.
IDI_ICON1 ICON "{iconWinPath}"

/////////////////////////////////////////////////////////////////////////////

#ifndef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 3 resource.
//


/////////////////////////////////////////////////////////////////////////////
#endif    // not APSTUDIO_INVOKED
''')
            
            resourceFilePath = f'{target.entry.basePath}/resource.h'
            Log.log(f'[VS]Generate resource.h file {resourceFilePath}')
            with open(resourceFilePath, 'w') as resourceFile:
                resourceFile.write(
f'''
//{{{{NO_DEPENDENCIES}}}}
// Microsoft Visual C++ generated include file
// For {target.name}.rc

#define IDI_ICON1                       101

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        102
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif

'''
                )
        
        vsTargetConfigs = [VSTargetConfig(config) for config in target.configs]

        projGUID = VisualStudioProjectGenerator.genGUID("__" + target.name + "__VS__PROJ")
        
        filters : set[str] = set()
        def getFilter(file : str) -> str:
            return os.path.normpath(os.path.dirname(file)).lstrip('.\\')

        def collectFilters(files : list, fileSet : set):
            for file in files:
                filter = getFilter(file)
                while filter:
                    filters.add(filter)
                    filter = os.path.dirname(filter)
                fileSet.add(file)
        
        includeFiles : set[str] = set()
        sourceFiles : set[str] = set()
        otherFiles : set[str] = set()

        if needResourceFiles:
            collectFilters([target.iconPath], otherFiles)
            includeFiles.add('resource.h')

        for config in vsTargetConfigs:
            collectFilters(config.includeFiles, includeFiles)
            collectFilters(config.sourceFiles, sourceFiles)
            collectFilters(config.otherFiles, otherFiles)
        
        if needResourceFiles:
            includeFiles.add(Path(os.path.relpath(os.path.abspath(resourceFilePath), target.getPath())).as_posix())
    
        projPath = f'{target.entry.basePath}/{target.name}.vcxproj'
        with open(projPath, 'w') as projFile:
            configurationTypeStr : str = getTargetTypeName(target.type)
            cppStandardName : str = getCppStandardName(target.cppStandard)
            charSetName : str = getCharSetName(target.charSet)
            subsystemName : str = getSubsystemName(target.subsystem)
            targetPreprocessorDefinitionStr : str = ''.join([item+";" for item in target.preprocessorDefinitions])
            targetPreprocessorDefinitionStr += getSubsystemMacro(target.subsystem) + ';'
            targetLibraryStr : str = ''.join([item+";" for item in target.libraries])

            w = VSProjFileWriter(projFile)
            w.wl('<?xml version="1.0" encoding="utf-8"?>')
            w.ss('<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">')

            w.ss('<ItemGroup Label="ProjectConfigurations">')
            for config in vsTargetConfigs:
                w.ss(f'<ProjectConfiguration Include="{config.conditionStr}">')
                w.wl(f'<Configuration>{config.name}</Configuration>')
                w.wl(f'<Platform>{config.plaformName}</Platform>')
                w.se('</ProjectConfiguration>')
            w.se('</ItemGroup>')

            w.ss('<PropertyGroup Label="Globals">')
            w.wl('<VCProjectVersion>16.0</VCProjectVersion>')
            w.wl('<Keyword>Win32Proj</Keyword>')
            w.wl(f'<ProjectGuid>{{{projGUID}}}</ProjectGuid>')
            w.wl(f'<RootNamespace>{target.name}</RootNamespace>')
            w.wl('<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>')
            w.se('</PropertyGroup>')

            w.wl('<Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />')

            for config in vsTargetConfigs:
                w.ss(f'<PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'{config.conditionStr}\'" Label="Configuration">')
                w.wl(f'<ConfigurationType>{configurationTypeStr}</ConfigurationType>')
                w.wl(f'<PlatformToolset>{self.platformToolSet}</PlatformToolset>')
                w.wl(f'<UseDebugLibraries>{config.enableDebug}</UseDebugLibraries>')
                w.wl(f'<WholeProgramOptimization>{not config.enableDebug}</WholeProgramOptimization>')
                w.wl(f'<CharacterSet>{charSetName}</CharacterSet>')
                w.se('</PropertyGroup>')
            
            w.wl('<Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />')
            w.wl('<ImportGroup Label="ExtensionSettings">')
            w.wl('</ImportGroup>')
            w.wl('<ImportGroup Label="Shared">')
            w.wl('</ImportGroup>')

            for config in vsTargetConfigs:
                w.ss(f'<ImportGroup Label="PropertySheets" Condition="\'$(Configuration)|$(Platform)\'==\'{config.conditionStr}\'">')
                w.wl('<Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists(\'$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props\')" Label="LocalAppDataPlatform" />')
                w.se('</ImportGroup>')
            
            w.wl('<PropertyGroup Label="UserMacros" />')

            for config in vsTargetConfigs:
                w.ss(f'<PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'{config.conditionStr}\'">')
                w.wl(f'<LinkIncremental>{config.enableDebug}</LinkIncremental>')
                w.wl(f'<IncludePath>{config.includePathStr}$(VC_IncludePath);$(WindowsSDK_IncludePath);</IncludePath>')
                w.wl(f'<LibraryPath>{config.libPathStr}$(LibraryPath)</LibraryPath>')
                w.wl(f'<SourcePath>{config.sourcePathStr}$(VC_SourcePath);</SourcePath>')
                
                if config.intermediatePath:
                    w.wl(f'<IntDir>{config.intermediatePath}/</IntDir>')
                
                if config.outputPath:
                    w.wl(f'<OutDir>{config.outputPath}/</OutDir>')
                
                if config.outputName:
                    w.wl(f'<TargetName>{config.outputName}</TargetName>')
                w.se('</PropertyGroup>')
            
            for config in vsTargetConfigs:
                w.ss(f'<ItemDefinitionGroup Condition="\'$(Configuration)|$(Platform)\'==\'{config.conditionStr}\'">')
                
                w.ss('<ClCompile>')
                w.wl(f'<WarningLevel>{self.warningLevel}</WarningLevel>')
                if not config.enableDebug:
                    w.wl(f'<FunctionLevelLinking>True</FunctionLevelLinking>')
                    w.wl(f'<IntrinsicFunctions>True</IntrinsicFunctions>')
                w.wl(f'<SDLCheck>{self.sdlCheck}</SDLCheck>')
                w.wl(f'<PreprocessorDefinitions>{config.preprocessorDefinitionStr}{targetPreprocessorDefinitionStr}%(PreprocessorDefinitions)</PreprocessorDefinitions>')
                w.wl(f'<ConformanceMode>{self.conformanceMode}</ConformanceMode>')
                w.wl(f'<LanguageStandard>{cppStandardName}</LanguageStandard>')
                w.wl(f'<AdditionalOptions>{config.additionalOptionStr}</AdditionalOptions>')
                w.se('</ClCompile>')

                w.ss('<Link>')
                w.wl(f'<SubSystem>{subsystemName}</SubSystem>')
                if not config.enableDebug:
                    w.wl(f'<EnableCOMDATFolding>True</EnableCOMDATFolding>')
                    w.wl(f'<OptimizeReferences>True</OptimizeReferences>')
                w.wl(f'<GenerateDebugInformation>{config.needDebugInfo}</GenerateDebugInformation>')
                w.wl(f'<AdditionalDependencies>{config.libraryStr}{targetLibraryStr}%(AdditionalDependencies)</AdditionalDependencies>')
                w.se('</Link>')

                w.ss('<Manifest>')
                w.wl(f'<EnableDpiAwareness>{target.enableDpiAwareness}</EnableDpiAwareness>')
                w.se('</Manifest>')

                w.ss('<CustomBuildStep>')
                if target.type == TargetType.Executable:
                    copyCmd = 'echo f|xcopy "$(ProjectDir){}" "$(TargetDir){}" /y /d\n'
                    inputFile = "$(ProjectDir){}"
                    outputFile = "$(TargetDir){}"
                    w.wl(f'<Command>{"".join([copyCmd.format(os.path.normpath(dllFile), os.path.basename(dllFile)) for dllFile in config.dllFiles])}</Command>')
                    w.wl(f'<Inputs>{";".join([inputFile.format(os.path.normpath(dllFile)) for dllFile in config.dllFiles])}</Inputs>')
                    w.wl(f'<Outputs>{";".join([outputFile.format(os.path.basename(dllFile)) for dllFile in config.dllFiles])}</Outputs>')
                w.se('</CustomBuildStep>')
                
                w.se('</ItemDefinitionGroup>')
            
            w.ss(f'<ItemGroup>')
            for file in includeFiles:
                w.wl(f'<ClInclude Include="{file}" />')
            w.se('</ItemGroup>')
            
            w.ss(f'<ItemGroup>')
            for file in sourceFiles:
                w.wl(f'<ClCompile Include="{file}" />')
            w.se('</ItemGroup>')
            
            w.ss(f'<ItemGroup>')
            for file in otherFiles:
                w.wl(f'<None Include="{file}" />')
            w.se('</ItemGroup>')
            
            if needResourceFiles:
                w.ss('<ItemGroup>')
                w.wl(f'<ResourceCompile Include="{target.name}.rc" />')
                w.se('</ItemGroup>')
            
            w.wl('<Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />')
            w.wl('<ImportGroup Label="ExtensionTargets">')
            w.wl('</ImportGroup>')

            w.se('</Project>')

        filterPath = projPath + '.filters'
        with open(filterPath, 'w') as filterFile:
            w = VSProjFileWriter(filterFile)
            w.wl('<?xml version="1.0" encoding="utf-8"?>')
            w.ss('<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">')

            w.ss(f'<ItemGroup>')
            for filter in filters:
                if filter:
                    w.ss(f'<Filter Include="{filter}">')
                    w.wl(f'<UniqueIdentifier>{{{VisualStudioProjectGenerator.genGUID(filter)}}}</UniqueIdentifier>')
                    w.se(f'</Filter>')
            w.se('</ItemGroup>')
            
            w.ss(f'<ItemGroup>')
            for file in includeFiles:
                w.ss(f'<ClInclude Include="{file}">')
                filter = getFilter(file)
                if filter:
                    w.wl(f'<Filter>{filter}</Filter>')
                w.se(f'</ClInclude>')
            w.se('</ItemGroup>')
            
            w.ss(f'<ItemGroup>')
            for file in sourceFiles:
                w.ss(f'<ClCompile Include="{file}">')
                filter = getFilter(file)
                if filter:
                    w.wl(f'<Filter>{filter}</Filter>')
                w.se(f'</ClCompile>')
            w.se('</ItemGroup>')
            
            w.ss(f'<ItemGroup>')
            for file in otherFiles:
                w.ss(f'<None Include="{file}">')
                filter = getFilter(file)
                if filter:
                    w.wl(f'<Filter>{filter}</Filter>')
                w.se(f'</None>')
            w.se('</ItemGroup>')
            
            w.se('</Project>')
        
        info = VisualStudioProjectGenerator.VSProjectInfo()
        info.name = target.name
        info.path = Path(os.path.relpath(projPath, os.curdir)).as_posix()
        info.guid = projGUID
        info.configStrs = [config.conditionStr for config in vsTargetConfigs]
        info.dependencies = target.targetDependencies
        return info
    
    def generate(self, name : str, targets : BuildObjectCollector[Target]):
        projs : list[VisualStudioProjectGenerator.VSProjectInfo] = []
        for target in targets:
            Log.log(f'[VS]Generting project file of Target "{target.name}"')
            proj = self.generateProjectFile(target)
            projs.append(proj)
            Log.log(f'[VS]Generted project file at "{proj.path}"')
        
        def findTargetGUID(name : str):
            for proj in projs:
                if proj.name == name:
                    return proj.guid
            raise NameError(f'Configuration of Target "{name}" not complete')
        
        slnPath = f'{name}.sln'
        with open(slnPath, 'w') as slnFile:
            w = VSProjFileWriter(slnFile, '\t')

            w.wl('Microsoft Visual Studio Solution File, Format Version 12.00')
            w.wl('# Visual Studio Version 16')
            w.wl('VisualStudioVersion = 16.0.32802.440')
            w.wl('MinimumVisualStudioVersion = 10.0.40219.1')
            for proj in projs:
                w.ss(f'Project("{self.cppProjEntryGuid}") = "{proj.name}", "{proj.path}", "{{{proj.guid}}}"')
                w.ss('ProjectSection(ProjectDependencies) = postProject')
                for dependedName in proj.dependencies:
                    guid = findTargetGUID(dependedName)
                    w.wl(f'{{{guid}}} = {{{guid}}}')
                w.se('EndProjectSection')
                w.se('EndProject')

            w.ss('Global')

            w.ss('GlobalSection(SolutionConfigurationPlatforms) = preSolution')
            configStrSet : set[str] = set()
            for proj in projs:
                for configStr in proj.configStrs:
                    configStrSet.add(configStr)
            for configStr in configStrSet:
                w.wl(f'{configStr}={configStr}')
            w.se('EndGlobalSection')

            w.ss('GlobalSection(ProjectConfigurationPlatforms) = postSolution')
            for proj in projs:
                for configStr in proj.configStrs:
                    w.wl(f'{{{proj.guid}}}.{configStr}.ActiveCfg={configStr}')
                    w.wl(f'{{{proj.guid}}}.{configStr}.Build.0={configStr}')
            w.se('EndGlobalSection')

            w.ss('GlobalSection(SolutionProperties) = preSolution')
            w.wl('HideSolutionNode = FALSE')
            w.se('EndGlobalSection')

            w.ss('GlobalSection(ExtensibilityGlobals) = preSolution')
            w.wl(f'SolutionGuid = {{{VisualStudioProjectGenerator.genGUID("__" + name + "__Solution__")}}}')
            w.se('EndGlobalSection')

            w.se('EndGlobal')
        
        Log.log(f'[VS]Generted solution file at "{slnPath}"')
