#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <math.h>

#define h(x) (1/(1+expf(-b*x))) // activation function
//To b brisketai sto arxeio utils.h

#define Input(x,y) (Input[x+length*y])
#define Y_out(x,y) (Y_out[x+outLength*y])

#define W1(x,y,p,s) ((W1[p+numOfSubNetworks*s])[x+N2*y])
#define W2(x,y,p,s) ((W2[p+numOfSubNetworks*s])[x+N3*y])
#define W3(x,y,p,s) ((W3[p+numOfSubNetworks*s])[x+N4*y])

void backpropagation_classify(float **W1,float **W2,float **W3,float *Input,float *Y_out,int length,int width,int outLength){

int N1=length/numOfSubNetworks;
int N2=M2/numOfSubNetworks;
int N3=M3/numOfSubNetworks;	
int N4=outLength/numOfSubNetworks;	
float sum;
float *u2;
float *u3;
int datSplWidth=(int)width/splitData; //Αριθμος δεδομένων ανα δίκτυο δεδομένων
u3=(float *)malloc(N3*sizeof(float));
u2=(float *)malloc(N2*sizeof(float));
	
for (int s=0; s < splitData; s++){	
	
for(int p=0;p<numOfSubNetworks;p++){	
for(int ww=0;ww<datSplWidth;ww++){//Για κάθε διάνυσμα-σημείο
//Input layer
 for(int i=0;i<N2;i++){
  sum=0.0;
  for(int j=0;j<N1;j++)
   sum+=W1(i,j,p,s)*Input((j+N1*p),(ww+datSplWidth*s));
   u2[i]=h(sum);
 }
 
//Hidden layer1
 for(int i=0;i<N3;i++){
  sum=0.0;
  for(int j=0;j<N2;j++)
   sum+=W2(i,j,p,s)*u2[j];
   u3[i]=h(sum);
   
 }
 
 //Hidden layer2
for(int i=0;i<N4;i++){
  sum=0.0;
  for(int j=0;j<N3;j++)
   sum+=W3(i,j,p,s)*u3[j];
 
Y_out((i+N4*p),(ww+datSplWidth*s))=roundf(h(sum)); 
}

}
 
} 

}
free(u2);
free(u3);

}

