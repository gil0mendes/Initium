import os
import shutil
from subprocess import Popen, PIPE
from time import time
from urllib.parse import urlparse

def msg(msg):
    """ Print out a message to the console """
    print('\033[0;32m>>>\033[0;1m %s\033[0m' % (msg))

def makedirs(path):
    try:
        os.makedirs(path)
    except:
        pass

def remove(path):
    if not os.path.lexists(path):
        return

    # Handle symbolic links first as isfile() and isdir() follow links.
    if os.path.islink(path) or os.path.isfile(path):
        os.remove(path)
    elif os.path.isdir(path):
        shutil.rmtree(path)
    else:
        raise Exception('Unhandled type during remove (%s)' % (path))
class ToolchainComponent:
    def __init__(self, manager):
        self.manager = manager
        self.destdir = manager.genericdir

    def check(self):
        """ Check if the component requires updating. """
        path = os.path.join(self.destdir, f'.{self.name}-{self.version}-installed')
        if not os.path.exists(path):
            return True

        return False

    def download(self):
        """ Download and unpack all sources for the component. """
        for url in self.source:
            name = urlparse(url).path.split('/')[-1]
            target = os.path.join(self.manager.destdir, name)
            if not os.path.exists(target):
                msg(f' Downloading source file: {name}')

                # Download to .part and then rename when complete so we can
                # easily do continuing of downloads.
                self.execute(f'wget -c -O {target}.part {url}')
                os.rename(target + '.part', target)

            # Unpack if this is a tarball.
            if name[-8:] == '.tar.bz2':
                self.execute(f'tar -C {self.manager.builddir} -xjf {target}')
            elif name[-7:] == '.tar.gz':
                self.execute(f'tar -C {self.manager.builddir} -xzf {target}')

    def execute(self, cmd, directory = '.', expected = 0):
        """
        Helper function to execute a command and throw an exception is
        required status not returned.
        """
        print(f'+ {cmd}')
        old_cwd = os.getcwd()
        os.chdir(directory)
        if os.system(cmd) != expected:
            os.chdir(old_cwd)
            raise Exception('Command did not return expected value')
        os.chdir(old_cwd)

    def _build(self):
        """
        Perform all required tasks to update this component.
        """
        msg(f"Building toolchain component '{self.name}'")
        self.download()

        # Measure time taken to build.
        start = time()
        self.build()
        end = time()
        self.manager.totaltime += (end - start)

        # Signal that we've updated this
        file = open(os.path.join(self.destdir, f'.{self.name}-{self.version}-installed'), 'w')
        file.write('')
        file.close()

class BinutilsComponent(ToolchainComponent):
    name = 'binutils'
    version = '2.28.1'
    generic = False
    source = [
        f'https://ftp.gnu.org/gnu/binutils/binutils-{version}.tar.bz2'
    ]

    def build(self):
        # Work out configure options to use.
        confopts  = f'--prefix={self.destdir} '
        confopts += f'--target={self.manager.target} '
        confopts += '--disable-werror '
        # This disables internationalization as i18n is not needed for the cross-compile tools
        confopts += '--disable-nls '
        # This adds 64 bit support to Binutils
        confopts += '--enable-64-bit-bfd '
        confopts += '--enable-targets=i386-efi-pe,x86_64-efi-pe'

        # Build and install it
        os.mkdir('binutils-build')
        self.execute(f'../binutils-{self.version}/configure {confopts}', 'binutils-build')
        self.execute(f'make -j{self.manager.makejobs}', 'binutils-build')
        self.execute('make install', 'binutils-build')

class ToolchainManager:
    def __init__(self, config):
        self.arch = config['ARCH']
        self.platform = config['PLATFORM']
        self.destdir = config['TOOLCHAIN_DIR']
        self.target = config['TOOLCHAIN_TARGET']
        self.makejobs = config['TOOLCHAIN_MAKE_JOBS']

        self.genericdir = os.path.join(self.destdir, 'generic')
        self.targetdir = os.path.join(self.destdir, self.target)
        self.builddir = os.path.join(self.destdir, 'build-tmp')

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

        remove(self.builddir)
        return False

    def build_component(self, component):
        """
        Build a component.
        """
        # Create the target directory and change into it.
        makedirs(self.builddir)
        olddir = os.getcwd()
        os.chdir(self.builddir)

        # Perform the actual build.
        try:
            component._build()
        finally:
            # Change to the old directory and clean up the build directory.
            os.chdir(olddir)
            remove(self.builddir)

    def update(self):
        """
        Rebuilds the toolchain if required
        """
        if not self.check():
            msg('Toolchain already up-to-date, nothing to be done')
            return 0

        # Create destinaltion directory.
        makedirs(self.genericdir)
        makedirs(self.targetdir)
        makedirs(os.path.join(self.targetdir, 'bin'))

        # Build necessary components.
        try:
            for c in self.components:
                if c.check():
                    self.build_component(c)
        except Exception as e:
            msg(f'Exception during toolchain build: \033[0;0m{str(e)}')
            raise e
            return 1

        remove(self.builddir)

        msg('Toolchain updated in %d seconds' % self.totaltime)
        return 0
