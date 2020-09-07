import os,sys
import numpy
import torch
if os.path.exists(os.path.join(os.path.dirname(__file__),"cytnxpath.tmp")):
    f = open(os.path.join(os.path.dirname(__file__),"cytnxpath.tmp"),'r')
    cytnx_path = f.readline()
    f.close()
    sys.path.append(os.path.expanduser(cytnx_path))

from cytnx import Bond, Symmetry
from .torcyx import *
from .CyTensor_conti import *

#__version__ = cytnx.__version__
#if(os.path.exists(os.path.join(os.path.dirname(__file__),"include"))):
#    # this only set if using anaconda install. 
#    __cpp_include__=os.path.join(os.path.dirname(__file__),"include")
#    __cpp_lib__=os.path.join(os.path.dirname(__file__),"lib")
#else:
#    __cpp_include__=os.path.join(os.path.dirname(os.path.dirname(__file__)),"include")
#    __cpp_lib__=os.path.join(os.path.dirname(os.path.dirname(__file__)),"lib")
#
#__blasINTsize__ = cytnx.__blasINTsize__

#def from_numpy(np_arr):
#    tmp = np_arr
#    if np_arr.flags['C_CONTIGUOUS'] == False:
#        tmp = numpy.ascontiguousarray(np_arr)
#    return cytnx._from_numpy(tmp)

#def _resolve_cpp_linkflags__():
#    f = open(os.path.join(os.path.dirname(os.path.abspath(__file__)),"linkflags.tmp"))
#    lines = f.readlines()
#    out = ""
#    for line in lines:
#        line = line.strip()
#       out+=line.replace(";"," ")
#        out+=" "
#    f.close()
#    return out


#__cpp_linkflags__ = _resolve_cpp_linkflags__()


