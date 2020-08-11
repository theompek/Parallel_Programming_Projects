#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include <pthread.h>


#define DIM 3

extern int NUMTHREADS;//Αριθμός νημάτων

typedef struct {
 unsigned int *permutation_vector;	
 float *Y;
 float *X;
 int panw_orio;
 int katw_orio;
 
}data_rearrangementworkers;

void *paralleldata_rearrangement(void *k)
{
	
	data_rearrangementworkers *t=k;
	int i=t->katw_orio;
	int Max=t->panw_orio;
	
	 for(; i<Max; i++)
  {                                                      
      memcpy(&(t->Y[i*DIM]), &(t->X[(t->permutation_vector[i])*DIM]), DIM*sizeof(float));//η memcpy(α,β,γ) αντιγραφει τους πρωτους γ χαρακτηρες του β στο α
  }
	
	
	
	
}


void data_rearrangement(float *Y, float *X,unsigned int *permutation_vector,int N)
{
if(N>200000)
{	
	 int numthread;  
	 
if(N<NUMTHREADS) {numthread=N;}//Ελέγχουμε να μην ξεπερνά ο αριθμός των νημάτων τα στοιχεία Ν
else{numthread=NUMTHREADS;}		   
		   
//Αντικείμενα για τα νήματα	   
pthread_t *threads;
threads=(pthread_t *) malloc(numthread*sizeof(pthread_t));

//Δημιουργία αντικειμένων struct για να εισάγουμε στα νήματα που τρέχουμε
//τις απαραίτης μεταβλητές χωρίς να δημιουργούμε κρίσιμες καταστάσεις
data_rearrangementworkers *callthreads;
callthreads=(data_rearrangementworkers *) malloc((numthread+2)*sizeof(data_rearrangementworkers));//Δυο παραπάνω,ενα για αρχικοποίηση,
																								  //και το άλλο για το master thread
   
//Αρχικοποιούμε τα πρώτα στοιχεία  
callthreads[0].panw_orio=0;
callthreads[0].katw_orio=0;

//Υπολογίζουμε το μήκος των for loop που θα τρέχει κάθε νήμα
float distance=(float) N/(numthread+1);

for(int i=1; i<=numthread; i++){
	
	callthreads[i].Y=Y;
    callthreads[i].X=X;
    callthreads[i].permutation_vector=permutation_vector;
   	//Υπολίζουμε τα όρια της foor μέσα στην paralleldata_rearrangement
	callthreads[i].katw_orio=callthreads[i-1].panw_orio;
	callthreads[i].panw_orio=(int) ceil(i*distance);//Η μετατρέπει στον αμέσως μεγαλύτερο
	
	
	pthread_create(&threads[i-1], NULL,paralleldata_rearrangement ,&callthreads[i]) ;
	
      }
	  
 //Η τελευταία επανάληψη γίνεται απο το νήμα αρχηγό επειδή δεν χρειάζεται να καλέσει άλλο
 //νήμα,διοτι το ίδιο δεν θα έχει δουλειά να κάνει και θα περιμένει  
 for(int i=callthreads[numthread].panw_orio; i<N; i++)
  {                                                      
      memcpy(&(Y[i*DIM]), &(X[(permutation_vector[i])*DIM]), DIM*sizeof(float));
	  
  }
  
 for(int i=0; i<numthread; i++){
 pthread_join(threads[i],NULL);
 }
 
 free(threads);
 free(callthreads);
}
else{
	for(int i=0; i<N; i++)
  {                                                       
      memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));
  }
	
}
}	
	
	
	
 




