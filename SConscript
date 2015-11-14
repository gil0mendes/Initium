#
# The MIT License (MIT)
#
# Copyright (c) 2015 Gil Mendes <gil00mendes@gmail.com>
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

Import('config', 'dirs', 'env', 'defaults')

# Generate the configuration header. We don't generate with Kconfig because its
# too much of a pain to get SCons to do it properly.
f = open('config.h', 'w')
f.write('/* This file is automatically-generated, do not edit. */\n\n')
for (k, v) in config.items():
        if isinstance(v, str):
                f.write("#define CONFIG_%s \"%s\"\n" % (k, v))
        elif isinstance(v, bool) or isinstance(v, int):
                f.write("#define CONFIG_%s %d\n" % (k, int(v)))
        else:
                raise Exception, "Unsupported type %s in config" % (type(v))
f.close()

SConscript(dirs = dirs, exports = ['env', 'defaults'])