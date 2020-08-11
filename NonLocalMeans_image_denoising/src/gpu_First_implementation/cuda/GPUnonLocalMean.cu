#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BLOCK_SIZE 16

#define MAX(i,j) ( (i)<(j) ? (j):(i) )
#define MIN(i,j) ( (i)<(j) ? (i):(j) )

#define SubArrayA(x,y) subArrayA[(x)*BLOCK_SIZE+(y)]
#define InputArrayA(x,y) inputArrayA[(x)*BLOCK_SIZE+(y)]
#define ImageOut(x,y) imageOut[(x)*imageSize+(y)]


__device__ float gaussianDistance(float  *inputArrayA,int xj,int yj,int halfPatchSWidth);
__device__ float weightingFunct(float  *inputArrayA,int xj,int yj,int halfPatchSWidth,float sigma,float Zi);
__device__ float normFactor(float  *inputArrayA,int halfPatchSWidth,float sigma);
__device__ float nonLocalMeans(float  *inputArrayA,float *imageOut,int halfPatchSWidth,int imageSize,float sigma);

__global__ void mainGpuFunction(float  const * const inputArray,float *imageOut,int halfPatchSWidth,int imageSize,float sigma)
{
 //Πάρε τις συντεταγμένες του αντίστοιχου pixel που κάνουμε αποθορυβοποίηση
  int xi = blockIdx.x * blockDim.x + threadIdx.x;
  int yi = blockIdx.y * blockDim.y + threadIdx.y;

//Πρέπει να είμαι μέσα στα όρια της εικόνας    
if((xi<imageSize)&&(yi<imageSize)){


__shared__ float subArrayA[BLOCK_SIZE*BLOCK_SIZE];

//Κάθε ένα απο τα thread θα φορτώσει μία τιμή του πίνακα τιμών των pixel
//απο την global μνήμη στην shared μνημη 	
SubArrayA(threadIdx.x,threadIdx.y)=inputArray[xi*imageSize+yi];
__syncthreads();

//Το κάθε νήμα εφαρμόζει τον αλγόριθμο για το τμήμα της εικόνας που φόρτωσε στην μνήμη shared
ImageOut(xi,yi)=nonLocalMeans(subArrayA,imageOut,halfPatchSWidth,imageSize,sigma);

 }//Τέλος if	
 
}


__device__ float nonLocalMeans(float  *inputArrayA,float *imageOut,int halfPatchSWidth,int imageSize,float sigma){

	 float ww=0;
	 
	 float Zi=normFactor(inputArrayA,halfPatchSWidth,sigma); //Υπολογίζουμε την τιμή Z(i)
     
	 //Τα αθροίσματα w(i,j)*f(j)
	 for(int xj=0;xj<BLOCK_SIZE;xj++)
     {	
      for(int yj=0;yj<BLOCK_SIZE;yj++)
       {
		  ww+=weightingFunct(inputArrayA,xj,yj,halfPatchSWidth,sigma,Zi)*InputArrayA(xj,yj); //w(i,j)*f(j)
		 
	   }
	 } 
	 
return(ww);
	 	 
}

//Η μεταβλητή w(i,j)=w([xi,yi] [xj,yj])
__device__ float weightingFunct(float  *inputArrayA,int xj,int yj,int halfPatchSWidth,float sigma,float Zi){

float distance=gaussianDistance(inputArrayA,xj,yj,halfPatchSWidth); 	
	
	return ( ( exp(-(distance/(sigma*sigma))) )/Zi);
}

//Η μεταβλητη Z(i)=Z(xi,yi)
__device__ float normFactor(float  *inputArrayA,int halfPatchSWidth,float sigma){

float square_sigma=sigma*sigma;
float z=0;

for(int i=0;i<BLOCK_SIZE;i++)
{
 for(int j=0;j<BLOCK_SIZE;j++)
 {
  float distance=gaussianDistance(inputArrayA,i,j,halfPatchSWidth); 
	 
	z+=exp(-(distance/square_sigma) );
	
   
 }	
}

return (z);	
}

//Υπολογισμός της διαφοράς |f(Ni)-f(Nj)|
//Χρησιμοποιούμε Gaussian Euclidean Distance
__device__ float gaussianDistance(float  *inputArrayA,int xj,int yj,int halfPatchSWidth){

int xi=threadIdx.x;
int yi=threadIdx.y;

//Ο δείκτης i αναφέρεται στο pixel του οποίο υπολογίζουμε την νέα τιμή
//ενώ ο δείκτης j στα υπόλοιπα pixel με τα οποία γίνεται σύγκριση

int ai;
int bi;
int aj;
int bj;

int SumWeight=0;  //Άθροισμα βαρών
float distance=0;//Συνολική διαφορά γειτονιάς pixel
float diff=0;     //Διαφορά μεταξύ 2 pixel γειτόνων
 
for(int i=-halfPatchSWidth;i<=halfPatchSWidth;i++)
 {
  for(int j=-halfPatchSWidth;j<=halfPatchSWidth;j++)
  {
  ai=xi+i; 
  bi=yi+j;
  aj=xj+i;
  bj=yj+j;
  
if((aj<0)||(aj>=BLOCK_SIZE)) aj=xj-i; 
if((bj<0)||(bj>=BLOCK_SIZE)) bj=yj-j;
  
if((ai<0)||(ai>=BLOCK_SIZE)) ai=xi-i; 
if((bi<0)||(bi>=BLOCK_SIZE)) bi=yi-j;  

   if (ai!=xi || bi!=yi)//Κάνε σύγκριση για όλη την γειτονιά εκτώς του κεντρικού 
   { 
    int weight=1/(MAX(ai-xi,xi-ai)+MAX(bi-yi,yi-bi));
    SumWeight+=weight;	
	diff=InputArrayA(ai,bi)-InputArrayA(aj,bj);
	distance+=diff*diff*weight;
	
   }
  }
 }

 return (distance/SumWeight);

}

