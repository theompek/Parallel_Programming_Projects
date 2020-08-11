#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <pthread.h>
#include <math.h>

extern void *backpropagation_train(void *dataTh);



void pthreadTrainNet(float **W1,float **W2,float **W3,float *Input,float *Output,int length,int width,int outLength){
//Ορισμος των νημάτων	
int num_threads = numOfSubNetworks*splitData;
pthread_t threads[num_threads];

PthreadData *dataTh;
dataTh=(PthreadData *) malloc(num_threads*sizeof(PthreadData));//Δεδομένα των νημάτων

for (int j = 0; j < splitData; j++) //Δημιουργία δικτύων δεδομένων για διαμοιρασμό των δεδομένων
for (int i = 0; i < numOfSubNetworks; i++) { //Διαχωρισμός των δικτύων δεδομένων σε υποδίκτυα
//Μοιράζουμε τα δεδομένα στα νήματα
dataTh[i+numOfSubNetworks*j].W1 = W1[i+numOfSubNetworks*j];
dataTh[i+numOfSubNetworks*j].W2 = W2[i+numOfSubNetworks*j];
dataTh[i+numOfSubNetworks*j].W3 = W3[i+numOfSubNetworks*j]; 
dataTh[i+numOfSubNetworks*j].length = length; //Συνολικού δικτύου
dataTh[i+numOfSubNetworks*j].width =floor(width/splitData); //Διαχωρισμος δεδομένων
dataTh[i+numOfSubNetworks*j].outLength = outLength;
dataTh[i+numOfSubNetworks*j].Input = &Input[j*length*dataTh[i+numOfSubNetworks*j].width ]; 
dataTh[i+numOfSubNetworks*j].Output = &Output[j*outLength*dataTh[i+numOfSubNetworks*j].width ];
dataTh[i+numOfSubNetworks*j].Id_x = i;
dataTh[i+numOfSubNetworks*j].Id_y = j;

pthread_create(&threads[i+numOfSubNetworks*j], NULL, backpropagation_train, &dataTh[i+numOfSubNetworks*j]);

}


for (int j = 0; j < splitData; j++)
for (int i = 0; i < numOfSubNetworks; i++) {
 if(pthread_join(threads[i+numOfSubNetworks*j], NULL)) {
  fprintf(stderr, "Error joining threadn");
 }
}   


}
