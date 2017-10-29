/**************************************************/
/*                                                */
/*  File:        handleBisonRules.c               */
/*  Description: Connect the Bison rules with the */
/* 	             source producing & printing      */
/*               functions.                       */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/


#include "definitions.h"



extern int 	line;
extern int 	pass;
extern int  runtimeSystem;
extern int 	kernels;
extern int  currentFile;
extern bool firstPass;
extern char **inputFiles;
extern FILE *outp;
extern FILE *inp, *outp_sw_main, *outp_sw_h, *outp_sw_threadpool, *outp_sw_threads, *outp_sw_tao_h;



/*********************** Handle Varialbe Lists **************************/


void handlePragma_variableList(int inWhichList, dataList *lists[], char *variableStr, char *variableType, char *reductionType, arrayIndex** indexes){
	
	dataList **tempDataList;
	
	
	if(firstPass)
	{
		switch(inWhichList){
		
				case IN_LINEAR:
						
						tempDataList = &lists[IN_LINEAR];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_ALIGNED:
						
						tempDataList = &lists[IN_ALIGNED];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_IS_DEVICE_PTR:
						
						tempDataList = &lists[IN_IS_DEVICE_PTR];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_USE_DEVICE_PTR:
						
						tempDataList = &lists[IN_USE_DEVICE_PTR];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_THREAD_PRIVATE:
				
						tempDataList = &lists[IN_THREAD_PRIVATE];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_FLUSH:
				
						tempDataList = &lists[IN_FLUSH];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_COPY_PRIVATE:
					
						tempDataList = &lists[IN_COPY_PRIVATE];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_COPY_IN:
				
						tempDataList = &lists[IN_COPY_IN];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_PRIVATE:
				
						tempDataList = &lists[IN_PRIVATE];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_FIRST_PRIVATE:
						
						tempDataList = &lists[IN_FIRST_PRIVATE];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_LAST_PRIVATE:
				
						tempDataList = &lists[IN_LAST_PRIVATE];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_SHARED:
	
						tempDataList = &lists[IN_SHARED];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_REDUCTION:
				
						tempDataList = &lists[IN_REDUCTION];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_DEPEND_IN:
				
						tempDataList = &lists[IN_DEPEND_IN];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_DEPEND_OUT:
				
						tempDataList = &lists[IN_DEPEND_OUT];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				case IN_DEPEND_INOUT:
				
						tempDataList = &lists[IN_DEPEND_INOUT];
						addToDataList(tempDataList, variableStr, variableType, reductionType, indexes);
						break;
						
				/*
				 * Add a new case here if you add a new list and a new definition list in definitions.h
				 */
								
				default:
						ERROR_IN_TRANSLATOR("%s", "Data List not found\n");
			}
	}
		
	
}



/******************* Handle [ threadprivate] Directive **********************/


void handlePragma_threadPrivateDirective(SG **Graph, dataList *lists[]){
	
	dataList **tempDataList = &lists[IN_THREAD_PRIVATE];
	dataList **tempThreadPrivateList = &(*Graph)->threadPrivateList;
	
	if(firstPass)
	{
		copyDataListToSG(tempThreadPrivateList, tempDataList);
	}
	
	if(!firstPass)
	{
		//WRITE("%s", "FOUND PRAGMA [ threadprivate ]\n");	
	}
	
}



/*********************** Handle [ Parallel ] Construct **************************/


void handlePragma_parallelConstruct(SG **Graph, int numKernels, int stateOfDefault, dataList *lists[], int currentFunction){
	
	parallel_function **tempFunction = &(*Graph)->parallel_functions;
	
	if(firstPass)
	{
		if(numKernels == 0) numKernels = kernels;
		if(numKernels > kernels)
		{
			WARNING("Number of kernels [ %d ] declared for this Parallel Construct is more than total number of kernels [ %d ]!\n\t\t--> [ %d ] kernels will be used instead!", line, numKernels, kernels, kernels);
			numKernels = kernels;
		}
		if(stateOfDefault == 0) stateOfDefault = DEFAULT_SHARED;
		
		addParallelFunctionToGraph(tempFunction, numKernels, stateOfDefault, lists);
	}
	else if(!firstPass)
	{
		printInMainFile_ParallelFunctions(tempFunction, currentFunction);
        
        if(runtimeSystem != RUNTIME_TAO)
            printInThreadsFile_ParallelFunctionsHeader(tempFunction, currentFunction);	
        else
            printInThreadsFile_ParallelFunctionsHeader_TAO(tempFunction, currentFunction);	
	}
}



/*********************** Handle [ Sections ] Construct **************************/


void handlePragma_sectionsConstruct(SG **Graph, dataList *lists[]){
	
	parallel_function **tempFunction = &(*Graph)->parallel_functions;
	
	if(firstPass)
	{
		addSectionToParallelFunction(tempFunction, lists);
	}
	
	if(!firstPass)
	{
		//WRITE("%s", "FOUND PRAGMA [ sections ]\n");	
	}
}



/*********************** Handle [ section ] Construct **************************/


void handlePragma_sectionDirective(SG **Graph, dataList *lists[], int currentFunction, int currentTask){
	
	parallel_function **tempFunction = &(*Graph)->parallel_functions;
	
	if(firstPass)
	{
		addTaskToSection(tempFunction, lists, currentTask);
	}
	
	if(!firstPass)
	{
        if(runtimeSystem != RUNTIME_TAO)
            printInThreadsFile_TaskSourceCode(tempFunction, currentFunction, currentTask);
        else
            printInThreadsFile_TaskSourceCode_TAO(tempFunction, currentFunction, currentTask);
	}
}



/*********************** Handle [ task ] Construct **************************/


void handlePragma_taskConstruct(SG **Graph, dataList *lists[], int type, int numKernels, int stateOfDefault, int priority, int schedulingPolicy, intStr schedulingPolicyChunk, int simdLength, int currentFunction, int currentTask){
	
	parallel_function **tempFunction = &(*Graph)->parallel_functions;
	
	if(firstPass)
	{
		if(stateOfDefault == 0) stateOfDefault = DEFAULT_SHARED;
		
		if((type == TASK_LOOP || type == TASK_REDUCTION) && schedulingPolicy == 0){					// Default Scheduling policy is STATIC(32)
			schedulingPolicy = LOOP_SCHED_STATIC;
			schedulingPolicyChunk.localInt = 32;
		}
		
		// Set default chunk size of STATIC scheduling policies
		if((type == TASK_LOOP || type == TASK_REDUCTION) && (schedulingPolicy == LOOP_SCHED_STATIC || schedulingPolicy == LOOP_SCHED_CROSS)){
			if(!schedulingPolicyChunk.localInt && !schedulingPolicyChunk.localStr)
				schedulingPolicyChunk.localInt = 32;
		}
		
		addTaskToParallelFunction(tempFunction, lists, type, numKernels, stateOfDefault, priority, schedulingPolicy, schedulingPolicyChunk, simdLength, currentTask);
		
	}
	
	if(!firstPass)
	{
        if(runtimeSystem != RUNTIME_TAO)
            printInThreadsFile_TaskSourceCode(tempFunction, currentFunction, currentTask);
        else
            printInThreadsFile_TaskSourceCode_TAO(tempFunction, currentFunction, currentTask);
	}
}





