/**************************************************/
/*                                                */
/*  File:        offlineScheduler.c               */
/*  Description: After building the first version */
/*               of the SG (while reading the     */
/*               1st parse). Then get the SG and  */
/*               apply the dependencies, the      */
/*               scheduling and any optimizations */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/


#include "definitions.h"


bool	**tred_array;
int		counter = 0;
bool	flag = FALSE;


extern int line;
extern int pass;
extern int kernels;
extern bool firstPass;
extern int targetSystem;
extern bool transactions;
extern char **inputFiles;
extern char *schedulingInputFile;
extern int totalInputFiles;
extern FILE *inp, *outp_sw_main, *outp_sw_h, *outp_sw_threadpool, *outp_sw_threads, *outp_sw_tao_h;
extern int      maxCores;
extern int      hThreads;
extern int      OSthread;



/*********************** Create the dependencies in the SG **************************/

void offlineScheduling_CreateDependencies(SG** Graph){
		
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	parallel_function 	*tempFunction2;
	section 			*tempSection;
	task 				*tempTask;
	dataList			*tempDataList;
	kernel				**tempKernel;
	
	section 			*tempSection2;
	task 				*tempTask2;
	dataList			*tempDataList2;
	kernel				*tempKernel2;
	
	producer			**tempProducers;
	producer			**tempConsumers;
	
	producer			*tempProducers2;
	producer			*tempConsumers2;


	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
            /** Take care of parallel functions dependencies **/
            
            /*** COPY FROM HERE ***/
					
				tempProducers = &(tempFunction->producers);
				tempConsumers = &(tempFunction->consumers);
				
				/* Check Depend [ IN ] list */
				 				 
				 tempDataList = tempFunction->dependInList; 
				 while(tempDataList)
				 {
					 tempFunction2 = tempGraph->parallel_functions;
					 while(tempFunction2)
					 {
						 if(tempFunction2->id < tempFunction->id)							// Check all my previous tasks
						 {
							 // Check if it is already a producer -- dont put in twice 
							 tempProducers2 = tempFunction->producers;
							 while(tempProducers2)
							 {
								 if(tempProducers2->id == tempFunction2->id)
									break;
								tempProducers2 = tempProducers2->next;
							 }
							 if(tempProducers2)
							 {
								 tempFunction2 = tempFunction2->next;
								 continue;
							 }
								
							 
							 /** Check the Depend [ OUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempFunction2->dependOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempFunction2->id;
										(*tempProducers)->kernels = NULL;           // NOT USED FOR PARALLEL FUNCTIONS DEPENDENCIES
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempFunction->producers;
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempFunction2->id;
										tempProducers2->next->kernels = NULL;
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempFunction2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempFunction2->id;
										(*tempProducers)->kernels = NULL;
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempFunction->producers; 
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempFunction2->id;
										tempProducers2->next->kernels = NULL;
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						tempFunction2 = tempFunction2->next;
					 }
					 tempDataList = tempDataList->next;
				 }
								
				
				/* Check Depend [ Out ] list */
				 
				 tempDataList = tempFunction->dependOutList; 
				 while(tempDataList)
				 {
					 tempFunction2 = tempGraph->parallel_functions;
					 while(tempFunction2)
					 { 
						 if(tempFunction2->id > tempFunction->id)							// Check all my proceeding tasks
						 {
							 // Check if it is already a consumer -- Dont put in the list twice
							 tempConsumers2 = tempFunction->consumers;
							 while(tempConsumers2)
							 {
								 if(tempConsumers2->id == tempFunction2->id)
									break;
								tempConsumers2 = tempConsumers2->next;
							 }
							 if(tempConsumers2)
							 {
								 tempFunction2 = tempFunction2->next;
								 continue;
							 }
							 
							 /** Check the Depend [ IN ] list of my previous tasks **/
							 
							 tempDataList2 = tempFunction2->dependInList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempFunction2->id;
										(*tempConsumers)->kernels = NULL;
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempFunction->consumers;
										while(tempConsumers2 && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempFunction2->id;
										tempConsumers2->next->kernels = NULL;
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempFunction2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempFunction2->id;
										(*tempConsumers)->kernels = NULL;
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempFunction->consumers;
										while(tempConsumers2 && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempFunction2->id;
										tempConsumers2->next->kernels = NULL;
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						tempFunction2 = tempFunction2->next;
					 }
					 tempDataList = tempDataList->next;
				 }
				 
				
				/* Check Depend [ INOUT ] list */
				 
				 
				 tempDataList = tempFunction->dependInOutList; 
				 while(tempDataList)
				 {
					 tempFunction2 = tempGraph->parallel_functions;
					 while(tempFunction2)
					 {
						 if(tempFunction2->id < tempFunction->id)							// Check all my previous tasks
						 {
							 // Check if it is already a producer -- Don't put it in the list twice
							 tempProducers2 = tempFunction->producers;
							 while(tempProducers2)
							 {
								 if(tempProducers2->id == tempFunction2->id)
									break;
								tempProducers2 = tempProducers2->next;
							 }
							 if(tempProducers2)
							 {
								 tempFunction2 = tempFunction2->next;
								 continue;
							 }
							 
							 /** Check the Depend [ OUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempFunction2->dependOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempFunction2->id;
										(*tempProducers)->kernels = NULL;
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempFunction->producers;
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempFunction2->id;
										tempProducers2->next->kernels = NULL;
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempFunction2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempFunction2->id;
										(*tempProducers)->kernels = NULL;
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempFunction->producers;
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempFunction2->id;
										tempProducers2->next->kernels = NULL;
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						
						
						if(tempFunction2->id > tempFunction->id)							// Check all my proceeding tasks
						 {
							 // Check if it is already in the list -- Dont put it in twice
							 tempConsumers2 = tempFunction->consumers;
							 while(tempConsumers2)
							 {
								 if(tempConsumers2->id == tempFunction2->id)
									break;
								tempConsumers2 = tempConsumers2->next;
							 }
							 if(tempConsumers2)
							 {
								 tempFunction2 = tempFunction2->next;
								 continue;
							 }
							 
							 /** Check the Depend [ IN ] list of my previous tasks **/
							 
							 tempDataList2 = tempFunction2->dependInList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempFunction2->id;
										(*tempConsumers)->kernels = NULL;
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempFunction->consumers;
										while(tempConsumers && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempFunction2->id;
										tempConsumers2->next->kernels = NULL;
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempFunction2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempFunction2->id;
										(*tempConsumers)->kernels = NULL;
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempFunction->consumers;
										while(tempConsumers2 && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempFunction2->id;
										tempConsumers2->next->kernels = NULL;
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						tempFunction2 = tempFunction2->next;
					 }
					 tempDataList = tempDataList->next;
				 }				
				 
				 
				 /*** COPY UNTIL HERE ***/
            
            
			/** Take care of functions tasks **/
					
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				/*** COPY FROM HERE ***/
					
				tempProducers = &(tempTask->producers);
				tempConsumers = &(tempTask->consumers);
				
				/* Check Depend [ IN ] list */
				 				 
				 tempDataList = tempTask->dependInList; 
				 while(tempDataList)
				 {
					 tempTask2 = tempFunction->tasks;
					 while(tempTask2)
					 {  
						 if(tempTask2->id < tempTask->id)							// Check all my previous tasks
						 {
							 // Check if it is already a producer -- dont put in twice 
							 tempProducers2 = tempTask->producers;
							 while(tempProducers2)
							 {
								 if(tempProducers2->id == tempTask2->id)
									break;
								tempProducers2 = tempProducers2->next;
							 }
							 if(tempProducers2)
							 {
								 tempTask2 = tempTask2->next;
								 continue;
							 }
								
							 
							 /** Check the Depend [ OUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempTask2->id;
										(*tempProducers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempProducers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempTask->producers;
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempTask2->id;
										tempProducers2->next->kernels = NULL;
										
										tempKernel = &(tempProducers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempTask2->id;
										(*tempProducers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempProducers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempTask->producers; 
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempTask2->id;
										tempProducers2->next->kernels = NULL;
										
										tempKernel = &(tempProducers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						tempTask2 = tempTask2->next;
					 }
					 tempDataList = tempDataList->next;
				 }
								
				
				/* Check Depend [ Out ] list */
				 
				 
				 tempDataList = tempTask->dependOutList; 
				 while(tempDataList)
				 {
					 tempTask2 = tempFunction->tasks;
					 while(tempTask2)
					 { 
						 if(tempTask2->id > tempTask->id)							// Check all my proceeding tasks
						 {
							 // Check if it is already a consumer -- Dont put in the list twice
							 tempConsumers2 = tempTask->consumers;
							 while(tempConsumers2)
							 {
								 if(tempConsumers2->id == tempTask2->id)
									break;
								tempConsumers2 = tempConsumers2->next;
							 }
							 if(tempConsumers2)
							 {
								 tempTask2 = tempTask2->next;
								 continue;
							 }
							 
							 /** Check the Depend [ IN ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempTask2->id;
										(*tempConsumers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempConsumers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempTask->consumers;
										while(tempConsumers2 && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempTask2->id;
										tempConsumers2->next->kernels = NULL;
										
										tempKernel = &(tempConsumers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempTask2->id;
										(*tempConsumers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempConsumers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempTask->consumers;
										while(tempConsumers2 && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempTask2->id;
										tempConsumers2->next->kernels = NULL;
										
										tempKernel = &(tempConsumers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						tempTask2 = tempTask2->next;
					 }
					 tempDataList = tempDataList->next;
				 }
				 
				
				/* Check Depend [ INOUT ] list */
				 
				 
				 tempDataList = tempTask->dependInOutList; 
				 while(tempDataList)
				 {
					 tempTask2 = tempFunction->tasks;
					 while(tempTask2)
					 {
						 if(tempTask2->id < tempTask->id)							// Check all my previous tasks
						 {
							 // Check if it is already a producer -- Don't put it in the list twice
							 tempProducers2 = tempTask->producers;
							 while(tempProducers2)
							 {
								 if(tempProducers2->id == tempTask2->id)
									break;
								tempProducers2 = tempProducers2->next;
							 }
							 if(tempProducers2)
							 {
								 tempTask2 = tempTask2->next;
								 continue;
							 }
							 
							 /** Check the Depend [ OUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempTask2->id;
										(*tempProducers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempProducers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempTask->producers;
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempTask2->id;
										tempProducers2->next->kernels = NULL;
										
										tempKernel = &(tempProducers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempTask2->id;
										(*tempProducers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempProducers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempTask->producers;
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempTask2->id;
										tempProducers2->next->kernels = NULL;
										
										tempKernel = &(tempProducers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						
						
						if(tempTask2->id > tempTask->id)							// Check all my proceeding tasks
						 {
							 // Check if it is already in the list -- Dont put it in twice
							 tempConsumers2 = tempTask->consumers;
							 while(tempConsumers2)
							 {
								 if(tempConsumers2->id == tempTask2->id)
									break;
								tempConsumers2 = tempConsumers2->next;
							 }
							 if(tempConsumers2)
							 {
								 tempTask2 = tempTask2->next;
								 continue;
							 }
							 
							 /** Check the Depend [ IN ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempTask2->id;
										(*tempConsumers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempConsumers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempTask->consumers;
										while(tempConsumers && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempTask2->id;
										tempConsumers2->next->kernels = NULL;
										
										tempKernel = &(tempConsumers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempTask2->id;
										(*tempConsumers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempConsumers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempTask->consumers;
										while(tempConsumers2 && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempTask2->id;
										tempConsumers2->next->kernels = NULL;
										
										tempKernel = &(tempConsumers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						tempTask2 = tempTask2->next;
					 }
					 tempDataList = tempDataList->next;
				 }				
				 
				 
				 /*** COPY UNTIL HERE ***/
				
				
				tempTask = tempTask->next;
			}
			
			
			
			/** Take care of section tasks **/
			
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask)
				{
					
					
					/*** COPY FROM HERE ***/
					
				tempProducers = &(tempTask->producers);
				tempConsumers = &(tempTask->consumers);
				
				/* Check Depend [ IN ] list */
				 
				 tempDataList = tempTask->dependInList; 
				 while(tempDataList)
				 {
					 tempTask2 = tempFunction->tasks;
					 while(tempTask2)
					 {  
						 if(tempTask2->id < tempTask->id)							// Check all my previous tasks
						 {
							 // Check if it is already a producer -- dont put in twice 
							 tempProducers2 = tempTask->producers;
							 while(tempProducers2)
							 {
								 if(tempProducers2->id == tempTask2->id)
									break;
								tempProducers2 = tempProducers2->next;
							 }
							 if(tempProducers2)
							 {
								 tempTask2 = tempTask2->next;
								 continue;
							 }
							 
							 /** Check the Depend [ OUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempTask2->id;
										(*tempProducers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempProducers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempTask->producers;
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempTask2->id;
										tempProducers2->next->kernels = NULL;
										
										tempKernel = &(tempProducers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempTask2->id;
										(*tempProducers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempProducers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempTask->producers; 
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempTask2->id;
										tempProducers2->next->kernels = NULL;
										
										tempKernel = &(tempProducers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						tempTask2 = tempTask2->next;
					 }
					 tempDataList = tempDataList->next;
				 }
								
				
				/* Check Depend [ Out ] list */
				 
				 
				 tempDataList = tempTask->dependOutList; 
				 while(tempDataList)
				 {
					 tempTask2 = tempFunction->tasks;
					 while(tempTask2)
					 { 
						 if(tempTask2->id > tempTask->id)							// Check all my proceeding tasks
						 {
							 // Check if it is already a consumer -- Dont put in the list twice
							 tempConsumers2 = tempTask->consumers;
							 while(tempConsumers2)
							 {
								 if(tempConsumers2->id == tempTask2->id)
									break;
								tempConsumers2 = tempConsumers2->next;
							 }
							 if(tempConsumers2)
							 {
								 tempTask2 = tempTask2->next;
								 continue;
							 }
							 
							 /** Check the Depend [ IN ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempTask2->id;
										(*tempConsumers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempConsumers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempTask->consumers;
										while(tempConsumers2 && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempTask2->id;
										tempConsumers2->next->kernels = NULL;
										
										tempKernel = &(tempConsumers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempTask2->id;
										(*tempConsumers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempConsumers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempTask->consumers;
										while(tempConsumers2 && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempTask2->id;
										tempConsumers2->next->kernels = NULL;
										
										tempKernel = &(tempConsumers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						tempTask2 = tempTask2->next;
					 }
					 tempDataList = tempDataList->next;
				 }
				 
				
				/* Check Depend [ INOUT ] list */
				 
				 
				 tempDataList = tempTask->dependInOutList; 
				 while(tempDataList)
				 {
					 tempTask2 = tempFunction->tasks;
					 while(tempTask2)
					 {
						 if(tempTask2->id < tempTask->id)							// Check all my previous tasks
						 {
							 // Check if it is already a producer -- Don't put it in the list twice
							 tempProducers2 = tempTask->producers;
							 while(tempProducers2)
							 {
								 if(tempProducers2->id == tempTask2->id)
									break;
								tempProducers2 = tempProducers2->next;
							 }
							 if(tempProducers2)
							 {
								 tempTask2 = tempTask2->next;
								 continue;
							 }
							 
							 /** Check the Depend [ OUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempTask2->id;
										(*tempProducers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempProducers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempTask->producers;
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempTask2->id;
										tempProducers2->next->kernels = NULL;
										
										tempKernel = &(tempProducers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempProducers) == NULL)
									{
										(*tempProducers) = (producer*)malloc(sizeof(producer));
										(*tempProducers)->id = tempTask2->id;
										(*tempProducers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempProducers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempProducers)->next = NULL;
									}
									else
									{
										tempProducers2 = tempTask->producers;
										while(tempProducers2 && tempProducers2->next)
											tempProducers2 = tempProducers2->next;
										
										tempProducers2->next = (producer*)malloc(sizeof(producer));
										tempProducers2->next->id = tempTask2->id;
										tempProducers2->next->kernels = NULL;
										
										tempKernel = &(tempProducers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempProducers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						
						
						if(tempTask2->id > tempTask->id)							// Check all my proceeding tasks
						 {
							 // Check if it is already in the list -- Dont put it in twice
							 tempConsumers2 = tempTask->consumers;
							 while(tempConsumers2)
							 {
								 if(tempConsumers2->id == tempTask2->id)
									break;
								tempConsumers2 = tempConsumers2->next;
							 }
							 if(tempConsumers2)
							 {
								 tempTask2 = tempTask2->next;
								 continue;
							 }
							 
							 /** Check the Depend [ IN ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {									 
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempTask2->id;
										(*tempConsumers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempConsumers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempTask->consumers;
										while(tempConsumers2 && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempTask2->id;
										tempConsumers2->next->kernels = NULL;
										
										tempKernel = &(tempConsumers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
							 
							 
							  /** Check the Depend [ INOUT ] list of my previous tasks **/
							 
							 tempDataList2 = tempTask2->dependInOutList;			
							 while(tempDataList2)
							 {
								 if(!strcmp(tempDataList->variableName, tempDataList2->variableName))
								 {
									if((*tempConsumers) == NULL)
									{
										(*tempConsumers) = (producer*)malloc(sizeof(producer));
										(*tempConsumers)->id = tempTask2->id;
										(*tempConsumers)->kernels = NULL;
										
										tempKernel2 = tempTask2->kernels;
										tempKernel = &((*tempConsumers)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempConsumers)->next = NULL;
									}
									else
									{
										tempConsumers2 = tempTask->consumers;
										while(tempConsumers2 && tempConsumers2->next)
											tempConsumers2 = tempConsumers2->next;
										
										tempConsumers2->next = (producer*)malloc(sizeof(producer));
										tempConsumers2->next->id = tempTask2->id;
										tempConsumers2->next->kernels = NULL;
										
										tempKernel = &(tempConsumers2->next->kernels);
										tempKernel2 = tempTask2->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempConsumers2->next->next = NULL;
									}
								 }
								 tempDataList2 = tempDataList2->next;
							 }
						}
						tempTask2 = tempTask2->next;
					 }
					 tempDataList = tempDataList->next;
				 }				
				 
				 
				 /*** COPY UNTIL HERE ***/
					
					
					tempTask = tempTask->next;
				}
				tempSection = tempSection->next;
			}
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;
	}
}



/*********************** Remove Unnecessary Consumers from the SG **************************/

void offlineScheduling_TransitiveReductionOfConsumers(SG** Graph){
		
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	parallel_function 	*tempFunction2;
	section 			*tempSection;
	task 				*tempTask;
	producer			*tempConsumers;
	
	producer			**tempConsumers2;
	producer			*tempConsumers3;
	kernel				**tempKernel;
	kernel				*tempKernel2;
	
	task				*tempTask2;
	
	long				taskCounter = 0;
	long				functionCounter = 0;
	Stack				tred_Stack;
	Stack				temp_Stack;
	
	int					i = 0, j = 0, k = 0;
	int					ii = 0, jj = 0;


	while(tempGraph)
	{
        /** Take care of parallel functions **/
			
        // Counter how many tasks there are in a function -- to allocate the tred_consumer_array[TASKS][TASKS]
        tempFunction = tempGraph->parallel_functions;;
        while(tempFunction)
        {
            functionCounter++;
            tempFunction = tempFunction->next;
        }
        
        // Allocate memory for tred_array & visited
        tred_array = (bool**)malloc(sizeof(bool*)*functionCounter);
        for(i = 0; i < functionCounter; i++)
            tred_array[i] = (bool*)malloc(sizeof(bool)*functionCounter);
        
        // Initialize tred_array
        for(i = 0; i < functionCounter; i++)
            for(j = 0; j< functionCounter; j++)
                tred_array[i][j] = FALSE;
        
        // Add consumers in the tred_array
        i = 0;
        tempFunction = tempGraph->parallel_functions;
        while(tempFunction)
        {
            tempConsumers = tempFunction->consumers;
            while(tempConsumers)
            {
                j = 0;
                tempFunction2 = tempGraph->parallel_functions;
                while(tempFunction2)
                {
                    if(tempFunction2->id == tempConsumers->id)
                    {
                        tred_array[i][j] = TRUE;
                        break;
                    }
                    j++;
                    tempFunction2 = tempFunction2->next;
                }
                tempConsumers = tempConsumers->next;
            }
            i++;
            tempFunction = tempFunction->next;
        }
        
        
        // Transitive Reduction of tred_array	-- ITERATIVE SOLUTION	
        
        Stack_Init(&tred_Stack, functionCounter*functionCounter);
        Stack_Init(&temp_Stack, functionCounter);
        
        for(i = 0; i < functionCounter; i++)
        {
            Stack_Push(&tred_Stack, i, functionCounter*functionCounter);
            
            while(!StackIsEmpty(&tred_Stack))
            {		
                k = Stack_Pop(&tred_Stack);
                
                for(j = 0; j < functionCounter; j++)
                {						
                    if(tred_array[k][j] == TRUE)
                    {
                        Stack_Push(&tred_Stack, j, functionCounter*functionCounter);
                        if(k == i)
                        {
                            Stack_Push(&temp_Stack, j, functionCounter*functionCounter);
                        }
                        else
                        {
                            for(ii = 0; ii < temp_Stack.size; ii++)
                            {
                                if(j == temp_Stack.data[ii])
                                    tred_array[i][j] = FALSE;
                            }
                        }
                    }
                }
            }
            while(!StackIsEmpty(&temp_Stack))
                Stack_Pop(&temp_Stack);
        }
        
        
        // Copy new consumers to tred_consumers in SG
        
        i = 0;
        tempFunction = tempGraph->parallel_functions;
        while(tempFunction)
        {
            tempConsumers2 = &(tempFunction->tred_consumers);
            
            tempConsumers = tempFunction->consumers;
            while(tempConsumers)
            {
                j = 0;
                tempFunction2 = tempGraph->parallel_functions;
                while(tempFunction2)
                {
                    if(tempFunction2->id == tempConsumers->id)
                    {
                        if(tred_array[i][j] == TRUE)
                        {
                            if((*tempConsumers2) == NULL)
                                {
                                    (*tempConsumers2) = (producer*)malloc(sizeof(producer));
                                    (*tempConsumers2)->id = tempConsumers->id;
                                    (*tempConsumers2)->kernels = NULL;
                                    (*tempConsumers2)->next = NULL;
                                }
                                else
                                {
                                    tempConsumers3 = tempFunction->tred_consumers;
                                    while(tempConsumers3 && tempConsumers3->next)
                                        tempConsumers3 = tempConsumers3->next;
                                    
                                    tempConsumers3->next = (producer*)malloc(sizeof(producer));
                                    tempConsumers3->next->id = tempConsumers->id;
                                    tempConsumers3->next->kernels = NULL;
                                    tempConsumers3->next->next = NULL;
                                }
                            break;
                        }
                    }
                    j++;
                    tempFunction2 = tempFunction2->next;
                }
                tempConsumers = tempConsumers->next;
            }
            i++;
            tempFunction = tempFunction->next;
        }
            
            
                    
        free(tred_array);
        taskCounter = 0;
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{	
            
			/** Take care of functions tasks **/
			
			// Counter how many tasks there are in a function -- to allocate the tred_consumer_array[TASKS][TASKS]
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				taskCounter++;
				tempTask = tempTask->next;
			}
			
			// Allocate memory for tred_array & visited
			tred_array = (bool**)malloc(sizeof(bool*)*taskCounter);
			for(i = 0; i < taskCounter; i++)
				tred_array[i] = (bool*)malloc(sizeof(bool)*taskCounter);
			
			// Initialize tred_array
			for(i = 0; i < taskCounter; i++)
				for(j = 0; j< taskCounter; j++)
					tred_array[i][j] = FALSE;
			
			// Add consumers in the tred_array
			i = 0;
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				tempConsumers = tempTask->consumers;
				while(tempConsumers)
				{
					j = 0;
					tempTask2 = tempFunction->tasks;
					while(tempTask2)
					{
						if(tempTask2->id == tempConsumers->id)
						{
							tred_array[i][j] = TRUE;
							break;
						}
						j++;
						tempTask2 = tempTask2->next;
					}
					tempConsumers = tempConsumers->next;
				}
				i++;
				tempTask = tempTask->next;
			}
			
			
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- START FROM HERE **/
			/*
			tempTask = tempFunction->tasks;
			if(tempTask)
			{
				fprintf(stderr, "\t");
				for(i = 0; i < taskCounter; i++)
					fprintf(stderr, "t_%d\t", i+1);
				fprintf(stderr, "\n");
	
				for(i = 0; i < taskCounter; i++)
				{
					fprintf(stderr, "t_%d:\t", i+1);
					for(j = 0; j < taskCounter; j++)
						fprintf(stderr, "%d\t", tred_array[i][j]);
					fprintf(stderr, "\n");
				}
				fprintf(stderr, "----------------------------------------------------------\n");
			}
			*/
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- UNTIL HERE **/
			

			
			// Transitive Reduction of tred_array	-- ITERATIVE SOLUTION	
			
			Stack_Init(&tred_Stack, taskCounter*taskCounter);
			Stack_Init(&temp_Stack, taskCounter);
			
			for(i = 0; i < taskCounter; i++)
			{
				Stack_Push(&tred_Stack, i, taskCounter*taskCounter);
				
				while(!StackIsEmpty(&tred_Stack))
				{		
					k = Stack_Pop(&tred_Stack);
					
					for(j = 0; j < taskCounter; j++)
					{						
						if(tred_array[k][j] == TRUE)
						{
							Stack_Push(&tred_Stack, j, taskCounter*taskCounter);
							if(k == i)
							{
								Stack_Push(&temp_Stack, j, taskCounter*taskCounter);
							}
							else
							{
								for(ii = 0; ii < temp_Stack.size; ii++)
								{
									if(j == temp_Stack.data[ii])
										tred_array[i][j] = FALSE;
								}
							}
						}
					}
				}
				while(!StackIsEmpty(&temp_Stack))
					Stack_Pop(&temp_Stack);
			}
			
			
			// Transitive Reduction of tred_array	-- RECURSION SOLUTION	
			
			/*
			for(i = 0; i < taskCounter; i++)
			{
				for(j = 0; j < taskCounter; j++)
				{
					if(tred_array[i][j] == TRUE)
					{							
						flag = FALSE;
						counter = 0;
						recursive_DFS(i, j, taskCounter);
						if(flag == TRUE)
							tred_array[i][j] = FALSE;
					}
				}
			}
			*/
			
			
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- START FROM HERE **/
			/*
			tempTask = tempFunction->tasks;
			if(tempTask)
			{
				fprintf(stderr, "\t");
				for(i = 0; i < taskCounter; i++)
					fprintf(stderr, "t_%d\t", i+1);
				fprintf(stderr, "\n");
	
				for(i = 0; i < taskCounter; i++)
				{
					fprintf(stderr, "t_%d:\t", i+1);
					for(j = 0; j < taskCounter; j++)
						fprintf(stderr, "%d\t", tred_array[i][j]);
					fprintf(stderr, "\n");
				}
				fprintf(stderr, "----------------------------------------------------------\n");
			}
			*/
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- UNTIL HERE **/
			
			
			
			// Copy new consumers to tred_consumers in SG
			
			i = 0;
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				tempConsumers2 = &(tempTask->tred_consumers);
				
				tempConsumers = tempTask->consumers;
				while(tempConsumers)
				{
					j = 0;
					tempTask2 = tempFunction->tasks;
					while(tempTask2)
					{
						if(tempTask2->id == tempConsumers->id)
						{
							if(tred_array[i][j] == TRUE)
							{
								if((*tempConsumers2) == NULL)
									{
										(*tempConsumers2) = (producer*)malloc(sizeof(producer));
										(*tempConsumers2)->id = tempConsumers->id;
										(*tempConsumers2)->kernels = NULL;
										
										tempKernel2 = tempConsumers->kernels;
										tempKernel = &((*tempConsumers2)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempConsumers2)->next = NULL;
									}
									else
									{
										tempConsumers3 = tempTask->tred_consumers;
										while(tempConsumers3 && tempConsumers3->next)
											tempConsumers3 = tempConsumers3->next;
										
										tempConsumers3->next = (producer*)malloc(sizeof(producer));
										tempConsumers3->next->id = tempConsumers->id;
										tempConsumers3->next->kernels = NULL;
										
										tempKernel = &(tempConsumers3->next->kernels);
										tempKernel2 = tempConsumers->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempConsumers3->next->next = NULL;
									}
								break;
							}
						}
						j++;
						tempTask2 = tempTask2->next;
					}
					tempConsumers = tempConsumers->next;
				}
				i++;
				tempTask = tempTask->next;
			}
			
			

			
			/** Take care of section tasks **/
			
			free(tred_array);
			taskCounter = 0;
			// Counter how many section tasks there are in a function -- to allocate the tred_consumer_array[TASKS][TASKS]
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask)
				{
					taskCounter++;
					tempTask = tempTask->next;
				}
				tempSection = tempSection->next;
			}
			
			// Allocate memory for tred_array & visited
			tred_array = (bool**)malloc(sizeof(bool*)*taskCounter);
			for(i = 0; i < taskCounter; i++)
				tred_array[i] = (bool*)malloc(sizeof(bool)*taskCounter);
			
			// Initialize tred_array
			for(i = 0; i < taskCounter; i++)
				for(j = 0; j< taskCounter; j++)
					tred_array[i][j] = FALSE;
					
			// Add consumers in the tred_array
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				i = 0;
				tempTask = tempFunction->tasks;
				while(tempTask)
				{
					tempConsumers = tempTask->consumers;
					while(tempConsumers)
					{
						j = 0;
						tempTask2 = tempFunction->tasks;
						while(tempTask2)
						{
							if(tempTask2->id == tempConsumers->id)
							{
								tred_array[i][j] = TRUE;
								break;
							}
							j++;
							tempTask2 = tempTask2->next;
						}
						tempConsumers = tempConsumers->next;
					}
					i++;
					tempTask = tempTask->next;
				}	
				tempSection = tempSection->next;
			}
			
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- START FROM HERE **/
			/*
			tempSection = tempFunction->sections;
			if(tempSection)
			{
				fprintf(stderr, "\t");
				for(i = 0; i < taskCounter; i++)
					fprintf(stderr, "t_%d\t", i+1);
				fprintf(stderr, "\n");
	
				for(i = 0; i < taskCounter; i++)
				{
					fprintf(stderr, "t_%d:\t", i+1);
					for(j = 0; j < taskCounter; j++)
						fprintf(stderr, "%d\t", tred_array[i][j]);
					fprintf(stderr, "\n");
				}
				fprintf(stderr, "----------------------------------------------------------\n");
			}
			*/
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- UNTIL HERE **/
			
			
			
			// Transitive Reduction of tred_array	-- ITERATIVE SOLUTION	
			
			Stack_Init(&tred_Stack, taskCounter*taskCounter);
			Stack_Init(&temp_Stack, taskCounter);
			
			for(i = 0; i < taskCounter; i++)
			{
				Stack_Push(&tred_Stack, i, taskCounter*taskCounter);
				
				while(!StackIsEmpty(&tred_Stack))
				{		
					k = Stack_Pop(&tred_Stack);
					
					for(j = 0; j < taskCounter; j++)
					{						
						if(tred_array[k][j] == TRUE)
						{
							Stack_Push(&tred_Stack, j, taskCounter*taskCounter);
							if(k == i)
							{
								Stack_Push(&temp_Stack, j, taskCounter*taskCounter);
							}
							else
							{
								for(ii = 0; ii < temp_Stack.size; ii++)
								{
									if(j == temp_Stack.data[ii])
										tred_array[i][j] = FALSE;
								}
							}
						}
					}
				}
				while(!StackIsEmpty(&temp_Stack))
					Stack_Pop(&temp_Stack);
			}
			
			
			
			
			// Transitive Reduction of tred_array	-- RECURSION SOLUTION	
			/*
			for(i = 0; i < taskCounter; i++)
			{
				for(j = 0; j < taskCounter; j++)
				{
					if(tred_array[i][j] == TRUE)
					{							
						flag = FALSE;
						counter = 0;
						recursive_DFS(i, j, taskCounter);
						if(flag == TRUE)
							tred_array[i][j] = FALSE;
					}
				}
			}
			*/
			
			
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- START FROM HERE **/
			/*
			tempTask = tempFunction->tasks;
			if(tempTask)
			{
				fprintf(stderr, "\t");
				for(i = 0; i < taskCounter; i++)
					fprintf(stderr, "t_%d\t", i+1);
				fprintf(stderr, "\n");
	
				for(i = 0; i < taskCounter; i++)
				{
					fprintf(stderr, "t_%d:\t", i+1);
					for(j = 0; j < taskCounter; j++)
						fprintf(stderr, "%d\t", tred_array[i][j]);
					fprintf(stderr, "\n");
				}
				fprintf(stderr, "----------------------------------------------------------\n");
			}
			*/
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- UNTIL HERE **/
			
			
			
			
			// Copy new consumers to tred_consumers in SG
			
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				i = 0;
				tempTask = tempSection->tasks;
				while(tempTask)
				{
					tempConsumers2 = &(tempTask->tred_consumers);
					
					tempConsumers = tempTask->consumers;
					while(tempConsumers)
					{
						j = 0;
						tempTask2 = tempFunction->tasks;
						while(tempTask2)
						{
							if(tempTask2->id == tempConsumers->id)
							{
								if(tred_array[i][j] == TRUE)
								{
									if((*tempConsumers2) == NULL)
										{
											(*tempConsumers2) = (producer*)malloc(sizeof(producer));
											(*tempConsumers2)->id = tempConsumers->id;
											(*tempConsumers2)->kernels = NULL;
											
											tempKernel2 = tempConsumers->kernels;
											tempKernel = &((*tempConsumers2)->kernels);
											
											while(tempKernel2)
											{
												if((*tempKernel) == NULL)
												{
													(*tempKernel) = (kernel*)malloc(sizeof(kernel));
													(*tempKernel)->id = tempKernel2->id;
													(*tempKernel)->vId = tempKernel2->vId;
													(*tempKernel)->next = NULL;
												}
												else
												{
													while((*tempKernel) && (*tempKernel)->next)
														tempKernel = &(*tempKernel)->next;
													
													(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
													(*tempKernel)->next->id = tempKernel2->id;
													(*tempKernel)->next->vId = tempKernel2->vId;
													(*tempKernel)->next->next = NULL;
												}
												tempKernel2 = tempKernel2->next;
											}
											(*tempConsumers2)->next = NULL;
										}
										else
										{
											tempConsumers3 = tempTask->tred_consumers;
											while(tempConsumers3 && tempConsumers3->next)
												tempConsumers3 = tempConsumers3->next;
											
											tempConsumers3->next = (producer*)malloc(sizeof(producer));
											tempConsumers3->next->id = tempConsumers->id;
											tempConsumers3->next->kernels = NULL;
											
											tempKernel = &(tempConsumers3->next->kernels);
											tempKernel2 = tempConsumers->kernels;
											
											while(tempKernel2)
											{
												if((*tempKernel) == NULL)
												{
													(*tempKernel) = (kernel*)malloc(sizeof(kernel));
													(*tempKernel)->id = tempKernel2->id;
													(*tempKernel)->vId = tempKernel2->vId;
													(*tempKernel)->next = NULL;
												}
												else
												{
													while((*tempKernel) && (*tempKernel)->next)
														tempKernel = &(*tempKernel)->next;
													
													(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
													(*tempKernel)->next->id = tempKernel2->id;
													(*tempKernel)->next->vId = tempKernel2->vId;
													(*tempKernel)->next->next = NULL;
												}
												
												tempKernel2 = tempKernel2->next;
											}
											tempConsumers3->next->next = NULL;
										}
									break;
								}
							}
							j++;
							tempTask2 = tempTask2->next;
						}
						tempConsumers = tempConsumers->next;
					}
					i++;
					tempTask = tempTask->next;
				}
					tempSection = tempSection->next;
			}		
			tempFunction = tempFunction->next;
			free(tred_array);
			taskCounter = 0;
		}
		tempGraph = tempGraph->next;
	}
}



/*********************** Remove Unnecessary Producers from the SG **************************/

void offlineScheduling_TransitiveReductionOfProducers(SG** Graph){
		
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
	producer			*tempProducers;
	
	producer			**tempProducers2;
	producer			*tempProducers3;
	kernel				**tempKernel;
	kernel				*tempKernel2;
	
	task				*tempTask2;
	
	long				taskCounter = 0;
	Stack				tred_Stack;
	Stack				temp_Stack;
	
	int					i = 0, j = 0, k = 0;
	int					ii = 0, jj = 0;


	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{			
			/** Take care of functions tasks **/
			
			// Counter how many tasks there are in a function -- to allocate the tred_consumer_array[TASKS][TASKS]
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				taskCounter++;
				tempTask = tempTask->next;
			}
			
			// Allocate memory for tred_array & visited
			tred_array = (bool**)malloc(sizeof(bool*)*taskCounter);
			for(i = 0; i < taskCounter; i++)
				tred_array[i] = (bool*)malloc(sizeof(bool)*taskCounter);
			
			// Initialize tred_array
			for(i = 0; i < taskCounter; i++)
				for(j = 0; j< taskCounter; j++)
					tred_array[i][j] = FALSE;
			
			// Add consumers in the tred_array
			i = 0;
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				tempProducers = tempTask->producers;
				while(tempProducers)
				{
					j = 0;
					tempTask2 = tempFunction->tasks;
					while(tempTask2)
					{
						if(tempTask2->id == tempProducers->id)
						{
							tred_array[i][j] = TRUE;
							break;
						}
						j++;
						tempTask2 = tempTask2->next;
					}
					tempProducers = tempProducers->next;
				}
				i++;
				tempTask = tempTask->next;
			}
			
			
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- START FROM HERE **/
			/*
			tempTask = tempFunction->tasks;
			if(tempTask)
			{
				fprintf(stderr, "\t");
				for(i = 0; i < taskCounter; i++)
					fprintf(stderr, "t_%d\t", i+1);
				fprintf(stderr, "\n");
	
				for(i = 0; i < taskCounter; i++)
				{
					fprintf(stderr, "t_%d:\t", i+1);
					for(j = 0; j < taskCounter; j++)
						fprintf(stderr, "%d\t", tred_array[i][j]);
					fprintf(stderr, "\n");
				}
				fprintf(stderr, "----------------------------------------------------------\n");
			}
			*/
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- UNTIL HERE **/
			

			
			// Transitive Reduction of tred_array	-- ITERATIVE SOLUTION	
			
			Stack_Init(&tred_Stack, taskCounter*taskCounter);
			Stack_Init(&temp_Stack, taskCounter);
			
			for(i = 0; i < taskCounter; i++)
			{
				Stack_Push(&tred_Stack, i, taskCounter*taskCounter);
				
				while(!StackIsEmpty(&tred_Stack))
				{		
					k = Stack_Pop(&tred_Stack);
					
					for(j = 0; j < taskCounter; j++)
					{						
						if(tred_array[k][j] == TRUE)
						{
							Stack_Push(&tred_Stack, j, taskCounter*taskCounter);
							if(k == i)
							{
								Stack_Push(&temp_Stack, j, taskCounter*taskCounter);
							}
							else
							{
								for(ii = 0; ii < temp_Stack.size; ii++)
								{
									if(j == temp_Stack.data[ii])
										tred_array[i][j] = FALSE;
								}
							}
						}
					}
				}
				while(!StackIsEmpty(&temp_Stack))
					Stack_Pop(&temp_Stack);
			}
			
			
			// Transitive Reduction of tred_array	-- RECURSION SOLUTION	
			
			/*
			for(i = 0; i < taskCounter; i++)
			{
				for(j = 0; j < taskCounter; j++)
				{
					if(tred_array[i][j] == TRUE)
					{							
						flag = FALSE;
						counter = 0;
						recursive_DFS(i, j, taskCounter);
						if(flag == TRUE)
							tred_array[i][j] = FALSE;
					}
				}
			}
			*/
			
			
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- START FROM HERE **/
			/*
			tempTask = tempFunction->tasks;
			if(tempTask)
			{
				fprintf(stderr, "\t");
				for(i = 0; i < taskCounter; i++)
					fprintf(stderr, "t_%d\t", i+1);
				fprintf(stderr, "\n");
	
				for(i = 0; i < taskCounter; i++)
				{
					fprintf(stderr, "t_%d:\t", i+1);
					for(j = 0; j < taskCounter; j++)
						fprintf(stderr, "%d\t", tred_array[i][j]);
					fprintf(stderr, "\n");
				}
				fprintf(stderr, "----------------------------------------------------------\n");
			}
			*/
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- UNTIL HERE **/
			
			
			
			// Copy new producers to tred_producers in SG
			
			i = 0;
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				tempProducers2 = &(tempTask->tred_producers);
				
				tempProducers = tempTask->producers;
				while(tempProducers)
				{
					j = 0;
					tempTask2 = tempFunction->tasks;
					while(tempTask2)
					{
						if(tempTask2->id == tempProducers->id)
						{
							if(tred_array[i][j] == TRUE)
							{
								if((*tempProducers2) == NULL)
									{
										(*tempProducers2) = (producer*)malloc(sizeof(producer));
										(*tempProducers2)->id = tempProducers->id;
										(*tempProducers2)->kernels = NULL;
										
										tempKernel2 = tempProducers->kernels;
										tempKernel = &((*tempProducers2)->kernels);
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											tempKernel2 = tempKernel2->next;
										}
										(*tempProducers2)->next = NULL;
									}
									else
									{
										tempProducers3 = tempTask->tred_producers;
										while(tempProducers3 && tempProducers3->next)
											tempProducers3 = tempProducers3->next;
										
										tempProducers3->next = (producer*)malloc(sizeof(producer));
										tempProducers3->next->id = tempProducers->id;
										tempProducers3->next->kernels = NULL;
										
										tempKernel = &(tempProducers3->next->kernels);
										tempKernel2 = tempProducers->kernels;
										
										while(tempKernel2)
										{
											if((*tempKernel) == NULL)
											{
												(*tempKernel) = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->id = tempKernel2->id;
												(*tempKernel)->vId = tempKernel2->vId;
												(*tempKernel)->next = NULL;
											}
											else
											{
												while((*tempKernel) && (*tempKernel)->next)
													tempKernel = &(*tempKernel)->next;
												
												(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
												(*tempKernel)->next->id = tempKernel2->id;
												(*tempKernel)->next->vId = tempKernel2->vId;
												(*tempKernel)->next->next = NULL;
											}
											
											tempKernel2 = tempKernel2->next;
										}
										tempProducers3->next->next = NULL;
									}
								break;
							}
						}
						j++;
						tempTask2 = tempTask2->next;
					}
					tempProducers = tempProducers->next;
				}
				i++;
				tempTask = tempTask->next;
			}
			
			

			
			/** Take care of section tasks **/
			
			free(tred_array);
			taskCounter = 0;
			// Counter how many section tasks there are in a function -- to allocate the tred_consumer_array[TASKS][TASKS]
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask)
				{
					taskCounter++;
					tempTask = tempTask->next;
				}
				tempSection = tempSection->next;
			}
			
			// Allocate memory for tred_array & visited
			tred_array = (bool**)malloc(sizeof(bool*)*taskCounter);
			for(i = 0; i < taskCounter; i++)
				tred_array[i] = (bool*)malloc(sizeof(bool)*taskCounter);
			
			// Initialize tred_array
			for(i = 0; i < taskCounter; i++)
				for(j = 0; j< taskCounter; j++)
					tred_array[i][j] = FALSE;
					
			// Add producers in the tred_array
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				i = 0;
				tempTask = tempFunction->tasks;
				while(tempTask)
				{
					tempProducers = tempTask->producers;
					while(tempProducers)
					{
						j = 0;
						tempTask2 = tempFunction->tasks;
						while(tempTask2)
						{
							if(tempTask2->id == tempProducers->id)
							{
								tred_array[i][j] = TRUE;
								break;
							}
							j++;
							tempTask2 = tempTask2->next;
						}
						tempProducers = tempProducers->next;
					}
					i++;
					tempTask = tempTask->next;
				}	
				tempSection = tempSection->next;
			}
			
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- START FROM HERE **/
			/*
			tempSection = tempFunction->sections;
			if(tempSection)
			{
				fprintf(stderr, "\t");
				for(i = 0; i < taskCounter; i++)
					fprintf(stderr, "t_%d\t", i+1);
				fprintf(stderr, "\n");
	
				for(i = 0; i < taskCounter; i++)
				{
					fprintf(stderr, "t_%d:\t", i+1);
					for(j = 0; j < taskCounter; j++)
						fprintf(stderr, "%d\t", tred_array[i][j]);
					fprintf(stderr, "\n");
				}
				fprintf(stderr, "----------------------------------------------------------\n");
			}
			*/
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- UNTIL HERE **/
			
			
			
			// Transitive Reduction of tred_array	-- ITERATIVE SOLUTION	
			
			Stack_Init(&tred_Stack, taskCounter*taskCounter);
			Stack_Init(&temp_Stack, taskCounter);
			
			for(i = 0; i < taskCounter; i++)
			{
				Stack_Push(&tred_Stack, i, taskCounter*taskCounter);
				
				while(!StackIsEmpty(&tred_Stack))
				{		
					k = Stack_Pop(&tred_Stack);
					
					for(j = 0; j < taskCounter; j++)
					{						
						if(tred_array[k][j] == TRUE)
						{
							Stack_Push(&tred_Stack, j, taskCounter*taskCounter);
							if(k == i)
							{
								Stack_Push(&temp_Stack, j, taskCounter*taskCounter);
							}
							else
							{
								for(ii = 0; ii < temp_Stack.size; ii++)
								{
									if(j == temp_Stack.data[ii])
										tred_array[i][j] = FALSE;
								}
							}
						}
					}
				}
				while(!StackIsEmpty(&temp_Stack))
					Stack_Pop(&temp_Stack);
			}
			
			
			
			
			// Transitive Reduction of tred_array	-- RECURSION SOLUTION	
			/*
			for(i = 0; i < taskCounter; i++)
			{
				for(j = 0; j < taskCounter; j++)
				{
					if(tred_array[i][j] == TRUE)
					{							
						flag = FALSE;
						counter = 0;
						recursive_DFS(i, j, taskCounter);
						if(flag == TRUE)
							tred_array[i][j] = FALSE;
					}
				}
			}
			*/
			
			
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- START FROM HERE **/
			/*
			tempTask = tempFunction->tasks;
			if(tempTask)
			{
				fprintf(stderr, "\t");
				for(i = 0; i < taskCounter; i++)
					fprintf(stderr, "t_%d\t", i+1);
				fprintf(stderr, "\n");
	
				for(i = 0; i < taskCounter; i++)
				{
					fprintf(stderr, "t_%d:\t", i+1);
					for(j = 0; j < taskCounter; j++)
						fprintf(stderr, "%d\t", tred_array[i][j]);
					fprintf(stderr, "\n");
				}
				fprintf(stderr, "----------------------------------------------------------\n");
			}
			*/
			/** PRINTING EVALUATION -- REMOVE AFTER FINISH -- UNTIL HERE **/
			
			
			
			
			// Copy new producers to tred_producers in SG
			
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				i = 0;
				tempTask = tempSection->tasks;
				while(tempTask)
				{
					tempProducers2 = &(tempTask->tred_producers);
					
					tempProducers = tempTask->producers;
					while(tempProducers)
					{
						j = 0;
						tempTask2 = tempFunction->tasks;
						while(tempTask2)
						{
							if(tempTask2->id == tempProducers->id)
							{
								if(tred_array[i][j] == TRUE)
								{
									if((*tempProducers2) == NULL)
										{
											(*tempProducers2) = (producer*)malloc(sizeof(producer));
											(*tempProducers2)->id = tempProducers->id;
											(*tempProducers2)->kernels = NULL;
											
											tempKernel2 = tempProducers->kernels;
											tempKernel = &((*tempProducers2)->kernels);
											
											while(tempKernel2)
											{
												if((*tempKernel) == NULL)
												{
													(*tempKernel) = (kernel*)malloc(sizeof(kernel));
													(*tempKernel)->id = tempKernel2->id;
													(*tempKernel)->vId = tempKernel2->vId;
													(*tempKernel)->next = NULL;
												}
												else
												{
													while((*tempKernel) && (*tempKernel)->next)
														tempKernel = &(*tempKernel)->next;
													
													(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
													(*tempKernel)->next->id = tempKernel2->id;
													(*tempKernel)->next->vId = tempKernel2->vId;
													(*tempKernel)->next->next = NULL;
												}
												tempKernel2 = tempKernel2->next;
											}
											(*tempProducers2)->next = NULL;
										}
										else
										{
											tempProducers3 = tempTask->tred_producers;
											while(tempProducers3 && tempProducers3->next)
												tempProducers3 = tempProducers3->next;
											
											tempProducers3->next = (producer*)malloc(sizeof(producer));
											tempProducers3->next->id = tempProducers->id;
											tempProducers3->next->kernels = NULL;
											
											tempKernel = &(tempProducers3->next->kernels);
											tempKernel2 = tempProducers->kernels;
											
											while(tempKernel2)
											{
												if((*tempKernel) == NULL)
												{
													(*tempKernel) = (kernel*)malloc(sizeof(kernel));
													(*tempKernel)->id = tempKernel2->id;
													(*tempKernel)->vId = tempKernel2->vId;
													(*tempKernel)->next = NULL;
												}
												else
												{
													while((*tempKernel) && (*tempKernel)->next)
														tempKernel = &(*tempKernel)->next;
													
													(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
													(*tempKernel)->next->id = tempKernel2->id;
													(*tempKernel)->next->vId = tempKernel2->vId;
													(*tempKernel)->next->next = NULL;
												}
												
												tempKernel2 = tempKernel2->next;
											}
											tempProducers3->next->next = NULL;
										}
									break;
								}
							}
							j++;
							tempTask2 = tempTask2->next;
						}
						tempProducers = tempProducers->next;
					}
					i++;
					tempTask = tempTask->next;
				}
					tempSection = tempSection->next;
			}		
			tempFunction = tempFunction->next;
			free(tred_array);
			taskCounter = 0;
		}
		tempGraph = tempGraph->next;
	}
}



/*********************** Assign Kernels to the Parallel Functions **************************/

void offlineScheduling_AssignKernelsToParallelFunctions(SG** Graph){
		
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	kernel				**tempKernel;
	int					tempKernelCounter = 0;


	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
			tempKernelCounter = 0;
			tempKernel = &(tempFunction->kernels);
			
			while(tempKernelCounter < tempFunction->number_of_kernels)
			{
				if((*tempKernel) == NULL)
				{
					(*tempKernel) = (kernel*)malloc(sizeof(kernel));
					(*tempKernel)->id 	  = tempKernelCounter;
					(*tempKernel)->vId 	  = tempKernelCounter;
					(*tempKernel)->next   = NULL;
				}
				else
				{
					while((*tempKernel) && (*tempKernel)->next)
						tempKernel = &(*tempKernel)->next;
					
					(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
					(*tempKernel)->next->id 	= tempKernelCounter;
					(*tempKernel)->next->vId 	= tempKernelCounter;
					(*tempKernel)->next->next 	= NULL;
				}
				tempKernelCounter++;
			}			
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;		
	}
}




/*********************** Assign Kernels to the Parallel Functions From File **************************/

void offlineScheduling_AssignKernelsToParallelFunctionsFromFile(SG** Graph){
		
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	kernel				**tempKernel;
	int					tempKernelCounter = 0;


	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
			tempKernelCounter = 0;
            tempFunction->number_of_kernels = maxCores * hThreads;      // All parallel Function will get the max number of hardware threads supported by the machine
			tempKernel = &(tempFunction->kernels);
			
			while(tempKernelCounter < tempFunction->number_of_kernels)
			{
				if((*tempKernel) == NULL)
				{
					(*tempKernel) = (kernel*)malloc(sizeof(kernel));
					(*tempKernel)->id 	  = tempKernelCounter;
					(*tempKernel)->vId 	  = tempKernelCounter;
					(*tempKernel)->next   = NULL;
				}
				else
				{
					while((*tempKernel) && (*tempKernel)->next)
						tempKernel = &(*tempKernel)->next;
					
					(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
					(*tempKernel)->next->id 	= tempKernelCounter;
					(*tempKernel)->next->vId 	= tempKernelCounter;
					(*tempKernel)->next->next 	= NULL;
				}
				tempKernelCounter++;
			}			
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;		
	}
}




/*********************** Assign Kernels to each task: ROUND ROBIN **************************/

void offlineScheduling_AssignKernelsToTasks_RoundRobin(SG** Graph){
		
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
	kernel				**tempKernel;
	kernel				*tempFunctionKernel;
	int					kernelAvailability[kernels];
	int 				i = 0;
	bool				reassignFlag = FALSE;


	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
			reassignFlag = FALSE;
			tempFunctionKernel = tempFunction->kernels;
			for(i = 0; i < kernels; i++)
				kernelAvailability[i] = TRUE;
			
			// Check tasks
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				i = tempTask->number_of_kernels;
				tempKernel = &(tempTask->kernels);
				while(tempFunctionKernel && i > 0)
				{
					if(kernelAvailability[tempFunctionKernel->id] == TRUE || reassignFlag == TRUE) 
					{
                        // If task will be executed by one kernel only
						if((*tempKernel)->id == -1)
						{
							if(tempTask->taskType == TASK_MASTER)
								(*tempKernel)->id = 0;
							else
								(*tempKernel)->id = tempFunctionKernel->id;
								
							(*tempKernel)->next = NULL;
						}
						else
						{
							while((*tempKernel) && (*tempKernel)->next)
								tempKernel = &(*tempKernel)->next;
							
							(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
							(*tempKernel)->next->id = tempFunctionKernel->id;
							(*tempKernel)->next->next = NULL;
						}
						i--;
						if(tempTask->taskType == TASK_MASTER)
							kernelAvailability[0] = FALSE;
						else
							kernelAvailability[tempFunctionKernel->id] = FALSE;
					}
					if(tempTask->taskType != TASK_MASTER)
					{
						tempFunctionKernel = tempFunctionKernel->next;
						if(!tempFunctionKernel)								// If not all tasks are assigned a kernel then restart
						{
							tempFunctionKernel = tempFunction->kernels;
							reassignFlag = TRUE;
						}
					}
				}
				tempTask = tempTask->next;
			}
			
			
			// Check section tasks
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask)
				{
					i = tempTask->number_of_kernels;
					tempKernel = &(tempTask->kernels);
					while(tempFunctionKernel && i > 0)
					{
						if(kernelAvailability[tempFunctionKernel->id] == TRUE || reassignFlag == TRUE) 
						{
							if((*tempKernel)->id == -1)
							{
								if(tempTask->taskType == TASK_MASTER)
									(*tempKernel)->id = 0;
								else
									(*tempKernel)->id = tempFunctionKernel->id;
									
								(*tempKernel)->next = NULL;
							}
							else
							{
								while((*tempKernel) && (*tempKernel)->next)
									tempKernel = &(*tempKernel)->next;
								
								(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
								(*tempKernel)->next->id = tempFunctionKernel->id;
								(*tempKernel)->next->next = NULL;
							}
							i--;
							if(tempTask->taskType == TASK_MASTER)
								kernelAvailability[0] = FALSE;
							else
								kernelAvailability[tempFunctionKernel->id] = FALSE;
						}
						if(tempTask->taskType != TASK_MASTER)
						{
							tempFunctionKernel = tempFunctionKernel->next;
							if(!tempFunctionKernel)								// If not all tasks are assigned a kernel then restart
							{
								tempFunctionKernel = tempFunction->kernels;
								reassignFlag = TRUE;
							}
						}
					}
					tempTask = tempTask->next;
				}
				tempSection = tempSection->next;	
			}						
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;		
	}
}




/*********************** Assign Kernels to each task: RANDOM **************************/

void offlineScheduling_AssignKernelsToTasks_Random(SG** Graph){
		
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
	kernel				**tempKernel;
	kernel				*tempFunctionKernel;
	kernel				*tempTempFunctionKernel;
	int					kernelAvailability[kernels];
	int 				i = 0;
    int                 randomKernel = -1;
    time_t              t;
	bool				reassignFlag = FALSE;


	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
			reassignFlag = FALSE;
			tempFunctionKernel = tempFunction->kernels;
			for(i = 0; i < kernels; i++)
				kernelAvailability[i] = TRUE;
			
			// Check tasks
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
                srand((unsigned) time(&t));                
				i = tempTask->number_of_kernels;
				tempKernel = &(tempTask->kernels);
				while(i > 0)
				{
                    // Calculate a random kernel id that exists in the function kernel list
                    do{
                        randomKernel = rand() % kernels;
                        tempFunctionKernel = tempFunction->kernels;
                        while(tempFunctionKernel)
                        {
                            if(tempFunctionKernel->id == randomKernel)
                                break;
                            else
                                tempFunctionKernel = tempFunctionKernel->next;
                        }                        
                    }while(!tempFunctionKernel);
                    
                   
                    
                    // Assign the kernel to the task                    
					if(kernelAvailability[randomKernel] == TRUE || reassignFlag == TRUE) 
					{
						if((*tempKernel)->id == -1)
						{
							if(tempTask->taskType == TASK_MASTER)
								(*tempKernel)->id = 0;
							else
								(*tempKernel)->id = randomKernel;
								
							(*tempKernel)->next = NULL;
						}
						else
						{
							while((*tempKernel) && (*tempKernel)->next)
								tempKernel = &(*tempKernel)->next;
							
							(*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
							(*tempKernel)->next->id = randomKernel;
							(*tempKernel)->next->next = NULL;
						}
                        
						i--;
                        
						if(tempTask->taskType == TASK_MASTER)
							kernelAvailability[0] = FALSE;
						else
							kernelAvailability[randomKernel] = FALSE;
					}
                    
					if(tempTask->taskType != TASK_MASTER)
					{                        
						tempFunctionKernel = tempFunction->kernels;
                        
						while(tempFunctionKernel && !kernelAvailability[tempFunctionKernel->id])
							tempFunctionKernel = tempFunctionKernel->next;
                        
                        if(!tempFunctionKernel)     								// If not all tasks are assigned a kernel then restart
                            reassignFlag = TRUE;
					}
				}
				tempTask = tempTask->next;
			}
			
			
			// Check section tasks
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask)
                {
                    srand((unsigned) time(&t));                
                    i = tempTask->number_of_kernels;
                    tempKernel = &(tempTask->kernels);
                    while(i > 0)
                    {
                        // Calculate a random kernel id that exists in the function kernel list
                        do{
                            randomKernel = rand() % kernels;
                            tempFunctionKernel = tempFunction->kernels;
                            while(tempFunctionKernel)
                            {
                                if(tempFunctionKernel->id == randomKernel)
                                    break;
                                else
                                    tempFunctionKernel = tempFunctionKernel->next;
                            }                        
                        }while(!tempFunctionKernel);
                        
                       
                        
                        // Assign the kernel to the task                    
                        if(kernelAvailability[randomKernel] == TRUE || reassignFlag == TRUE) 
                        {
                            if((*tempKernel)->id == -1)
                            {
                                if(tempTask->taskType == TASK_MASTER)
                                    (*tempKernel)->id = 0;
                                else
                                    (*tempKernel)->id = randomKernel;
                                    
                                (*tempKernel)->next = NULL;
                            }
                            else
                            {
                                while((*tempKernel) && (*tempKernel)->next)
                                    tempKernel = &(*tempKernel)->next;
                                
                                (*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
                                (*tempKernel)->next->id = randomKernel;
                                (*tempKernel)->next->next = NULL;
                            }
                            
                            i--;
                            
                            if(tempTask->taskType == TASK_MASTER)
                                kernelAvailability[0] = FALSE;
                            else
                                kernelAvailability[randomKernel] = FALSE;
                        }
                        
                        if(tempTask->taskType != TASK_MASTER)
                        {                        
                            tempFunctionKernel = tempFunction->kernels;
                            
                            while(tempFunctionKernel && !kernelAvailability[tempFunctionKernel->id])
                                tempFunctionKernel = tempFunctionKernel->next;
                            
                            if(!tempFunctionKernel)     								// If not all tasks are assigned a kernel then restart
                                reassignFlag = TRUE;
                        }
                    }
                    tempTask = tempTask->next;
                }
				tempSection = tempSection->next;	
			}				
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;		
	}
		
}




/*********************** Assign Kernels to each task: FILE **************************/

void offlineScheduling_AssignKernelsToTasks_File(SG** Graph){
		
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
	kernel				**tempKernel;
	int					kernelAvailability[kernels];
	int 				i = 0;
    FILE                *inp;
    int                 fileKernelID = -1;
    char                temp_ch;


    // Open Scheduling Input File    
    inp = fopen(schedulingInputFile, "r");
    if(!inp){
        ERROR_COMMANDS("Scheduling File [ %s ] not found!", schedulingInputFile)
	    exit(-1);
    }
    
    
    while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
			// Check tasks
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				i = tempTask->number_of_kernels;
				tempKernel = &(tempTask->kernels);
				while(i > 0)
				{
                    // If task will be executed by one kernel only
                    if((*tempKernel)->id == -1)
                    {
                        if(tempTask->taskType == TASK_MASTER)
                        {
                            (*tempKernel)->id = 0;
                        }
                        else
                        {
                            if(fscanf(inp, "%d", &fileKernelID) != EOF)
                            {
                                (*tempKernel)->id = fileKernelID;
                            }
                            else
                            {
                                ERROR_COMMANDS("%s", "Not enough kernels in the scheduling file")
                                exit(-1);
                            }
                        }
                            
                        (*tempKernel)->next = NULL;
                    }
                    else
                    {
                        while((*tempKernel) && (*tempKernel)->next)
                            tempKernel = &(*tempKernel)->next;
                        
                        (*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
                        if(fscanf(inp, "%d", &fileKernelID) != EOF)
                        {
                             (*tempKernel)->next->id  = fileKernelID;
                             (*tempKernel)->next->next = NULL;
                        }
                        else
                        {
                            ERROR_COMMANDS("%s", "Not enough kernels in the scheduling file")
                            exit(-1);
                        }
                    }
                    i--;
				}
				tempTask = tempTask->next;
			}
			
			
			// Check section tasks
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask)
				{
					i = tempTask->number_of_kernels;
					tempKernel = &(tempTask->kernels);
					while(i > 0)
					{
                        if((*tempKernel)->id == -1)
                        {
                            if(tempTask->taskType == TASK_MASTER)
                            {
                                (*tempKernel)->id = 0;
                            }
                            else
                            {
                                if(fscanf(inp, "%d", &fileKernelID) != EOF)
                                {
                                    (*tempKernel)->id = fileKernelID;
                                }
                                else
                                {
                                    ERROR_COMMANDS("%s", "Not enough kernels in the scheduling file")
                                    exit(-1);
                                }
                            }   
                            (*tempKernel)->next = NULL;
                        }
                        else
                        {
                            while((*tempKernel) && (*tempKernel)->next)
                                tempKernel = &(*tempKernel)->next;
                            
                            (*tempKernel)->next = (kernel*)malloc(sizeof(kernel));
                            if(fscanf(inp, "%d", &fileKernelID) != EOF)
                            {
                                 (*tempKernel)->next->id  = fileKernelID;
                                 (*tempKernel)->next->next = NULL;
                            }
                            else
                            {
                                ERROR_COMMANDS("%s", "Not enough kernels in the scheduling file")
                                exit(-1);
                            }
                        }
                        i--;
					}
					tempTask = tempTask->next;
				}
				tempSection = tempSection->next;	
			}						
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;		
	}

    fclose(inp);       // Close scheduling file
}




/*********************** Count all Kernels used for all tasks **************************/

int offlineScheduling_CountKernelsToTasks_NSGA(SG** Graph){
		
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
    int                 tempTotalCounter = 0;
    
    while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
			// Count tasks
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				tempTotalCounter += tempTask->number_of_kernels;
				tempTask = tempTask->next;
			}
			
			// Count section tasks
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask)
				{
					tempTotalCounter += tempTask->number_of_kernels;					
					tempTask = tempTask->next;
				}
				tempSection = tempSection->next;	
			}						
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;		
	}
    return tempTotalCounter;
}



/*********************** Assign Fake Kernels to each task **************************/

void offlineScheduling_AssignVirtualKernelsToTasks(SG** Graph){
		
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
	kernel				*tempKernel;
	int					fakeKernelCv = 0;
	


	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
			// Check tasks
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				fakeKernelCv = 0;
				tempKernel = tempTask->kernels;
				while(tempKernel)
				{		
					tempKernel->vId = fakeKernelCv;
					fakeKernelCv++;
					tempKernel = tempKernel->next;
				}
				tempTask = tempTask->next;
			}	
			
			// Check section tasks
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask)
				{
					fakeKernelCv = 0;
					tempKernel = tempTask->kernels;
					while(tempKernel)
					{		
						tempKernel->vId = fakeKernelCv;
						fakeKernelCv++;
						tempKernel = tempKernel->next;
					}
					tempTask = tempTask->next;
				}
				tempSection = tempSection->next;	
			}						
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;		
	}
}
