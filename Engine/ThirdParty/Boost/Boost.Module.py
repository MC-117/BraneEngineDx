import glob
import os
from Core.BuildObject import Module, TargetConfig
from Core.Foundation import Log, BuildParam

class BoostModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.includePaths = [f'{self.getPath()}']
