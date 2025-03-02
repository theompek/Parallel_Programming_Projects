#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "omp.h"

#define DIM 3

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
/*===============================================================================*/
/* The function that transform the morton codes into hash codes */ 
void morton_encoding(unsigned long int *mcodes, unsigned int *codes, int N, int max_level){
  
 if(N>200000)
 { 
  #pragma omp parallel for shared(mcodes,codes,N) schedule(static)            
      for(int i=0; i<N; i++)
      {
       // Compute the morton codes from the hash codes using the magicbits mathod
       mcodes[i] = mortonEncode_magicbits(codes[i*DIM], codes[i*DIM + 1], codes[i*DIM + 2]);
      }
  
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


/*===============================================================================*/
