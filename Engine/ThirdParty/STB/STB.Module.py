from Core.BuildObject import Module
from Core.Foundation import BuildParam

class STBModule(Module):
    def __init__(self, param : BuildParam):
        Module.__init__(self, param)
        self.fileSearcher.addPath('.', True)
