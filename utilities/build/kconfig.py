#
# Copyright (C) 2012-2013 Gil Mendes <gil00mendes@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

# Class to parse a Kconfig configuration file.
class ConfigParser(dict):
	def __init__(self, path):
		dict.__init__(self)

		# Parse the configuration file. If it doesn't exist, just
		# return - the dictionary will be empty so configured() will
		# return false.
		try:
			f = open(path, 'r')
		except IOError:
			return

		# Read and parse the file contents. We return without adding
		# any values if there is a parse error, this will cause
		# configured() to return false and require the user to reconfig.
		lines = f.readlines()
		f.close()
		values = {}
		for line in lines:
			line = line.strip()

			# Ignore blank lines or comments.
			if not len(line) or line[0] == '#':
				continue

			# Split the line into key/value.
			line = line.split('=', 1)
			if len(line) != 2:
				return
			key = line[0].strip()
			value = line[1].strip()
			if len(key) < 8 or key[0:7] != 'CONFIG_' or not len(value):
				return
			key = line[0].strip()[7:]

			# Work out the correct value.
			if value == 'y':
				value = True
			elif value[0] == '"' and value[-1] == '"':
				value = value[1:-1]
			elif value[0:2] == '0x' and len(value) > 2:
				value = int(value, 16)
			elif value.isdigit():
				value = int(value)
			else:
				print "Unrecognised value type: %s" % (value)
				return

			# Add it to the dictionary.
			values[key] = value

		# Everything was OK, add stuff into the real dictionary.
		for (k, v) in values.items():
			self[k] = v

	# Get a configuration value. This returns None for any accesses to
	# undefined keys.
	def __getitem__(self, key):
		try:
			return dict.__getitem__(self, key)
		except KeyError:
			return None

	# Check whether the build configuration exists.
	def configured(self):
		return len(self) > 0
