#include "stdio.h"
#include "stdlib.h"
#include "sys/time.h"
#include "utils.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>

#define DIM 3

int numthreads;
int NN;


int main(int argc, char** argv){

  // Time counting variables //Μεταβλητές για μετρηση χρονου
  struct timeval startwtime, endwtime;
  
/*--------------------------------Προειδοποίηση σε περίπτωση έλλειψης δεδομένων------------------------------------*/
  if (argc != 7) { // Check if the command line arguments are correct 
    printf("Usage: %s N dist pop rep P\n"
	   "where\n"
	   "       N    : number of points\n"
	   "       dist : distribution code (0-cube, 1-sphere)\n"
	   "       pop  : population threshold\n"
	   "       rep  : repetitions\n"
	   "       L    : maximum tree height.\n"
	   "numthreads  : number of threads <-- <-- <--type (const char) not (int)\n", argv[0]);
    return (1);
  }
/*-----------------------------------------------------------------------------------------------------------------*/

/*-------------------------Ανάθεση των τιμών που δόθηκαν,στις αντίστοιχες μεταβλητές-----------------------------*/
/*--------------------------------------------του προγράμματος---------------------------------------------------*/
  // Input command line arguments
  int N = atoi(argv[1]); // Number of points(αριθμος σωματιδιων)
  int dist = atoi(argv[2]); // Distribution identifier (αναγνωριστικό διανομής για 0 εχουμε κυβο και για 1 εχουμε σφαιρα,κατα default ειναι σφαιρα )πληροφοριες στο αρχειο datasets.c
  int population_threshold = atoi(argv[3]); // populatiton threshold(όριο πληθυσμού)
  int repeat = atoi(argv[4]); // number of independent runs(αριθμό των ανεξάρτητων κυκλων)
  int maxlev = atoi(argv[5]); // maximum tree height(μεγιστο υψος δεντρου)
  const char *char_numthreads=&argv[6][0]; //Ορισμός του αριθμού των threads
/*-----------------------------------------------------------------------------------------------------------------*/

  NN=N;//Η ΝΝ χρησιμοποιείται για τον έλεγχο του αριθμού των σωμάτων στην radix ωστέ να ανοίγουν νήματα όταν χρειάζεται και όχι συνέχεια 
 
cilk::reducer_opadd<int> count(0); //Ορισμό reducer για την count μεταβλητη στην Radix_Sort
   
__cilkrts_set_param("nworkers",char_numthreads); //Ορισμός νημάτων

numthreads=__cilkrts_get_nworkers();
 
 
 printf("Running for %d particles with maximum height %d and number of threads %d:\n", N, maxlev,numthreads);
  
  
/*-----------------Δημιουργεία χώρου για την αποθήκευσει σχετικών πινάκων----------------------*/
/*---------------------------------------------------------------------------------------------*/
  float *X = (float *) malloc(N*DIM*sizeof(float));//Περιέχει τις συντεταγμένες
  float *Y = (float *) malloc(N*DIM*sizeof(float));

  unsigned int *hash_codes = (unsigned int *) malloc(DIM*N*sizeof(unsigned int));
  unsigned long int *morton_codes = (unsigned long int *) malloc(N*sizeof(unsigned long int));
  unsigned long int *sorted_morton_codes = (unsigned long int *) malloc(N*sizeof(unsigned long int));
  unsigned int *permutation_vector = (unsigned int *) malloc(N*sizeof(unsigned int)); 
  unsigned int *index = (unsigned int *) malloc(N*sizeof(unsigned int));
  unsigned int *level_record = (unsigned int *) calloc(N,sizeof(unsigned int)); // record of the leaf of the tree and their level
/*-----------------------------------------------------------------------------------------------*/

 


     // initialize the index
     for(int i=0; i<N; i++){        
      index[i] = i;
      }


 
/*-------------------- Generate a 3-dimensional data distribution-----------------------*/
/*---------------Παραγωγή συντεταγμένων και εισαγωγή στον πίνακα Χ----------------------*/
/*-----------ανάλογα με την διαταξη που έχουμε στον χώρο(κύβος,σφαίρα)------------------*/
 
    create_dataset(X, N, dist);   //πληροφοριες στο αρχειο datasets.c
  
/*--------------------------------------------------------------------------------------*/



/*-------------------------------Find the boundaries of the space----------------------------------------*/
/*----------Βρίσκουμε τα όρια του χώρου εντοπίζοντας το έλαχιστο και μέγιστο των συντεταγμένων-----------*/
 
  float max[DIM], min[DIM];
  find_max(max, X, N);//Στο αρχειο general_functions
  find_min(min, X, N);//Στο αρχειο general_functions
  
/*-------------------------------------------------------------------------------------------------------*/



  int nbins = (1 << maxlev); // maximum number of boxes at the leaf level

  
  //Independent runs|Τρέχουμε τόν κώδικα repeat φορές
  
  for(int it = 0; it<repeat; it++){
	  
/*======================================Ηash codes=================================================*/
    gettimeofday (&startwtime, NULL); //Ξεκινάει η μέτρηση χρόνου
  
/*A*/   compute_hash_codes(hash_codes, X, N, nbins, min, max); // compute the hash codes

    gettimeofday (&endwtime, NULL);//Τελειώνει η μέτρηση χρόνου

    double hash_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
    
    printf("Time to compute the hash codes            : %fs\n", hash_time); //Χρόνος εκτέλεσης

/*======================================Morton codes=================================================*/
    gettimeofday (&startwtime, NULL);

/*B*/   morton_encoding(morton_codes, hash_codes, N, maxlev); // computes the Morton codes of the particles

    gettimeofday (&endwtime, NULL);


    double morton_encoding_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);


    printf("Time to compute the morton encoding       : %fs\n", morton_encoding_time);

/*======================================Ρadix sort==================================================*/
    gettimeofday (&startwtime, NULL); 

    // Truncated msd radix sort
/*C*/   truncated_radix_sort(morton_codes, sorted_morton_codes, 
			 permutation_vector, 
			 index, level_record, N, 
			 population_threshold, 3*(maxlev-1), 0,count);

    gettimeofday (&endwtime, NULL);

    double sort_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

    printf("Time for the truncated radix sort         : %fs\n", sort_time);
/*================================Data rearrangement=================================================*/
    gettimeofday (&startwtime, NULL); 

/*D*/  data_rearrangement(Y, X, permutation_vector, N);

    gettimeofday (&endwtime, NULL);


    double rearrange_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
    

    printf("Time to rearrange the particles in memory : %fs\n", rearrange_time);
/*===================================================================================================*/


/*=========================Επαλήθευση των αποτελεσμάτων και έλεγχος δεδομένων=============================*/
    /* The following code is for verification */ 
    // Check if every point is assigned to one leaf of the tree
    int pass = check_index(permutation_vector, N); 

    if(pass){
      printf("Index test PASS\n");
    }
    else{
      printf("Index test FAIL\n");
    }

    // Check is all particles that are in the same box have the same encoding. 
    pass = check_codes(Y, sorted_morton_codes, 
		       level_record, N, maxlev);

    if(pass){
      printf("Encoding test PASS\n");
    }
    else{
      printf("Encoding test FAIL\n");
    }
/*===================================================================================================*/
  }

  /* clear memory */
  free(X);
  free(Y);
  free(hash_codes);
  free(morton_codes);
  free(sorted_morton_codes);
  free(permutation_vector);
  free(index);
  free(level_record);
}





