from cytnx.utils import *
from torcyx import *
import torch
## load the submodule from pybind and inject the methods


"""
@add_method(Storage)
def astype(self, new_type):
    if(self.dtype() == new_type):
        return self

    else:
        return self.astype_different_type(new_type)
"""


@add_method(CyTensor)
def dtype(self):
    if self.is_blockform():
        return self.get_blocks_()[0].dtype
    else:
        return self.get_block_().dtype

@add_method(CyTensor)
def device(self):
    if self.is_blockform():
        return self.get_blocks_()[0].device
    else:
        return self.get_block_().device

@add_method(CyTensor)
def to(self,device,non_blocking=False):
    if(self.device()== device):
        return self
    else:
        return self.c_to(device,non_blocking)


@add_method(CyTensor)
def to_(self,device,non_blocking=False):
   self.c_to_(device,non_blocking)

