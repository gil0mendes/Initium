import os
import shutil
from subprocess import Popen, PIPE
from time import time
from urllib.parse import urlparse

def msg(msg):
    """ Print out a message to the console """
    print('\033[0;32m>>>\033[0;1m %s\033[0m' % (msg))

class ToolchainComponent:
    def __init__(self, manager):
        self.manager = manager
        self.destdir = manager.generic_dir

    def check(self):
        """ Check if the component requires updating. """
        path = os.path.join(self.destdir, f'.{self.name}-{self.version}-installed')
        if not os.path.exists(path):
            return True

        return False

    def download(self):
        """ Download and unpack all sources for the component. """
        for url in self.sources:
            name = urlparse(url).path.split('/')[-1]
            target = os.path(self.manager.destdir, name)
            if not os.path.exists(target):
                msg(f' Downloading source file: {name}')

                # Download to .part and then rename when complete so we can
                # easily do continuing of downloads.
                self.execute(f'wget -c -O {target}.part {url}')
                os.rename(target + '.part', target)

            # Unpack if this is a tarball.
            if name[-8:] == '.tar.bz2':
                self.execute(f'tar -C {self.manager.build_dir} -xjf {target}')
            elif name[-7:] == '.tar.gz':
                self.execute(f'tar -C {self.manager.build_dir} -xzf {target}')

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
