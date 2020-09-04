#ifndef _TYPECONVERT_H_
#define _TYPECONVERT_H_


// cytnx:
#include "Type.hpp"
#include "Device.hpp"
#include "cytnx.hpp"

// pytorch:
#include <torch/torch.h>

namespace torcyx{
    
    using cytnx::cytnx_double;
    using cytnx::cytnx_float;
    using cytnx::cytnx_uint64;
    using cytnx::cytnx_uint32;
    using cytnx::cytnx_uint16;
    using cytnx::cytnx_int64;
    using cytnx::cytnx_int32;
    using cytnx::cytnx_int16;
    using cytnx::cytnx_size_t;
    using cytnx::cytnx_complex64;
    using cytnx::cytnx_complex128;
    using cytnx::cytnx_bool;

    using cytnx::Type;
    using cytnx::Device;

    /// @cond
        //typedef torch::TensorOptions (*Tor2Cy_io)(const unsigned int &dtype, const unsigned int &device);

        class TypeCvrt_class{
            public:

                //Cast
                //std::vector<Tor2Cy_io> _t2c;            
                TypeCvrt_class();
                torch::TensorOptions Cy2Tor(const unsigned int &dtype, const int &device);
                unsigned int Tor2Cy(const torch::ScalarType &scalar_type);

        };
        extern TypeCvrt_class type_converter;
    /// @endcond

}
namespace torcyx{
    using cytnx::Symmetry;
    using cytnx::Bond;
    using cytnx::bondType;
    using cytnx::Accessor; 
}


#endif
