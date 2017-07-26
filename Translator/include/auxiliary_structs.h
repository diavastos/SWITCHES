/**************************************************/
/*                                                */
/*  File:        translator_structs.h             */
/*  Description: Structures that are used by the  */
/*               translator for internal purposes */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/



/* Structure for counting the brackets for pragma directives */

typedef struct bracket{
	
	int 				id;
	int   				counter;
	struct bracket* 	next;
	
}bracket;



/* Application global data structures [ Retrieved from __global keyword ]*/

typedef struct global{
	
	char* 				variableName;
	struct global* 		next;
	
}global;


/* Structures used fro the iterative implementation of the Transitive Reduction operation */

typedef struct Stack{
	
    int     *data;
    int     size;
}Stack;
