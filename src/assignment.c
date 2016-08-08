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
static WordListIterator_t GridWords;
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
static void FormWordsFromGrid(void);

void AddNewWord(char *new_word);
void findNextAdjCell(Cell_t * pGrid, int *visited, char * new_word, char *cont_str);

int cmpstr(const void* a, const void* b);

/*
*----------------------------------------------------------------------
*   Private Functions Definitions
*----------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
	FILE * 	inputFP; 		/*!< Input File Pointer */
	FILE * 	outputFP; 		/*!< Input File Pointer */
	char 	buf[BUF_SIZE];	/*!< buffer to hold data to read/write */
	int 	iter = 0;
	size_t strings_len;
	char *lookup;
	int lookup_i;
	char **ArrayOfWords = NULL; 		/* String List */

#if 1
	char file_word[MAX_WORD_SIZE+2];
	char **ArrayOfWordsFromFile = NULL; /* String List */
	int noOfLines = 0;


	int found = 0;
	int count = 0;
#endif
	Wordlist_t *prev_word, *cur_word = &GridWords.word_list;

	if (argc != 4 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s /usr/share/dict/words /home/amedev/test/temp abanzqzdrrorrnrr\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	InitializeGrid(argv[3]);

	FormWordsFromGrid();

	ArrayOfWords = (char**)malloc(GridWords.count * sizeof(char*));
	/*printf("finished malloc of string pointers\n");*/
	cur_word = &GridWords.word_list;
	while (cur_word)
	{
		ArrayOfWords[iter] = cur_word->search_word;
		/* printf("[%d] = %s\n",iter, ArrayOfWords[iter]);*/
		cur_word = cur_word->next;
		iter++;
	}
	strings_len = sizeof(ArrayOfWords) / sizeof(char *);
	qsort(ArrayOfWords, strings_len, sizeof(char *), cmpstr);

	/* Open input and output files */
	if ((inputFP = fopen(argv[1], "r")) == NULL)
	{
		printf("ERR: open %s file.\n", argv[1]);
		exit(1);
	}

	if ((outputFP = fopen(argv[2], "w+")) == NULL)
	{
		printf("ERR: open %s file.\n", argv[2]);
		exit(1);
	}

	GridWords.outputFP = outputFP;

	/* Read and store in a string list.*/
	while(fgets(file_word, MAX_WORD_SIZE+2, inputFP) != NULL)
	{
		/* Remove the trailing newline character */
		if(strchr(file_word,'\n'))
			file_word[strlen(file_word)-1] = '\0';
		ArrayOfWordsFromFile = (char**)realloc(ArrayOfWordsFromFile, sizeof(char**)*(noOfLines+1));
		ArrayOfWordsFromFile[noOfLines] = (char*)calloc(MAX_WORD_SIZE,sizeof(char));
		strcpy(ArrayOfWordsFromFile[noOfLines], file_word);
		noOfLines++;
	}
	strings_len = sizeof(ArrayOfWordsFromFile) / sizeof(char *);
	qsort(ArrayOfWordsFromFile, strings_len, sizeof(char *), cmpstr);

	iter = 0;
	/* until we encounter end of input or an error */
	while (iter<noOfLines)
	{
		int checker = 0;
		int is_punct = 0;

		/* printf("searching for %s\n...", ArrayOfWordsFromFile[iter]);*/
		while (ArrayOfWordsFromFile[iter][checker])
		{
			ArrayOfWordsFromFile[iter][checker] = tolower(ArrayOfWordsFromFile[iter][checker]);
			if (ispunct(ArrayOfWordsFromFile[iter][checker]))
			{
				is_punct = 1;
				break;
			}
			checker++;
		}

		if(is_punct || (strlen(ArrayOfWordsFromFile[iter]) > 16) || (strlen(ArrayOfWordsFromFile[iter]) == 0))
		{
			iter++;
			continue;
		}

		/* core search goes here */
		lookup_i = 0;
		while (lookup_i < GridWords.count)
		{
			lookup = ArrayOfWords[lookup_i];
			/* word match : sorted arrays
			 * if the first character is <= the search string first char
			 * process else continue with looping
			 * if the string len matches, then process else continue looping
			 * */
			if(ArrayOfWordsFromFile[iter][0] <= lookup[0])
			if( strlen(ArrayOfWordsFromFile[iter]) == strlen(lookup) )
			{
				if (strcmp(lookup, ArrayOfWordsFromFile[iter]) == 0)
				{
					/* list the found word */
					printf("%s\n", lookup);

					/* clear the array */
					ArrayOfWords[lookup_i][0] = '\0';
					break;
				}
			}
			lookup_i++;
		}
		iter++;
	}
	/* free memory */
	cur_word = &GridWords.word_list;
	while(cur_word)
	{
		Wordlist_t *temp;
		temp = cur_word;
		cur_word = cur_word->next;
		free(temp);
	}
	iter = 0;
	while(iter<noOfLines)
	{
		free(ArrayOfWordsFromFile[iter]);
		iter++;
	}
	free(ArrayOfWordsFromFile);

	printf("finished searching...\n");

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
	memset(&GridWords, 0, sizeof(GridWords));
	GridWords.curr = &GridWords.word_list;
	GridWords.word_list.next = NULL;
}

/* Form all possible word combinations in
* the grid and store in GridWords
* depth first traversal
* */
static void FormWordsFromGrid(void)
{
	int i;

	/* for each cell in the grid */
	for (i = 0; i < GRID_SIZE; i++)
	{
		char new_word[MAX_WORD_SIZE + 1];
		int visited[GRID_SIZE] = { 0 };
		Cell_t *cur_cell = &grid[i];

		/* for each adjacent cell of the current cell
		 find out all the possible words that
		 can be formed
		 set all characters to -1, so that
		 we can start filling in new characters
		 along a path */
		findNextAdjCell(cur_cell, visited, new_word, new_word);
	}
}

void findNextAdjCell(Cell_t * pGrid, int *visited, char * new_word, char *cont_str)
{
	if (pGrid)
	{
		int i;
		visited[pGrid->cell_no] = 1;
		*cont_str = pGrid->value;
		cont_str++;
		*cont_str = '\0';

		/* Add a new word because a new letter just got added
		 to the current word */
		AddNewWord(new_word);

		/* iterate over all the adjacent cells
		* connected to the curent cell */
		for (i = 0; i<pGrid->num_adj_cells; i++)
		{
			if (!visited[pGrid->adjacent[i]->cell_no])
			{
				findNextAdjCell(pGrid->adjacent[i], visited, new_word, cont_str);
			}
		}
		visited[pGrid->cell_no] = 0;
	}
	*cont_str = '\0';
}

void AddNewWord(char *new_word)
{
	Wordlist_t *temp = (Wordlist_t *)malloc(sizeof (Wordlist_t));
	if(temp)
	{
		strcpy(temp->search_word, new_word);
		temp->word_len = strlen(new_word);
		GridWords.curr->next = temp;
		temp->next = NULL;
		GridWords.curr = temp;
		GridWords.count++;
	}
}

int cmpstr(const void* a, const void* b)
{
    const char* aa = (const char*)a;
    const char* bb = (const char*)b;
    return strcmp(aa, bb);
}


/*
*----------------------------------------------------------------------
*   Export Functions Definitions
*----------------------------------------------------------------------
*/




/* End of File assignment.c */
