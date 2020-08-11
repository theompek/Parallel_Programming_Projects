extern int M2;
extern int M3;
#define b 0.5

#define n1 10 //Μέγεθος εισόδου κάθε υποδικτύου
#define n2 50 //Πρώτο κρυφό στρώμα υποδικτύου
#define n3 50 //Δεύτερο κρυφό στρώμα υποδικτύου
#define n4 10 //Μέγεθος εξόδου καθε υποδικτύου

#define VectLength n1 //Διάσταση διανύσματος είσοδου υποδικτύων.Συνολικό μέγεθος εισόδου ΟΛΟΚΛΗΡΟΥ του δικτύου = VectLength*numOfSubNetworks
#define VectNumber 400 //Αριθμός δεδομένων ανά ομάδα δεδομένων(Συνολικός αριθμός δεδομένων=VectNumber*groupsNum,τα μισά θα πάνε για εκπαίδευση )
#define groupsNum  3  //Ομάδες ανεξάρτητων διανυσμάτων
#define numOfSubNetworks 1 //Αριθμός υποδικτύων.Καθε υποδίκτυο το αναλαμβάνει ένα νήμα.
#define splitData 1 //Αριθμος τμημάτων που θα χωρίσουμε τα δεδομένα.Καθε τμήμα το αναλαμβάνει ένα νήμα.
#define outNetLength n4 //Μέγεθος εξόδου υποδικτύων.Συνολικό μέγεθος εξόδου ΟΛΟΚΛΗΡΟΥ του δικτύου = outNetLength*numOfSubNetworks

void prodData(float **dataIn,float **desrdOut,int *length,int *width,int *numGroups);
						   
void backpropagation_classify(float **W1,float **W2,float **W3,float *Input,float *Output,int length,int width,int numGroups);

void pthreadTrainNet(float **W1,float **W2,float **W3,float *Input,float *Output,int length,int width,int outLength);

typedef struct {//Δομή για την εισαγωγή μεταβλητών στα νήματα
 float *W1;
 float *W2;
 float *W3;
 float *Input;
 float *Output;
 int length;
 int width;
 int outLength;
 int Id_x;
 int Id_y;
 
}PthreadData;
