#include <mpi.h>
#include <omp.h>
#define Board(x,y) board[(x)*N + (y)]
#define NewBoard(x,y) newboard[(x)*N + (y)]

//=====Ορίζουμε επιπλέον μια παραλλαγή της συνάρτησης adjacent_to======
int adjacent_toHelpFunction(int **board, int i, int j,int N) ;
//=====================================================================


//=========Η συνάρτηση είναι για την περίπτωση 1 διεργασίας============
void IamTheOnlyPlayer(int *board, int *newboard, int N);
//=====================================================================

/* set everthing to zero */

void initialize_board (int *board,int numOfRows, int N);

/* add to a width index, wrapping around like a cylinder */

int xadd (int i, int a, int N);

/* add to a height index, wrapping around */

int yadd (int i, int a, int N);

/* return the number of on cells adjacent to the i,j cell */

int adjacent_to (int *board, int i, int j,int numOfRows,int N);

/* play the game through one generation */

void play (int *board, int *newboard,MPI_Request *RecvRequest1,
           MPI_Request *RecvRequest2,int *firstRow,int *lastRow,int numOfRows, int N);
/* print the life board */

void print (int *board,int numOfRows,int N);

/* generate random table */

void generate_table (int *board,int numOfRows,int N,float threshold,int seedRandom);

/* display the table with delay and clear console */

void display_table(int *board,int numOfRows,int N) ;

/* #endif // FOO_H_ */
