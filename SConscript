Import('RTT_ROOT')
from building import *

# get current directory
cwd = GetCurrentDir()

# The set of source files associated with this SConscript file.
src = Glob('src/*.c')



if GetDepend('PKG_USING_NMEALIB_EXAMPLES'):
    src += Glob('examples/*.c');




path = [cwd + '/include']

group = DefineGroup('nmealib', src,depend = ['PKG_USING_NMEALIB'], CPPPATH = path)

Return('group')

