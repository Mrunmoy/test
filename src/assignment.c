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
#include <ctype.h>
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
	int word_len;
	char search_word[MAX_WORD_SIZE + 1];
	struct Wordlist_t *next;
};
typedef struct Wordlist_t Wordlist_t;

struct WordListIterator_t
{
	Wordlist_t word_list;
	Wordlist_t *curr;
	int count;
	FILE *outputFP;
};
typedef struct WordListIterator_t WordListIterator_t;

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
	{ 1, 4, 5 },
	{ 0, 2, 4, 5, 6 },
	{ 1, 3, 5, 6, 7 },
	{ 2, 6, 7 },
	{ 0, 1, 5, 8, 9 },
	{ 0, 1, 2, 4, 6, 8, 9, 10 },
	{ 1, 2, 3, 5, 7, 9, 10, 11 },
	{ 2, 3, 6, 10, 11 },
	{ 4, 5, 9, 12, 13 },
	{ 4, 5, 6, 8, 10, 12, 13, 14 },
	{ 5, 6, 7, 9, 11, 13, 14, 15 },
	{ 6, 7, 10, 14, 15 },
	{ 8, 9, 13 },
	{ 8, 9, 10, 12, 14 },
	{ 9, 10, 11, 13, 15 },
	{ 10, 11, 14 }
};

static Cell_t grid[16];
char gridletters[GRID_SIZE] = { 0 };

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

static void InitializeGrid(char *grid_chars);
int CheckWordMatch(Cell_t * pGrid, int *visited, char * search_word, int remaining);


/*
*----------------------------------------------------------------------
*   Private Functions Definitions
*----------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
	FILE * 	inputFP; 		/*!< Input File Pointer */
	char file_word[BUF_SIZE];
	int iter;
	/* char *str = "abanzqzdrrorrnrr"; */

#if 1
	if (argc != 3 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s /usr/share/dict/words pbanzqrdrkhrrnwr\n", argv[0]);
		exit(EXIT_FAILURE);
	}
#endif
	/* InitializeGrid(str); */
	InitializeGrid(argv[2]);


	/* Open input and output files */
	if ((inputFP = fopen(argv[1], "r")) == NULL)
	{
		printf("ERR: open %s file.\n", argv[1]);
		exit(1);
	}

	iter = 0;
	/* Read and store in a string list.*/
	while(fgets(file_word, BUF_SIZE, inputFP) != NULL)
	{
		int i;
		int checker = 0;
		int is_punct = 0;
		int visited[GRID_SIZE] = {0};
		/* Remove the trailing newline character */

		/*if(strcmp(file_word, ""))
			printf("%s\n", file_word);*/
		while (file_word[checker])
		{
			file_word[checker] = tolower(file_word[checker]);
			if (ispunct(file_word[checker]))
			{
				is_punct = 1;
				break;
			}
			else if(file_word[checker] == '\n')
			{
				file_word[checker] = '\0';
			}
			checker++;
		}

		if(is_punct || (strlen(file_word) > GRID_SIZE) || (strlen(file_word) == 0))
		{
			iter++;
			continue;
		}

		for(i=0;i<GRID_SIZE;i++)
		{
			if(file_word[0] == grid[i].value)
			{
				if(strlen(file_word) == 1)
				{
					printf("%s\n", file_word);
					break;
				}
				if(CheckWordMatch(&grid[i], visited, file_word, strlen(file_word)))
				{
					printf("%s\n", file_word);
					break;
				}
			}
		}
		iter++;
	}

	/*printf("finished searching...\n");*/

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
	int i, j, k;
	char letters[GRID_SIZE + 1];
	strcpy(letters, grid_chars);
	memset(grid, 0, sizeof(grid));

	for (i = 0; i<GRID_SIZE; i++)
	{
		gridletters[i] = tolower(letters[i]);
		grid[i].value = tolower(letters[i]);
		grid[i].visited = 0;
		grid[i].cell_no = i;
		grid[i].num_adj_cells = 0;

		/* init grid key indices */
		for (j = 0, k = 0; j<MAX_LIST; j++)
		{
			if (key_indices[i][j])
			{
				grid[i].adjacent[k++] = &grid[key_indices[i][j]];
				grid[i].num_adj_cells++;
			}
		}
	}
}

int CheckWordMatch(Cell_t * pGrid, int *visited, char * search_word, int remaining)
{
	int result = 0;
	/* printf("to match %s [%d]\n", search_word, remaining);*/
	if (pGrid)
	{
		int i;
		if( *search_word == pGrid->value)
		{
			remaining--;
			visited[pGrid->cell_no] = 1;

			if(remaining > 0)
			{
				/* keep searching until a match is
				 * found or we are out of options */
				/* iterate over all the adjacent cells
				 * connected to the current cell */
				for (i = 0; i<pGrid->num_adj_cells; i++)
				{
					if (!visited[pGrid->adjacent[i]->cell_no])
					{
						result = CheckWordMatch(pGrid->adjacent[i], visited, search_word+1, remaining);
						if(result)
							break;
					}
				}
			}
			else
			{
				result = 1;
			}
			visited[pGrid->cell_no] = 0;
		}
	}
	return result;
}

/*
*----------------------------------------------------------------------
*   Export Functions Definitions
*----------------------------------------------------------------------
*/




/* End of File assignment.c */
