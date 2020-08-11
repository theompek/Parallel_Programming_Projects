#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <game-of-life.h>


void play (int *board, int *newboard,MPI_Request *RecvRequest1,MPI_Request *RecvRequest2,
		                       int *firstRow,int *lastRow,int numOfRows, int N) {
  
  int   i, j, a;
//printf("I am a happy Task and I play now: %d\n",numOfRows);
MPI_Status stat1;
MPI_Status stat2;
int flag1;
int flag2;
int count=0;
int *ArrayBegin[3];
int *ArrayEnd[3];

//Εκτέλεσε τον έλεγχο για όλες τις γραμμές εκτώς των οριακών οι οποίες 
//περιμένουν το send απο τις άλλες διεργασίες
#pragma omp parallel for schedule(static) private(i,j,a) shared(newboard,board,N)
  for (i=1; i<(numOfRows-1); i++){
    for (j=0; j<N; j++) {
      a = adjacent_to (board, i, j,numOfRows,N);
	  
      if (a == 2) NewBoard(i,j) = Board(i,j);
      if (a == 3) NewBoard(i,j) = 1;
      if (a < 2) NewBoard(i,j) = 0;
      if (a > 3) NewBoard(i,j) = 0;	  
    }
  }	
  	
int A=1;
int B=1;	
while(count<2)
{
//Ελέγχουμε αν έχουν έρθει οι οριακές γραμμές	
MPI_Test(RecvRequest1,&flag1,&stat1);
MPI_Test(RecvRequest2,&flag2,&stat2);
//printf("I am into whileloop %d\n",count);

if(flag1&A)//Αν έχει έρθει η πρώτη γραμμή(η τελευταία της προηγούμενης διεργασίας)
{
//Κάνουμε έλεγχο για την πρώτη γραμμή του πίνακα board ,δηλαδή μόνο για το ArrayBegin[1]
ArrayBegin[0]=lastRow;	    //Η τελευταία γραμμή του προηγούμενου
ArrayBegin[1]=board;	    //Η πρώτη γραμμή του Board
ArrayBegin[2]=&Board(1,0);	//Η δεύτερη γραμμή του Board

#pragma omp parallel for schedule(static) private(j,a) shared(ArrayBegin,newboard,N) num_threads(2)
 for (j=0; j<N; j++) {
      a = adjacent_toHelpFunction(ArrayBegin, 1, j,N); 
      if (a == 2) NewBoard(0,j) = ArrayBegin[1][j];
      if (a == 3) NewBoard(0,j) = 1;
      if (a < 2) NewBoard(0,j) = 0;
      if (a > 3) NewBoard(0,j) = 0;
    }
	count++;
	A=0;//Για να μην ξανα μπεί το πρόγραμμα σε αυτό το κομμάτι
		
}


if(flag2&B)//Αν έχει έρθει η τελευταία γραμμή(η πρώτη της επόμενης διεργασίας)
{
//Κάνουμε έλεγχο για την τελευταία γραμμή του πίνακα board ,δηλαδή μόνο για το ArrayBegin[1]
ArrayEnd[0]=&Board((numOfRows-2),0);	//Η προτελευταία γραμμή του Board
ArrayEnd[1]=&Board((numOfRows-1),0);	//Η τελευταία γραμμή του Board
ArrayEnd[2]=firstRow;             	    //Η πρώτη γραμμή του επόμενου

#pragma omp parallel for schedule(static) private(j,a) shared(ArrayEnd,newboard,N) num_threads(2)
 for (j=0; j<N; j++) {
      a = adjacent_toHelpFunction(ArrayEnd, 1, j, N); 
      if (a == 2) NewBoard((numOfRows-1),j) = ArrayEnd[1][j];
      if (a == 3) NewBoard((numOfRows-1),j) = 1;
      if (a < 2) NewBoard((numOfRows-1),j) = 0;
      if (a > 3) NewBoard((numOfRows-1),j) = 0;
    }
	count++;
	B=0;//Για να μην ξανα μπει το πρόγραμμα σε αυτό το κομμάτι
	
}
//printf("I am into whileloop %d\n",count);
 }
 
 
}

//Η παρακάτω συνάτρηση είναι για την περίπτωση της μίας διεργασίας
//Στην ουσία είναι η ίδια με την αρχική μη παραλληλοποιημένη συνάρτηση
void IamTheOnlyPlayer(int *board, int *newboard, int N) {
 
  int   i, j, a;

#pragma omp parallel for schedule(static) private(i,j,a) shared(newboard,board,N)
  for (i=0; i<N; i++)
    for (j=0; j<N; j++) {
      a = adjacent_to (board, i, j,N,N);//<-- <--  Η μοναδική διαφορά ειναι ότι εδώ απλά βάζουμε σαν όρισμα στην adjacent_to
      if (a == 2) NewBoard(i,j) = Board(i,j);    //όπου numOfRows==Ν ώστε να μήν ορίσουμε επιπλέον συναστήσεις
      if (a == 3) NewBoard(i,j) = 1;
      if (a < 2) NewBoard(i,j) = 0;
      if (a > 3) NewBoard(i,j) = 0;
    }
  
}

