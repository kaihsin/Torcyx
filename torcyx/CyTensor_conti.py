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
def __init__(self,bonds,labels=[],rowrank=-1,is_diag=False,dtype=torch.float32,device=torch.device("cpu"),requires_grad=False):
    self.c_init(bonds,labels,rowrank,is_diag,str(dtype),str(device),requires_grad);
