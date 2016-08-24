#
# The MIT License (MIT)
#
# Copyright (c) 2014-2016 Gil Mendes <gil00mendes@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Build flags for both host and target.
build_flags = {
    'CCFLAGS': [
        '-Wall', '-Wextra', '-Wno-variadic-macros', '-Wno-unused-parameter',
        '-Wwrite-strings', '-Wmissing-declarations', '-Wredundant-decls',
        '-Wno-format', '-Werror', '-Wno-error=unused', '-pipe',
    ],
    'CFLAGS': ['-std=gnu99'],
    'ASFLAGS': ['-D__ASM__'],
}

# GCC-specific build flags.
gcc_flags = {
    'CCFLAGS': ['-Wno-unused-but-set-variable'],
}

# Clang-specific build flags.
clang_flags = {
    # Clang's integrated assembler doesn't support 16-bit code.
    'ASFLAGS': ['-no-integrated-as'],
}

# Build flags for target.
target_flags = {
    'CCFLAGS': [
        '-gdwarf-2', '-pipe', '-nostdlib', '-nostdinc', '-ffreestanding',
        '-fno-stack-protector', '-Os', '-fno-omit-frame-pointer',
        '-fno-optimize-sibling-calls'
    ],
    'ASFLAGS': ['-nostdinc'],
    'LINKFLAGS': ['-nostdlib', '-Wl,--build-id=none'],
}

###############
# Build setup #
###############

import os, sys, SCons.Errors
from subprocess import Popen, PIPE

sys.path = [os.path.abspath(os.path.join('utilities', 'build'))] + sys.path
from kconfig import ConfigParser
import util

# Create the configuration parser
config = ConfigParser('.config')
Export('config')

# Create the build environment.
env = Environment(ENV = os.environ)

# Make the output nice.
verbose = ARGUMENTS.get('V') == '1'
if not verbose:
    def compile_str(msg):
        return ' \033[0;34m%-6s\033[0m $TARGET' % (msg)
    env['ARCOMSTR']     = compile_str('AR')
    env['ASCOMSTR']     = compile_str('ASM')
    env['ASPPCOMSTR']   = compile_str('ASM')
    env['CCCOMSTR']     = compile_str('CC')
    env['CXXCOMSTR']    = compile_str('CXX')
    env['LINKCOMSTR']   = compile_str('LINK')
    env['RANLIBCOMSTR'] = compile_str('RANLIB')
    env['GENCOMSTR']    = compile_str('GEN')
    env['STRIPCOMSTR']  = compile_str('STRIP')

# Merge in build flags.
for (k, v) in build_flags.items():
    env[k] = v

# Add a builder to preprocess linker scripts.
env['BUILDERS']['LDScript'] = Builder(action = Action(
    '$CC $_CCCOMCOM $ASFLAGS -E -x c $SOURCE | grep -v "^\#" > $TARGET',
    '$GENCOMSTR'))

################################
# Host build environment setup #
################################

# Set up the host build environment.
host_env = env.Clone()

# Add compiler-specific flags.
output = Popen([host_env['CC'], '--version'], stdout=PIPE, stderr=PIPE).communicate()[0].strip()
host_env['IS_CLANG'] = output.find('clang') >= 0
if host_env['IS_CLANG']:
    for (k, v) in clang_flags.items():
        host_env[k] += v
else:
    for (k, v) in gcc_flags.items():
        host_env[k] += v

# Build host system utilities.
SConscript('utilities/SConscript',
    variant_dir = os.path.join('build', 'host'),
    exports = {'env': host_env})

# Add target to run the configuration interface.
Alias('config', host_env.ConfigMenu('__config', ['Kconfig']))

# Only do the rest of the build if the configuration exists.
if not config.configured() or 'config' in COMMAND_LINE_TARGETS:
    if GetOption('help') or 'config' in COMMAND_LINE_TARGETS:
        Return()
    else:
        raise SCons.Errors.StopError(
            "Configuration missing or out date. Please update using 'config' target.")

##################################
# Target build environment setup #
##################################

# Detect which compiler to use.
compilers = ['cc', 'gcc', 'clang']
compiler = None
for name in compilers:
    path = config['CROSS_COMPILER'] + name
    if util.which(path):
        compiler = path
        break
if not compiler:
    util.StopError('Toolchain has no usable compiler available.')

# Set paths to the various build utilities. The stuff below is to support use
# of clang's static analyzer.
if os.environ.has_key('CC') and os.path.basename(os.environ['CC']) == 'ccc-analyzer':
    env['CC'] = os.environ['CC']
    env['ENV']['CCC_CC'] = compiler
else:
    env['CC'] = compiler
env['AS']      = config['CROSS_COMPILER'] + 'as'
env['OBJDUMP'] = config['CROSS_COMPILER'] + 'objdump'
env['READELF'] = config['CROSS_COMPILER'] + 'readelf'
env['NM']      = config['CROSS_COMPILER'] + 'nm'
env['STRIP']   = config['CROSS_COMPILER'] + 'strip'
env['AR']      = config['CROSS_COMPILER'] + 'ar'
env['RANLIB']  = config['CROSS_COMPILER'] + 'ranlib'
env['OBJCOPY'] = config['CROSS_COMPILER'] + 'objcopy'
env['LD']      = config['CROSS_COMPILER'] + 'ld'

# Override default assembler - it uses as directly, we want to use GCC.
env['ASCOM'] = '$CC $_CCCOMCOM $ASFLAGS -c -o $TARGET $SOURCES'

# Merge in build flags.
for (k, v) in target_flags.items():
    env[k] += v

# Add compiler-specific flags.
output = Popen([compiler, '--version'], stdout=PIPE, stderr=PIPE).communicate()[0].strip()
env['IS_CLANG'] = output.find('clang') >= 0
if env['IS_CLANG']:
    for (k, v) in clang_flags.items():
        env[k] += v
else:
    for (k, v) in gcc_flags.items():
        env[k] += v

# Add the compiler include directory for some standard headers.
incdir = Popen([env['CC'], '-print-file-name=include'], stdout=PIPE).communicate()[0].strip()
env['CCFLAGS'] += ['-isystem', incdir]
env['ASFLAGS'] += ['-isystem', incdir]

# Change the Decider to MD5-timestamp to speed up the build a bit.
Decider('MD5-timestamp')

# We place the final output binaries in a single directory.
env['OUTDIR'] = Dir('build/%s-%s/bin' % (config['ARCH'], config['PLATFORM']))

# Don't use the Default function within the sub-SConscripts for compatibility
# with the main InfinityOS build system.
defaults = []

SConscript('SConscript',
    variant_dir = os.path.join('build', '%s-%s' % (config['ARCH'], config['PLATFORM'])),
    exports = {'env': env, 'dirs': ['source'], 'defaults': defaults})

Default(defaults)

################
# Test targets #
################

SConscript('test/SConscript',
    variant_dir = os.path.join('build', '%s-%s' % (config['ARCH'], config['PLATFORM']), 'test'),
    exports = ['config', 'defaults', 'env'])

# get QEMU script to run
qemu = ARGUMENTS.get('QEMU', '')
if len(qemu):
    qemu = '%s-%s.sh' % (config['PLATFORM'], qemu)
else:
    qemu = '%s.sh' % config['PLATFORM']

# Add a target to run the test script for this configuration (if it exists).
script = os.path.join('test', 'qemu', qemu)
if os.path.exists(script):
    Alias('qemu', env.Command('__qemu', defaults + ['test'], Action(script, None)))
