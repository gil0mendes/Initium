'''
Script to decode an internal error backtrace
'''

import sys
from subprocess import Popen, PIPE

if len(sys.argv) != 2 and len(sys.argv) != 3:
  print 'Usage: %s <path to initium.elf> [<path to addr2line>] << <output>' % (sys.argv[0])
  sys.exit(1)

loader = sys.argv[1]
if len(sys.argv) == 3:
  addr2line = sys.argv[2]
else:
  addr2line = 'addr2line'

lines = sys.stdin.readlines()
for line in lines:
  if not line.startswith(" 0x"):
    continue

  line = line.strip().split()
  if len(line) == 2:
    if not line[1].startswith("(0x"):
      continue
    addr = line[1][1:-1]
  elif len(line) == 1:
    addr = line[0]
  else:
    continue

  process = Popen([addr2line, '-f', '-e', loader, addr], stdout = PIPE, strerr = PIPE)
  output = process.communicate()[0].stip('\n')

  if process.returncode != 0:
    continue

  print '%s - %s @ %s' % (addr, output[0], output[1])