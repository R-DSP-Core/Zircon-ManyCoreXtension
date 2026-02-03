#!/bin/bash
# Temporary python3-config wrapper script
# This script provides python3-config functionality using Python's sysconfig module

python3 -c "
import sysconfig
import sys

arg = sys.argv[1] if len(sys.argv) > 1 else ''

if arg == '--prefix':
    print(sysconfig.get_config_var('prefix'))
elif arg == '--includes':
    print('-I' + sysconfig.get_path('include'))
elif arg == '--libs':
    print('-lpython' + sysconfig.get_config_var('LDVERSION'))
elif arg == '--ldflags':
    libdir = sysconfig.get_config_var('LIBDIR')
    ldversion = sysconfig.get_config_var('LDVERSION')
    print('-L' + libdir + ' -lpython' + ldversion)
elif arg == '--cflags':
    print('-I' + sysconfig.get_path('include'))
elif arg == '--version':
    import subprocess
    result = subprocess.run(['python3', '--version'], capture_output=True, text=True)
    print(result.stdout.strip().split()[1])
else:
    sys.exit(1)
" "$@"



