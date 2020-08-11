/*
Θα δημιουργήσουμε μία συνάρτηση που παράγει διανύσματα-σημεία (VectLength)x1 διαστασης.
Θα δημιουργήσουμε groupsNum ομάδες απο τα διανύσματα-σημεία,η κάθε ομάδα θα διαφέρει με τις 
υπόλοιπες ως πρός την θέση των διανυσμάτων ("στον χώρο").Οι συντεταγμένες της κάθε ομάδας 
θα βρίσκονται γύρο απο μία κέντρική τιμή kernDat,για τον 3d χώρο για παράδειγμα αυτο σημαίνει
οτι τα σημεία κάθε ομάδας θα βρίσκονται γύρο απο ένα κεντρικό σημείο στα όρια μία νοητής σφαίρας.
Επίσης κάθε όμαδα θα σηματοδοτείται με μια τιμή που θα την θεωρήσουμε ως την επιθυμητή τιμή της εξόδου
του νευρονικού δικτύου.
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

#define dataIn(x,y,z) ((*dataIn)[x+VectLength*y+VectLength*VectNumber*z])

void prodData(float **dataIn,float **desrdOut,int *length,int *width,int *outLength){

*length=VectLength;
*width=VectNumber*groupsNum;
*outLength=outNetLength;

srand((unsigned int)time(NULL));

//Δέσμευση θέσεων για πίνακα συντεταγμένων (VectLength)x(VectNumber*groupsNum) 
(*dataIn)=(float *)malloc(VectLength*(*width)*sizeof(float));

//Για κάθε ομάδα αντιστοιχίζουμε και έναν αριθμό που χαρακτηρίζει την ομάδα
(*desrdOut)=(float *)malloc(outNetLength*(*width)*sizeof(float));

//Αρχικοποίηση Πίνακα
for(int k=0;k<groupsNum;k++)
for(int y=0;y<VectNumber;y++)	
for(int i=0;i<outNetLength;i++)
(*desrdOut)[i+y*outNetLength+outNetLength*VectNumber*k]=0;

	
//Ο πυρήνας-σημείο γύρο απο τον οποίο θα βρίσκεται η κάθε ομάδα και τα όρια του χώρου
float kernDat[groupsNum];
int maxDist=10;

//Αποσταση σημείων γύρο απο το κέντρο
int maxRadius=2;

//Για κάθε ομάδα
for(int i=0;i<groupsNum;i++){

kernDat[i] = (float)((rand()%maxDist)-(maxDist/2));	//Κεντρα στα όρια (-maxDist/2,maxDist/2)

//Παράγουμε τις συντεταγμένες 
 for(int y=0;y<VectNumber;y++){ //Για κάθε διανύσμα
 
  (*desrdOut)[i+y*outNetLength+outNetLength*VectNumber*i]=1;
 
  for(int x=0;x<VectLength;x++)
  dataIn(x,y,i)=kernDat[i] + groupsNum + (float)(0.354*(rand()%(2*maxRadius))-maxRadius);
		
 }	
 
}

int R1;
int R2;
float temp;
int numSwap=VectNumber*groupsNum;

//Αφού παράξουμε τα δεδομένα θα ανακατέψουμε τα διανύσματα
for(int i=0;i<10*numSwap;i++){
	
R1=rand()%numSwap;
R2=rand()%numSwap;

for(int x=0;x<outNetLength;x++){
temp=(*desrdOut)[x+outNetLength*R2];
(*desrdOut)[x+outNetLength*R2]=(*desrdOut)[x+outNetLength*R1];
(*desrdOut)[x+outNetLength*R1]=temp;
}
 
for(int x=0;x<VectLength;x++){
temp=(*dataIn)[x+VectLength*R2];
(*dataIn)[x+VectLength*R2]=(*dataIn)[x+VectLength*R1];
(*dataIn)[x+VectLength*R1]=temp;
}
	
}


}