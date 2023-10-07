import sys
import os
from pathlib import Path

buildToolBasePath = os.path.relpath(os.path.dirname(sys.argv[0]), os.curdir)
sys.path.append(os.path.join(buildToolBasePath, 'Core'))
sys.path.append(os.path.join(buildToolBasePath, 'Generators'))

from Core.Foundation import *
from Core.BuildObject import Target, Module

from Generators.VisualStudio.VisualStudioProjectGenerator import VisualStudioProjectGenerator
from Core.ProjectGenerator import ProjectGenerator

class Solution:
    def __init__(self, rootPath):
        self.moduleCollector : BuildObjectCollector[Module] = BuildObjectCollector()
        self.targetCollector : BuildObjectCollector[Target] = BuildObjectCollector()
        self.rootPath = Path(os.path.normpath(rootPath)).absolute().as_posix()
        os.chdir(self.rootPath)
        Log.log(f'Set working folder to "{self.rootPath}"')
        
    def search(self):
        for cur, _, _ in os.walk(self.rootPath):
            if os.path.basename(cur).startswith('__'):
                continue

            entry = BuildEntry(cur)

            module = entry.mount(Module)
            if module is not None:
                if self.moduleCollector.exist(module.name):
                    Log.err('Module name({}) is collided'.format(module.name))
                    continue
                self.moduleCollector.add(module.name, module)
                Log.log("Mount Module '{}' Successed".format(module.name))
            
            target = entry.mount(Target)
            if target is not None:
                if self.targetCollector.exist(target.name):
                    Log.err('Target name({}) is collided'.format(target.name))
                    continue
                self.targetCollector.add(target.name, target)
                Log.log("Mount Target '{}' Successed".format(target.name))
    
    def analyze(self):
        for target in self.targetCollector:
            target.resolveModules(self.moduleCollector)

        pendingTargets : list[Target] = [target for target in self.targetCollector]
        finishedTargets : set[Target] = set()
        workingTargets : list[Target] = []
        while len(pendingTargets) > 0:
            target = pendingTargets[-1]

            if len(workingTargets) > 0 and target == workingTargets[-1]:
                pendingTargets.pop()
                workingTargets.pop()
                finishedTargets.add(target)
                Log.log(f'Target "{target.name}" resolved')
                continue

            if target in finishedTargets:
                pendingTargets.pop()
                continue
            
            if target in workingTargets:
                errStr = f'Circular dependency is found on target "{target.name}"'
                Log.err(errStr)
                raise RecursionError(errStr)
            
            if len(target.targetDependencies) == 0:
                pendingTargets.pop()
                finishedTargets.add(target)
                Log.log(f'Target "{target.name}" resolved')
                continue
            else:
                for targetName in target.targetDependencies:
                    dependedTarget = self.targetCollector.get(targetName)
                    if dependedTarget is None:
                        errStr = f'Depended Target "{targetName}" not found in Target "{target.name}"'
                        Log.err(errStr)
                        raise LookupError(errStr)
                    else:
                        pendingTargets.append(dependedTarget)
                workingTargets.append(target)
                Log.log(f'Resolving dependencies of Target "{target.name}"')
    
    def setup(self):
        for target in self.targetCollector:
            target.setupModules(self.moduleCollector, self.targetCollector)
    
    def generateProject(self, generatorName : str, solutionName : str):
        generator = ProjectGenerator.Get(generatorName)
        generator.generate(solutionName, self.targetCollector)

if __name__ == '__main__':
    argc = len(sys.argv)
    if argc != 3 and argc != 4:
        print(f'{sys.argv[0]} IDE_NAME WORKING_PATH [SOLUTION_NAME]')
        print(f'    IDE_NAME: {", ".join(ProjectGenerator.Names())}')
        exit(-1)
    ideName = sys.argv[1]
    workingPath = sys.argv[2]
    if not os.path.exists(workingPath) or not os.path.isdir(workingPath):
        print(f'{workingPath} is not a valid folder path')
        exit(-1)
    workingPath = Path(workingPath).absolute().as_posix()
    solutionName = sys.argv[3] if argc == 4 else os.path.basename(workingPath)
    solution = Solution(workingPath)
    Log.create()
    solution.search()
    solution.analyze()
    solution.setup()
    solution.generateProject(ideName, solutionName)
