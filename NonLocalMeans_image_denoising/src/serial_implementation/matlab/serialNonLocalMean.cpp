#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mex.h"



#define MAX(i,j) ( (i)<(j) ? (j):(i) )

#define ImageArray(x,y) imageArray[(x)*imageSize+(y)]
#define ImageOut(x,y) imageOut[(x)*imageSize+(y)]

double gaussianDistance(double *imageArray,int xi,int yi,int xj,int yj,int halfPatchSWidth,int imageSize);
double weightingFunct(double *imageArray,int xi,int yi,int xj,int yj,int halfPatchSWidth,int imageSize,double sigma,double Zi);
double normFactor(double *imageArray,int xi,int yi,int halfPatchSWidth,int imageSize,double sigma);

//imageArray:Ο πίνακας με τις τιμές της εικόνας
//halfPatchSWidth:Το μήκος του μισού patch
//imageSize:Μέγεθος εικόνας σε pixel πχ 64,128,256
//sigma:Τιμή της μεταβλητής σ
//xi,yi:Pixel του οποίου ψαχνουμε την νέα τιμή
//xj,yj:Pixel με τα οποία συγκρίνουμε το pixel που αποθορυβοποιούμε

//Η συνάρτηση υπολογίζει και επιστρέφει ένα πίνακα με τις νεές αποθορυβοποίημενες τιμές των pixel
//χρησιμοποιώντας τις υπόλοιπες συναρτήσεις που έχουν υλοποιηθεί στο πρόγραμμα
void nonLocalMeans(double *imageArray,double *imageOut,int halfPatchSWidth,int imageSize,double sigma){

//Για όλα τα pixel i υπολόγισε τις νέες τιμές τους	
for(int xi=0;xi<imageSize;xi++)
{	
 for(int yi=0;yi<imageSize;yi++)
 {
	 
	 ImageOut(xi,yi)=0; //Αρχικοποίηση
	 double Zi=normFactor(imageArray,xi,yi,halfPatchSWidth,imageSize,sigma); //Υπολογίζουμε την τιμή Z(i)
	
	//Τα αθροίσματα w(i,j)*f(j)
	 for(int xj=0;xj<imageSize;xj++)
     {	
      for(int yj=0;yj<imageSize;yj++)
       {
		 ImageOut(xi,yi)+=weightingFunct(imageArray,xi,yi,xj,yj,halfPatchSWidth,imageSize,sigma,Zi)*ImageArray(xj,yj); //w(i,j)*f(j)
	   }
	  
	 }
 }
 
} 
	
}

//Η μεταβλητή w(i,j)=w([xi,yi] [xj,yj])
double weightingFunct(double *imageArray,int xi,int yi,int xj,int yj,int halfPatchSWidth,int imageSize,double sigma,double Zi){

double distance=gaussianDistance(imageArray,xi,yi,xj,yj,halfPatchSWidth,imageSize); 	
	
	return ( ( exp(-(distance/(sigma*sigma))) )/Zi);
}

//Η μεταβλητη Z(i)=Z(xi,yi)
double normFactor(double *imageArray,int xi,int yi,int halfPatchSWidth,int imageSize,double sigma){

double square_sigma=sigma*sigma;
double z=0;

for(int i=0;i<imageSize;i++)
{
 for(int j=0;j<imageSize;j++)
 {
  double distance=gaussianDistance(imageArray,xi,yi,i,j,halfPatchSWidth,imageSize); 
	 
	z+=exp(-(distance/square_sigma) ); 
 }	
}

return (z);	
}

//Υπολογισμός της διαφοράς |f(Ni)-f(Nj)|
//Χρησιμοποιούμε Gaussian Euclidean Distance
double gaussianDistance(double *imageArray,int xi,int yi,int xj,int yj,int halfPatchSWidth,int imageSize){

//Ο δείκτης i αναφέρεται στο pixel του οποίο υπολογίζουμε την νέα τιμή
//ενώ ο δείκτης j στα υπόλοιπα pixel με τα οποία γίνεται σύγκριση

int ai;
int bi;
int aj;
int bj;

int SumWeight=0;   //Άθροισμα βαρών
double distance=0; //Συνολική διαφορά γειτονιάς pixel
double diff=0;     //Διαφορά μεταξύ 2 pixel γειτόνων
 
for(int i=-halfPatchSWidth;i<=halfPatchSWidth;i++)
 {
  for(int j=-halfPatchSWidth;j<=halfPatchSWidth;j++)
  {
  ai=xi+i; 
  bi=yi+j;
  aj=xj+i;
  bj=yj+j;
  
if((aj<0)||(aj>=imageSize)) aj=xj-i; 
if((bj<0)||(bj>=imageSize)) bj=yj-j;

if((ai<0)||(ai>=imageSize)) ai=xi-i; 
if((bi<0)||(bi>=imageSize)) bi=yi-j;
  
   if (ai!=xi || bi!=yi) //Δεν συγκρίνουμε τα κεντρικά pixel την γειτονιάς που θεωρούμε οτι έχουν θόρυβο
   { 
    int weight=1/(MAX(ai-xi,xi-ai)+MAX(bi-yi,yi-bi));
    SumWeight+=weight;	
	diff=ImageArray(ai,bi)-ImageArray(aj,bj);
	distance+=diff*diff*weight;
   }
   
  }
 }

 return (distance/SumWeight);

}

//Συνάρτηση για την κλήση του προγράμματος απο το matlab
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){

//Πάρε τις διαστάσεις των ορισμάτων(πίνακας ή σταθερά) που μπαίνουν
//στην συνάστηση mexFunction 
const mwSize * dims = mxGetDimensions(prhs[0]);

int imageSize=(int) dims[0]; //Διάσταση του εισαγόμενου πίνακα
int halfWidth=mxGetScalar(prhs[1]); //Πάρε την σταθερά για το μήκος των patches
double sigma=mxGetScalar(prhs[2]); //Πάρε την σταθερά για την τιμή sigma

//Δημιούργησε πίνακα εξόδου στο κατάλληλο μήκος
plhs[0] = mxCreateDoubleMatrix(imageSize,imageSize,mxREAL);


//Αντιστοίχισε τον pointer εξοδου με μία μεταβλητή(pointer) arrayOut
double * arrayOut=mxGetPr(plhs[0]);

//Αντιστοίχισε και τον pointer εισώδου
double *array=mxGetPr(prhs[0]);


//Καλόυμε την συνάρτηση αποθορυβοποίησης
nonLocalMeans(array,arrayOut,halfWidth,imageSize,sigma);


}