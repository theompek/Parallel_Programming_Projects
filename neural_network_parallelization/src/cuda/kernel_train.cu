#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <math.h>
#include <pthread.h>

#define lamda 0.8
#define h(x) (1/(1+expf(-b*x))) // activation function
//To b brisketai sto arxeio utils.h

#define Input(x,y) (Input[x+length*y])
#define Output(x,y) (Output[x+outLength*y])

#define W1(x,y) (W1[x+n2*y])
#define W2(x,y) (W2[x+n3*y])
#define W3(x,y) (W3[x+n4*y])

#define d_W1(x,y) (d_W1[x+n2*y])
#define d_W2(x,y) (d_W2[x+n3*y])
#define d_W3(x,y) (d_W3[x+n4*y])

#define epochMax 500 //Megistos ari8mos epoxwn


__global__ void kernel(float *d_W1, float *d_W2, float *d_W3, float *Input, float *Output,int length,int width,int outLength){
float sum;
__shared__ float U_int[n1];
__shared__ float u2Out[n2];
__shared__ float u3Out[n3];
__shared__ float u4Out[n4];
__shared__ float error[n4];
__shared__ float delta3[n4];
__shared__ float delta2[n3];
__shared__ float delta1[n2];

__shared__ float W1[n2*n1];
__shared__ float W2[n3*n2];
__shared__ float W3[n4*n3];

//Αντιγραφή από την Gloabal memory στην Shared Memory
if((threadIdx.y==0)&&(threadIdx.x<n2))
for(int i=0;i<n1;i++)
W1(threadIdx.x,i)=d_W1(threadIdx.x,i);


if((threadIdx.y==1)&&(threadIdx.x<n3))
for(int i=0;i<n2;i++)
W2(threadIdx.x,i)=d_W2(threadIdx.x,i);


if((threadIdx.y==2)&&(threadIdx.x<n4))
for(int i=0;i<n3;i++)
W3(threadIdx.x,i)=d_W3(threadIdx.x,i);


int epoch=0;

while(epoch<epochMax){
	
for(int Id=0;Id<width;Id++)
{
if((threadIdx.y==0)&&(threadIdx.x<n2)){//------->1111111111HiddenLayer

	//-----FORWARD PATH-----	 
	for(int i=0;i<n1;i++)
	U_int[i]=Input(i,Id);
	
	sum=0.0;
	for(int i=0;i<n1;i++)
	sum+= W1(threadIdx.x,i)*U_int[i];
		
	u2Out[threadIdx.x]=h(sum); 
}	

__syncthreads(); //1
	
if((threadIdx.y==1)&&(threadIdx.x<n3)){//------->2222222222HiddenLayer

	//-----FORWARD PATH----
	sum=0.0;
	for(int i=0;i<n2;i++)
	sum+= W2(threadIdx.x,i)*u2Out[i];
		
	u3Out[threadIdx.x]=h(sum);
}	
__syncthreads(); //2

if((threadIdx.y==2)&&(threadIdx.x<n4)){//------->33333333Outtt
	//-----FORWARD PATH-----	 
	sum=0.0;
	for(int i=0;i<n3;i++)
	sum+= W3(threadIdx.x,i)*u3Out[i];
		
	u4Out[threadIdx.x]=h(sum); 
	   
	//-----BACKWARD PATH------- 
	error[threadIdx.x]=Output(threadIdx.x,Id)-u4Out[threadIdx.x];
	delta3[threadIdx.x]=error[threadIdx.x]*(b*u4Out[threadIdx.x]*(1.0-u4Out[threadIdx.x])); //u4Out=h(Σ(U3)) αρα απο τον τύπο dh(x)=(b*h(x)*(1-h(x)))
	
	//printf("delt3-->33333= %f \n----\n",delta3[threadIdx.x]);
}
__syncthreads(); //3

if((threadIdx.y==1)&&(threadIdx.x<n3)){//------->2222222222HiddenLayer

//-----BACKWARD PATH-------   
	sum=0.0;
	for(int j=0;j<n4;j++)
	sum+=W3(j,threadIdx.x)*delta3[j];

//printf("delt3-->2222= %f \n----\n",delta3[j]);
	delta2[threadIdx.x]=(b*u3Out[threadIdx.x]*(1.0-u3Out[threadIdx.x]))*sum; //u3Out=h(Σ(U2)) αρα απο τον τύπο dh(x)=(b*h(x)*(1-h(x)))	

}

__syncthreads(); //4

if((threadIdx.y==0)&&(threadIdx.x<n2)){//------->1111111111InputLayer
	//-----BACKWARD PATH-------	
	sum=0.0;
	for(int j=0;j<n3;j++)
	sum+=W2(j,threadIdx.x)*delta2[j];

	
	delta1[threadIdx.x]=(b*u2Out[threadIdx.x]*(1.0-u2Out[threadIdx.x]))*sum; //u2Out=h(Σ(U1)) αρα απο τον τύπο dh(x)=(b*h(x)*(1-h(x)))
}
__syncthreads(); //5

if((threadIdx.y==0)&&(threadIdx.x<n2)){//------->1111111111HiddenLayer
for(int j=0;j<n1;j++)
W1(threadIdx.x,j)+=lamda*delta1[threadIdx.x]*U_int[j];
}

if((threadIdx.y==1)&&(threadIdx.x<n3)){//------->2222222222HiddenLayer
for(int j=0;j<n2;j++)
W2(threadIdx.x,j)+=lamda*delta2[threadIdx.x]*u2Out[j];	
}	

if((threadIdx.y==2)&&(threadIdx.x<n4)){//------->33333333HOuttt	
for(int j=0;j<n3;j++)
W3(threadIdx.x,j)+=lamda*delta3[threadIdx.x]*u3Out[j];
}	

} 

epoch++;	
}//Τελος while	


//Αντιγραφή πίσω στην Gloabal memory
if((threadIdx.y==0)&&(threadIdx.x<n2))
for(int i=0;i<n1;i++)
d_W1(threadIdx.x,i)=W1(threadIdx.x,i);


if((threadIdx.y==1)&&(threadIdx.x<n3))
for(int i=0;i<n2;i++)
d_W2(threadIdx.x,i)=W2(threadIdx.x,i);


if((threadIdx.y==2)&&(threadIdx.x<n4))
for(int i=0;i<n3;i++)
d_W3(threadIdx.x,i)=W3(threadIdx.x,i);
		
}//Telos kernel
