/*
 * assignment.c
 *
 *  Created on: 08-Aug-2016
 *      Author: Mrunmoy Samal
 *
 *  LICENSE:-
 *  The MIT License (MIT)
 *  Copyright (c) 2016 Mrunmoy Samal
 *
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom
 *  the Software is furnished to do so, subject to the following
 *  conditions:
 *
 *  The above copyright notice and this permission notice shall
 *  be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 *  OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */


/*
 *----------------------------------------------------------------------
 *   Include Files
 *----------------------------------------------------------------------
 */
#include <stdio.h>      /* Standard I/O functions */
#include <stdlib.h>     /* Prototypes of commonly used library functions,
                           plus EXIT_SUCCESS and EXIT_FAILURE constants */
#include <fcntl.h>      /* File control options */
#include <errno.h>      /* Declares errno and defines error constants */
#include <sys/types.h>  /* Type definitions used by many programs */
#include <sys/stat.h>   /* The Mode Bits for Access Permission */
#include <unistd.h>     /* Prototypes for many system calls */
#include <errno.h>      /* Declares errno and defines error constants */
#include <string.h>     /* Commonly used string-handling functions */
/*
*----------------------------------------------------------------------
*   Private Defines
*----------------------------------------------------------------------
*/
#ifndef BUF_SIZE 		/* Allow "cc -D" to override definition */
#define BUF_SIZE 200
#endif


#define GRID_SIZE		(16)
#define MAX_LIST		(8)
#define MAX_WORD_SIZE	(16) /* since words are not repeated */
/*
*----------------------------------------------------------------------
*   Private Macros
*----------------------------------------------------------------------
*/

/*
*----------------------------------------------------------------------
*   Private Data Types
*----------------------------------------------------------------------
*/

struct Wordlist_t
{
	char search_word[MAX_WORD_SIZE+1];
	struct Wordlist_t *next;
};
typedef struct Wordlist_t Wordlist_t;



struct Cell_t
{
	int cell_no;
	char value;
	int visited;
	int num_adj_cells;
	struct Cell_t * adjacent[MAX_LIST];  /* 8 max adjacent cells in this case */
};
typedef struct Cell_t Cell_t;
typedef struct Cell_t * pCell_t;

/*
*----------------------------------------------------------------------
*   Public Variables
*----------------------------------------------------------------------
*/


/*
*----------------------------------------------------------------------
*   Private Variables (static)
*----------------------------------------------------------------------
*/

int key_indices[GRID_SIZE][MAX_LIST] =
{
    {1, 4, 5}, 			//0
    {0, 2, 4, 5, 6},	//1
    {1, 3, 5, 6, 7},	//2
    {2, 6, 7},			//3
    {0, 1, 5, 8, 9},	//4
    {0, 1, 2, 4, 6, 8, 9, 10},	//5
    {1, 2, 3, 5, 7, 9, 10, 11},	//6
    {2, 3, 6, 10, 11},			//7
    {4, 5, 9, 12, 13},			//8
    {4, 5, 6, 8, 10, 12, 13, 14},//9
    {5, 6, 7, 9, 11, 13, 14, 15},//10
    {6, 7, 10, 14, 15},			//11
    {8, 9, 13},			//12
    {8, 9, 10, 12, 14},//13
    {9, 10, 11, 13, 15},//14
    {10, 11, 14}		//15
};

static Cell_t grid [16];
static Wordlist_t GridWords;
char gridletters[GRID_SIZE] = {0};

/*
*----------------------------------------------------------------------
*   Public Constants
*----------------------------------------------------------------------
*/

/*
*----------------------------------------------------------------------
*   Private Constants (static)
*----------------------------------------------------------------------
*/

/*
*----------------------------------------------------------------------
*   Private Function Prototypes (static)
*----------------------------------------------------------------------
*/

int compchar(const void *ap, const void *bp);
static void InitializeGrid(char *grid_chars);
static char GetLetterAt(Cell_t *pCell, int adj);
static void FormWordsFromGrid(void);
/*
*----------------------------------------------------------------------
*   Private Functions Definitions
*----------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
	FILE * 	inputFP; 		/*!< Input File Pointer */
	char 	buf[BUF_SIZE];	/*!< buffer to hold data to read/write */

    Wordlist_t *prev_word, *cur_word = &GridWords;

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s /usr/share/dict/words abanzqzdrrorrnrr\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    InitializeGrid(argv[2]);

    FormWordsFromGrid();

	/* Open input and output files */
	if ((inputFP = fopen(argv[1], "r")) == NULL)
	{
		printf("ERR: open %s file.\n", argv[1]);
		exit(1);
	}

	/* until we encounter end of input or an error */
	while (fgets(buf, BUF_SIZE, inputFP) != NULL)
	{
		// Remove the trailing newline character
		if (strchr(buf, '\n'))
		{
			buf[strlen(buf) - 1] = '\0';
		}

		/* core search goes here */
		cur_word = &GridWords;
	    while(cur_word)
	    {
	    	prev_word = cur_word;
	    	// word match
	    	if ( strcmp(cur_word->search_word, buf) == 0)
	    	{
	    		Wordlist_t *temp;
	    		// list the found word
	    		printf("%s\n", buf);

	    		temp = cur_word;

	    		/* throw away the found word */
	    		prev_word->next = cur_word->next;
	    		cur_word = prev_word;
	    		free(temp);
	    	}
	    	cur_word = cur_word->next;
	    }
	}

	/* Close the Input and Output files */
	if (fclose(inputFP) == -1)
	{
		printf("ERR: close input\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

static void InitializeGrid(char *grid_chars)
{
	int i, j;

	memset(grid, 0, sizeof(grid));

	for (i =0;i<GRID_SIZE; i++)
	{
		gridletters[i] = grid_chars[i];
		grid[i].value = grid_chars[i];
		grid[i].visited = 0;
		grid[i].cell_no = i;
		grid[i].num_adj_cells = 0;

		/* init grid key indices */
		for(j=0;j<MAX_LIST;j++)
		{
			if(key_indices[i][j])
			{
				grid[i].adjacent[j] = &grid[key_indices[i][j]];
				grid[i].num_adj_cells++;
			}
			else
			{
				grid[i].adjacent[j] = NULL;
				grid[i].num_adj_cells = 0;
			}
		}
	}
	memset(&GridWords, 0, sizeof(GridWords));
}

/* Form all possible word combinations in
 * the grid and store in GridWords
 * depth first traversal
 * */
static void FormWordsFromGrid(void)
{
	int i, j;
	Wordlist_t *cur = &GridWords;

	// for each cell in the grid
	for (i = 0; i < GRID_SIZE; i++)
	{
		char new_word[MAX_WORD_SIZE+1] = {0};
		int visited[GRID_SIZE] = {0};
		int visited_count = 0;
		Cell_t *cur_cell = &grid[i];
		// for each adjacent cell of the current cell
		// find out all the possible words that
		// can be formed
		for(j=0;j<MAX_LIST;j++)
		{
			int k = 0;
			if(visited[j] == 0)
			{
				char ch = GetLetterAt(cur_cell, k);
				if (ch != '\0')
				{
					visited[j] = 1;
					visited_count++;
					new_word[j] = ch;
					new_word[j+1] = '\0';
				}
				else
				{
					while(visited_count<GRID_SIZE)
					{
						ch = GetLetterAt(cur_cell->adjacent[k], 0);
					}
				}
			}
			/* make words here*/
			/* add to Gridwords list */
			strcpy(cur->search_word, new_word);
			cur->next = (Wordlist_t *)malloc(sizeof(Wordlist_t));
			cur  = cur->next ;
		}
	}
}

static char GetLetterAt(Cell_t *pCell, int adj)
{
	char ch = '\0';
	if(pCell)
	{
		if (pCell->adjacent[adj])
		{
			ch = pCell->adjacent[adj]->value;
		}
	}
	return ch;
}

void Iterate(Cell_t * pGrid, int *visited, char *form_str)
{
	if(pGrid)
	{
		visited[pGrid->cell_no] = 1;
		*form_str = pGrid->value;
		/* iterate over all the adjacent cells
		 * connected to the curent cell
		 * and perform dfs on those cells
		 * if they have not been visited before */
		int i;
		for(i=0;i<pGrid->num_adj_cells;i++)
		{
			if(!visited[pGrid->adjacent[i]->cell_no])
			{
				Iterate(pGrid->adjacent[i],visited, form_str+1);
			}
		}
	}
}


int compchar(const void *ap, const void *bp)
{
	const char *a = (char*)ap;
	const char *b = (char*)bp;
	if (*a < *b)
		return (-1);
	else if (*a == *b)
		return (0);
	else
		return (1);
}
/*
*----------------------------------------------------------------------
*   Export Functions Definitions
*----------------------------------------------------------------------
*/




/* End of File assignment.c */
