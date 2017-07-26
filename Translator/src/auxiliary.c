/**************************************************/
/*                                                */
/*  File:        auxiliary_functions.c            */
/*  Description: Functions that are used for the  */
/*               internal tasks of the Translator */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/


#include "definitions.h"



extern int line;
extern int pass;
extern int kernels;
extern bool firstPass;
extern int targetSystem;
extern bool transactions;
extern char **inputFiles;
extern int totalInputFiles;
extern bool inMainFunction;
extern FILE *inp, *outp_sw_main, *outp_sw_h, *outp_sw_threadpool, *outp_sw_threads;


extern bool	**tred_array;
extern int	counter;
extern bool	flag;



/*********************** Add new brackets counter **************************/


void addBracketsCounter(bracket** brackets, int counter){
	
	bracket* tempBracket = *brackets;
	
	if(!(*brackets))
	{
		(*brackets) = (bracket *)malloc(sizeof(bracket));
		(*brackets)->id 	  = 1;
		(*brackets)->counter  = counter;
		(*brackets)->next 	  = NULL;
	}
	else
	{
		while(tempBracket && tempBracket->next)
			tempBracket = tempBracket->next;
		
		tempBracket->next = (bracket *)malloc(sizeof(bracket));
		tempBracket->next->id 		= tempBracket->id+1;
		tempBracket->next->counter	= counter;
		tempBracket->next->next 	= NULL;
	}	
}



/*********************** Remove brackets counter **************************/


void removeBracketsCounter(bracket** brackets){
	
	bracket* tempBracket = *brackets;
	
	if(!(tempBracket->next))
	{
		free(tempBracket);
		*brackets = NULL;		
	}
	else
	{	
		while(tempBracket && tempBracket->next && tempBracket->next->next)
			tempBracket = tempBracket->next;
		
		free(tempBracket->next);
		tempBracket->next = NULL;
	}	
}



/*********************** Check current brackets counter **************************/


bool checkBracketCounter(bracket** brackets, int counter){
	
	bracket* tempBracket = *brackets;
	
	while(tempBracket && tempBracket->next)
		tempBracket = tempBracket->next;
	
	if(tempBracket->counter == counter)
		return TRUE;
	else
		return FALSE;
}



/*********************** Depth-First Search to find duplicate paths between two nodes of the SG **************************/

void recursive_DFS(int start, int end, int N){

	int j;
	
	if(start == end && counter == 1)
		flag = TRUE;
	
	if(start == end && counter == 0)
		counter = 1;
  
	for(j = 0; j < N; j++)
		if(tred_array[start][j] == TRUE)
			recursive_DFS(j, end, N);
}



/*********************** Stack functions used for the iterative implementation of the Transitive Reduction Operation **************************/


void Stack_Init(Stack *S, int stackSize)
{
	S->data = (int*)malloc(sizeof(int)*stackSize);
    S->size = 0;
}

int Stack_Top(Stack *S)
{
    if (S->size == 0) {
        fprintf(stderr, "Error: stack empty\n");
        return -1;
    } 

    return S->data[S->size-1];
}

void Stack_Push(Stack *S, int d, int stackSize)
{
    if (S->size < stackSize)
        S->data[S->size++] = d;
    else
        fprintf(stderr, "Error: stack full\n");
}

int Stack_Pop(Stack *S)
{
    if (S->size == 0)
        fprintf(stderr, "Error: stack empty\n");
    else
        S->size--;
      
    return S->data[S->size];
}

int StackIsEmpty(Stack *S)
{
  return (S->size == 0);
}



/*********************** Print possible error messages detected **************************/


void printErrorMessages(){
	
	if(inMainFunction)
	{
		ERROR_COMMANDS("Main Function must have a [ %s ] statement!\n\t-> Execution may not finish correctly otherwise.", "return")
	}
	
}


	     


