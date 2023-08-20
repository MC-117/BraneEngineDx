from BuildObject import Target
from Core.Foundation import BuildObjectCollector
from Core.BuildObject import *

class ProjectGenerator:
    pass

class ProjectGenerator:
    __Generators = {}
    def __init__(self):
        pass

    def __init_subclass__(cls, name : str):
        if name in ProjectGenerator.__Generators:
            raise NameError('Duplicated ProjectGenerator Name')
        ProjectGenerator.__Generators[name] = cls
    
    @classmethod
    def Get(cls, name : str) -> ProjectGenerator:
        if name not in cls.__Generators:
            raise NameError(f'ProjectGenerator "{name}" not found')
        return cls.__Generators[name]()
    
    @classmethod
    def Names(cls) -> Iterator[str]:
        return cls.__Generators.keys().__iter__()
    
    def generate(self, name : str, targets : BuildObjectCollector[Target]):
        raise NotImplementedError(self)