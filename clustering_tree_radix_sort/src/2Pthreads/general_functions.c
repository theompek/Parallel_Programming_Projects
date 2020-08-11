#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#define DIM 3

void find_max(float *max_out,float *X, int N){//Απο εναν πινακα Χ[] με 3N στοιχεια (οπως φαινεται παρακατω) παραγει πινακα max_out 3 στοιχειων με τα μεγιστα των x,y,z


  for(int i=0; i<DIM; i++){
    max_out[i] = -FLT_MAX;
    for(int j=0; j<N; j++){
      
      if(max_out[i]<X[j*DIM + i]){//για i=0 εχουμε x[0] x[3] x[6] x[9]                                                   0  1  2   3  4  5   6  7 8  .....                  3N
                                  //για i=1 εχουμε x[1] x[4] x[7] x[10] Αρα ο x[] πινακας με 3*Ν στοιχεια με διαταξη x[(x0,y0,z0)(x1,y1,z1)(x2,y2,z2).....(x(n-1),y(n-1),z(n-1))]
                                  //για i=2 εχουμε x[2] x[5] x[8] x[11]
	      max_out[i] = X[j*DIM + i];
      } 
    }
  }

}

void find_min(float *min_out, float *X, int N){//Το ιδιο για ελαχιστα

  for(int i=0; i<DIM; i++){ 
    min_out[i] = FLT_MAX;
    for(int j=0; j<N; j++){
      if(min_out[i]>X[j*DIM + i]){
	min_out[i] = X[j*DIM + i];
      }
    }
  }

}
