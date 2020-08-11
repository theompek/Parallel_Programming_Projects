/*
Θα δημιουργήσουμε μία συνάρτηση που παράγει διανύσματα-σημεία (VectLength)x1 διαστασης.
Θα δημιουργήσουμε groupsNum ομάδες απο τα διανύσματα-σημεία,η κάθε ομάδα θα διαφέρει με τις 
υπόλοιπες ως πρός την θέση των διανυσμάτων ("στον χώρο").Οι συντεταγμένες της κάθε ομάδας 
θα βρίσκονται γύρο απο μία κέντρική τιμή kernDat,για τον 3d χώρο για παράδειγμα αυτο σημαίνει
οτι τα σημεία κάθε ομάδας θα βρίσκονται γύρο απο ένα κεντρικό σημείο στα όρια μία νοητής σφαίρας.
Επίσης κάθε όμαδα θα σηματοδοτείται με ένα διάνυσμα που θα το θεωρήσουμε ως την επιθυμητή έξοδο του
νευρονικού δικτύου.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

#define dataIn(x,y,z) ((*dataIn)[x+(*length)*y+(*length)*VectNumber*z])

void prodData(float **dataIn,float **desrdOut,int *length,int *width,int *outLength){
	
*length=VectLength*numOfSubNetworks;  //Η δίασταση των διανυσμάτων
*width=VectNumber*groupsNum;      //Αριθμός δεδομένων(διανυσμάτων) 
*outLength=outNetLength*numOfSubNetworks; //Αριθμός(μέγεθος) δεδομένων εξόδου

srand((unsigned int)time(NULL));

//Δέσμευση θέσεων για πίνακα συντεταγμένων ((*length))x(VectNumber*groupsNum) 
(*dataIn)=(float *)malloc((*length)*(*width)*sizeof(float));

//Για κάθε ομάδα αντιστοιχίζουμε και διάνυσμα που χαρακτηρίζει την ομάδα
(*desrdOut)=(float *)malloc((*outLength)*(*width)*sizeof(float));

//Αρχικοποίηση Πίνακα
for(int k=0;k<groupsNum;k++)
for(int y=0;y<VectNumber;y++)	
for(int i=0;i<(*outLength);i++)
(*desrdOut)[i+y*(*outLength)+(*outLength)*VectNumber*k]=0;

	
//Ο πυρήνας-σημείο γύρο απο τον οποίο θα βρίσκεται η κάθε ομάδα και τα όρια του χώρου
float kernDat[groupsNum];
int maxDist=10;

//Αποσταση σημείων γύρο απο το κέντρο
int maxRadius=2;

for(int i=0;i<groupsNum;i++)
kernDat[i] = (float)((rand()%maxDist)-(maxDist/2));	//Κεντρα στα όρια (-maxDist/2,maxDist/2)

for(int j=0;j<numOfSubNetworks;j++){
//Για κάθε ομάδα

for(int i=0;i<groupsNum;i++){
//Παράγουμε τις συντεταγμένες 
 for(int y=0;y<VectNumber;y++){ //Για κάθε διανύσμα
 
  (*desrdOut)[(i+groupsNum*j)+y*(*outLength)+(*outLength)*VectNumber*i]=1;
 
  for(int x=0;x<VectLength;x++)
  dataIn((x+VectLength*j),y,i)=kernDat[i] + groupsNum + (float)(0.354*(rand()%(2*maxRadius))-maxRadius);
		
 }	
 
}
}

int R1;
int R2;
float temp;
int numSwap=(*width);

//Αφού παράξουμε τα δεδομένα θα ανακατέψουμε τα διανύσματα
for(int i=0;i<10*numSwap;i++){
	
R1=rand()%numSwap;
R2=rand()%numSwap;

for(int x=0;x<(*outLength);x++){
temp=(*desrdOut)[x+(*outLength)*R2];
(*desrdOut)[x+(*outLength)*R2]=(*desrdOut)[x+(*outLength)*R1];
(*desrdOut)[x+(*outLength)*R1]=temp;
}
 
for(int x=0;x<(*length);x++){
temp=(*dataIn)[x+(*length)*R2];
(*dataIn)[x+(*length)*R2]=(*dataIn)[x+(*length)*R1];
(*dataIn)[x+(*length)*R1]=temp;
}
	
}


}
