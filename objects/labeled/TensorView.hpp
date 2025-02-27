/*
 * This file is part of cnine, a lightweight C++ tensor library. 
 *  
 * Copyright (c) 2021, Imre Risi Kondor
 *
 * This source code file is subject to the terms of the noncommercial 
 * license distributed with cnine in the file LICENSE.TXT. Commercial 
 * use is prohibited. All redistributed versions of this file (in 
 * original or modified form) must retain this copyright notice and 
 * must be accompanied by a verbatim copy of the license. 
 *
 */


#ifndef _CnineTensorView
#define _CnineTensorView

#include "Cnine_base.hpp"
#include "Gdims.hpp"
#include "GstridesB.hpp"
#include "Gindex.hpp"
#include "MemArr.hpp"
#include "device_helpers.hpp"

#ifdef _WITH_CUDA
#include <cuda.h>
#include <cuda_runtime.h>
#endif 

#ifdef _WITH_CUBLAS
#include <cublas_v2.h>
extern cublasHandle_t cnine_cublas;
#endif 


namespace cnine{

  template<typename TYPE>
  class TensorView{
  public:

    MemArr<TYPE> arr;
    Gdims dims;
    GstridesB strides;
    int dev;


  public: // ---- Constructors ------------------------------------------------------------------------------


    TensorView(){}

    TensorView(const MemArr<TYPE>& _arr, const Gdims& _dims, const GstridesB& _strides):
      arr(_arr),
      dims(_dims), 
      strides(_strides), 
      dev(_arr.device()){}


  public: // ---- Copying -----------------------------------------------------------------------------------


    TensorView(const TensorView<TYPE>& x):
      arr(x.arr),
      dims(x.dims),
      strides(x.strides),
      dev(x.dev){
    }
        
    TensorView& operator=(const TensorView& x){
      CNINE_ASSRT(dims==x.dims);
      CNINE_ASSIGN_WARNING();

      if(is_contiguous() && x.is_contiguous()){
	if(device()==0){
	  if(x.device()==0) std::copy(x.get_arro(),x.get_arro()+memsize(),get_arro());
	  if(x.device()==1) CUDA_SAFE(cudaMemcpy(get_arro(),x.get_arro(),memsize()*sizeof(TYPE),cudaMemcpyDeviceToHost)); 
	}
	if(device()==1){
	  if(x.device()==0) CUDA_SAFE(cudaMemcpy(get_arro(),x.get_arro(),memsize()*sizeof(float),cudaMemcpyHostToDevice));
	  if(x.device()==1) CUDA_SAFE(cudaMemcpy(get_arro(),x.get_arro(),memsize()*sizeof(float),cudaMemcpyDeviceToDevice));  
	}      
      }else{
	for_each([&](const Gindex& ix, TYPE& v) {v=x(ix);});
      }

      return *this;
    }


  public: // ---- Devices ------------------------------------------------------------------------------------


    TensorView(const TensorView<TYPE>& x, const int _dev):
      TensorView<TYPE>(MemArr<TYPE>(x.dims.total(),_dev),x.dims,GstridesB(x.dims)){
      (*this)=x;
    }


  public: // ---- Access -------------------------------------------------------------------------------------


    int device() const{
      return dev;
    }
    
    int ndims() const{
      return dims.size();
    }

    bool is_regular() const{
      return strides.is_regular(dims);
    }

    bool is_contiguous() const{
      return strides.is_contiguous(dims);
    }

    int asize() const{
      return dims.asize();
    }

    int memsize() const{
      return strides.memsize(dims);
    }

    TYPE* get_arr(){
      return arr.get_arr();
    } 

    const TYPE* get_arr() const{
      return arr.get_arr();
    } 

   TYPE* get_arro(){
      return arr.get_arr()+strides.offset;
    } 

    const TYPE* get_arro() const{
      return arr.get_arr()+strides.offset;
    } 


  public: // ---- Getters ------------------------------------------------------------------------------------


    TYPE operator()(const Gindex& ix) const{
      CNINE_CHECK_RANGE(dims.check_in_range(ix,string(__PRETTY_FUNCTION__)));
      return arr[strides.offs(ix)];
    }

    TYPE operator()(const int i0) const{
      CNINE_CHECK_RANGE(dims.check_in_range(i0,string(__PRETTY_FUNCTION__)));
      return arr[strides.offs(i0)];
    }

    TYPE operator()(const int i0, const int i1) const{
      CNINE_CHECK_RANGE(dims.check_in_range(i0,i1,string(__PRETTY_FUNCTION__)));
      return arr[strides.offs(i0,i1)];
    }

    TYPE operator()(const int i0, const int i1, const int i2) const{
      CNINE_CHECK_RANGE(dims.check_in_range(i0,i1,i2,string(__PRETTY_FUNCTION__)));
      return arr[strides.offs(i0,i1,i2)];
    }

    TYPE operator()(const int i0, const int i1, const int i2, const int i3) const{
      CNINE_CHECK_RANGE(dims.check_in_range(i0,i1,i2,i3,string(__PRETTY_FUNCTION__)));
      return arr[strides.offs(i0,i1,i2,i3)];
    }


  public: // ---- Setters ------------------------------------------------------------------------------------


    void set(const Gindex& ix, const TYPE x){
      CNINE_CHECK_RANGE(dims.check_in_range(ix,string(__PRETTY_FUNCTION__)));
      arr[strides.offs(ix)]=x;
    }

    void set(const int i0, const TYPE x){
      CNINE_CHECK_RANGE(dims.check_in_range(i0,string(__PRETTY_FUNCTION__)));
      arr[strides.offs(i0)]=x;
    }

    void set(const int i0, const int i1,  const TYPE x){
      CNINE_CHECK_RANGE(dims.check_in_range(i0,i1,string(__PRETTY_FUNCTION__)));
      arr[strides.offs(i0,i1)]=x;
    }

    void set(const int i0, const int i1, const int i2, const TYPE x){
      CNINE_CHECK_RANGE(dims.check_in_range(i0,i1,i2,string(__PRETTY_FUNCTION__)));
      arr[strides.offs(i0,i1,i2)]=x;
    }

    void set(const int i0, const int i1, const int i2, const int i3, const TYPE x){
      CNINE_CHECK_RANGE(dims.check_in_range(i0,i1,i2,i3,string(__PRETTY_FUNCTION__)));
      arr[strides.offs(i0,i1,i2,i3)]=x;
    }


  public: // ---- Incrementers -------------------------------------------------------------------------------


    void inc(const Gindex& ix, const TYPE x){
      CNINE_CHECK_RANGE(dims.check_in_range(ix,string(__PRETTY_FUNCTION__)));
      arr[strides.offs(ix)]+=x;
    }

    void inc(const int i0, const TYPE x){
      CNINE_CHECK_RANGE(dims.check_in_range(i0,string(__PRETTY_FUNCTION__)));
      arr[strides.offs(i0)]+=x;
    }

    void inc(const int i0, const int i1,  const TYPE x){
      CNINE_CHECK_RANGE(dims.check_in_range(i0,i1,string(__PRETTY_FUNCTION__)));
      arr[strides.offs(i0,i1)]+=x;
    }

    void inc(const int i0, const int i1, const int i2, const TYPE x){
      CNINE_CHECK_RANGE(dims.check_in_range(i0,i1,i2,string(__PRETTY_FUNCTION__)));
      arr[strides.offs(i0,i1,i2)]+=x;
    }

    void inc(const int i0, const int i1, const int i2, const int i3, const TYPE x){
      CNINE_CHECK_RANGE(dims.check_in_range(i0,i1,i2,i3,string(__PRETTY_FUNCTION__)));
      arr[strides.offs(i0,i1,i2,i3)]+=x;
    }


  public: // ---- Lambdas -----------------------------------------------------------------------------------


    void for_each(const std::function<void(const Gindex&, TYPE& x)>& lambda){
      dims.for_each_index([&](const Gindex& ix){
	  lambda(ix,arr[strides.offs(ix)]);});
    }

    void for_each(const std::function<void(const Gindex&, TYPE x)>& lambda) const{
      dims.for_each_index([&](const Gindex& ix){
	  lambda(ix,arr[strides.offs(ix)]);});
    }


  public: // ---- Index changes ------------------------------------------------------------------------------


    TensorView<TYPE> transp(){
      return TensorView<TYPE>(arr,dims.transp(),strides.transp());
    }

    TensorView<TYPE> permute_indices(const vector<int>& p){
      return TensorView<TYPE>(arr,dims.permute(p),strides.permute(p));
    }

    TensorView<TYPE> reshape(const Gdims& _dims){
      CNINE_ASSRT(_dims.asize()==asize());
      CNINE_ASSRT(is_regular());
      return TensorView<TYPE>(arr,_dims,GstridesB(_dims));
    }

    TensorView<TYPE> slice(const int d, const int i) const{
      CNINE_CHECK_RANGE(dims.check_in_range_d(d,i,string(__PRETTY_FUNCTION__)));
      return TensorView<TYPE>(arr,dims.remove(d),strides.remove(d).inc_offset(strides[d]*i));
    }


  public: // ---- In-place Operations ------------------------------------------------------------------------


    void set_zero(){
      if(dev==0){
	if(is_contiguous())
	  std::fill(arr.get_arr(),arr.get_arr()+asize(),0);
	else
	  CNINE_UNIMPL();
      }
      if(dev==1){
	if(is_contiguous()){
	  CUDA_SAFE(cudaMemset(arrg,0,asize*sizeof(TYPE)));
	}else
	  CNINE_UNIMPL();
      }
    }


    void inplace_times(const float c){
      if(dev==0){
	if(is_contiguous())
	  for(int i=0; i<asize(); i++) arr[i]*=c;
	else
	  for_each([&](const Gindex& ix, const TYPE& x){x*=c;});
      }
      if(dev==1){
	if(is_contiguous()){
	  const float cr=c;
	  CUBLAS_SAFE(cublasSaxpy(cnine_cublas,asize(),&cr,get_arr(), 1,get_arr(), 1));
	}else
	  CNINE_UNIMPL();
      }
    }


  public: // ---- Cumulative Operations ----------------------------------------------------------------------


    void add(const TensorView& x){
      CNINE_DEVICE_SAME(x);
      CNINE_CHECK_SIZE(dims.check_eq(x.dims));
      assert(asize()==x.asize());
      if(dev==0){
	if(is_contiguous() && x.is_contiguous() && strides==x.strides())
	  for(int i=0; i<asize(); i++) arr[i]+=x.arr[i];
	else
	  for_each([&](const Gindex& ix, const TYPE& v){v+=x(ix);});
      }
      if(dev==1){
	if(is_contiguous() && x.is_contiguous() && strides==x.strides()){
	  const TYPE alpha=1.0;
	  CUBLAS_SAFE(cublasSaxpy(cnine_cublas, asize, &alpha, x.arrg, 1, arrg, 1));
	}else
	  CNINE_UNIMPL();
      }
    }

    void add(const TensorView& x, const TYPE c){
      CNINE_DEVICE_SAME(x);
      CNINE_CHECK_SIZE(dims.check_eq(x.dims));
      assert(asize()==x.asize());
      if(dev==0){
	if(is_contiguous() && x.is_contiguous() && strides==x.strides())
	  for(int i=0; i<asize(); i++) arr[i]+=c*x.arr[i];
	else
	  for_each([&](const Gindex& ix, const TYPE& v){v+=c*x(ix);});
      }
      if(dev==1){
	if(is_contiguous() && x.is_contiguous() && strides==x.strides()){
	  const TYPE alpha=c;
	  CUBLAS_SAFE(cublasSaxpy(cnine_cublas, asize, &alpha, x.arrg, 1, arrg, 1));
	}else
	  CNINE_UNIMPL();
      }
    }


  public: // ---- Matrix multiplication ---------------------------------------------------------------------


    void add_mvprod(const TensorView& x, const TensorView& y){
      reconcile_devices<TensorView<TYPE> >(*this,x,y,[](TensorView<TYPE>& r, const TensorView<TYPE>& x, const TensorView<TYPE>& y){
	  CNINE_NDIMS_IS_1(r);
	  CNINE_NDIMS_IS_2(x);
	  CNINE_NDIMS_IS_1(y);
	  CNINE_ASSRT(x.dims[0]==r.dims[0]);
	  CNINE_ASSRT(x.dims[1]==y.dims[0]);

	  if(r.dev==0){
	    for(int i=0; i<r.dims[0]; i++){
	      TYPE t=0;
	      for(int k=0; k<x.dims[1]; k++)
		t+=x(i,k)*y(k);
	      r.inc(i,t);
	    }
	  }
	  if(r.dev==1){
	    CNINE_UNIMPL();
	  }
	});
    }


    void add_mvprod_T(const TensorView& x, const TensorView& y){
      reconcile_devices<TensorView<TYPE> >(*this,x,y,[](TensorView<TYPE>& r, const TensorView<TYPE>& x, const TensorView<TYPE>& y){
	  CNINE_NDIMS_IS_1(r);
	  CNINE_NDIMS_IS_2(x);
	  CNINE_NDIMS_IS_1(y);
	  CNINE_ASSRT(x.dims[1]==r.dims[0]);
	  CNINE_ASSRT(y.dims[0]==y.dims[0]);

	  if(r.dev==0){
	    for(int i=0; i<r.dims[0]; i++){
	      TYPE t=0;
	      for(int k=0; k<x.dims[1]; k++)
		t+=x(k,i)*y(k);
	      r.inc(i,t);
	    }
	  }
	  if(r.dev==1){
	    CNINE_UNIMPL();
	  }
	});
    }


    void add_mprod(const TensorView& x, const TensorView& y){
      reconcile_devices<TensorView<TYPE> >(*this,x,y,[](TensorView<TYPE>& r, const TensorView<TYPE>& x, const TensorView<TYPE>& y){
	  CNINE_NDIMS_IS_2(r);
	  CNINE_NDIMS_IS_2(x);
	  CNINE_NDIMS_IS_2(y);
	  CNINE_ASSRT(x.dims[0]==r.dims[0]);
	  CNINE_ASSRT(y.dims[1]==r.dims[1]);
	  CNINE_ASSRT(x.dims[1]==y.dims[0]);

	  if(r.dev==0){
	    for(int i=0; i<r.dims[0]; i++)
	      for(int j=0; j<r.dims[1]; j++){
		TYPE t=0;
		for(int k=0; k<x.dims[1]; k++)
		  t+=x(i,k)*y(k,j);
		r.inc(i,j,t);
	      }
	  }
	  if(r.dev==1){
	    CNINE_UNIMPL();
	  }
	});
    }


  public: // ---- I/O ---------------------------------------------------------------------------------------


    string classname() const{
      return "TensorView";
    }

    string describe() const{
      ostringstream oss;
      oss<<"TensorView"<<dims<<" ["<<strides<<"]"<<endl;
      return oss.str();
    }

    string str(const string indent="") const{
      CNINE_CPUONLY();
      ostringstream oss;

      if(ndims()==1){
	oss<<indent<<"[ ";
	for(int i0=0; i0<dims[0]; i0++)
	  oss<<(*this)(i0)<<" ";
	oss<<"]"<<endl;
	return oss.str();
      }

      if(ndims()==2){
	for(int i0=0; i0<dims[0]; i0++){
	  oss<<indent<<"[ ";
	  for(int i1=0; i1<dims[1]; i1++)
	    oss<<(*this)(i0,i1)<<" ";
	  oss<<"]"<<endl;
	}
	return oss.str();
      }

      return oss.str();
    }

    friend ostream& operator<<(ostream& stream, const TensorView<TYPE>& x){
      stream<<x.str(); return stream;
    }

  };


}

#endif


    /*
    TensorView(const TensorView<TYPE>& x, const nowarn_flag& dummy):
      arr(x.arr),
      dims(x.dims),
      strides(x.strides),
      dev(x.dev){
    }
    */

    /*
    TensorView(TensorView<TYPE>&& x):
      arr(std::move(x.arr)),
      dims(x.dims),
      strides(x.strides),
      dev(x.dev),
      regular(x.regular){
      CNINE_MOVE_WARNING();
    }
    */
