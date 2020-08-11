#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "omp.h"


#define MAXBINS 8

int count=0; //Μεταβλητή για την μέτρηση των ενεργών threads κατα την διάρκεια εκτέλεσης
extern int numthreads;
extern int NN; //Εχει αποθηκευτέι ο αρχικός αριθμός των στοιχείων για 
			  //την πρώτη κλίση της Radix_Sort και την παραλληλοποίηση της πρώτης for του κώδικα

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

    return;
  }
  else if(N<=population_threshold || sft < 0) { // Base case. The node is a leaf

    level_record[0] = lv; // record the level of the node
    memcpy(permutation_vector, index, N*sizeof(unsigned int)); // Copy the pernutation vector
    memcpy(sorted_morton_codes, morton_codes, N*sizeof(unsigned long int)); // Copy the Morton codes 

    return;
  }
 else{
      level_record[0] = lv;
	  
  if((N>(NN/8))&N>100000)                            //Εαν εχουμε μεγάλο Ν και συμφέρει η παραλληλοποίηση τοτε κανε την αλλιώς κανε την σειριακά
  {                                               //Η παραλληλοποίηση μας ενδιαφέρει κυρίως για την πρώτη κλίση της Radix ή για περίπτωση ασυμμετρίας 
											//στην κατανομή αρα:N>(NN/8)
  
    int HelpArray[MAXBINS]={0,0,0,0,0,0,0,0};    //Ορίζουμε πίνακα για την καταμέτρηση
    #pragma omp parallel firstprivate(HelpArray)  //Δημιουργόυμε παράλληλη περιοχή και αντίγραφα πινάκων(firstprivate(HelpArray)) για την μέτρηση
    {
       
   // Find which child each point belongs to 
    #pragma omp for schedule(static) nowait      //Γίνεται η καταμέτρηση απο τα νήματα
     for(int j=0; j<N; j++)
     {
      HelpArray[(morton_codes[j]>>sft) & 0x07]++;
     }
	
    #pragma omp critical                         //Προσθέτουμε τα αποτελέσματα κάθε νήματος στον πίνακα BinSizes[]
    {  
	
     for(int i = 0; i < MAXBINS; i++) 
     {       
      BinSizes[i]+=HelpArray[i];
     }
    }
	  
   }
 } 	
  else{                  //Ειδικά στην αναδρομή,σε κάθε κλίση της truncated_radix_sort τα αντιστοιχα Ν μειώνονται δραμάτικα
		                 //και η σειριακή εκτέλεση ειναι απαραίτητη
		
	for(int j=0; j<N; j++){
     unsigned int ii = (morton_codes[j]>>sft) & 0x07;
     BinSizes[ii]++;
     }
				
  }
	
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
	
	
	 #pragma omp flush (count)
	int diafora=numthreads-count;                         //Υπολόγισε πόσα νήματα είναι διαθέσημα
	
if((diafora>0)&(N>100000)){                               //Αν υπάρχουν διαθέσημα νήματα και έχουμε και ικανοποιητικό αριθμό Ν τότε 
	
	if(diafora>8) {diafora=8;}
	
	#pragma omp parallel for schedule(static)                
    for(int i=0; i<diafora; i++){                         //τρέξε οσα νήματα ειναι διαθέσιμα ενώ
      //Καταργούμε τα offset και size για να αποφύγουμε τυχόν ταυτόχρονη εγραφή απο  
	  //διαφορετικά νήματα στην ίδια θέση και γενικά περαιτέρω πολυπλότητα στο πρόγραμμα
      // int offset = (i>0) ? BinSizes[i-1] : 0; 
      //int size = BinSizes[i] - ((i>0) ? BinSizes[i-1] : 0);
	  
      #pragma omp atomic
      count++;
	 
      truncated_radix_sort(&morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			   &sorted_morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			   &permutation_vector[(i>0) ? BinSizes[i-1] : 0], 
			   &index[(i>0) ? BinSizes[i-1] : 0], &level_record[(i>0) ? BinSizes[i-1] : 0], 
			   BinSizes[i] - ((i>0) ? BinSizes[i-1] : 0), 
			   population_threshold,
			   sft-3, lv+1);

     #pragma omp atomic
      count--;		
		   
        } 

     for(int i=diafora; i<MAXBINS; i++){                  //τίς υπόλοιπες ανάδρομές κάντες σειριακά μέχρι να ελευθερωθούν νήματα
		 
		 truncated_radix_sort(&morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			   &sorted_morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			   &permutation_vector[(i>0) ? BinSizes[i-1] : 0], 
			   &index[(i>0) ? BinSizes[i-1] : 0], &level_record[(i>0) ? BinSizes[i-1] : 0], 
			   BinSizes[i] - ((i>0) ? BinSizes[i-1] : 0), 
			   population_threshold,
			   sft-3, lv+1); 
	 }
 
}
 else{
   for(int i=0; i<MAXBINS; i++){
             
         truncated_radix_sort(&morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			      &sorted_morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			      &permutation_vector[(i>0) ? BinSizes[i-1] : 0], 
			      &index[(i>0) ? BinSizes[i-1] : 0], &level_record[(i>0) ? BinSizes[i-1] : 0], 
			      BinSizes[i] - ((i>0) ? BinSizes[i-1] : 0), 
			      population_threshold,
			      sft-3, lv+1);
              }     
        }  


  } 
  
}

