"""
    Basic wrapper script to invoke executables based on the platform
    These applications have been compiled using cmake and opencv libraries

"""

import os
import sys

#print( sys.platform,os.name)

if(sys.platform == "win32"):
    os.system("RedShiftBlurWin.exe")
elif(sys.platform == "linux"):
    os.system("./RedShiftBlurLin.exe")
