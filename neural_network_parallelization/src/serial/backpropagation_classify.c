#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <math.h>

#define h(x) (1/(1+exp(-b*x))) // activation function
//To b brisketai sto arxeio utils.h

#define Input(x,y) (Input[x+length*y])
#define Y_out(x,y) (Y_out[x+outLength*y])

#define W1(x,y) (W1[x+n2*y])
#define W2(x,y) (W2[x+n3*y])
#define W3(x,y) (W3[x+n4*y])

void backpropagation_classify(float *W1,float *W2,float *W3,float *Input,float *Y_out,int length,int width,int outLength){

int n1=length;
int n2=M2;
int n3=M3;	
int n4=outLength;	
float sum;
float *u2;
float *u3;

u3=(float *)malloc(n3*sizeof(float));
u2=(float *)malloc(n2*sizeof(float));
	
for(int Id=0;Id<width;Id++){//Για κάθε διάνυσμα-σημείο

//Input layer
 for(int i=0;i<n2;i++){
  sum=0.0;
  for(int j=0;j<n1;j++)
   sum+=W1(i,j)*Input(j,Id);
   u2[i]=h(sum);
 }
 
//Hidden layer1
 for(int i=0;i<n3;i++){
  sum=0.0;
  for(int j=0;j<n2;j++)
   sum+=W2(i,j)*u2[j];
   u3[i]=h(sum);
   
 }
 
 //Hidden layer2
for(int i=0;i<n4;i++){
  sum=0.0;
  for(int j=0;j<n3;j++)
   sum+=W3(i,j)*u3[j];
 
Y_out(i,Id)=roundf(h(sum)); 
}

}
 
free(u2);
free(u3);

}

