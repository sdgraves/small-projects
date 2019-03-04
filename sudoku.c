#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define DIM 2
#define DIM_2 (DIM*DIM)
#define DIM_4 (DIM_2*DIM_2)

/******************************************************************************/
//This module solves Sudoku boards, returning the set of solved boards which
//satisfy the conditions of the inputted board. DIM_2 is the highest number in
//the set of numbers which may be played (e.g. 9 for a conventional board),
//specified by DIM, its root.
/******************************************************************************/

//map[i][j] contains the validity of playing the number i+1 at the jth square,
//stored at vec[j]. 
typedef struct Board{
    char vec[DIM_4];
    char map[DIM_2][DIM_4];
    int counter;
} Board;

typedef struct Pair{
    char c;
    int index;
} Pair;

/******************************************************************************/

//Simply indicates that an element is set in a location,
//preventing any other number from being put in.
void set_filled( Board *b, int index )
{
    for ( int ii = 0; ii < DIM_2; ii++ )
	b->map[ii][index] = 0;
}

/******************************************************************************/
//updates the logic map due to one value at index ii.
//Leaves board unchanged.
void update_map( Board *b, int c, int index )
{
    int row = index/DIM_2;
    int col = index - DIM_2*row;

    //row
    for ( int ii = 0; ii < DIM_2; ii++ )
	b->map[c][DIM_2*row + ii] = 0;

    //col
    for ( int ii = 0; ii < DIM_2; ii++ )
	b->map[c][DIM_2*ii + col] = 0;

    int group_x = col/DIM;
    int group_y = row/DIM;
    //NxN block
    for ( int ii = 0; ii < DIM; ii++ )
    {
	for ( int jj = 0; jj < DIM; jj++ )
	    b-> map[c][ DIM*group_x + jj + DIM_2*( DIM*group_y + ii ) ] = 0;
    }


}

/******************************************************************************/
//loops through every single entry on the board, finding nonzero members and
//updating the board's logic map associated with each numer [1, DIM_2].
void get_map( Board *b )
{
    for ( int ii = 0; ii < DIM_4; ii++ )
    {
	if ( b->vec[ii] != '0')
	{
	    set_filled( b, ii );
	    update_map( b, b->vec[ii] - '1', ii );
	}
    }
}

/******************************************************************************/
//prints a list b of length DIM_4, with some formatting. "logical" shifts
//integers into the range of characters corresponding to integers.
void print( char* b, int logical )
{
    int offset = ( logical ? '0' : 0 );
    
    for ( int ii = 0; ii < DIM_2; ii++ )
    {
	for ( int jj = 0; jj < DIM_2; jj++ )
	{
	    printf("%c ", *(b + DIM_2*ii + jj ) + offset);
	    if ( (jj+1) % DIM == 0 )
		printf(" ");
	}
	if ( (ii+1) % DIM == 0 )
	    printf("\n");
	printf("\n");
    }
    
}

/******************************************************************************/
//for checking work.
void print_map( Board *b, char c )
{
    printf("Map for %c:\n", c );
    print( &b->map[c - '1'][0], 1 );
}

void check_maps( Board *b )
{
    for ( char c = '1'; c < '0' + DIM_2; c++ )
	print_map( b, c );
}


void print_board( Board *b )
{
    print( &b->vec[0], 0 );
}

/******************************************************************************/
//places a character c on the board at the specified index. Updates board
//properties and the map of what characters are legal moves.
void set ( Board *b, char c, int index )
{
    b->vec[index] = c;
    set_filled( b, index );
    update_map( b, c - '1', index );
    b->counter--;
}

//method for when a guess is shown to be wrong.
inline void elim ( Board *b, Pair p )
{
    b->map[p.c - '1'][p.index] = 0;
}


/******************************************************************************/
//loops through every row, column, and group to see if any tile on the board
//can only be filled with one character; fills it and reports the character
//and location.
Pair get_char( Board *b )
{
    //check for squares that must be 1,2,...9
    for ( int ii = 0; ii < DIM_2; ii++ )
    {
	int sum_row, sum_col, sum_group;

	//check each row
	for ( int kk = 0; kk < DIM_2; kk++ )
	{
	    int index = -1;
	    sum_row = 0;
	    for ( int jj = kk*DIM_2; jj < (kk+1)*DIM_2; jj++ )
	    {
	  	if ( b->map[ii][jj] )
	  	{
	  	    sum_row++;
	  	    index = jj;
	  	}
	    }
	    if ( sum_row == 1 )
	    {
		set( b, ii + '1', index );
		Pair p = { ii + '1', index };
		return p;
	    }
	}

	//check each col
	for ( int kk = 0; kk < DIM_2; kk++ )
	{
	    int index = -1;
	    sum_col = 0;
	    for ( int jj = kk; jj < DIM_4 ; jj += DIM_2 )
	    {
		if ( b->map[ii][jj] )
		{
		    sum_col++;
		    index = jj;
		}
	    }
	    if ( sum_col == 1 )
	    {
		set( b, ii + '1', index );
		Pair p = { ii + '1', index };
		return p;
	    }

	}

	//check each NxN box
	for ( int jj = 0; jj < DIM_4; jj += DIM*DIM_2 )
	{
	    for ( int kk = 0; kk < DIM_2; kk += DIM )
	    {
		sum_group = 0;
		int index = -1;
		for ( int y = 0; y < DIM*DIM_2; y += DIM_2  )
		{
		    for ( int x = 0; x < DIM; x++ )
		    {
			if ( b->map[ii][ jj + kk + x + y ] )
			{
			    sum_group++;
			    index = jj + kk + x + y;
			}
		    }
		}
		if ( sum_group == 1 )
		{
		    set( b, ii + '1', index );
		    Pair p = { ii + '1', index };
		    return p;		
		}
	    }
	}

    }
    Pair p = { 0, 0 };
    return p;		


}


/******************************************************************************/
//for the board as the user inputs it -- fills all squares which could only
//be one value.
void reduce ( Board *b )
{
    while ( get_char(b).c )
	;
}

/******************************************************************************/
//since the logical operations within get_char are biconditional, i.e. filling
// (n, i) implies filling (m, j), then filling (m, j) implies (n,i), with i,j
//indices and n,m integers on [1, DIM_2], any result which necessarily follows
//from a guess does not have to be tried on the board which supplied the initial
//guess. 
void solve_reduce( Board *b, Board *ref )
{
    Pair p;
    while ( (p = get_char(b)).c )
	elim( ref, p );

}

/******************************************************************************/
//create a new copy of board b with a guess ch set at the specified index.
Board* set_and_copy( Board *b, char ch, int index )
{
    Board *c = malloc( sizeof(Board) );
    memcpy( c, b, sizeof(Board) );
    set( c, ch, index );
    return c;
}

/******************************************************************************/
//some helper data structures for the list of solutions

typedef struct Node {
    Board *board;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct BoardList {
    Node *first;
    Node *last;
} BoardList;

void list_push_front( BoardList *list, Board *b )
{
    Board *cpy = malloc(sizeof(Board));
    Node *n = malloc( sizeof(Node) );
    memcpy( cpy, b, sizeof(Board) );
	
    n->board = cpy;
    Node *temp = list->first;
    n->next = temp->next;
    temp->next = n;
    n->prev = temp;
    n->next->prev = n;
}


/******************************************************************************/
//the main method responsible for inputting guesses to boards which are not
//sufficiently constrained. 
void solve( Board *b, Board *ref, BoardList *list )
{
    static int count = 0;
    Board *c;
    if ( b->counter )
    {
	int hasGuesses = 0;
	for ( int ii = 0; ii < DIM_2; ii++ )
	{
	    for ( int jj = 0; jj < DIM_4; jj++ )
	    {
		if ( b->map[ii][jj] )
		{
		    hasGuesses = 1;
		    c = set_and_copy( b, ii + '1', jj );
		    solve_reduce( c, b );
		    solve(c, b, list);
		    free(c);
		}
	    }
	}
	if ( hasGuesses == 0 )
	{
	    count++;
	}
    }
    else
    {
	count++;
	list_push_front(list, b);
    }

}

/******************************************************************************/
//Sets up the list to store the solutions of board b, uses a slightly different
//reduce() function than the one used by the recursive solve to impose the
//condition that all boards entering the solve function do not have moves which
//must be played.
BoardList* get_solns( Board *b )
{
    reduce( b );
    
    BoardList *l = malloc(sizeof(BoardList));
    Node *head = malloc(sizeof(Node));
    *head = (Node){ b, NULL, NULL };
    Node *tail = malloc(sizeof(Node));
    *tail = (Node){ b, head, NULL };
    head->next = tail;
    
    l->first = head;
    l->last = tail;

    solve( b, b, l );
    return l;
}


/******************************************************************************/
//allocate memory and clear the logic map for the new board (i.e. allow all
//characters on all squares)
void board_init( Board *b )
{
    memset( &b->vec[0], '0', DIM_4 );
    memset( &b->map, 1, DIM_4*DIM_2 );
    b->counter = DIM_4;
    get_map( b );

}

/******************************************************************************/
//These methods simply store a few boards
void set_2x2( Board *b )
{
    set( b, '1', 0 );
    set( b, '3', 5 );
    set( b, '2', 10 );
//    set( b, '4', 13 );
}

void set_minimal( Board *b )
{
    set( b, '9', 10 );
    set( b, '1', 13 );
    set( b, '3', 16 );
    set( b, '6', 20 );
    set( b, '2', 22 );
    set( b, '7', 24 );
    set( b, '3', 30 );
    set( b, '4', 32 );
    set( b, '2', 36 );
    set( b, '1', 37 );
    set( b, '9', 43 );
    set( b, '8', 44 );
    set( b, '2', 56 );
    set( b, '5', 57 );
    set( b, '6', 59 );
    set( b, '4', 60 );
    set( b, '8', 64 );
    set( b, '1', 70 );
        
    get_map( b );

}

//not necessarily unique -- simply removed a few clues from the above.
void set_hard( Board *b )
{
    
    set( b, '9', 10 );
    set( b, '1', 13 );
    set( b, '3', 16 );
    set( b, '6', 20 );
    set( b, '2', 22 );
    set( b, '7', 24 );
    set( b, '3', 30 );
    set( b, '4', 32 );
    set( b, '2', 36 ); 
    set( b, '1', 37 );
    set( b, '8', 44 );
    set( b, '2', 56 );
    set( b, '5', 57 );
    set( b, '6', 59 );
    set( b, '4', 60 );
    set( b, '8', 64 );
    set( b, '1', 70 );
        
    get_map( b );

}
    


void set_test( Board *b )
{
    b->vec[57] = '1';
    b->vec[58] = '2';
    b->vec[59] = '3';
    b->vec[66] = '4';
//    b->vec[64] = '5';
    b->vec[68] = '6';
    b->vec[75] = '7';
    b->vec[76] = '8';
    b->vec[77] = '9';
    get_map(b);

}

void set_diag( Board *b )
{    
    set( b, '5',  0 );
    set( b, '3',  1 );
    set( b, '7',  4 );
    set( b, '6',  9 );
    set( b, '1', 12 );
    set( b, '9', 13 );
    set( b, '5', 14 );
    set( b, '9', 19 );
    set( b, '8', 20 );
    set( b, '6', 25 );
    set( b, '8', 27 );
    set( b, '6', 31 );
    set( b, '3', 35 );
    set( b, '4', 36 );
    set( b, '8', 39 );
    set( b, '3', 41 );
    set( b, '1', 44 );
    set( b, '7', 45 );
    set( b, '2', 49 );
    set( b, '6', 53 );
    set( b, '6', 55 );
    set( b, '2', 60 );
    set( b, '8', 61 );
    set( b, '4', 66 );
    set( b, '1', 67 );
    set( b, '9', 68 );
    set( b, '5', 71 );
    set( b, '8', 76 );
    set( b, '7', 79 );
    set( b, '9', 80 );
    
    get_map(b);
    
}

/******************************************************************************/

int main( char argc, char** argv )
{
    //set up the current board
    Board top;
    Board ref;


    board_init( &top );

    set_2x2( &top );
    print_board( &top );
    printf("\nsolving\n");
    BoardList *l = get_solns( &top );

    for ( Node *n = l->first->next; n->next != NULL; n = n->next )
    {
	print_board( n->board );
	printf("--------\n");
    }
    
    return 0;
}

