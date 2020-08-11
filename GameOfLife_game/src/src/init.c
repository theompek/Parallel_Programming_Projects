#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <game-of-life.h>


/* generate random table */

void generate_table (int *board,int numOfRows,int N,float threshold,int seedRandom){

int   i, j;
unsigned seed;

//Η κάθε διεργασία πρέπει να έχει διαφορετικό seed για την παραγωγή των τυχαίων αριθμών και για να
//το επιτύχουμε αυτό εισάγουμε την μεταβλητή seedRandom η οποία συσχετίζεται με το ID της διεργασίας.
//Επιπλέον επειδή ορίζουμε και περιοχή με νήματα που τρέχουν παράλληλα θα παράγουν αρχικά ίδια 
//ακολουθία αριθμών άρα πρέπει το seed να μεταβάλλεται και απο τα νήματα.
//Τέλος έχουμε αντικαταστήσει την συνάρτηση rand με την rand_r η οποία είναι thread safe και μπορεί να
//χρησιμοποιηθεί σε παράλληλη περιοχή σε αντίθεση με την rand που δεν μπορεί.

#pragma omp parallel private(i,j,seed) shared(board,N,threshold,numOfRows)
 { 
  //Δημιουργούμε ένα τυχαίο seed για κάθε διεργασία και κάθε νήμα
  seed = time(NULL)+(((2^(seedRandom+2))*3+2*(omp_get_thread_num()+1))^1034);

  #pragma omp for schedule(static)
    for (i=0; i<numOfRows; i++) {
		for (j=0; j<N; j++) {
      Board(i,j) = ( (float)rand_r(&seed) / (float)RAND_MAX ) < threshold;
     }
   }
 
 }
}

//Δεν χρησιμοποιείται
/* set everthing to zero */
void initialize_board (int *board,int numOfRows, int N){
  int   i, j;
  
  for (i=0; i<numOfRows; i++)
    for (j=0; j<N; j++) 
      Board(i,j) = 0;
}


