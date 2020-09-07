#include "torcyx.hpp"
#include <torch/torch.h>
#include <iostream>

using namespace std;
int main(int argc, char* argv[]){
   auto A = torch::ones({3,4});
   auto cA = torcyx::CyTensor(A,1);
   auto cB = torcyx::CyTensor({torcyx::Bond(2),torcyx::Bond(2)},{},1);
   
   //cA.print_diagram();
   //cout << cA << endl;
    return 0;
}
