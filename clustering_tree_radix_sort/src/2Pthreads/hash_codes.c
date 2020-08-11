#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "float.h"
#include <pthread.h>

#define DIM 3

extern int NUMTHREADS;//Αριθμός νημάτων

typedef struct {
 unsigned int *codes;
 float *X;
 float *low;
 float step;
 int panw_orio;
 int katw_orio;

} hashworkers;


inline unsigned int compute_code(float x, float low, float step){

  return floor((x - low) / step);//(επιστρέφει την μεγαλύτερη(ή ιση) ακέραια τιμη μεσα στην παρενθεση)συναρτηση της βιβλιοθηκης math.h

}
/*===============================================================================================
                                  Παραλληλοποιημένη περιοχή
  ===============================================================================================*/
void *parallelquantize(void *k)//Βοηθητική Συνάρτηση για την pthread_create
{
	hashworkers *t=k;
	int i=t->katw_orio;
	int Max=t->panw_orio;
	
  for(; i<Max; i++)
  {
	
   for(int j=0; j<DIM; j++)
	 { 
      t->codes[i*DIM + j] = compute_code(t->X[i*DIM + j], t->low[j], t->step); 
     } 
  }

				
}
	

/* Function that does the quantization */
void quantize(unsigned int *codes, float *X, float *low, float step, int N){

if(N>200000)
{
     int numthread;    
if(N<NUMTHREADS) {numthread=N;}//Εαν ο αριθμός των αριθμοςνημάτων>N τότε ορισε αριθμοςνημάτων=N;
else{numthread=NUMTHREADS;}	

//Αντικείμενα για τα νήματα
pthread_t *threads;
threads=(pthread_t *) malloc(numthread*sizeof(pthread_t));

//Βοηθητικό structor δομή για να περάσουμε τα δεδομένα στην pthread_create
hashworkers *callthreads;
callthreads=(hashworkers *) malloc((numthread+2)*sizeof(hashworkers));//Δυο παραπάνω,ενα για αρχικοποίηση,
																	  //και το άλλο για το master thread

//Αρχικοποίηση τιμών
callthreads[0].panw_orio=0;
callthreads[0].katw_orio=0;

//Υπολογίζουμε το μήκος των for loop που θα τρέχει κάθε νήμα
float distance=(float) N/(numthread+1);

for(int i=1; i<=numthread; i++){
	//Εισαγωγή παράμετρων
	callthreads[i].codes=codes;
    callthreads[i].X=X;
    callthreads[i].low=low;
    callthreads[i].step=step;
	//Υπολίζουμε τα όρια της foor μέσα στην parallelquantize
	callthreads[i].katw_orio=callthreads[i-1].panw_orio;
	callthreads[i].panw_orio=(int) ceil(i*distance);//Η ceil μετατρέπει στον αμέσως μεγαλύτερο
	
	
	pthread_create(&threads[i-1], NULL,parallelquantize ,&callthreads[i]) ;

}
//Η τελευταία επανάληψη γίνεται απο το νήμα αρχηγό επειδή δεν χρειάζεται να καλέσει άλλο
 //νήμα,διοτι το ίδιο δεν θα έχει δουλειά να κάνει και θα περιμένει
for(int i=callthreads[numthread].panw_orio; i<N; i++)
  {
	
   for(int j=0; j<DIM; j++)
	 { 
      codes[i*DIM + j] = compute_code(X[i*DIM + j], low[j], step); 
     } 
  }
 
 for(int i=0; i<numthread; i++){
 pthread_join(threads[i],NULL);
 }
 
 free(threads);
 free(callthreads);
}
else
  {
	for(int i=0; i<N; i++){
    for(int j=0; j<DIM; j++){
      codes[i*DIM + j] = compute_code(X[i*DIM + j], low[j], step); 
    }
  }
	
  }

}
/*===============================================================================================
                              Τελος παραλληλοποιημένης περιοχής
===============================================================================================*/

float max_range(float *x){//βρισκει το μεγιστο απο εναν πινακα τυπου float

  float max = -FLT_MAX;
  for(int i=0; i<DIM; i++){
    if(max<x[i]){
      max = x[i];
    }
  }

  return max;

}

void compute_hash_codes(unsigned int *codes, float *X, int N,int nbins, float *min,float *max){
  
  float range[DIM];
  float qstep;

  for(int i=0; i<DIM; i++){
    range[i] = fabs(max[i] - min[i]); // The range of the data(Το εύρος των δεδομένων)
    range[i] += 0.01*range[i]; // Add somthing small to avoid having points exactly at the boundaries (προσθετουμε κατι μικρο για να αποφυγουμε τα ορια)
  }

  qstep = max_range(range) / nbins; // The quantization step (Το βήμα κβαντισμού)
  
  quantize(codes, X, min, qstep, N); // Function that does the quantization(Λειτουργία που κάνει το κβαντισμού)

}



