#include "Cnine_base.cpp"
#include "CnineSession.hpp"
#include "SparseRmatrix.hpp"

using namespace cnine;

int main(int argc, char** argv){

  cnine_session session;

  SparseRmatrix M(10,10);

  M.set(2,3,5.0);
  M.set(2,8,1.0);
  M.set(3,1,4.0);

  cout<<M<<endl;
  
  

}

