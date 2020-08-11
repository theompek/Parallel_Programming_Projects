#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "sys/time.h"

#define data(x,y) data[x+length*y]
#define desiredOut(x,y) (desiredOut[x+outLength*y])
#define Y_out(x,y) (Y_out[x+outLength*y])

int main (int argc, char *argv[]) {

// Time counting variables //Μεταβλητές για μετρηση χρονου
struct timeval startwtime, endwtime;

float *data;
float *desiredOut;
float *Y_out;
int length; //Μηκος διανύσματος δεδομενων
int width;  //Αριθμός Συνόλου δεδομενων
int outLength; //Αριθμός διαφορετικών ομάδων δεδομένων
int count;
int countTrue=0;

//Πίνακες βαρών των νευρώνων για νευρονικό με 4layers(2 hidden)
float *W1; 
float *W2; 
float *W3; 
printf("-----------##########################----------\n");
printf("-->Paragwgh dedomenwn\n");
//---------Παραγωγή δεδομένων(τα δεδομένα εξάγονται ανακατεμένα)-----
prodData(&data,&desiredOut,&length,&width,&outLength);

printf("Ari8mos Dedomenwn = %d apo ta opoia ta %d 8a pane gia ekpaideush\n",width,(int)width/2);
printf("Mege8os(eisodou) dedomenwn = %d\n",length);
printf("Mege8os(e3odou) dedomenwn = %d\n",outLength);
printf("\n");

//Ξεκινάει η μέτρηση χρόνου
gettimeofday (&startwtime, NULL);
printf("-->Ekpaideysh diktuou\n");
//----------------------Εκπαίδευση δικτύου-------------------
//Παίρνουμε το μισό από το σύνολο των δεδομένων για εκπαίδευση
backpropagation_train(&W1,&W2,&W3,data,desiredOut,length,width,outLength);
gettimeofday (&endwtime, NULL);

double h_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
	/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);    
printf("Time to compute code : %fs\n\n", h_time); //Χρόνος εκτέλεσης

//-------------------Χρησιμοποιουμε το δικτυο---------------
//Πίνακας εξόδου νευρονικού
Y_out=(float *)malloc(outLength*width*sizeof(float));
printf("-->Xrhsh diktuou gia ola ta (%d) dedomena\n\n",width);
//Δοκιμάζουμε το δίκτυο χρησιμοποιώντας όλο το σύνολο δεδομέων
backpropagation_classify(W1,W2,W3,data,Y_out,length,width,outLength);

//--------------------Έλεγχος αποτελεσμάτων------------------
printf("-->Elegxoume thn epityxeia ekpaideyshs\n");
for(int j=0;j<width;j++){ //Για κάθε διάνυσμα
count=0;
for(int i=0;i<outLength;i++) //Έλεγξε τις αντίστοιχες συντεταγμένες
if(Y_out(i,j)==desiredOut(i,j)) 

count++;
if(count==outLength) //Αν είναι σωστές όλες οι συντεταγμένες
countTrue++; 
}
 
float a;
a=(float) 100*countTrue/width;
printf("Swsta = %d  Sunolo = %d \n",countTrue,width);
printf("Epityxeia se pososto %4.2f ths ekato\n",a);
//-------------------------------------------------------------

/*
//Gia na doume optika ta apotelesmata apla prepei na 8esoume
//mikro ari8mo dedomenwn sto arxeio data.c 8etoume px VectNumber=5
printf("\nPragmatikes times e3odou dedomenwn\n");	
for(int k=0;k<outLength;k++){
for(int j=0;j<width;j++)
printf("%f ",desiredOut[j*outLength+k]);
printf("\n");
}	

printf("\nTimes twn dedomenwn-Dianysmatwn(Eisodoi diktuoy)\n");
for(int i=0;i<length;i++){
for(int j=0;j<width;j++)	
printf("%f ",data(i,j));	
printf("\n");	
}

printf("\nTimes e3odou pou prokuptoun apo to diktyo(E3odoi diktuoy)\n");	
for(int k=0;k<outLength;k++){
for(int j=0;j<width;j++)
printf("%f ",Y_out[j*outLength+k]);
printf("\n");
}	
//*/
free(data);	
free(desiredOut);
free(Y_out);
free(W1);	
free(W2);
free(W3);		
printf("-----------##########################----------\n");
}