#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BLOCK_SIZE 16

#define MAX(i,j) ( (i)<(j) ? (j):(i) )
#define MIN(i,j) ( (i)<(j) ? (i):(j) )

#define SubArrayA(x,y) subArrayA[(x)*BLOCK_SIZE+(y)]
#define SubArrayB(x,y) subArrayB[(x)*BLOCK_SIZE+(y)]


#define InputArrayA(x,y) inputArrayA[(x)*BLOCK_SIZE+(y)]
#define InputArrayB(x,y) inputArrayB[(x)*BLOCK_SIZE+(y)]

#define ImageOut(x,y) imageOut[(x)*imageSize+(y)]
#define ZiArray(x,y) ziArray[(x)*imageSize+(y)]

__device__ float gaussianDistance(float  *inputArrayA,float *inputArrayB,int xj,int yj,int halfPatchWidth);
__device__ float weightingFunct(float  *inputArrayA,float *inputArrayB,int xj,int yj,int halfPatchWidth,float sigma,float Zi);
__device__ float normFactor(float  *inputArrayA,float *inputArrayB,int halfPatchWidth,float sigma);
__device__ float nonLocalMeans(float  *inputArrayA,float *inputArrayB,float *imageOut,int xi,int yi,int halfPatchWidth,int imageSize,float sigma,float Zi);

__global__ void mainGpuFunction(float  const * const inputArray,float *imageOut,float *ziArray,int halfPatchWidth,int imageSize,float sigma,int flag)
{
 //Πάρε τις συντεταγμένες του αντίστοιχου pixel που κάνουμε αποθορυβοποίηση
  int xi = blockIdx.x * blockDim.x + threadIdx.x;
  int yi = blockIdx.y * blockDim.y + threadIdx.y;
    
//Πρέπει να είμαι μέσα στα όρια της εικόνας	
if((xi<imageSize)&&(yi<imageSize)){

 __shared__ float subArrayA[BLOCK_SIZE*BLOCK_SIZE];
 __shared__ float subArrayB[BLOCK_SIZE*BLOCK_SIZE];

//Αν ανοίκω σε επίπεδο Z=0
//if(blockIdx.z==0) ImageOut(xi,yi)=0; 

//===========================
//Υπολόγιζουμε σύμφωνα με την μεταβλητή blockIdx.z ποιό block θα 
//φορτώσουμε για υπολογισμό μαζί με το κύριο block.
//Ισχύει η σχέση blockIdx.z=neighblockX*blockDim.x+neighblockY

int neighblockX;
int neighblockY;


for(int i=0;i<blockDim.x;i++){
for(int j=0;j<blockDim.y;j++){
	 if(blockIdx.z==(i*blockDim.x+j))
	 {
	  neighblockX=i;
	  neighblockY=j;
	  i=blockDim.x;//Για να βγούμε απο το loop
	  break;
	 }
 }
}
//===========================

int xj = neighblockX*blockDim.x + threadIdx.x;
int yj = neighblockY* blockDim.y + threadIdx.y;

//Αν θέλω να υπολογίσω την τιμή Zi
if(flag==1)
{
//Φορτώνουμε το κύριο block
//Κάθε thread φορτώνει μονάχα ενα στοιχείο στον πίνακα
SubArrayA(threadIdx.x,threadIdx.y)=inputArray[xi*imageSize+yi];
__syncthreads();

//Φορτώνουμε στον πίνακα Β ένα απο τα block εικόνας ανάλογα με τον 
//αριθμό του blockIdx.z στον οποίο βρισκόμαστε
SubArrayB(threadIdx.x,threadIdx.y)=inputArray[xj*imageSize+yj];	
__syncthreads();

float Zi=normFactor(subArrayA,subArrayB,halfPatchWidth,sigma);


atomicAdd(&ZiArray(xi,yi),Zi);

//Περιμένουμε να υπολογιστούν και να προστεθούν ολα τα Ζi
__syncthreads();  	
		
}
else //Αν θέλω να υπολογίσω τα αθροίσματα w(i,j)*f(j)
{
//Φορτώνουμε το κύριο block
//Κάθε thread φορτώνει μοναχα ενα στοιχείο στον πίνακα
SubArrayA(threadIdx.x,threadIdx.y)=inputArray[xi*imageSize+yi];
__syncthreads();

//Φορτώνουμε στον πίνακα Β ένα απο τα block εικόνας ανάλογα με τον 
//αριθμό του blockIdx.z στον οποίο βρισκόμαστε
SubArrayB(threadIdx.x,threadIdx.y)=inputArray[xj*imageSize+yj];	
__syncthreads();

float SumWeight=nonLocalMeans(subArrayA,subArrayB,imageOut,xi,yi,halfPatchWidth,imageSize,sigma,ZiArray(xi,yi));


atomicAdd(&ImageOut(xi,yi),SumWeight);

 __syncthreads();  
 
	
}

}


}

__device__ float nonLocalMeans(float  *inputArrayA,float *inputArrayB,float *imageOut,int xi,int yi,int halfPatchWidth,int imageSize,float sigma,float Zi){

 
	 float ww=0;
	

	 for(int xj=0;xj<BLOCK_SIZE;xj++)
     {	
      for(int yj=0;yj<BLOCK_SIZE;yj++)
       {
		  ww+=weightingFunct(inputArrayA,inputArrayB,xj,yj,halfPatchWidth,sigma,Zi)*InputArrayB(xj,yj); //w(i,j)*f(j)
		 
	   }
	 } 
	 
return(ww);
	 	
}

//Η μεταβλητή w(i,j)=w([xi,yi] [xj,yj])
__device__ float weightingFunct(float  *inputArrayA,float *inputArrayB,int xj,int yj,int halfPatchWidth,float sigma,float Zi){

float distance=gaussianDistance(inputArrayA,inputArrayB,xj,yj,halfPatchWidth); 	
	
	return ( ( expf(-(distance/(sigma*sigma))) )/Zi);
}

//Η μεταβλητη Z(i)=Z(xi,yi)
__device__ float normFactor(float *inputArrayA,float *inputArrayB,int halfPatchWidth,float sigma){

float square_sigma=sigma*sigma;
float z=0;

for(int i=0;i<BLOCK_SIZE;i++)
{
 for(int j=0;j<BLOCK_SIZE;j++)
 {
  float distance=gaussianDistance(inputArrayA,inputArrayB,i,j,halfPatchWidth); 
	 
	z+=expf(-(distance/square_sigma) );   
 }	
}
 
return (z);	
}

//Υπολογισμός της διαφοράς |f(Ni)-f(Nj)|
//Χρησιμοποιούμε Gaussian Euclidean Distance
__device__ float gaussianDistance(float  *inputArrayA,float *inputArrayB,int xj,int yj,int halfPatchWidth){

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
 
for(int i=-halfPatchWidth;i<=halfPatchWidth;i++)
 {
  for(int j=-halfPatchWidth;j<=halfPatchWidth;j++)
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
	diff=InputArrayA(ai,bi)-InputArrayB(aj,bj);
	distance+=diff*diff*weight;
	
   }
  }
 }


 return (distance/SumWeight);


}

