#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include <pthread.h>

#define DIM 3

extern int NUMTHREADS; //Αριθμός νημάτων

typedef struct {//Δομή για την εισαγωγή μεταβλητών στα νήματα
 unsigned long int *mcodes;
 unsigned int *codes;
 int max_level;
 int panw_orio;
 int katw_orio;
 
}mortonworkers;


inline unsigned long int splitBy3(unsigned int a){
    unsigned long int x = a & 0x1fffff; // we only look at the first 21 bits
    x = (x | x << 32) & 0x1f00000000ffff;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
    x = (x | x << 16) & 0x1f0000ff0000ff;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
    x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
    x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
    x = (x | x << 2) & 0x1249249249249249;
    return x;
}


inline unsigned long int mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z){//Συναρτηση ενθεσης αριθμων
    unsigned long int answer;
    answer = splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
    return answer;
}

/*===============================================================*/
/*===============================================================*/

void *parallelmorton_encoding(void *k)
{ 
	mortonworkers *t=k;
	int i=t->katw_orio;
	int Max=t->panw_orio;
	
   for(; i<Max; i++)
   {	
       // Compute the morton codes from the hash codes using the magicbits mathod
       t->mcodes[i] = mortonEncode_magicbits( t->codes[i*DIM], t->codes[i*DIM + 1], t->codes[i*DIM + 2] );  
   }

		
}

/* The function that transform the morton codes into hash codes */ 
void morton_encoding(unsigned long int *mcodes, unsigned int *codes, int N, int max_level){
if(N>200000)  
{
int numthread;    

if(N<NUMTHREADS) {numthread=N;} //Ελέγχουμε να μην ξεπερνά ο αριθμός των νημάτων τα στοιχεία Ν
else{numthread=NUMTHREADS;}		   
		   
//Αντικείμενα για τα νήματα		   
pthread_t *threads;       
threads=(pthread_t *) malloc(numthread*sizeof(pthread_t));     

//Δημιουργία αντικειμένων struct για να εισάγουμε στα νήματα που τρέχουμε
//τις απαραίτης μεταβλητές χωρίς να δημιουργούμε κρίσιμες καταστάσεις
mortonworkers *callthreads;   
callthreads=(mortonworkers *) malloc((numthread+2)*sizeof(mortonworkers));//Δυο παραπάνω,ενα για αρχικοποίηση,και το άλλο για το master thread
   
//Αρχικοποιούμε τα πρώτα στοιχεία 
callthreads[0].panw_orio=0;
callthreads[0].katw_orio=0;

//Υπολογίζουμε το μήκος των for loop που θα τρέχει κάθε νήμα
float distance=(float) N/(numthread+1);

for(int i=1; i<=numthread; i++){
	
	callthreads[i].mcodes=mcodes;
    callthreads[i].codes=codes;
	//Υπολίζουμε τα όρια της foor μέσα στην	parallelmorton_encoding
	callthreads[i].katw_orio=callthreads[i-1].panw_orio;
	callthreads[i].panw_orio=(int) ceil(i*distance);
	
	
	pthread_create(&threads[i-1], NULL,parallelmorton_encoding ,&callthreads[i]) ;
	
      }
	  
//Η τελευταία επανάληψη γίνεται απο το νήμα αρχηγό επειδή δεν χρειάζεται να καλέσει άλλο
 //νήμα,διοτι το ίδιο δεν θα έχει δουλειά να κάνει και θα περιμένει	 
for(int i=callthreads[numthread].panw_orio; i<N; i++)
   {	
       // Compute the morton codes from the hash codes using the magicbits mathod
       mcodes[i] = mortonEncode_magicbits(codes[i*DIM],codes[i*DIM + 1],codes[i*DIM + 2] );  
   }
	  
	   
 for(int i=0; i<numthread; i++){
 pthread_join(threads[i],NULL);
 }
 
 free(threads);
 free(callthreads);
}
 else
 {
   for(int i=0; i<N; i++)
      {
       // Compute the morton codes from the hash codes using the magicbits mathod
       mcodes[i] = mortonEncode_magicbits(codes[i*DIM], codes[i*DIM + 1], codes[i*DIM + 2]);
      }
	
 }
}  
  
  
/*===============================================================*/
/*===============================================================*/



