import torcyx
import torch

A = torch.zeros(3,4,5)
B = torcyx.CyTensor(A,1)

C = B.permute([0,2,1])
print(B)
print(C)
print(B.is_contiguous())
print(C.is_contiguous())

#B.permute_([0,2,1])
#print(B)

