#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#define DIM 3

void data_rearrangement(float *Y, float *X,unsigned int *permutation_vector,int N)
{ 
if(N>200000)
{
	cilk_for(int i=0; i<N; i++)
  {                                           
      memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));//η memcpy(α,β,γ) αντιγραφει τους πρωτους γ χαρακτηρες του β στο α
  }
}
else{
for(int i=0; i<N; i++)
  {                                           
      memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));//η memcpy(α,β,γ) αντιγραφει τους πρωτους γ χαρακτηρες του β στο α
  }

}
}
