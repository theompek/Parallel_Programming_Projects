#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <pthread.h>
#include "math.h"

#define MAXBINS 8

int count=0;//Μετρητής ενεργών νημάτων
extern int NUMTHREADS; //Αριθμός νημάτων
extern pthread_mutex_t radix_mutex;
extern int NN; //Εχει αποθηκευτέι ο αρχικός αριθμός των στοιχείων για 
			  //την πρώτη κλίση της Radix_Sort και την παραλληλοποίηση της πρώτης for του κώδικα
			  
typedef struct {//Δομή για την εισαγωγή μεταβλητών στα νήματα
  unsigned long int *morton_codes;
  unsigned long int  *sorted_morton_codes;
  unsigned int  *permutation_vector;
  unsigned int *index;
  unsigned int *level_record;
  int N;
  int population_threshold;
  int sft;
  int lv;
  
} radixworkers;

typedef struct {//Δομή για την εισαγωγή μεταβλητών στα νήματα
 unsigned long int *morton_codes;
 int sft;
 int HelpBins[8];
 int panw_orio;
 int katw_orio;
 
}Binsworkers;

//---------Κάνουμε απλώς μια δήλωση για να την ---------//
//-------------αναγνωρίζουν η συναρτήσεις---------------//
void truncated_radix_sort(unsigned long int *morton_codes, 
			  unsigned long int *sorted_morton_codes, 
			  unsigned int *permutation_vector,
			  unsigned int *index,
			  unsigned int *level_record,
			  int N, 
			  int population_threshold,
			  int sft, int lv);
//------------------------------------------------------//


void *parallelHelpFunction(void *q)
{
	
	radixworkers *t=q;
		
	truncated_radix_sort(
			  t->morton_codes, 
			  t->sorted_morton_codes, 
			  t->permutation_vector,
			  t->index,
			  t->level_record,
			  t->N, 
			  t->population_threshold,
			  t->sft,
			  t->lv);
	
	
	 pthread_mutex_unlock(&radix_mutex);
	 count--;
	 pthread_mutex_unlock(&radix_mutex);
	pthread_exit(NULL);
	
	
}

void * parallel_first_step(void *q)
{
	Binsworkers *t=q;
	int i=t->katw_orio;
	int Max=t->panw_orio;
	 for(int j=0;j<MAXBINS;j++){t->HelpBins[j]=0; }
   for(; i<Max; i++){
      unsigned int ii = (t->morton_codes[i]>>t->sft) & 0x07;
      t->HelpBins[ii]++;
    }
	
	pthread_exit(NULL);
}




inline void swap_long(unsigned long int **x, unsigned long int **y){

  unsigned long int *tmp;
  tmp = x[0];
  x[0] = y[0];
  y[0] = tmp;

}

inline void swap(unsigned int **x, unsigned int **y){

  unsigned int *tmp;
  tmp = x[0];
  x[0] = y[0];
  y[0] = tmp;

}



/*=====================================================================================*/
/*=====================================================================================*/
void truncated_radix_sort(unsigned long int *morton_codes, 
			  unsigned long int *sorted_morton_codes, 
			  unsigned int *permutation_vector,
			  unsigned int *index,
			  unsigned int *level_record,
			  int N, 
			  int population_threshold,
			  int sft, int lv){
				  
  int BinSizes[MAXBINS] = {0};
  int BinCursor[MAXBINS] = {0};
  unsigned int *tmp_ptr;
  unsigned long int *tmp_code;
  
   if(N<=0){

    // return;
  }
  else if(N<=population_threshold || sft < 0) { // Base case. The node is a leaf

    level_record[0] = lv; // record the level of the node
    memcpy(permutation_vector, index, N*sizeof(unsigned int)); // Copy the pernutation vector
    memcpy(sorted_morton_codes, morton_codes, N*sizeof(unsigned long int)); // Copy the Morton codes 

    // return;
  }
  else{
	  
    level_record[0] = lv;
 
/*=============================================================*/
/*=============================================================*/
 
 
if((count<NUMTHREADS-1)&(N>(NN/8))&(N>100000)) //Κάνουμε έναν πρώτο βασικό έλεγχο αν υπάρχουν νήματα>1 και ειναι ικανοποιητικό το Ν(αρκετή δουλειά)
{											//Η παραλληλοποίηση μας ενδιαφέρει κυρίως για την πρώτη κλίση της Radix ή για περίπτωση ασυμμετρίας αρα:N>(NN/8)

     pthread_mutex_unlock(&radix_mutex);
	 
	 int numworkers=NUMTHREADS-count;//Aπο τον έλεγχο του count μέσα στην προηγούμενη if μέχρι αυτο το σημέιο
	 count+=numworkers;              //το count μπορεί να έχει αλλάξει οπότε το ξανά υπολογίζουμε με mutex
	 
	 pthread_mutex_unlock(&radix_mutex);	
	 


if(!numworkers)//Εάν δεν έχουμε κανένα νήμα τότε τρέχουμε σειριακά το πρόγραμμα
 {
	  for(int j=0; j<N; j++){
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      BinSizes[ii]++;
    }
  }
	else{
	
 pthread_t *threadsA;
 threadsA=(pthread_t *) malloc(numworkers*sizeof(pthread_t));
 
 Binsworkers *callthreads;
 callthreads=(Binsworkers *) malloc((numworkers+2)*sizeof(Binsworkers));//Δυο παραπάνω,ενα για αρχικοποίηση,και το άλλο για το master thread

 //Αρχικοποιούμε τις τιμές των ορίων    
 callthreads[0].panw_orio=0;
 callthreads[0].katw_orio=0;

float distance=(float) N/(numworkers+1);

for(int i=1; i<=numworkers; i++){
	
	callthreads[i].morton_codes=morton_codes;
    callthreads[i].sft=sft;
	callthreads[i].katw_orio=callthreads[i-1].panw_orio;
	callthreads[i].panw_orio=(int) ceil(i*distance);
	pthread_create(&threadsA[i-1], NULL,parallel_first_step ,&callthreads[i]) ;
		
      }
	  
 //Η τελευταία επανάληψη γίνεται απο το νήμα αρχηγό επειδή δεν χρειάζεται να καλέσει άλλο
 //νήμα,διοτι το ίδιο δεν θα έχει δουλειά να κάνει και θα περιμένει
	
 for(int j=0;j<MAXBINS;j++){callthreads[numworkers+1].HelpBins[j]=0; }
 
   for(int i=callthreads[numworkers].panw_orio; i<N; i++){
      unsigned int ii = (morton_codes[i]>>sft) & 0x07;
      callthreads[numworkers+1].HelpBins[ii]++;
    }

	   
 
 for(int i=0; i<numworkers; i++){
 pthread_join(threadsA[i],NULL);
 }
 //Πρόσθεση των αποτελεσμάτων απο κάθε νήμα στον BinSizes[]
 for(int i=1; i<=numworkers+1; i++)                  //Ο αριμθός των επαναλήψεων εξαρτάται απο τον αριθμό νημάτων ο οποίος
 {                                                   //ειναι μικρός οπότε δεν έχουμε πρόβλημα καθυστέρησης
	 for(int j=0;j<MAXBINS;j++)
	 {
		BinSizes[j]+=callthreads[i].HelpBins[j];
		
	 }
	 	 
 }
 
 free(threadsA);
 free(callthreads);
	}
	
  pthread_mutex_unlock(&radix_mutex);	
  count-=numworkers;
  pthread_mutex_unlock(&radix_mutex);
	
}
 else
  {
	  // Find which child each point belongs to 
    for(int j=0; j<N; j++){
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      BinSizes[ii]++;
    }
	 
	 
  }
 
 /*=============================================================*/
 /*=============================================================*/

    // scan prefix (must change this code)  
    int offset = 0;
    for(int i=0; i<MAXBINS; i++){
      int ss = BinSizes[i];
      BinCursor[i] = offset;
      offset += ss;
      BinSizes[i] = offset;
    }
    
    for(int j=0; j<N; j++){
	
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      permutation_vector[BinCursor[ii]] = index[j];
      sorted_morton_codes[BinCursor[ii]] = morton_codes[j];
      BinCursor[ii]++;
	
    }
    
	
	
    //swap the index pointers  
    swap(&index, &permutation_vector);

    //swap the code pointers 
    swap_long(&morton_codes, &sorted_morton_codes);


    /* Call the function recursively to split the lower levels */
	
	
if(N>100000) //Αν υπάρχει δουλεία τότε να γίνει έλεχγος για δημιουργεία νέων νημάτων
{
    pthread_t threads[MAXBINS];
	
	radixworkers *helpme;//Δομή για να περάσουμε τα δεδομένα
	helpme=(radixworkers *)malloc(8*sizeof(radixworkers));
	
	int sub[8]={0,0,0,0,0,0,0,0};	//Είναι για να ελέγχουμε ποία νήματα καλέσαμε παράλληλα
	
for (int i = 0; i<MAXBINS-1; i++) //Εκτελούμε μόνο της 7 απο τις 8 επαναλήψεις,η τελευταία θα εκτελεστεί απο το "master" νήμα
{     	
  int offset = (i>0) ? BinSizes[i-1] : 0;
  int size = BinSizes[i] - offset;
  
    if(count<NUMTHREADS&size>10000)			//Αν υπάρχει ελεύθερο νήμα και υπάρχει και πολύ 
	{	                           			//δουλειά τότε κάλεσε νέο νήμα για να την κάνει
		helpme[i].morton_codes=&(morton_codes[(i>0) ? BinSizes[i-1] : 0]);
		helpme[i].sorted_morton_codes=&(sorted_morton_codes[(i>0) ? BinSizes[i-1] : 0]);
		helpme[i].permutation_vector=&permutation_vector[(i>0) ? BinSizes[i-1] : 0];
		helpme[i].index=&(index[(i>0) ? BinSizes[i-1] : 0]);
		helpme[i].level_record=&(level_record[(i>0) ? BinSizes[i-1] : 0]);
		helpme[i].N=(BinSizes[i] - ((i>0) ? BinSizes[i-1] : 0));
		helpme[i].population_threshold=population_threshold;
		helpme[i].sft=sft-3;
		helpme[i].lv=lv+1;
		sub[i]=1;
	 pthread_mutex_unlock(&radix_mutex);	
	 count++;
	 pthread_mutex_unlock(&radix_mutex);	

   	pthread_create (&threads[i], NULL, parallelHelpFunction,&helpme[i]);
			
  }
  else{                                  //Αλλιώς την κάνει την δουλειά μόνο του το νήμα αρχηγός
	   
      truncated_radix_sort(&morton_codes[offset], 
			   &sorted_morton_codes[offset], 
			   &permutation_vector[offset], 
			   &index[offset], &level_record[offset], 
			   size, 
			   population_threshold,
			   sft-3, lv+1);		     
  }

	}
	//Για την τελευαία επανάληψη δεν χρειάζεται να καλέσει άλλο νήμα για να 
	//την κάνει διότι το ίδιο δεν θα έχει δουλειά να κάνει και θα περιμένει
	int offset = (7>0) ? BinSizes[6] : 0;
    int size = BinSizes[7] - offset;
	
	truncated_radix_sort(&morton_codes[offset], 
			   &sorted_morton_codes[offset], 
			   &permutation_vector[offset], 
			   &index[offset], &level_record[offset], 
			   size, 
			   population_threshold,
			   sft-3, lv+1);
	
     for (int i = 0; i<MAXBINS; i++) 
     {
		if(sub[i]) pthread_join(threads[i],NULL);
     }
	  free(helpme);

  }
   else
   {
	   for(int i=0; i<MAXBINS; i++)
	   {
      int offset = (i>0) ? BinSizes[i-1] : 0;
      int size = BinSizes[i] - offset;


      truncated_radix_sort(&morton_codes[offset], 
			                &sorted_morton_codes[offset], 
			                &permutation_vector[offset], 
							&index[offset], &level_record[offset], 
							size, 
							population_threshold,
							sft-3, lv+1);
       }
  
   }
   
  }
}