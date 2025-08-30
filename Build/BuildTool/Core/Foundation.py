import os
import re
import glob
import copy
import traceback
import importlib
import importlib.util
from datetime import datetime
from pathlib import Path
from typing import Generic, TypeVar, Iterator

class Log:
    path = 'Build.log'
    
    @classmethod
    def create(cls):
        try:
            file = open(cls.path, 'w')
            file.close()
            return True
        except:
            return False
    
    @classmethod
    def _write(cls, type, line):
        line = '[{}][{}]{}'.format(type, datetime.now().strftime('%Y-%m-%d %H:%M:%S'), line)
        print(line)
        if os.path.exists(cls.path):
            file = open(cls.path, 'a')
            file.write(line + '\n')
            file.close()
    
    @classmethod
    def log(cls, line):
        cls._write('log', line)
    
    @classmethod
    def wrn(cls, line):
        cls._write('wrn', line)
    
    @classmethod
    def err(cls, line):
        cls._write('err', line)

class BuildEntry:
    pass

class BuildParam:
    def __init__(self):
        self.entry : BuildEntry = None
        self.name : str = ''
        self.defFile : str = ''

class BuildEntry:
    def __init__(self, folder):
        self.basePath : str = Path(folder).as_posix()
        self.name : str = os.path.basename(self.basePath)
        self.specName : str = self.basePath.replace('/', '.')
    
    def mount(self, cls : type):
        clsName = cls.__name__
        path = '{}/{}.{}.py'.format(self.basePath, self.name, clsName)
        if not os.path.exists(path):
            return
        mountName = self.name + clsName
        spec = importlib.util.spec_from_file_location(name=mountName, location=path)
        if spec is None:
            Log.err(f'spec load failed at {path}')
            return
        try:
            pyModule = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(pyModule)
            param = BuildParam()
            param.entry = self
            param.name = self.name
            param.defFile = path
            buildObject = pyModule.__dict__[mountName](param)
            if issubclass(type(buildObject), cls):
                return buildObject
            else:
                Log.err(f'{clsName} {mountName} init failed')
        except:
            Log.err(traceback.format_exc())
        Log.log("Mount {} '{}' Failed".format(clsName, self.name))

class BuildParam:
    def __init__(self):
        self.entry : BuildEntry = None
        self.name : str = ''
        self.defFile : str = ''

class BuildObject:
    def __init__(self, param : BuildParam):
        self.entry : BuildEntry = param.entry
        self.name : str = param.name
        self.defFile : str = param.defFile
    
    def getPath(self) -> str:
        return self.entry.basePath
    
    def getFullName(self) -> str:
        return self.name + '.' + self.configName

BuildObjectT = TypeVar('BuildObjectT')

class BuildObjectCollector(Generic[BuildObjectT]):
    def __init__(self) -> None:
        self.slot : dict[str, BuildObjectT] = {}
        self.absPaths : set[str] = set()
    
    def add(self, name : str, buildObject : BuildObjectT):
        if name in self.slot:
            return False
        self.slot[name] = buildObject
        self.absPaths.add(Path(buildObject.entry.basePath).absolute().as_posix())
        return True
    
    def exist(self, name : str):
        return name in self.slot
    
    def get(self, name : str) -> BuildObjectT:
        if name in self.slot:
            return self.slot[name]
    
    def __iter__(self) -> Iterator[BuildObjectT]:
        return self.slot.values().__iter__()

class FileSearcher:
    def __init__(self):
        self.searchPaths : list[(str, bool)] = []
        self.excludedPaths : list[(str, bool)] = []
    
    def addPath(self, path : str, recursive : bool):
        self.searchPaths.append((path, recursive))
    
    def excludePath(self, path : str):
        self.excludedPaths.append(path)
    
    def search(self, basePath : str, pattern : str, ignoredAbsPaths : set) -> list:
        rePattern = re.compile(pattern, re.I)
        absBasePath = Path(basePath).absolute().as_posix() if basePath else ''

        if len(self.excludedPaths) > 0:
            ignoredAbsPaths = ignoredAbsPaths.copy()
            for path in self.excludedPaths:
                path = (Path(path) if os.path.isabs(path) else Path(basePath + '/' + path).absolute()).absolute().as_posix()
                ignoredAbsPaths.add(path)

        pendingSearchPath = []
        for path, recursive in self.searchPaths:
            path = (Path(path) if os.path.isabs(path) else Path(basePath + '/' + path).absolute()).absolute().as_posix()
            if (absBasePath and absBasePath == path) or (os.path.isdir(path) and os.path.exists(path) and path not in ignoredAbsPaths):
                pendingSearchPath.append((path, recursive))

        pendingSearchPath.sort(key=lambda x : x[0])
        finishedSearchPath = set()
        results = []

        while len(pendingSearchPath) > 0:
            path, recursive = pendingSearchPath.pop(0)
            if path in finishedSearchPath:
                continue
            subDir = []
            for subPath in os.listdir(path):
                subPath = Path(path + '/' + subPath).as_posix()
                if os.path.isfile(subPath):
                    if rePattern.match(subPath) is not None:
                        results.append(subPath)
                elif recursive and os.path.isdir(subPath) \
                    and subPath not in ignoredAbsPaths \
                    and subPath not in finishedSearchPath:
                    subDir.append((subPath, recursive))
            
            pendingSearchPath = subDir + pendingSearchPath
            finishedSearchPath.add(path)
        return results

if __name__ == '__main__':
    basePath = 'C:/Users/MC117/Documents/Visual Studio 2019/Projects/BraneEngineDx/BraneEngineDx'
    intermediatePath = basePath + '/' + 'Intermediate'
    thirdPartyPath = basePath + '/' + 'ThirdParty'
    Log.create()
    searcher = FileSearcher()
    searcher.addPath(basePath, True)
    ignoredAbsPaths = set()
    ignoredAbsPaths.add(Path(thirdPartyPath).as_posix())
    searcher.search('', f'^.*\.(h|hpp|c|cpp|inl)$', ignoredAbsPaths)
