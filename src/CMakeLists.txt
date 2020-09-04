
target_sources_local(torcyx
  PRIVATE
    CyTensor_base.cpp
    CyTensor.cpp
    DenseCyTensor.cpp
    SparseCyTensor.cpp
    TypeConvert.cpp
)
add_subdirectory(xlinalg)
