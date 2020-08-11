#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "sys/time.h"

#include <game-of-life.h>

#define MASTER 0 //Η διεργασία αρχηγός

int main (int argc, char *argv[]) {

//Μεταβλητές για μετρηση χρονου	
struct timeval startwtime, endwtime;
	
//Ορίζουμε τον αριθμό των νημάτων για το κομμάτι της OpenMP
omp_set_num_threads(8);   	
	
int numOfTasks=0;  //Ο αριθμός των διεργασιών
int myId=0;       //Η ταυτότητα της εκάστοτε διεργασίας
int lastTask=0;   //Η τελευταία κατα αριθμό διεργασία
int RowsOfOther=0;//Ο αριθμός των γραμμών που αντιστοιχούν στις διεργασίες εκτώς του MASTER
int initial=0;    //Έλεγχος για την αρχικοποίηση

int numOfRows=0;  //Μεταβλητή για το αριθμό των γραμμών των πινάκων

MPI_Request SendRequest1; //Requests για την προηγούμενη διεργασία
MPI_Request RecvRequest1; 
MPI_Request SendRequest2; //Requests για την επόμενη διεργασία
MPI_Request RecvRequest2; 

MPI_Request DisplayRequest; //Request για το display
MPI_Status statdisplay;     //Για το display


int *board;
int *newboard;

if (argc != 5) { // Check if the command line arguments are correct 
    printf("Usage: %s N thres disp\n"
	   "where\n"
	   "  N     : size of table (N x N)\n"
	   "  thres : propability of alive cell\n"
           "  t     : number of generations\n"
	   "  disp  : {1: display output, 0: hide output}\n"
           , argv[0]);
    return (1);
  }
    
  // Input command line arguments
int N = atoi(argv[1]);        // Array size
double thres = atof(argv[2]); // Propability of life cell
int t = atoi(argv[3]);        // Number of generations 
int disp = atoi(argv[4]);     // Display output?


//Αρχικοποίηση για το περιβάλλον εκτέλεσης του MPI
//και έλεγχος για την επιτυχία της διαδικασίας
initial=MPI_Init(&argc,&argv);	

   if (initial!= MPI_SUCCESS) {
     printf ("Error starting MPI program. Terminating.\n");
     MPI_Abort(MPI_COMM_WORLD,initial);
     }
	 
//Παίρνουμε τα στοιχεία για τον αριθμό 
//και την ταυτότητα των διεργασιών
MPI_Comm_size(MPI_COMM_WORLD,&numOfTasks);
MPI_Comm_rank(MPI_COMM_WORLD,&myId);


lastTask=numOfTasks-1;

//==============================Υπολογισμός των διαστάσεων των πινάκων=============================
//Για να διατηρήσουμε την γενικότητα στην γραφή του κώδικα,δηλαδή θεωρούμε οτι ο αρχικός πίνακας είναι NxN,
//και επειδή η εκφώνηση ζητάει για 2 διεργασίες πίνακα με διαστάσεις 80000x40000,για την περίπτωση
//της 1 και 2 διεργασιών θέτουμε άμεσα τις διαστάσεις των πινάκων.
if((numOfTasks==1)||(numOfTasks==2)){
numOfRows=40000;
N=40000;
RowsOfOther=40000;
}
else{//Σε οποιαδήποτε γενική περίπτωση αριθού στοιχείων και διεργασιών	

//Υπολογίζουμε τον αριθμό των γραμμών και άν δεν είναι ακέραιο το αποτέλεσμα κάνουμε κατάλληλα την
//στρογγυλοποίηση προς τα πάνω ή προς τα κάτω ώστε να μοιραστούν ομοιόμορφα οι γραμμές στις διεργασίες    
numOfRows=(int)((double) N/numOfTasks);    //Αριθμός των γραμμών του πίνακα για κάθε διεργασία
if(((double) (numOfRows+0.5))<(N/numOfTasks)) numOfRows++;
	
//Επειδή η παραπάνω διαίρεση μπορεί να μην είναι ακριβής(ακέραιο αποτέλεσμα) ο MASTER πρέπει
//να προσαρμόσει κατάλληλα το μήκους του πίνακα που του αντιστοιχεί.Επιπλέον
//αποθηκέυει στην μεταβλητή RowsOfOther τον αριθμό των γραμμών που έχουν οι άλλες διεργασίες
if((myId==MASTER)&(numOfTasks>1))
 {
  RowsOfOther=numOfRows;
  numOfRows=N-(numOfTasks-1)*numOfRows;
 }

}
//==================================================================================================

printf("Size %dx%d with propability: %0.1f%%\n",numOfRows, N, thres*100);

board = NULL;
newboard = NULL;
 
//Δεσμεύουμε θέσεις μνήμης σύμφωνα με το μήκος numOfRows
board = (int *)malloc(numOfRows*N*sizeof(int));
newboard = (int *)malloc(numOfRows*N*sizeof(int));

  if (board == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }

  if (newboard == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }

//Πίνακες για την αποστολή και την παραλαβή των δεδομένων μεταξύ των διεργασιών
int *firstRow;
firstRow=(int *)malloc(N*sizeof(int));
int *lastRow;
lastRow=(int *)malloc(N*sizeof(int));

   if (firstRow == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }

  if (lastRow == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }
  
  //Ξεκινάει η μέτρηση χρόνου για την generate
 gettimeofday (&startwtime, NULL); 
  //initialize_board (board,numOfRows,N); //Δεν χρησιμοποιείται
  //printf("Board initialized\n");
  generate_table (board,numOfRows,N,thres,myId+1);
  printf("Board generated\n");

 gettimeofday (&endwtime, NULL);//Τελειώνει η μέτρηση χρόνου

  double ge_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
    
 printf("I am process ID:%d-->Time to compute the genarate code: %fs\n",myId,ge_time); //Χρόνος εκτέλεσης
   
 MPI_Barrier(MPI_COMM_WORLD);
 
 if(myId==MASTER) printf("<===========================================>\n");
 
  /* play game of life some times */
  
   gettimeofday (&startwtime, NULL); //Ξεκινάει η μέτρηση χρόνου
   
  for (int i=0; i<t; i++) {
	//Αν έχει οριστεί display τότε ο MASTER ζητάει με μία ανασταλτική εντολή(για να εκτυπωθούν με την σωστή σειρά)  
    //να του σταλούν οι πίνακες απο τις άλλες διεργασίες και τους εκτυπώνει.Οι υπόλοιπες διεργασίες του στέλνουν
	//τους πίνακες(απλά για να εκτυπωθούν)με μία μη-ανασταλτική εντολή και προχωράν παρακάτω να κάνουν δουλειά.
	//Να σημειωθεί οτι τα δεδομένα που λαμβάνει ο MASTER δεν χρειάζεται να αποθηκευτούν όλα μαζί παρα μόνο προσωρινά 
	//ένα-ένα και επίσης μπορούμε να ορίσουμε το μέγεθος των δεδομένων που θα στέλνουμε ώστε να χωράνε στην μνήμη
//Στο κομμάτι των μετρήσεων δεν χρησιμοποιώ την εκτύπωση εφόσον δεν χρειάζεται και θα εισάγει καθυστερίσεις
//======================In the case of Display===================
   if (disp&(myId==MASTER)) 
	{
		int count=RowsOfOther*N;
		int tagTable=3;
		display_table (board,numOfRows,N);
		
		for(int source=1;source<numOfTasks;source++)
		{
	      MPI_Recv(newboard,count,MPI_INT,source,tagTable,MPI_COMM_WORLD,&statdisplay);
		  display_table (newboard,RowsOfOther,N); //Σε αυτό το σημειό τον πίνακα newboard τον χρησιμοποιώ ως βοηθητικό για την εκτύπωση
		}
		   printf ("---------------------\n");
	}
	else if(disp){
		int tagTable=3;
		int count=numOfRows*N;
		MPI_Isend(board,count,MPI_INT,MASTER,tagTable,MPI_COMM_WORLD,&DisplayRequest);
		
	}
//================================================================	
	
	//Κύριο κομμάτι του προγράμματος για την ανταλαγή και επεξεργασία 
	//των δεδομένων μεταξύ των διεργασιών
	if((numOfTasks==1)){//Αν είμαι η μονάδικη διεργασίας
	
		IamTheOnlyPlayer(board,newboard,N); //Βρίσκεται μέσα στην play.c
	}
	else if((myId==MASTER)){//Αν είμαι ο αρχηγός σε μία ομάδα διεργασιών(με ID==0)
		int prevTask=lastTask;
		int nextTask=1;
		int tag1=1;
		int tag2=2;
		MPI_Irecv(lastRow,N,MPI_INT,prevTask,tag1,MPI_COMM_WORLD,&RecvRequest1);//Πάρε την τελευταία γραμμή απο την προηγούμενη διεργασία
	    MPI_Irecv(firstRow,N,MPI_INT,nextTask,tag2,MPI_COMM_WORLD,&RecvRequest2);//Πάρε την πρώτη γραμμή απο την επόμενη διεργασία
		
		MPI_Isend(board,N,MPI_INT,prevTask,tag2,MPI_COMM_WORLD,&SendRequest1); //Στείλε την πρώτη γραμμή στη προηγούμενη διεργασία
	    MPI_Isend(&Board((numOfRows-1),0),N,MPI_INT,nextTask,tag1,MPI_COMM_WORLD,&SendRequest2); //Στείλε την τελευταία γραμμή στη επόμενη διεργασία
		
		//printf("I am the MASTER and my numder is %d\n",myId);
		play(board, newboard,&RecvRequest1,&RecvRequest2,firstRow,lastRow,numOfRows,N);    
		
	} else if(myId==lastTask){//Αν είμαι κατα αριθμό η τελευταία διεργασία
		int prevTask=lastTask-1;
		int nextTask=MASTER;
		int tag1=1;
		int tag2=2;		
		MPI_Irecv(lastRow,N,MPI_INT,prevTask,tag1,MPI_COMM_WORLD,&RecvRequest1);//Πάρε την τελευταία γραμμή απο την προηγούμενη διεργασία
	    MPI_Irecv(firstRow,N,MPI_INT,nextTask,tag2,MPI_COMM_WORLD,&RecvRequest2); //Πάρε την πρώτη γραμμή απο την επόμενη διεργασία
		
		MPI_Isend(board,N,MPI_INT,prevTask,tag2,MPI_COMM_WORLD,&SendRequest1); //Στείλε την πρώτη γραμμή στη προηγούμενη διεργασία
	    MPI_Isend(&Board((numOfRows-1),0),N,MPI_INT,nextTask,tag1,MPI_COMM_WORLD,&SendRequest2); //Στείλε την τελευταία γραμμή στη επόμενη διεργασία
		
		//printf("I am the lastTask and my number is:%d\n",myId);
         play (board, newboard,&RecvRequest1,&RecvRequest2,firstRow,lastRow,numOfRows,N);		
		}
		else{//Αν είμαι κάποια άλλη διεργασία
		int prevTask=myId-1;
		int nextTask=myId+1;
		int tag1=1;
		int tag2=2;
		MPI_Irecv(lastRow,N,MPI_INT,prevTask,tag1,MPI_COMM_WORLD,&RecvRequest1);//Πάρε την τελευταία γραμμή απο την προηγούμενη διεργασία
	    MPI_Irecv(firstRow,N,MPI_INT,nextTask,tag2,MPI_COMM_WORLD,&RecvRequest2); //Πάρε την πρώτη γραμμή απο την επόμενη διεργασία
		
		MPI_Isend(board,N,MPI_INT,prevTask,tag2,MPI_COMM_WORLD,&SendRequest1); //Στείλε την πρώτη γραμμή στη προηγούμενη διεργασία
	    MPI_Isend(&Board((numOfRows-1),0),N,MPI_INT,nextTask,tag1,MPI_COMM_WORLD,&SendRequest2); //Στείλε την τελευταία γραμμή στη επόμενη διεργασία
			
			//printf("I am a happy Slave and my number is: %d\n",myId);
         play (board, newboard,&RecvRequest1,&RecvRequest2,firstRow,lastRow,numOfRows,N);
		}
		
//Ελέγχουμε και περιμένουμε μέχρι να σταλόυν
//οι οριακές γραμμές για να πάμε στην επόμενη γενιά
//=============================================	
if(numOfTasks!=1){
MPI_Status stat1;
MPI_Status stat2;
int flag1;
int flag2;

MPI_Test(&SendRequest1,&flag1,&stat1);
MPI_Test(&SendRequest2,&flag2,&stat2);

while((!flag1)||(!flag2)){
MPI_Test(&SendRequest1,&flag1,&stat1);
MPI_Test(&SendRequest2,&flag2,&stat2);
}
}
//=============================================

//Όταν επεξεργαστούμε τα δεδομένα μέσα στην συνάρτηση play αντί να αντιγράψουμε
//τα στοιχεία απο τον πίκανα newboard στον board απλά αντιστρέφουμε τους δείκτες τους
//και πλέον ο πίνακας board έχει τα νέα δεδομένα.		
int *k;
k=newboard;
newboard=board;
board=k;


 }//Τέλος της for
  
  MPI_Barrier(MPI_COMM_WORLD); 
 
  gettimeofday (&endwtime, NULL);//Τελειώνει η μέτρηση χρόνου

  double GameOfLife_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
    
  printf("I am process ID:%d-->Time to compute the GameOfLife code: %fs\n",myId,GameOfLife_time); //Χρόνος εκτέλεσης
  
  MPI_Barrier(MPI_COMM_WORLD); 
  if(myId==MASTER) printf("Game finished after %d generations.\n", t);//Όταν τελειώσει το πρόγραμμα αν ειναι ο MASTER
 
  MPI_Finalize();
  
  //Τέλος προγράμματος
}
