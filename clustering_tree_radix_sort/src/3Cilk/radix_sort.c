#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "math.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>

#define MAXBINS 8

extern int numthreads;
extern int NN; //Εχει αποθηκευτέι ο αρχικός αριθμός των στοιχείων για 
			  //την πρώτη κλίση της Radix_Sort και την παραλληλοποίηση της πρώτης for του κώδικα

typedef struct{	
int bins[8];	
int katw_orio;
int panw_orio;
}workerBins;



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
			  int sft, int lv,
			  cilk::reducer_opadd<int> &count){

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
    // Find which child each point belongs to 
		
 if(N>1000000&(N>(NN/8))) //Το συγκεκριμένο κομμάτι μας ενδιαφέρει κυριώς στης πρώτη κλίσης της Radix ή σε περίπτωση
{						  //ασύμμετρης κατανομής άρα εχουμε: N>(NN/8) και εκτελούμε παράλληλα για μεγάλη δουλεία: Ν>1000000

workerBins *helpStructor;//Ορίζουμε δομές για ευκολία στην διαχείρηση 
helpStructor=(workerBins *)malloc((numthreads+1)*sizeof(workerBins));

float distance=(float) N/numthreads;

helpStructor[0].katw_orio=0;
helpStructor[0].panw_orio=0;

//Αρχηκοποιούμε τα δεδομένα και ορίζουμε τα όρια των foor loop
	for(int i=1; i<=numthreads;i++)
	{
	 helpStructor[i].katw_orio=helpStructor[i-1].panw_orio;
	 helpStructor[i].panw_orio=(int) ceil(i*distance);
		for(int j=0;j<MAXBINS;j++)
		{
			helpStructor[i].bins[j]=0;
		}
	}
	//Η τελευταία επανάληψη γίνεται απέξω απο την for για να αποφύγουμε την χρήση της ceil(i*distance); και τυχόν 
	//σφάλματα απο στρογγυλοποιήσεις που θα έχουν ως αποτέλεσμα στη τελευτάια επανάληψη να μην είναι το πάνω οριο το Ν 
	 helpStructor[numthreads].katw_orio=helpStructor[numthreads-1].panw_orio;
	 helpStructor[numthreads].panw_orio=N;
	
	//Ανόιγουμε παράλληλα όσα νήματα είναι διαθέσημα 
	
	cilk_for(int i=1;i<=numthreads;i++)
	{
		
     for(int j=helpStructor[i].katw_orio; j<helpStructor[i].panw_orio; j++)
	 {
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
	  
      helpStructor[i].bins[ii]++;
     }
	
	}
	//Προσθέτουμε ολα τα αποτελέσματα στον πίνακα BinSizes
	for(int i=1;i<=numthreads;i++)
	{
		for(int j=0;j<MAXBINS;j++)
		{
			BinSizes[j] +=helpStructor[i].bins[j];
		}
		
	}
	
	
}
 else
   { 
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

    /* Call the function recursively to split the lower levels */
   
   
  int diafora=numthreads-count.get_value();    
   
  if((diafora>1)&(N>=(NN/8))&(N>100000)){        //Αν μπορούν να ανοίξουν παράλληλα νήματα και ειναι
	                                          //περισσοτερα απο 1 και υπάρχει και αρκετή δουλεία τοτε
	if(diafora>8){diafora=8;}
	
	cilk_for(int i=0; i<diafora; i++){     //ανοιξε όσα μπορείς παράλληλα
	
      count++;
	 	
      truncated_radix_sort(&morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			   &sorted_morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			   &permutation_vector[(i>0) ? BinSizes[i-1] : 0], 
			   &index[(i>0) ? BinSizes[i-1] : 0], &level_record[(i>0) ? BinSizes[i-1] : 0], 
			   BinSizes[i] - ((i>0) ? BinSizes[i-1] : 0), 
			   population_threshold,
			   sft-3, lv+1,count);

      count--;		
		 
        } 
 for(int i=diafora; i<MAXBINS; i++){           //ενώ τις υπόλοιπες κλίσεις κάντες σειριακά
		 
		 truncated_radix_sort(&morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			   &sorted_morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			   &permutation_vector[(i>0) ? BinSizes[i-1] : 0], 
			   &index[(i>0) ? BinSizes[i-1] : 0], &level_record[(i>0) ? BinSizes[i-1] : 0], 
			   BinSizes[i] - ((i>0) ? BinSizes[i-1] : 0), 
			   population_threshold,
			   sft-3, lv+1,count); 
	 }
  
  }
   else
  {
	  for(int i=0; i<MAXBINS; i++){
             
         truncated_radix_sort(&morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			      &sorted_morton_codes[(i>0) ? BinSizes[i-1] : 0], 
			      &permutation_vector[(i>0) ? BinSizes[i-1] : 0], 
			      &index[(i>0) ? BinSizes[i-1] : 0], &level_record[(i>0) ? BinSizes[i-1] : 0], 
			      BinSizes[i] - ((i>0) ? BinSizes[i-1] : 0), 
			      population_threshold,
			      sft-3, lv+1,count);
                }   
	  
  }
  } 
  
}

