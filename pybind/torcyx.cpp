#include <vector>
#include <map>
#include <random>
  
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <pybind11/buffer_info.h>
#include <pybind11/functional.h>

#include "torcyx.hpp"
#include "complex.h"
#include <torch/extension.h> //v1.6 need to link explicitly against libtorch_python.so for this to work. 

using namespace torcyx;
namespace py = pybind11;
using namespace pybind11::literals;

//ref: https://developer.lsst.io/v/DM-9089/coding/python_wrappers_for_cpp_with_pybind11.html
//ref: https://pybind11.readthedocs.io/en/stable/advanced/classes.html
//ref: https://block.arch.ethz.ch/blog/2016/07/adding-methods-to-python-classes/
//ref: https://medium.com/@mgarod/dynamically-add-a-method-to-a-class-in-python-c49204b85bd6
//ref: https://github.com/pybind/pybind11/issues/1201


PYBIND11_MODULE(torcyx,m){

    //m.attr("__version__") = "0.5.5a";
    //m.attr("__blasINTsize__") = cytnx::__blasINTsize__;
    
    py::add_ostream_redirect(m, "ostream_redirect");    

    
    py::class_<CyTensor>(m,"CyTensor") 
                .def(py::init<>())
                .def(py::init<const torch::Tensor&, const cytnx_uint64&, const bool &>(),py::arg("Tin")
                     ,py::arg("rowrank"),py::arg("is_diag")=false)

                .def("__init__",[](CyTensor &self,const std::vector<Bond> &bonds, const std::vector<cytnx_int64> & labels, const cytnx_int64 &rowrank, const bool &is_diag, const py::object &dtype, const py::object &device, const bool &requires_grad){
                
                    auto options  = torch::TensorOptions().requires_grad(requires_grad);
                    if(dtype != py::none())
                        options.dtype(torch::python::detail::py_object_to_dtype(dtype));
                    
                    if(device != py::none())
                        options.device(torch::python::detail::py_object_to_device(device));
                    
                    //std::cout << options << std::endl;
                    self.Init(bonds,labels,rowrank,is_diag,options);

                },py::arg("bonds"),py::arg("labels")=std::vector<cytnx_int64>(),py::arg("rowrank")=(cytnx_int64)(-1),py::arg("is_diag")=false, py::arg("dtype")=py::none(),py::arg("device")=py::none(),py::arg("requres_grad")=false)

                .def("print_diagram",&CyTensor::print_diagram,py::arg("bond_info")=false,py::call_guard<py::scoped_ostream_redirect,
                     py::scoped_estream_redirect>())
                .def("__repr__",[](CyTensor &self)->std::string{
                    std::cout << self << std::endl;
                    return std::string("");
                },py::call_guard<py::scoped_ostream_redirect,py::scoped_estream_redirect>())

                .def("set_name",&CyTensor::set_name)
                .def("set_label",&CyTensor::set_label,py::arg("idx"),py::arg("new_label"))
                .def("set_labels",&CyTensor::set_labels,py::arg("new_labels"))
                .def("set_rowrank",&CyTensor::set_rowrank, py::arg("new_rowrank"))
            
                .def("rowrank",&CyTensor::rowrank)
                //.def("dtype",&CyTensor::dtype) // handle in python side (hips).
                //.def("dtype_str",&UniTensor::dtype_str) // hips
                //.def("device",&UniTensor::device) //hips
                //.def("device_str",&UniTensor::device_str) //hips
                .def("name",&CyTensor::name)
                .def("is_blockform",&CyTensor::is_blockform)


                .def("get_block", [](const CyTensor &self, const cytnx_uint64&idx){
                                        return self.get_block(idx);
                                  },py::arg("idx")=(cytnx_uint64)(0))

                .def("get_block", [](const CyTensor &self, const std::vector<cytnx_int64>&qnum){
                                        return self.get_block(qnum);
                                  },py::arg("qnum"))

                .def("get_block_",[](const CyTensor &self, const std::vector<cytnx_int64>&qnum){
                                        return self.get_block_(qnum);
                                  },py::arg("qnum"))
                .def("get_block_",[](CyTensor &self, const std::vector<cytnx_int64>&qnum){
                                        return self.get_block_(qnum);
                                  },py::arg("qnum"))
                .def("get_block_", [](const CyTensor &self, const cytnx_uint64&idx){
                                        return self.get_block_(idx);
                                  },py::arg("idx")=(cytnx_uint64)(0))
                .def("get_block_", [](CyTensor &self, const cytnx_uint64&idx){
                                        return self.get_block_(idx);
                                  },py::arg("idx")=(cytnx_uint64)(0))
                .def("get_blocks", [](const CyTensor &self){
                                        return self.get_blocks();
                                  })
                .def("get_blocks_", [](const CyTensor &self){
                                        return self.get_blocks_();
                                  })
                .def("get_blocks_", [](CyTensor &self){
                                        return self.get_blocks_();
                                  })
                .def("c_to_",[](CyTensor &self,py::object device,const bool &non_blocking){
                                    self.to_(torch::python::detail::py_object_to_device(device),non_blocking);
                                },py::arg("device"),py::arg("non_blocking")=false)
                .def("c_to" ,[](CyTensor &self,py::object device, const bool &non_blocking){
                                    //cytnx_error_msg(self.device() == device, "[ERROR][pybind][to_diffferent_device] same device for to() should be handle in python side.%s","\n");
                                    return self.to(torch::python::detail::py_object_to_device(device),non_blocking);
                                     }, py::arg("device"),py::arg("non_blocking")=false)
                /*
                .def("reshape",[](UniTensor &self, py::args args, py::kwargs kwargs)->UniTensor{
                    std::vector<cytnx::cytnx_int64> c_args = args.cast< std::vector<cytnx::cytnx_int64> >();
                    cytnx_uint64 rowrank = 0;
                   
                    if(kwargs){
                        if(kwargs.contains("rowrank")) rowrank = kwargs["rowrank"].cast<cytnx::cytnx_int64>();
                    }
 
                    return self.reshape(c_args,rowrank);
                })
                .def("reshape_",[](UniTensor &self, py::args args, py::kwargs kwargs){
                    std::vector<cytnx::cytnx_int64> c_args = args.cast< std::vector<cytnx::cytnx_int64> >();
                    cytnx_uint64 rowrank = 0;
                   
                    if(kwargs){
                        if(kwargs.contains("rowrank")) rowrank = kwargs["rowrank"].cast<cytnx::cytnx_int64>();
                    }
 
                    self.reshape_(c_args,rowrank);
                })
                .def("elem_exists",&UniTensor::elem_exists)
                .def("item",[](UniTensor &self){
                    py::object out;
                    if(self.dtype() == cytnx::Type.Double) 
                        out =  py::cast(self.item<cytnx::cytnx_double>());
                    else if(self.dtype() == cytnx::Type.Float) 
                        out = py::cast(self.item<cytnx::cytnx_float>());
                    else if(self.dtype() == cytnx::Type.ComplexDouble) 
                        out = py::cast(self.item<cytnx::cytnx_complex128>());
                    else if(self.dtype() == cytnx::Type.ComplexFloat) 
                        out = py::cast(self.item<cytnx::cytnx_complex64>());
                    else if(self.dtype() == cytnx::Type.Uint64) 
                        out = py::cast(self.item<cytnx::cytnx_uint64>());
                    else if(self.dtype() == cytnx::Type.Int64) 
                        out = py::cast(self.item<cytnx::cytnx_int64>());
                    else if(self.dtype() == cytnx::Type.Uint32) 
                        out = py::cast(self.item<cytnx::cytnx_uint32>());
                    else if(self.dtype() == cytnx::Type.Int32) 
                        out = py::cast(self.item<cytnx::cytnx_int32>());
                    else if(self.dtype() == cytnx::Type.Uint16) 
                        out = py::cast(self.item<cytnx::cytnx_uint16>());
                    else if(self.dtype() == cytnx::Type.Int16) 
                        out = py::cast(self.item<cytnx::cytnx_int16>());
                    else if(self.dtype() == cytnx::Type.Bool) 
                        out = py::cast(self.item<cytnx::cytnx_bool>());
                    else cytnx_error_msg(true, "%s","[ERROR] try to get element from a empty UniTensor.");
                    return out;
                 })

                .def("__getitem__",[](const UniTensor &self, py::object locators){
                    cytnx_error_msg(self.shape().size() == 0, "[ERROR] try to getitem from a empty UniTensor%s","\n");
                    cytnx_error_msg(self.uten_type() == UTenType.Sparse,"[ERROR] cannot get element using [] from SparseUniTensor. Use at() instead.%s","\n");
 
                    ssize_t start, stop, step, slicelength; 
                    std::vector<cytnx::Accessor> accessors;
                    if(py::isinstance<py::tuple>(locators)){
                        py::tuple Args = locators.cast<py::tuple>();
                        cytnx_uint64 cnt = 0;
                        // mixing of slice and ints
                        for(cytnx_uint32 axis=0;axis<Args.size();axis++){
                                cnt ++;
                                // check type:
                                if(py::isinstance<py::slice>(Args[axis])){
                                    py::slice sls = Args[axis].cast<py::slice>();
                                    if(!sls.compute((ssize_t)self.shape()[axis],&start,&stop,&step, &slicelength))
                                        throw py::error_already_set();
                                    //std::cout << start << " " << stop << " " << step << slicelength << std::endl;
                                    //if(slicelength == self.shape()[axis]) accessors.push_back(cytnx::Accessor::all());
                                    accessors.push_back(cytnx::Accessor::range(cytnx_int64(start),cytnx_int64(stop),cytnx_int64(step)));
                                }else{
                                    accessors.push_back(cytnx::Accessor(Args[axis].cast<cytnx_int64>()));
                                }
                            
                        }
                        while(cnt<self.shape().size()){
                            cnt++;
                            accessors.push_back(Accessor::all());
                        }
                    }else if(py::isinstance<py::slice>(locators)){
                        py::slice sls = locators.cast<py::slice>();
                        if(!sls.compute((ssize_t)self.shape()[0],&start,&stop,&step, &slicelength))
                            throw py::error_already_set();
                        //if(slicelength == self.shape()[0]) accessors.push_back(cytnx::Accessor::all());
                        accessors.push_back(cytnx::Accessor::range(start,stop,step));
                        for(cytnx_uint32 axis=1;axis<self.shape().size();axis++){
                            accessors.push_back(Accessor::all());
                        }


                    }else{
                        // only int
                        for(cytnx_uint32 i=0;i<self.shape().size();i++){
                            if(i==0) accessors.push_back(cytnx::Accessor(locators.cast<cytnx_int64>()));
                            else accessors.push_back(cytnx::Accessor::all());
                        }
                    }
                    

                    return self.get(accessors);
                    
                })
                .def("__setitem__",[](UniTensor &self, py::object locators, const cytnx::Tensor &rhs){
                    cytnx_error_msg(self.shape().size() == 0, "[ERROR] try to setelem to a empty UniTensor%s","\n");
                    cytnx_error_msg(self.uten_type() == UTenType.Sparse,"[ERROR] cannot set element using [] from SparseUniTensor. Use at() instead.%s","\n");
                    
                    ssize_t start, stop, step, slicelength; 
                    std::vector<cytnx::Accessor> accessors;
                    if(py::isinstance<py::tuple>(locators)){
                        py::tuple Args = locators.cast<py::tuple>();
                        cytnx_uint64 cnt = 0;
                        // mixing of slice and ints
                        for(cytnx_uint32 axis=0;axis<Args.size();axis++){
                                cnt ++;
                                // check type:
                                if(py::isinstance<py::slice>(Args[axis])){
                                    py::slice sls = Args[axis].cast<py::slice>();
                                    if(!sls.compute((ssize_t)self.shape()[axis],&start,&stop,&step, &slicelength))
                                        throw py::error_already_set();
                                    //std::cout << start << " " << stop << " " << step << slicelength << std::endl;
                                    //if(slicelength == self.shape()[axis]) accessors.push_back(cytnx::Accessor::all());
                                    accessors.push_back(cytnx::Accessor::range(cytnx_int64(start),cytnx_int64(stop),cytnx_int64(step)));
                                }else{
                                    accessors.push_back(cytnx::Accessor(Args[axis].cast<cytnx_int64>()));
                                }
                            
                        }
                        while(cnt<self.shape().size()){
                            cnt++;
                            accessors.push_back(Accessor::all());
                        }
                    }else if(py::isinstance<py::slice>(locators)){
                        py::slice sls = locators.cast<py::slice>();
                        if(!sls.compute((ssize_t)self.shape()[0],&start,&stop,&step, &slicelength))
                            throw py::error_already_set();
                        //if(slicelength == self.shape()[0]) accessors.push_back(cytnx::Accessor::all());
                        accessors.push_back(cytnx::Accessor::range(start,stop,step));
                        for(cytnx_uint32 axis=1;axis<self.shape().size();axis++){
                            accessors.push_back(Accessor::all());
                        }


                    }else{
                        // only int
                        for(cytnx_uint32 i=0;i<self.shape().size();i++){
                            if(i==0) accessors.push_back(cytnx::Accessor(locators.cast<cytnx_int64>()));
                            else accessors.push_back(cytnx::Accessor::all());
                        }
                    }
                    

                    self.set(accessors,rhs);
                    
                })
                .def("get_elem",[](UniTensor &self, const std::vector<cytnx_uint64> &locator){
                    py::object out;
                    if(self.dtype() == cytnx::Type.Double) 
                        out = py::cast(self.get_elem<cytnx::cytnx_double>(locator));
                    else if(self.dtype() == cytnx::Type.Float) 
                        out = py::cast(self.get_elem<cytnx::cytnx_float>(locator));
                    else if(self.dtype() == cytnx::Type.ComplexDouble) 
                        out = py::cast(self.get_elem<cytnx::cytnx_complex128>(locator));
                    else if(self.dtype() == cytnx::Type.ComplexFloat) 
                        out = py::cast(self.get_elem<cytnx::cytnx_complex64>(locator));
                    else cytnx_error_msg(true, "%s","[ERROR] try to get element from a void Storage.");
                    return out;
                })
                .def("set_elem",&f_UniTensor_setelem_scal_cd)
                .def("set_elem",&f_UniTensor_setelem_scal_cf)
                .def("set_elem",&f_UniTensor_setelem_scal_d)
                .def("set_elem",&f_UniTensor_setelem_scal_f)
                .def("set_elem",&f_UniTensor_setelem_scal_u64)
                .def("set_elem",&f_UniTensor_setelem_scal_i64)
                .def("set_elem",&f_UniTensor_setelem_scal_u32)
                .def("set_elem",&f_UniTensor_setelem_scal_i32)
                .def("set_elem",&f_UniTensor_setelem_scal_u16)
                .def("set_elem",&f_UniTensor_setelem_scal_i16)
                .def("set_elem",&f_UniTensor_setelem_scal_b)


                .def("is_contiguous", &UniTensor::is_contiguous)
                .def("is_diag",&UniTensor::is_diag)
                .def("is_tag" ,&UniTensor::is_tag)
                .def("is_braket_form",&UniTensor::is_braket_form)
                .def("labels",&UniTensor::labels)
                .def("bonds",[](UniTensor &self){
                    return self.bonds();
                    })
                .def("shape",&UniTensor::shape)
                .def("clone",&UniTensor::clone)
                .def("__copy__",&UniTensor::clone)
                .def("__deepcopy__",&UniTensor::clone)
                .def("Save",[](UniTensor &self, const std::string &fname){self.Save(fname);},py::arg("fname"))
                .def_static("Load",[](const std::string &fname){return UniTensor::Load(fname);},py::arg("fname"))
                //.def("permute",&UniTensor::permute,py::arg("mapper"),py::arg("rowrank")=(cytnx_int64)-1,py::arg("by_label")=false)
                //.def("permute_",&UniTensor::permute_,py::arg("mapper"),py::arg("rowrank")=(cytnx_int64)-1,py::arg("by_label")=false)

                .def("permute_",[](UniTensor &self, const std::vector<cytnx::cytnx_int64> &c_args, py::kwargs kwargs){
                    cytnx_int64 rowrank = -1;
                    bool by_label = false;
                    if(kwargs){
                        if(kwargs.contains("rowrank")){
                            rowrank = kwargs["rowrank"].cast<cytnx_int64>();
                        }
                        if(kwargs.contains("by_label")){ 
                            by_label = kwargs["by_label"].cast<bool>();
                        }
                    }
                    self.permute_(c_args,rowrank,by_label);
                })
                .def("permute",[](UniTensor &self,const std::vector<cytnx::cytnx_int64> &c_args, py::kwargs kwargs)->UniTensor{
                    cytnx_int64 rowrank = -1;
                    bool by_label = false;
                    if(kwargs){
                        if(kwargs.contains("rowrank")){
                            rowrank = kwargs["rowrank"].cast<cytnx_int64>();
                        }
                        if(kwargs.contains("by_label")){ 
                            by_label = kwargs["by_label"].cast<bool>();
                        }
                    }
                    return self.permute(c_args,rowrank,by_label);
                })

                .def("make_contiguous",&UniTensor::contiguous)
                .def("contiguous_",&UniTensor::contiguous_)
                        
                .def("put_block", [](UniTensor &self, const cytnx::Tensor &in, const cytnx_uint64&idx){
                                        self.put_block(in,idx);
                                  },py::arg("in"),py::arg("idx")=(cytnx_uint64)(0))

                .def("put_block", [](UniTensor &self, const cytnx::Tensor &in, const std::vector<cytnx_int64>&qnum){
                                        self.put_block(in,qnum);
                                  },py::arg("in"),py::arg("qnum"))
                .def("put_block_", [](UniTensor &self, cytnx::Tensor &in, const cytnx_uint64&idx){
                                        self.put_block_(in,idx);
                                  },py::arg("in"),py::arg("idx")=(cytnx_uint64)(0))

                .def("put_block_", [](UniTensor &self, cytnx::Tensor &in, const std::vector<cytnx_int64>&qnum){
                                        self.put_block_(in,qnum);
                                  },py::arg("in"),py::arg("qnum"))
                .def("to_dense",&UniTensor::to_dense)
                .def("to_dense_",&UniTensor::to_dense_)
                .def("combineBonds",&UniTensor::combineBonds,py::arg("indicators"),py::arg("permute_back")=true,py::arg("by_label")=true)
                .def("contract", &UniTensor::contract)
		
        		//arithmetic >>
                .def("__neg__",[](UniTensor &self){
                                    if(self.dtype() == Type.Double){
                                        return linalg::Mul(cytnx_double(-1),self);
                                    }else if(self.dtype()==Type.ComplexDouble){
                                        return linalg::Mul(cytnx_complex128(-1,0),self);
                                    }else if(self.dtype()==Type.Float){
                                        return linalg::Mul(cytnx_float(-1),self);
                                    }else if(self.dtype()==Type.ComplexFloat){
                                        return linalg::Mul(cytnx_complex64(-1,0),self);
                                    }else{
                                        return linalg::Mul(-1,self);
                                    }
                                  })
                .def("__pos__",[](UniTensor &self){return self;})
                .def("__add__",[](UniTensor &self, const UniTensor &rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Add(rhs);})
                .def("__add__",[](UniTensor &self, const cytnx::cytnx_bool      &rhs){return self.Add(rhs);})
                
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_complex128&lhs){return linalg::Add(lhs,self);})
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_complex64 &lhs){return linalg::Add(lhs,self);})
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_double    &lhs){return linalg::Add(lhs,self);})
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_float     &lhs){return linalg::Add(lhs,self);})
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_int64     &lhs){return linalg::Add(lhs,self);})
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_uint64    &lhs){return linalg::Add(lhs,self);})
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_int32     &lhs){return linalg::Add(lhs,self);})
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_uint32    &lhs){return linalg::Add(lhs,self);})
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_int16     &lhs){return linalg::Add(lhs,self);})
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_uint16    &lhs){return linalg::Add(lhs,self);})
                .def("__radd__",[](UniTensor &self, const cytnx::cytnx_bool      &lhs){return linalg::Add(lhs,self);})
                
                .def("__iadd__",[](UniTensor &self, const UniTensor &rhs){return self.Add_(rhs);}) // these will return self!
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Add_(rhs);})
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Add_(rhs);})
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Add_(rhs);})
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Add_(rhs);})
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Add_(rhs);})
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Add_(rhs);})
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Add_(rhs);})
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Add_(rhs);})
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Add_(rhs);})
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Add_(rhs);})
                .def("__iadd__",[](UniTensor &self, const cytnx::cytnx_bool      &rhs){return self.Add_(rhs);})

                .def("__sub__",[](UniTensor &self, const UniTensor &rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Sub(rhs);})
                .def("__sub__",[](UniTensor &self, const cytnx::cytnx_bool      &rhs){return self.Sub(rhs);})

                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_complex128&lhs){return linalg::Sub(lhs,self);})
                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_complex64 &lhs){return linalg::Sub(lhs,self);})
                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_double    &lhs){return linalg::Sub(lhs,self);})
                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_float     &lhs){return linalg::Sub(lhs,self);})
                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_int64     &lhs){return linalg::Sub(lhs,self);})
                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_uint64    &lhs){return linalg::Sub(lhs,self);})
                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_int32     &lhs){return linalg::Sub(lhs,self);})
                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_uint32    &lhs){return linalg::Sub(lhs,self);})
                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_int16     &lhs){return linalg::Sub(lhs,self);})
                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_uint16    &lhs){return linalg::Sub(lhs,self);})
                .def("__rsub__",[](UniTensor &self, const cytnx::cytnx_bool      &lhs){return linalg::Sub(lhs,self);})
 
                .def("__isub__",[](UniTensor &self, const UniTensor &rhs){return self.Sub_(rhs);}) // these will return self!
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Sub_(rhs);})
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Sub_(rhs);})
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Sub_(rhs);})
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Sub_(rhs);})
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Sub_(rhs);})
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Sub_(rhs);})
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Sub_(rhs);})
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Sub_(rhs);})
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Sub_(rhs);})
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Sub_(rhs);})
                .def("__isub__",[](UniTensor &self, const cytnx::cytnx_bool      &rhs){return self.Sub_(rhs);})

                .def("__mul__",[](UniTensor &self, const UniTensor &rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Mul(rhs);})
                .def("__mul__",[](UniTensor &self, const cytnx::cytnx_bool    &rhs){return self.Mul(rhs);})

                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_complex128&lhs){return linalg::Mul(lhs,self);})
                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_complex64 &lhs){return linalg::Mul(lhs,self);})
                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_double    &lhs){return linalg::Mul(lhs,self);})
                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_float     &lhs){return linalg::Mul(lhs,self);})
                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_int64     &lhs){return linalg::Mul(lhs,self);})
                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_uint64    &lhs){return linalg::Mul(lhs,self);})
                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_int32     &lhs){return linalg::Mul(lhs,self);})
                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_uint32    &lhs){return linalg::Mul(lhs,self);})
                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_int16     &lhs){return linalg::Mul(lhs,self);})
                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_uint16    &lhs){return linalg::Mul(lhs,self);})
                .def("__rmul__",[](UniTensor &self, const cytnx::cytnx_bool      &lhs){return linalg::Mul(lhs,self);})

                .def("__mod__",[](UniTensor &self, const UniTensor &rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Mod(rhs);})
                .def("__mod__",[](UniTensor &self, const cytnx::cytnx_bool    &rhs){return self.Mod(rhs);})

                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_complex128&lhs){return linalg::Mod(lhs,self);})
                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_complex64 &lhs){return linalg::Mod(lhs,self);})
                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_double    &lhs){return linalg::Mod(lhs,self);})
                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_float     &lhs){return linalg::Mod(lhs,self);})
                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_int64     &lhs){return linalg::Mod(lhs,self);})
                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_uint64    &lhs){return linalg::Mod(lhs,self);})
                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_int32     &lhs){return linalg::Mod(lhs,self);})
                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_uint32    &lhs){return linalg::Mod(lhs,self);})
                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_int16     &lhs){return linalg::Mod(lhs,self);})
                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_uint16    &lhs){return linalg::Mod(lhs,self);})
                .def("__rmod__",[](UniTensor &self, const cytnx::cytnx_bool      &lhs){return linalg::Mod(lhs,self);})
 
                .def("__imul__",[](UniTensor &self, const UniTensor &rhs){return self.Mul_(rhs);}) // these will return self!
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Mul_(rhs);})
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Mul_(rhs);})
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Mul_(rhs);})
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Mul_(rhs);})
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Mul_(rhs);})
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Mul_(rhs);})
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Mul_(rhs);})
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Mul_(rhs);})
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Mul_(rhs);})
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Mul_(rhs);})
                .def("__imul__",[](UniTensor &self, const cytnx::cytnx_bool      &rhs){return self.Mul_(rhs);})

                .def("__truediv__",[](UniTensor &self, const UniTensor &rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Div(rhs);})
                .def("__truediv__",[](UniTensor &self, const cytnx::cytnx_bool      &rhs){return self.Div(rhs);})

                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_complex128&lhs){return linalg::Div(lhs,self);})
                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_complex64 &lhs){return linalg::Div(lhs,self);})
                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_double    &lhs){return linalg::Div(lhs,self);})
                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_float     &lhs){return linalg::Div(lhs,self);})
                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_int64     &lhs){return linalg::Div(lhs,self);})
                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_uint64    &lhs){return linalg::Div(lhs,self);})
                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_int32     &lhs){return linalg::Div(lhs,self);})
                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_uint32    &lhs){return linalg::Div(lhs,self);})
                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_int16     &lhs){return linalg::Div(lhs,self);})
                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_uint16    &lhs){return linalg::Div(lhs,self);})
                .def("__rtruediv__",[](UniTensor &self, const cytnx::cytnx_bool      &lhs){return linalg::Div(lhs,self);})
 
                .def("__itruediv__",[](UniTensor &self, const UniTensor &rhs){return self.Div_(rhs);}) // these will return self!
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Div_(rhs);})
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Div_(rhs);})
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Div_(rhs);})
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Div_(rhs);})
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Div_(rhs);})
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Div_(rhs);})
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Div_(rhs);})
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Div_(rhs);})
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Div_(rhs);})
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Div_(rhs);})
                .def("__itruediv__",[](UniTensor &self, const cytnx::cytnx_bool      &rhs){return self.Div_(rhs);})

                .def("__floordiv__",[](UniTensor &self, const UniTensor &rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Div(rhs);})
                .def("__floordiv__",[](UniTensor &self, const cytnx::cytnx_bool      &rhs){return self.Div(rhs);})

                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_complex128&lhs){return linalg::Div(lhs,self);})
                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_complex64 &lhs){return linalg::Div(lhs,self);})
                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_double    &lhs){return linalg::Div(lhs,self);})
                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_float     &lhs){return linalg::Div(lhs,self);})
                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_int64     &lhs){return linalg::Div(lhs,self);})
                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_uint64    &lhs){return linalg::Div(lhs,self);})
                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_int32     &lhs){return linalg::Div(lhs,self);})
                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_uint32    &lhs){return linalg::Div(lhs,self);})
                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_int16     &lhs){return linalg::Div(lhs,self);})
                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_uint16    &lhs){return linalg::Div(lhs,self);})
                .def("__rfloordiv__",[](UniTensor &self, const cytnx::cytnx_bool      &lhs){return linalg::Div(lhs,self);})
 
                .def("__ifloordiv__",[](UniTensor &self, const UniTensor &rhs){return self.Div_(rhs);}) // these will return self!
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_complex128&rhs){return self.Div_(rhs);})
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_complex64 &rhs){return self.Div_(rhs);})
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_double    &rhs){return self.Div_(rhs);})
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_float     &rhs){return self.Div_(rhs);})
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_int64     &rhs){return self.Div_(rhs);})
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_uint64    &rhs){return self.Div_(rhs);})
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_int32     &rhs){return self.Div_(rhs);})
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_uint32    &rhs){return self.Div_(rhs);})
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_int16     &rhs){return self.Div_(rhs);})
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_uint16    &rhs){return self.Div_(rhs);})
                .def("__ifloordiv__",[](UniTensor &self, const cytnx::cytnx_bool      &rhs){return self.Div_(rhs);})
                .def("__pow__",[](UniTensor &self, const cytnx::cytnx_double &p){return self.Pow(p);})
                .def("c__ipow__",[](UniTensor &self, const cytnx::cytnx_double &p){self.Pow_(p);})
                .def("Pow",&UniTensor::Pow)
                .def("cPow_",&UniTensor::Pow_)
                .def("cConj_",&UniTensor::Conj_)
                .def("Conj",&UniTensor::Conj)
                .def("cTrace_",&UniTensor::Trace_,py::arg("a"),py::arg("b"),py::arg("by_label")=false)
                .def("Trace",&UniTensor::Trace,py::arg("a"),py::arg("b"),py::arg("by_label")=false)
                .def("cTranspose_",&UniTensor::Transpose_)
                .def("Transpose",&UniTensor::Transpose)
                .def("cDagger_",&UniTensor::Dagger_)
                .def("Dagger",&UniTensor::Dagger)
                .def("ctag",&UniTensor::tag)
                .def("truncate",&UniTensor::truncate,py::arg("bond_idx"),py::arg("dim"),py::arg("by_label")=false)
                .def("ctruncate_",&UniTensor::truncate_)
                */
                ;
    //m.def("Contract",Contract);
   

}

