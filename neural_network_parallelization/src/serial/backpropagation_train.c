#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <math.h>
#include <time.h>

#define lamda 0.8
#define h(x) (1/(1+exp(-b*x))) // activation function
//To b brisketai sto arxeio utils.h

#define Input(x,y) (Input[x+length*y])
#define Output(x,y) (Output[x+outLength*y])

#define W1(x,y) ((*W1)[x+n2*y])
#define W2(x,y) ((*W2)[x+n3*y])
#define W3(x,y) ((*W3)[x+n4*y])

#define epochMax 500

void backpropagation_train(float **W1,float **W2,float **W3,float *Input,float *Output,int length,int width,int outLength){
	
int epoch=0;
float sum;
float *u2;
float *u3;
float *u4;	
float *error;
float *delta1;
float *delta2;
float *delta3;

//Διαστάσεις
int n1=length;
int n2=M2;
(*W1)=(float *)malloc(n2*n1*sizeof(float));
	
int n3=M3;	
(*W2)=(float *)malloc(n3*n2*sizeof(float));	
	
int n4=outLength;	
(*W3)=(float *)malloc(n4*n3*sizeof(float));	

printf("Diktyo me 4 layer me diastaseis %d x %d x %d x %d\n\n",n1,n2,n3,n4);

u4=(float *)malloc(n4*sizeof(float));
u3=(float *)malloc(n3*sizeof(float));
u2=(float *)malloc(n2*sizeof(float));
error=(float *)malloc(n4*sizeof(float));
delta3=(float *)malloc(n4*sizeof(float));
delta2=(float *)malloc(n3*sizeof(float));
delta1=(float *)malloc(n2*sizeof(float));


//Αρχικοποίηση
for(int i=0;i<n2*n1;i++)
(*W1)[i]=((float)rand()/RAND_MAX)/10;


for(int i=0;i<n3*n2;i++)
(*W2)[i]=((float)rand()/RAND_MAX)/10;

for(int i=0;i<n4*n3;i++)
(*W3)[i]=((float)rand()/RAND_MAX)/10;


while(epoch<epochMax){

for(int Id=0;Id<width;Id++){//Για κάθε διάνυσμα-σημείο

//-----FORWARD PATH-----

//Input layer
 for(int i=0;i<n2;i++){
  sum=0.0;
  for(int j=0;j<n1;j++)
   sum+= W1(i,j)*Input(j,Id);
     
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
   
   u4[i]=h(sum);
  // printf("fddf = %f\n",roundf(h(sum)));
 }
// printf("-------\n ");
//-------------------------

//-----BACKWARD PATH-------
sum=0.0;
for(int i=0;i<n4;i++){
error[i]=Output(i,Id)-u4[i];

//printf("Output = %f  u4[i]=  %f    error = %f\n",Output(i,Id),u4[i],error[i]);
}
// printf("-------\n ");

for(int i=0;i<n4;i++)
delta3[i]=error[i]*(b*u4[i]*(1.0-u4[i])); //u4=h(Σ(U3)) αρα απο τον τύπο dh(x)=(b*h(x)*(1-h(x)))

for(int i=0;i<n3;i++){
 sum=0.0;
for(int j=0;j<n4;j++)
  sum+=W3(j,i)*delta3[j];
 
delta2[i]=(b*u3[i]*(1.0-u3[i]))*sum; //u3=h(Σ(U2)) αρα απο τον τύπο dh(x)=(b*h(x)*(1-h(x)))
}

for(int i=0;i<n2;i++){
 sum=0.0;
 for(int j=0;j<n3;j++)
  sum+=W2(j,i)*delta2[j];
 
delta1[i]=(b*u2[i]*(1.0-u2[i]))*sum; //u2=h(Σ(U1)) αρα απο τον τύπο dh(x)=(b*h(x)*(1-h(x)))

}


for(int i=0;i<n2;i++)
 for(int j=0;j<n1;j++)
  W1(i,j)+=lamda*delta1[i]*Input(j,Id);

for(int i=0;i<n3;i++)
 for(int j=0;j<n2;j++)
  W2(i,j)+=lamda*delta2[i]*u2[j];
 
for(int i=0;i<n4;i++)
 for(int j=0;j<n3;j++)
  W3(i,j)+=lamda*delta3[i]*u3[j];
 
//-------------------------


}

epoch++;	
}//Τελος while


free(u2);
free(u3);
free(u4);
free(error);
free(delta3);
free(delta2);
free(delta1);
	
	
}
