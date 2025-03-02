#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <game-of-life.h>

/* print the life board */

void print (int *board,int numOfRows,int N) {
  int   i, j;
  
  
 /* for each row */
   for (i=0; i<numOfRows; i++) {
 /* print each column position... */
   for (j=0; j<N; j++) {
    printf ("%c", Board(i,j) ? 'x' : ' ');
    }
    /* followed by a carriage return */
    printf ("\n");
  }

}

/* display the table with delay and clear console */
void display_table(int *board,int numOfRows,int N) {
  print (board,numOfRows,N);
  usleep(100000);  
  /* clear the screen using VT100 escape codes */
  //puts ("\033[H\033[J");
}
