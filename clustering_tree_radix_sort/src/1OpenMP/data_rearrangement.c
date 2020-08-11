#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "omp.h"

#define DIM 3


void data_rearrangement(float *Y, float *X,unsigned int *permutation_vector,int N){
	
if(N>200000)
 { 
	
#pragma omp parallel for shared(X,Y,permutation_vector,N) schedule(static)
  for(int i=0; i<N; i++)
  {                                                      
      memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));//η memcpy(α,β,γ) αντιγραφει τους πρωτους γ χαρακτηρες του β στο α
  }

}
else {
for(int i=0; i<N; i++)
  {                                                      
      memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));//η memcpy(α,β,γ) αντιγραφει τους πρωτους γ χαρακτηρες του β στο α
  }

 }
}