
#define b 0.5

//Diastaseis eswterikwn layer diktyou
#define M2 50
#define M3 50

#define VectLength 10 //Διάσταση διανύσματος είσοδου υποδικτύων.
#define VectNumber 400 //Αριθμός δεδομένων ανά ομάδα δεδομένων(Συνολικός αριθμός δεδομένων=VectNumber*groupsNum,τα μισά θα πάνε για εκπαίδευση )
#define groupsNum  3 //Ομάδες ανεξάρτητων διανυσμάτων(δεδομένων)
#define outNetLength 10 //Μέγεθος εξόδου υποδικτύων

void prodData(float **dataIn,float **desrdOut,int *length,int *width,int *numGroups);

void backpropagation_train(float **W1,float **W2,float **W3,float *Input,
                           float *Output,int length,int width,int numGroups);
						   
void backpropagation_classify(float *W1,float *W2,float *W3,float *Input,float *Output,int length,int width,int numGroups);