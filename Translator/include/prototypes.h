/**************************************************/
/*                                                */
/*  File:        prototypes.h                     */
/*  Description: Contains the prototypes of all   */
/*               functions in the translator      */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/

#include "SG_Structs.h"
#include "NSGA_Structs.h"
#include "auxiliary_structs.h"



/******************************************************************************/
/*                         commandLines.c                                     */
/******************************************************************************/

void printHelp();																/* Print in the terminal the help message of how to use Switches */
void printVersion();															/* Print the version of the SWITCHES Translator */
void recognizeCommandlineArguments(int argc, char **argv);						/* Read & recognize the command line arguments given by the user */




/******************************************************************************/
/*                        printSourceCode.c                                   */
/******************************************************************************/

void printInMainFile_ParallelFunctions(parallel_function**, int);					/* Print source code in the main files */
void printInThreadpoolFile(SG** );													/* Print the threadpool creating functions in the [ sw_threadpool.c ] file */
void printInSwFile(SG** );															/* Print declarations and definitions in the [ sw.h ] file */
void printInTaoFile(SG** );															/* Print TAO classes in the [ sw_tao.h ] file */
void printInThreadsFile_headerSourceCode(SG** );									/* Print the header source code for the threads in the [ sw_threads.c ] file */
void printInThreadsFile_SwitchesDeclaration(SG**);									/* Print switches in threads files */
void printInThreadsFile_ResetSwitchesFunctions(SG**);								/* Print Reset Switches Functions in [ sw_threads.c ] */
void printInThreadsFile_JobsThreadsFunction_SWITCHES(SG**);							/* Print the Jobs Thread Function in [ sw_threads.c ] for SWITCHES */
void printInThreadsFile_JobsThreadsFunction_TAO(SG**);							/* Print the Jobs Thread Function in [ sw_threads.c ] for TAOSW */
void printInThreadsFile_ParallelFunctionsHeader(parallel_function**, int);			/* Print Parallel Functions Header in [ sw_threads.c ] */
void printInThreadsFile_ParallelFunctionsHeader_TAO(parallel_function**, int);			/* Print Parallel Functions Header in [ sw_threads.c ] */
void printInThreadsFile_TaskSourceCode(parallel_function **, int, int);				/* Print the source code of task in a parallel function in [ sw_threads.c ] */
void printInThreadsFile_TaskSourceCode_TAO(parallel_function **, int, int);				/* Print the source code of task in a parallel function in [ sw_threads.c ] */
void printInThreadsFile_TurnSwitchesOn(parallel_function **, int, int);				/* Print the turn ON switches for each task in [ sw_threads.c ] */
void printInThreadsFile_BreakLoopWhenFinished(parallel_function **, int);			/* Print the break from the infinite loop of each parallel function in [ sw_threads.c ] */
void printInThreadsFile_ForStatement();												/* Print the FOR (...) statement */
void printInThreadsFile_ReductionVariables(parallel_function *);					/* Print the reduction variables that exist in the Graph */
void printInThreadsFile_taskCounters(SG** Graph);                                   /* Print the taskCompleted counters */



/******************************************************************************/
/*                        handleBisonRules.c                                  */
/******************************************************************************/

void handlePragma_parallelConstruct(SG **, int, int, dataList **, int);			/* What to do when a [ parallel ] construct is found:
																				 *  -> Add a new parallel function in the SG
																				 * 
																				 *  Parameters:
																				 *     - [SG** ]      : The SG
																				 *     - [int ]       : The number of kernels from the parallel pragma directive
																				 *     - [int ]       : The default(...) state
																				 *     - [dataList** ]: The temporary keeper of all the lists given on the directive 
																				 *                      (e.g. shared(...), private(...), ...)
																				 *     - [int ]       : The id of the current paralle function
																				 */
																				 
void handlePragma_variableList(int, dataList **, char*, char*, char*, arrayIndex**);		/* What to do when a [ variable list ] clause is found:
																							 *  -> Add all elements of the lists found on a single pragma to a 
																							 *     temporary structure based on the type of list (e.g. shared(...), 
																							 *     private(...), etc.)
																							 * 
																							 *  Parameters:
																							 *     - [int ]          : From which list are you reading now
																							 *     - [dataList** ] 	 : A pointer to the entire temporary structure that hold all the lists
																							 *     - [char* ]     	 : The variable name to copy to the temporary list
																							 *     - [char* ]     	 : The variable type to copy to the temporary list
																							 *     - [char* ]     	 : The reduction type to copy to the temporary list (if list is a reduction list)
																							 *     - [arrayIndex** ] : The depend list array indexes
																							 */
																				 
void handlePragma_threadPrivateDirective(SG **, dataList **);					/* What to do when a [ threadprivate ] directive is found:
																				 *  -> Copy the temporary threadprivate list to the SG threadPrivate data list
																				 * 
																				 *  Parameters:
																				 *     - [SG** ]       : A pointer to the SG
																				 *     - [dataList** ] : A pointer to the entire temporary structure that hold all the lists
																				 */
																				 
void handlePragma_sectionsConstruct(SG **, dataList **);						/* What to do when a [ sections ] directive is found:
																				 *  -> Add a new section in a parallel function of the SG
																				 * 
																				 *  Parameters:
																				 *     - [SG** ]       : A pointer to the SG
																				 *     - [dataList** ] : A pointer to the entire temporary structure that hold all the lists
																				 */
																				 
void handlePragma_sectionDirective(SG **, dataList **, int, int);				/* What to do when a [ section ] directive is found:
																				 *  -> Add a new task in a section of a parallel function of the SG
																				 * 
																				 *  Parameters:
																				 *     - [SG** ]       : A pointer to the SG
																				 *     - [dataList** ] : A pointer to the entire temporary structure that hold all the lists -- NOT REALLY NEEDED
																				 *     - [int ]        : The id of the current parallel function
																				 *     - [int ]        : The id of the current task
																				 */
																				 
void handlePragma_taskConstruct(SG **, dataList **, int, int, int, int, int, intStr, int, int, int);				/* What to do when a [ task ] directive is found:
																													 *  -> Add a new task in a parallel function of the SG
																													 *  
																													 * Parameters:
																													 *     - [SG** ]       : A pointer to the SG
																													 *     - [dataList** ] : A pointer to the entire temporary structure that hold all the lists -- NOT REALLY NEEDED
																													 *     - [int ]        : Type of the task
																													 *     - [int ]        : Number of kernels to execute this task
																													 *     - [int ]        : Local State of default varialbes (SHARED | NONE)
																													 *     - [int ]        : Task Priority
																													 *     - [int ]        : Scheduling Policy
																													 *     - [int ]        : Scheduling Policy Chunk size
																													 *     - [int ]        : SIMD Length
																													 *     - [int ]        : The id of the current parallel function
																													 *     - [int ]        : The id of the current task
																													 */																				 




/******************************************************************************/
/*                       SGFunctions.c                                        */
/******************************************************************************/

void printSG(SG**, int);														/* -> Print the SG for debugging */

void addParallelFunctionToGraph(parallel_function **, int, int, dataList **);	/* -> Add a new parallel function in the SG graph when you find a parallel construct
																				 * 
																				 * Parameters:
																				 *     - [parallel_function** ] : A pointer to the parallel functions of Graph
																				 *     - [int ]                 : The number of kernels for this function (if not
																				 *                                not specified in the pragma, then use the global 
																				 *                                kernel number)
																				 *     - [int ]                 : The default(...) state (SHARED or NONE)
																				 *     - [dataList** ]          : A pointer to the temporary structure of the data lists
																				 */ 
																				 
void addTaskToParallelFunction(parallel_function **, dataList **, int, int, int ,int, int ,intStr, int, int);				/* -> Add a new task in a specific parallel function in the SG
																															 *
																															 * Parameters: 
																															 *     - [parallel_function** ] : A pointer to the parallel functions of Graph
																															 *     - [dataList** ]          : A pointer to the temporary structure of the data lists
																															 *     - [int ]                 : Type of the task
																															 *     - [int ]                 : Number of kernels to execute this task
																															 *     - [int ]                 : Local State of default varialbes (SHARED | NONE)
																															 *     - [int ]                 : Task Priority
																															 *     - [int ]                 : Scheduuling Policy
																															 *     - [int ]                 : Scheduling Policy Chunk size
																															 *     - [int ]                 : SIMD Length
																															 *     - [int ]                 : The current task
																															 */
																				 
void addToDataList(dataList**, char*, char*, char*, arrayIndex**);				/* -> Add a new variable in the temporary data structure with the lists
																				 * -> If this new list doesnt exist, then create it
																				 *  
																				 * Parameters:
																				 *     - [dataList** ]   : A pointer to the specific temporary list that will copy the new variable
																				 *     - [char* ]        : New Variable name
																				 *     - [char* ]        : New variable type
																				 *     - [char* ]        : Reduction Type
																				 *     - [arrayIndex** ] : Array Indexes
																				 */
																				 
void addSynchronizationGraph(SG**);												/* -> Add a new Graph in the SG
																				 * -> In case a program needs more than one SG
																				 *  
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */
																				 
void copyDataListToSG(dataList **, dataList**);									/* -> Copy temp data of a single list to a list in the SG
																				 * 
																				 *  Parameters:
																				 *     - [dataList** ] : The temp list to copy from
																				 *     - [dataList** ] : The SG list to copy to
																				 */
																				 
void addSectionToParallelFunction(parallel_function **, dataList **);			/* -> Add a new section in a parallel function of the SG graph																				 * Parameters:
																				 * 
																				 * Parameters:
																				 *     - [parallel_function** ] : A pointer to the parallel functions of the SG
																				 * 								To find to which function to link the new section
																				 *     - [dataList** ]          : A pointer to the temporary structure of the data lists
																				 */
																				  
void addTaskToSection(parallel_function **, dataList **, int);			 		/* -> Add a new task in a section of a parallel function of the SG graph																				 * Parameters:
																				 *     - [parallel_function** ] : A pointer to the parallel functions of the SG
																				 * 								To find to which function to link the new task 
																				 *     - [dataList** ]          : A pointer to the temporary structure of the data lists
																				 *     - [int ]       	   		 : The current task
																				 */
																				  
void addToIndexes(arrayIndex**, int, int, char*, char*);						/* -> Add a indexes to temporary indexes list for array variables
																				 * -> If this new list doesnt exist, then create it
																				 * 
																				 *  Parameters:
																				 *     - [arrayIndex** ]  : Array Indexes
																				 *     - [int ]           : arrayIndex Start  value if it is integer (I_CONSTANT)
																				 *     - [int ]           : arrayIndex End value if it is integer (I_CONSTANT)
																				 *     - [char* ]         : arrayIndex Start value if it is a string (IDENTIFIER)
																				 *     - [char* ]         : arrayIndex End value if it is a string (IDENTIFIER)
																				 */		
																				 
void addTypeOfVariable(SG **, char*, char*);									/* -> Add the type of variables that exist in the SG
																				 * -> Only basic types are recognized for now (char, int, float, double, bool, long, short)
																				 * 
																				 *  Parameters:
																				 *     - [SG**  ]  : The SG graph
																				 *     - [char* ]  : The variable type
																				 *     - [char* ]  : The variable name
																				 */		
																				 
void searchVariableList(dataList **, char*, char*);								/* -> Search a given list for a specific variable and add its type
																				 * -> Only basic types are recognized for now (char, int, float, double, bool, long, short)
																				 * 
																				 *  Parameters:
																				 *     - [dataList**  ]  : The given data list
																				 *     - [char* 	  ]  : The variable type
																				 *     - [char* 	  ]  : The variable name
																				 */		
																				 
																				 
																				 
/******************************************************************************/
/*                        auxiliary_functions.c                               */
/******************************************************************************/

void addBracketsCounter(bracket**, int);										/* Add a bracket counter in the linked list when you find a bracket that belongs to a pragma
																				 * 
																				 * Parameters:
																				 *     - [bracket** ]      : The bracket list
																				 *     - [int ]       	   : The current bracket counter
																				 */
																				 
void removeBracketsCounter(bracket**);											/* Remove a bracket counter from the linked list when you find the matching pragma bracket is closed
																				 * 
																				 * Parameters:
																				 *     - [bracket** ]      : The bracket list
																				 */

bool checkBracketCounter(bracket**, int);										/* Check the current bracket counter if it matches the last inserted bracket counter in the list
																				 * 
																				 * Parameters:
																				 *     - [bracket** ]      : The bracket list
																				 *     - [int ]  	       : The current bracket counter
																				 */
																				 
void recursive_DFS(int, int, int);												/* Depth-first search implementation to find duplicate paths between two nodes of the SG
																				 * 
																				 * Parameters:
																				 *     - [int ] : The source node
																				 *     - [int ] : The destination node
																				 *     - [int ] : The number of the tasks
																				 */
																				 
void Stack_Init(Stack*, int);													/* Stack functions for iterative implementation of the Transitive Reduction Operations */
int  Stack_Top(Stack*);
void Stack_Push(Stack*, int, int);
int  Stack_Pop(Stack*);
int  StackIsEmpty(Stack*);

void printErrorMessages();														/* Print possible error messages that were recognized during parsing */



/******************************************************************************/
/*                        offlineScheduler.c                                  */
/******************************************************************************/

void offlineScheduling_CreateDependencies(SG**);								/* Read the in/out/inout data lists and find the dependencies between tasks of the same function.
																				 * Update the consumer/producer lists of each task approriately.
																				 * 
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */
																				 								 
void offlineScheduling_TransitiveReductionOfConsumers(SG**);					/* Remove unnecessary dependencies from tasks
																				 * Transitive reduction operation on the consumers
																				 * 
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */
																				 
void offlineScheduling_TransitiveReductionOfProducers(SG**);					/* Remove unnecessary dependencies from tasks
																				 * Transitive reduction operation on the consumers
																				 * 
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */
                                                                                 
void offlineScheduling_AssignKernelsToParallelFunctions(SG**);					/* Assign which kernels will execute at least a task in the current function
																				 * 
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */
                                                                                 
void offlineScheduling_AssignKernelsToParallelFunctionsFromFile(SG**);          /* Assign which kernels will be used for executing tasks of a parallel function
                                                                                 * Get these values from the scheduling input file
                                                                                 * 
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */

void offlineScheduling_AssignVirtualKernelsToTasks(SG**);						/* Assign Fake kernels to each task in the Graph for calculating correctly the loop_start of each 
																				 * kernel && Optimize the Switche ON process.
																				 * 
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */
																				 
void offlineScheduling_AssignKernelsToTasks_RoundRobin(SG**);					/* Assign kernels to task in the current function in a Round-Robin technique
																				 * No other optimizations are taken into account with this policy
                                                                                 * 
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */
                                                                                 
void offlineScheduling_AssignKernelsToTasks_Random(SG**);					    /* Assign kernels to task in the current function in a Random order
																				 * 
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */
                                                                                 
void offlineScheduling_AssignKernelsToTasks_File(SG**);					        /* Load the scheduling policy from a file
                                                                                 * 
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */
                                                                                 
int offlineScheduling_CountKernelsToTasks_NSGA(SG**);					        /* Count all Kernels used for all tasks
                                                                                 * 
																				 * Parameters:
																				 *     - [SG** ] : A pointer to the SG strucutre
																				 */                                                                                 
                                                                                 
                                                                                 

/******************************************************************************/
/*                       NSGA_Functions.c                                        */
/******************************************************************************/

void allocatePopulation(Population**, int);     									/* -> Allocate Memory for the population */
void deallocatePopulation(Population**);											/* -> Deallocate memory for the population */
void randomize();	                        										/* -> Get seed number for random and start it up */
void warmup_random(double);               										    /* -> Get randomize off and running */
void advance_random();                     										    /* -> Create next batch of RANDOM_NUM random numbers */
double randomperc();                      										    /* -> Fetch a single random number between 0.0 and 1.0 */
int rnd(int, int);                      										    /* -> Fetch a single random integer between low and high including the bounds */
void NSGA_initializePopulation(SG**, Population**);   							    /* -> Randomly initialize population */
void NSGA_printPopulationToFiles(Population**, int);    					        /* -> Print Generation to scheduling files */
void NSGA_evaluatePopulation(Population**, int);            					    /* -> Evaluate Population */
void NSGA_assign_rank_and_crowding_distance(Population**);    					    /* -> Assign Rank and Crowding Distance */
void NSGA_selection(Population*, Population*);            					        /* ->  Routine for tournament selection, it creates a new_pop from old_pop by performing TOURNAMENT SELECTION and CROSSOVER */
Child* tournament(Child*, Child*);                      					        /* ->  Routine for Binary Tournament */
void crossover(Child*, Child*, Child*, Child*);                      		        /* ->  Function to cross two individuals */
void NSGA_mutation(Population **);                     		                        /* ->  Function to perform mutation in a population */
void insert(list*, int);                                                            /* ->  Insert an element in list */
list* del(list*);                                                                   /* ->  Delete an element from the list */
int check_dominance(Child*, Child*);                                                /* ->  Check Dominance of a child against another */
void assign_crowding_distance_list (Population*, list*, int);                       /* ->  Compute crowding distance based on ojbective function values when the population in the form of a list */
void assign_crowding_distance(Population*, int*, int**, int);                       /* ->  Compute crowding distances */
void quicksort_front_obj(Population*, int, int*, int);                              /* ->  Randomized quick sort routine to sort a population based on a particular objective chosen */
void q_sort_front_obj(Population *, int , int*, int, int);                          /* ->  Actual implementation of the randomized quick sort used to sort a population based on a particular objective chosen */
void NSGA_merge(Population**, Population**, Population**);                          /* ->  Merge two populations into one */
void copy_child(Child*, Child*);                                                    /* ->  Copy child by child from two different populations */
void NSGA_fill_nondominated_sort(Population**, Population**);                            /* ->  Non-dominated Sorting */
void assign_crowding_distance_indices(Population*, int, int);                       /* ->  Non-dominated Sorting */
void crowding_fill(Population*, Population*, int, int, list*);                      /* ->  Fill a population with individuals in the decreasing order of crowding distance */
void quicksort_dist(Population*, int*, int);                                        /* ->  Randomized quick sort routine to sort a population based on crowding distance */
void q_sort_dist(Population*, int*, int, int);                                      /* ->  Actual implementation of the randomized quick sort used to sort a population based on crowding distance */









