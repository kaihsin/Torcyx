# Torcyx
    Torcyx is a extension of Cytnx Tensor network library with ML capability base on pytorch. 


# Simple demo:

* C++ API:
```c++
    #include "torcyx.hpp"
    #include <torch/torch.h>
    #include <iostream>

    using namespace std;
    int main(int argc, char* argv[]){
       auto A = torch::ones({3,4});
       auto cA = torcyx::CyTensor(A,1);

       cA.print_diagram();
       cout << cA << endl;

    } 
```

* Python API:
```python
    import torcyx
    import torch
    
    A = torch.ones([3,4])
    cA = torcyx.CyTensor(A,1)
    cA.print_diagram()
    print(cA)
```

* Output:

```text
    --------CyTensor-------
    tensor Name : 
    tensor Rank : 2
    block_form  : false
    is_diag     : False
    TensorOptions(dtype=float, device=cpu, layout=Strided, requires_grad=false (default), pinned_memory=false (default), memory_format=(nullopt))
                -------------      
               /             \     
         0 ____| 3         4 |____ 1  
               \             /     
                -------------      
    Tensor name: 
    braket_form : False
    is_diag    : False
     1  1  1  1
     1  1  1  1
     1  1  1  1
    [ CPUFloatType{3,4} ]
```



# For Contributors:

    To build Torcyx, there are two dependency:
        
    * 1. pytorch
    * 2. cytnx 

    
    Step:
    
    1) install pytorch and cytnx
    2) change the -DCYTNX_PATH to the path of installed cytnx directory. 
    3) create a build folder:
        $mkdir build
        $cd build
        
    4) copy compile.sh into build folder
    5) execute compile.sh, and make
        $sh compile.sh
        $make 

# Developer:

    Kai-Hsin Wu: kaihsinwu@gmail.com
    
        



