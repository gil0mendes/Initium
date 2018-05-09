import os
import shutil

def msg(msg):
    """ Print out a message to the console """
    print('\033[0;32m>>>\033[0;1m %s\033[0m' % (msg))

class ToolchainComponent:
    def __init__(self, manager):
        self.manager = manager

class BinutilsComponent(ToolchainComponent):
    name = 'binutils'

class ToolchainManager:
    def __init__(self, configs):
        self.destdir = configs['TOOLCHAIN_DIR']

        self.generic_dir = os.path.join(self.destdir, 'generic')
        self.build_dir = os.path.join(self.destdir, 'build-tmp')

        self.totaltime = 0
        self.components = [
            BinutilsComponent(self)
        ]

    def check(self):
        """
        Check if an update is required
        """
        for component in self.components:
            if component.check():
                return True

        shutil.rmtree(self.build_dir)
        return False

    def update(self, target, source, env):
        """
        Rebuilds the toolchain if required
        """
        if not self.check():
            msg('Toolchain already up-to-date, nothing to be done')
            return 0
        
        return 0
