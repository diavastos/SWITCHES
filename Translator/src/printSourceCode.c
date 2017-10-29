/**************************************************/
/*                                                */
/*  File:        printSourceCode.c                */
/*  Description: Print source code in output files*/
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/


#include "definitions.h"


int  maxCores = 0;
int  hThreads = 0;
int  OSthread = 0;


extern int line;
extern int pass;
extern int kernels;
extern bool firstPass;
extern int affinityPolicy;
extern int targetSystem;
extern int runtimeSystem;
extern int globalDeclarationCounter;
extern char globalDeclarationString[SIZE][SIZE];
extern char includesDeclaration[SIZE][SIZE];
extern int  includesDeclarationCounter;
extern int currentFor;
extern bool transactions;
extern char	**stringFor;
extern char **inputFiles;
extern int totalInputFiles;
extern FILE *inp, *outp_sw_main, *outp_sw_h, *outp_sw_threadpool, *outp_sw_threads, *outp_sw_tao_h;
extern FILE *outp;





/*********************** Print in [ sw_threads.c ] file **************************/


void printInMainFile_ParallelFunctions(parallel_function **GraphFunc, int currentFunction){
	
	
	parallel_function* tempParallelFunctions = *GraphFunc;
	parallel_function* tempFunction = *GraphFunc;
    producer*          tempConsumer;
    producer*          tempProducer;
    int functionsCounter = 0;
    int TAOwidth = 0;
    
	
	//Redirect output
	__OUTP_IS_NOW_MAIN_FILE
	
    switch(runtimeSystem){
        
        case RUNTIME_STATIC:
                WRITE("__threadpool_initialize(__FUNCTION_%d);\n\n", currentFunction);
                break;
        case RUNTIME_TAO:
        case RUNTIME_TAOSW:
        
                // Count total number of parallel functions (TAOs)
                functionsCounter = 0;
                tempFunction = *GraphFunc;
                while(tempFunction)
                {
                    functionsCounter++;
                    tempFunction = tempFunction->next;
                }
                
        
                if(currentFunction == 1){
                    
                    WRITE("\t%s\n", "/***** 	Start TAOs *****/");
                    WRITE("\t%s\n", "PolyTask *st1;");                    
                    WRITE("\t%s\n\n", "gotao_init();"); 
                    
                                       
                    WRITE("\t%s\n", "/***** Declare TAOs *****/");
                    tempFunction = *GraphFunc;
                    while(tempFunction)
                    {                    
                        WRITE("\t__FUNC_%d* __function_%d;\n", tempFunction->id, tempFunction->id);
                        tempFunction = tempFunction->next;
                    }
                    
                    WRITE("\n\t%s\n", "/***** Allocate TAOs *****/");                    
                    tempFunction = *GraphFunc;
                    while(tempFunction)
                    {
                        WRITE("\t__function_%d = new __FUNC_%d(%d, %d, %d);\n", tempFunction->id, tempFunction->id, tempFunction->id, functionsCounter, tempFunction->number_of_kernels);
                        tempFunction = tempFunction->next;
                    }
                    
                    WRITE("\n\t%s\n", "/***** 	Declare TAO Dependencies *****/");
                    tempFunction = *GraphFunc;
                    while(tempFunction)
                    {       
                        tempConsumer = tempFunction->tred_consumers;
                        while(tempConsumer)
                        {
                            WRITE("\t__function_%d->make_edge(__function_%d);\n", tempFunction->id, tempConsumer->id);    
                            tempConsumer = tempConsumer->next;                        
                        }                                         
                        tempFunction = tempFunction->next;
                    }
                    WRITE("%s", "\n");
                }
                
                
                tempFunction = *GraphFunc;
                while(tempFunction)
                {
                    if(tempFunction->id == currentFunction){
                        tempProducer = tempFunction->producers;
                        if(!tempProducer)
                        {
                            WRITE("\tgotao_push_init(__function_%d, %d %% gotao_nthreads);\n", currentFunction, currentFunction);
                        }
                        break;
                    }
                    tempFunction = tempFunction->next;
                }
                
                
                if(currentFunction == functionsCounter){
                    WRITE("\n\t%s\n\n", "gotao_start();");
                    WRITE("\t%s\n\n", "gotao_fini();");                    
                }
                break;
                
        default:
                ERROR_COMMANDS("Runtime System [ %d ] not recognized!", runtimeSystem)
                exit(-1);
    }
}



/*********************** Print in [ sw_threadpool.c ] file **************************/


void printInThreadpoolFile(SG** Graph){

	int 				i = 0;
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;

	// Redirect output
	__OUTP_IS_NOW_THREADPOOL_FILE

	WRITE("%s", "#include \"sw.h\"\n");
	WRITE("%s", "\n\n");
	WRITE("%s", "bool __threads_created = FALSE;\n");
	WRITE("%s", "\n\n");
	
	/* Print __threadpool_initialize() */
	
	WRITE("%s", "/*** Initialize Threadpool ***/\n\n");
	WRITE("%s", "void __threadpool_initialize(int __currentFunction)\n");
	WRITE("%s", "{\n");
	WRITE("%s", "    int         i = 0;\n");
	WRITE("%s", "    __arguments *arguments;\n\n");
	WRITE("%s", "    if(!__threads_created)\n");
	WRITE("%s", "    {\n");
	
	WRITE("%s", "        /* Initialize the function barriers */\n\n");
	WRITE("%s", "        pthread_barrier_init(&barrier[0], NULL, __KERNELS);\n");

	tempGraph = *Graph;
	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{	
			WRITE("        pthread_barrier_init(&barrier[__FUNCTION_%d], NULL, __KERNELS);\n", tempFunction->id);
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;
	}
	WRITE("%s", "\n\n");
	
	WRITE("%s", "        /* Initialize the kernels in the threadpool */\n\n");
	WRITE("%s", "        for(i = 1; i <= __PTHREADS; i++)\n");
	WRITE("%s", "        {\n");
	WRITE("%s", "            int rc = 0;\n");
	WRITE("%s", "            arguments = (__arguments*)malloc(sizeof(__arguments));\n");
	WRITE("%s", "            arguments->id = i;\n");
	WRITE("%s", "            arguments->function_id = 0;  // Only Main Kernel defines the currently executed parallel function\n\n");
	WRITE("%s", "            /* Create Pthread and assign its ID */\n");
	WRITE("%s", "            rc = pthread_create(&__kernels[i-1], NULL, (void *)thread_jobs, (void *)arguments);\n");
	WRITE("%s", "            if(rc)\n");
	WRITE("%s", "            {\n");
	WRITE("%s", "                ERROR(\"__threadpool_initialize(): Return code from pthread_create is [ %d ]\", rc);\n");
	WRITE("%s", "                exit(-1);\n");
	WRITE("%s", "            }\n");
	WRITE("%s", "        }\n");
	WRITE("%s", "        __threads_created = TRUE;\n");
	WRITE("%s", "        arguments = (__arguments*)malloc(sizeof(__arguments));\n");
	WRITE("%s", "        arguments->id = __MAIN_KERNEL;\n");
	WRITE("%s", "        arguments->function_id = __currentFunction;\n");
	WRITE("%s", "        thread_jobs((void *)arguments);\n");
	WRITE("%s", "    }\n");
	WRITE("%s", "    else\n");
	WRITE("%s", "    {\n");
	WRITE("%s", "        arguments = (__arguments*)malloc(sizeof(__arguments));\n");
	WRITE("%s", "        arguments->id = __MAIN_KERNEL;\n");
	WRITE("%s", "        arguments->function_id = __currentFunction;\n");
	WRITE("%s", "        thread_jobs((void *)arguments);\n");
	WRITE("%s", "    }\n");
	WRITE("%s", "}\n");
	WRITE("%s", "\n\n");
	
	
	/* Print __threadpool_destroy() */
	
	WRITE("%s", "/*** Destroy Threadpool ***/\n\n");
	WRITE("%s", "void __threadpool_destroy()\n");
	WRITE("%s", "{\n");
	WRITE("%s", "    int i = 0;\n\n");
	WRITE("%s", "    /* Wait for all kernels to finish */\n\n");
	WRITE("%s", "    for(i = 0; i < __PTHREADS; i++)\n");
	WRITE("%s", "    {\n");
	WRITE("%s", "        pthread_join(__kernels[i], NULL);\n");
	WRITE("%s", "    }\n");
	WRITE("%s", "}\n");
	WRITE("%s", "\n\n");
	
	
}



/*********************** Print in [ sw.h ] file ****************************/


void printInSwFile(SG** Graph){
	
	int 				i = 0, j = 0;
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	task 				*tempTask;
    section             *tempSection;
    kernel              *tempKernel;
	
	//Redirect output
	__OUTP_IS_NOW_SW_FILE
	
	/* Print include files */
	
	WRITE("%s", "#define _GNU_SOURCE\n");
	WRITE("%s", "#include <stdio.h>\n");
	WRITE("%s", "#include <stdlib.h>\n");
	WRITE("%s", "#include <stdint.h>\n");
	WRITE("%s", "#include <pthread.h>\n");
	WRITE("%s", "#include <sched.h>\n");
	WRITE("%s", "#include <stdbool.h>\n");
	WRITE("%s", "#include <math.h>\n");
	WRITE("%s", "#include <time.h>\n");
	WRITE("%s", "\n\n");
	
	
	
    /* Print TAO + TAOSW Defintions */
    
    if(runtimeSystem == RUNTIME_TAO || runtimeSystem == RUNTIME_TAOSW){
    
        WRITE("%s", "/************************** TAO Definitions *****************************/\n\n");
        WRITE("#define GOTAO_NTHREADS  %d\n", kernels);
        //WRITE("#define MAXTHREADS  %d\n", ????);
        //WRITE("#define IDLE_SWITCH  %d\n", maxCores*hThreads);
        WRITE("%s", "\n\n");
    }
    
    
    /* Print General Defintions */
    
    if(runtimeSystem != RUNTIME_TAO){
	
        WRITE("%s", "/************************** General Definitions *****************************/\n\n");
        WRITE("#define __KERNELS  %d\n", kernels);
        WRITE("%s", "#define __PTHREADS (__KERNELS-1)\n");
        WRITE("%s", "#define __ON       1\n");
        WRITE("%s", "#define __OFF      0\n");
        WRITE("%s", "#define TRUE       1\n");
        WRITE("%s", "#define FALSE      0\n");
        WRITE("%s", "\n\n");
	}
	
	/* Print Error Printing Codes */
	
	WRITE("%s", "/************************** Error Printing Colors *****************************/\n\n");
	WRITE("%s", "#define ANSI_COLOR_RED     \"\\x1b[31m\"\n");
	WRITE("%s", "#define ANSI_COLOR_GREEN   \"\\x1b[32m\"\n");
	WRITE("%s", "#define ANSI_COLOR_YELLOW  \"\\x1b[33m\"\n");
	WRITE("%s", "#define ANSI_COLOR_BLUE    \"\\x1b[34m\"\n");
	WRITE("%s", "#define ANSI_COLOR_MAGENTA \"\\x1b[35m\"\n");
	WRITE("%s", "#define ANSI_COLOR_CYAN    \"\\x1b[36m\"\n");
	WRITE("%s", "#define ANSI_COLOR_WHITE   \"\\x1B[37m\"\n");
	WRITE("%s", "#define ANSI_COLOR_RESET   \"\\x1b[0m\"\n");
	WRITE("%s", "\n");
	WRITE("%s", "#define ERROR(FormalLiteral, ...)   fprintf(stderr, ANSI_COLOR_RED \"%s:%d: ERROR: \" FormalLiteral ANSI_COLOR_RESET \"\\n\", __FILE__, __LINE__, __VA_ARGS__);\n");
	WRITE("%s", "\n");
	WRITE("%s", "\n\n");


	/* Print Kernel Declaration */
	
    if(runtimeSystem != RUNTIME_TAO){
    
        WRITE("%s", "/************************ Declare all the KERNELS here ************************/\n\n");
        WRITE("%s", "#define __MAIN_KERNEL 0\n");
        for(i = 1; i < kernels; i++)
            WRITE("#define __KERNEL_%d    %d\n", i, i);
            
        WRITE("%s", "\n\n");
    }

	/* Print Threadpool Function Prototypes */
	if(runtimeSystem == RUNTIME_STATIC){
        WRITE("%s", "/********************** Threadpool Function Prototypes ************************/\n\n");
        WRITE("%s", "pthread_t __kernels[__PTHREADS];\n");
        i = 1;
        tempGraph = *Graph;
        while(tempGraph)
        {
            tempFunction = tempGraph->parallel_functions;
            while(tempFunction)
            {	
                i++;
                tempFunction = tempFunction->next;
            }
            tempGraph = tempGraph->next;
        }
        WRITE("pthread_barrier_t barrier[%d];                   // # of functions + 1 (for the entire thread_jobs)\n\n", i);
        WRITE("%s", "void __threadpool_initialize(int);\n");
        WRITE("%s", "void __threadpool_destroy();\n");
        WRITE("%s", "\n\n");
    }
	


	/* Print Pthreads complex arguments declaration */
	
	WRITE("%s", "/******************** Thread Complex Arguments Definition **********************/\n\n");
	WRITE("%s", "typedef struct __arguments{\n\n");
	WRITE("%s", "    long   id;\n");
	WRITE("%s", "    long   function_id;\n\n");
	WRITE("%s", "}__arguments;\n");
	WRITE("%s", "\n\n");
	
	
	
	/* Declare Loop Chunk Sizes */
    
    if(runtimeSystem != RUNTIME_TAO){
	
        WRITE("%s", "/************************** Declare Loop Chunk Sizes **************************/\n\n");
        tempGraph = *Graph;
        while(tempGraph)
        {
            tempFunction = tempGraph->parallel_functions;
            while(tempFunction)
            {	
                tempTask = tempFunction->tasks;
                while(tempTask)
                {
                    if(tempTask->taskType == TASK_LOOP || tempTask->taskType == TASK_REDUCTION)
                    {
                        if(tempTask->chunkSize.localInt)
                        {
                            WRITE("#define __LOOP_%d_CHUNK    %d\n", tempTask->id, tempTask->chunkSize.localInt);
                        }
                        else
                        {
                            WRITE("#define __LOOP_%d_CHUNK    %s\n", tempTask->id, tempTask->chunkSize.localStr);
                        }
                    }
                    tempTask = tempTask->next;			
                }
                tempFunction = tempFunction->next;
            }
            tempGraph = tempGraph->next;
        }
        WRITE("%s", "\n\n");
        
        
        
        /* Declare Loop Kernel Number */
        
        WRITE("%s", "/************************** Declare Loop Kernel Number **************************/\n\n");
        tempGraph = *Graph;
        while(tempGraph)
        {
            tempFunction = tempGraph->parallel_functions;
            while(tempFunction)
            {	
                tempTask = tempFunction->tasks;
                while(tempTask)
                {
                    if(tempTask->taskType == TASK_LOOP || tempTask->taskType == TASK_REDUCTION)
                    {
                        WRITE("#define __LOOP_%d_KERNELS    %d\n", tempTask->id, tempTask->number_of_kernels);
                    }
                    tempTask = tempTask->next;			
                }
                tempFunction = tempFunction->next;
            }
            tempGraph = tempGraph->next;
        }
        WRITE("%s", "\n\n");
        
        
        
        /* Declare Loop Counters */
        
        WRITE("%s", "/************************** Declare Loop Counters **************************/\n\n");
        tempGraph = *Graph;
        while(tempGraph)
        {
            tempFunction = tempGraph->parallel_functions;
            while(tempFunction)
            {	
                tempTask = tempFunction->tasks;
                while(tempTask)
                {
                    if(tempTask->taskType == TASK_LOOP || tempTask->taskType == TASK_REDUCTION)
                    {
                        WRITE("#define __LOOP_%d_COUNTER    (__LOOP_%d_CHUNK * __LOOP_%d_KERNELS)\n", tempTask->id, tempTask->id, tempTask->id);
                    }
                    tempTask = tempTask->next;			
                }
                tempFunction = tempFunction->next;
            }
            tempGraph = tempGraph->next;
        }
        WRITE("%s", "\n\n");
    }
	
	
	
	/* Declare Parallel Functions */
	
	WRITE("%s", "/************************** Declare Parallel Functions **************************/\n\n");
	tempGraph = *Graph;
	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{	
			WRITE("#define __FUNCTION_%d    %d\n", tempFunction->id, tempFunction->id);
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;
	}
	WRITE("%s", "\n\n");
    	
	

	/* Print Parallel Functions Prototype */

	WRITE("%s", "/*********************** Functions Prototypes ************************/\n\n");
	
    if(runtimeSystem != RUNTIME_TAO){
        
        tempGraph = *Graph;
        while(tempGraph)
        {
            tempFunction = tempGraph->parallel_functions;
            while(tempFunction)
            {	
                WRITE("void __sw_resetSWitches_%d(int);\n", tempFunction->id);
                tempFunction = tempFunction->next;
            }
            tempGraph = tempGraph->next;
        }
    }
    
	WRITE("%s", "void *thread_jobs(void *);\n");
	
    tempGraph = *Graph;
	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{	
			WRITE("void *parallel_function_%d(void *);\n", tempFunction->id);
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;
	}
	WRITE("%s", "\n\n");

}





/*********************** Print Header source code in [ sw_threads.c ] file **************************/


void printInThreadsFile_headerSourceCode(SG** Graph){
	
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
	dataList			*tempList;
	kernel				*tempKernel;
	int 				i = 0;
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	

	WRITE("%s", "#include \"sw.h\"\n");
	WRITE("%s", "\n\n");
	
	
	/* Print all #includes & #defines from main files to [ sw_threads.c ] */
	
	WRITE("%s", "\n/*** APPLICATION Definitions & Includes ****/\n\n");
	
	for(i = 0; i < includesDeclarationCounter; i++)
	{
		WRITE("%s", includesDeclaration[i]);
	}
	WRITE("%s", "\n");
	
	
	
	/* Print __sw_global__ lines */
	
	WRITE("%s", "\n/*** APPLICATION declarations ****/\n\n");
	
	for(i = 0; i <= globalDeclarationCounter; i++)
	{
		WRITE("extern %s", globalDeclarationString[i]);
	}
	WRITE("%s", "\n");
	
	
	/* Print Global variables */
	
	WRITE("%s", "long currentFunction = 0;\n");
	WRITE("%s", "\n\n");
	
	
}



/***************** Print Header of Parallel Functions in [ sw_threads.c ] file ********************/


void printInThreadsFile_ParallelFunctionsHeader(parallel_function **GraphFunc, int currentFunction){
	
	parallel_function	*tempFunction = *GraphFunc;
	section 			*tempSection;
	task 				*tempTask;
	task 				*tempTask2;
	dataList			*tempList;
	dataList			*tempReductionList;
	kernel				*tempKernel;
	bool				loopTask = FALSE;
	bool				crossLoopTask = FALSE;
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
	
	/* Print Application parallel functions source code */
	
	WRITE("%s", "\n/*** Application Parallel Function ***/\n\n");

	while(tempFunction && tempFunction->next)
		if(tempFunction->id == currentFunction)
			break;
		else
			tempFunction = tempFunction->next;
	
	
	if(tempFunction->id == currentFunction)
	{
		printInThreadsFile_ReductionVariables(tempFunction);				// Print Reduction global variables of this parallel function in [ sw_threads.c ]
		
		WRITE("%s", "\n\n");
			
		WRITE("void *parallel_function_%d(void *arg)\n", tempFunction->id);
		WRITE("%s", "{\n");
		WRITE("%s", "    long tid;\n");
		WRITE("%s", "    long vTid = 0;\n");
		WRITE("%s", "    tid = (long)arg;\n");
		WRITE("%s", "    bool ready = FALSE;\n");
		WRITE("    long __sw_tasksCounter = __sw_taskCounter_Function_%d[tid];\n", tempFunction->id);
	
		/*** Print Thread local Variables ***/
		WRITE("%s", "\n    /*** Declare private/firstprivate variables ***/\n\n");
		
		// Print Function Private List
		tempList = tempFunction->privateList;
		if(tempList)
		{
			while(tempList)
			{
				WRITE("    %s %s = 0;\n", tempList->variableType, tempList->variableName);
				tempList = tempList->next;
			}
		}
        
        // Print Function Firstprivate List
		tempList = tempFunction->firstPrivateList;
		if(tempList)
		{
			while(tempList)
			{
				WRITE("    %s private_%s = %s;\n", tempList->variableType, tempList->variableName, tempList->variableName);
				WRITE("    %s %s = private_%s;\n", tempList->variableType, tempList->variableName, tempList->variableName);
				tempList = tempList->next;
			}
		}
		
		tempTask = tempFunction->tasks;
		while(tempTask)
		{
			// Print Tasks private list
			tempList = tempTask->privateList;
			if(tempList)
			{
				while(tempList)
				{
					WRITE("    %s %s = 0;\n", tempList->variableType, tempList->variableName);
					tempList = tempList->next;
				}
			}
            
            // Print Tasks Firstprivate list
			tempList = tempTask->firstPrivateList;
			if(tempList)
			{
				while(tempList)
				{
					WRITE("    %s private_%s = %s;\n", tempList->variableType, tempList->variableName, tempList->variableName);
					WRITE("    %s %s = private_%s;\n", tempList->variableType, tempList->variableName, tempList->variableName);
					tempList = tempList->next;
				}
			}
			
			// Print SW loop private variables if there is at least a task that is a loop
			if(tempTask->taskType == TASK_LOOP || tempTask->taskType == TASK_REDUCTION)
			{
                if(!loopTask)
                {
                    WRITE("%s", "    // Loop task private variables\n");
                    WRITE("%s", "    long __sw_i               = 0;\n");
                    WRITE("%s", "    long __sw_loop_start      = 0;\n");
                    WRITE("%s", "    long __sw_loop_end        = 0;\n");
                    WRITE("%s", "    long __sw_loop_chunk      = 0;\n");
                    loopTask = TRUE;
                }
				
                if(tempTask->crossLoopProducer && !crossLoopTask)
				{
					WRITE("%s", "    long __sw_loop_switches   = 0;\n");
					WRITE("%s", "    long __sw_loop_switch     = 0;\n");
					//WRITE("%s", "    long __sw_loop_kernel     = 0;\n");
                    crossLoopTask = TRUE;
				}
			}
			
			if(tempTask->taskType == TASK_REDUCTION)
			{
				WRITE("%s", "    // Reduction task private variables\n");
				tempReductionList = tempTask->reductionList;
				while(tempReductionList)
				{
					WRITE("    %s %s = ", tempReductionList->variableType, tempReductionList->variableName);
					if(!strcmp(tempReductionList->reductionType, "*") || !strcmp(tempReductionList->reductionType, "&&"))
					{
						WRITE("%s", "1;\n");
					}
					else
					{
						WRITE("%s", "0;\n");
					}
					tempReductionList = tempReductionList->next;
				}
			}
			tempTask = tempTask->next;			
		}
		
		tempSection = tempFunction->sections;
		if(tempSection)
		{
			while(tempSection)
			{
				// Print Section Private List
				tempList = tempSection->privateList;
				if(tempList)
				{
					while(tempList)
					{
						WRITE("    %s private_%s = 0;\n", tempList->variableType, tempList->variableName);
						tempList = tempList->next;
					}
				}
                
                // Print Section Firstprivate List
				tempList = tempSection->firstPrivateList;
				if(tempList)
				{
					while(tempList)
					{
						WRITE("    %s private_%s = %s;\n", tempList->variableType, tempList->variableName, tempList->variableName);
						tempList = tempList->next;
					}
				}
                
				tempSection = tempSection->next;
			}
		}
	}
	else
	{
		ERROR_IN_TRANSLATOR("Function_%d not found in SG\n", currentFunction);
		exit(-1);
	}
    
    /*** ADD THIS FOR multiple executions of the same parallel function without returning to the main program ***/
    
    //WRITE("\n\n    __sw_resetSWitches_%d(tid);\n", currentFunction);
    
    /************************************************************************************************************/
	    
	/* Print the start of the infinite loop */
	WRITE("%s", "\n    do{\n\n");
	
}



/***************** Print Header of Parallel Functions in [ sw_threads.c ] file ********************/


void printInThreadsFile_ParallelFunctionsHeader_TAO(parallel_function **GraphFunc, int currentFunction){
	
	parallel_function	*tempFunction = *GraphFunc;
	section 			*tempSection;
	task 				*tempTask;
	task 				*tempTask2;
	dataList			*tempList;
	dataList			*tempReductionList;
	kernel				*tempKernel;
	bool				loopTask = FALSE;
	bool				crossLoopTask = FALSE;
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
	
	/* Print Application parallel functions source code */
	
	WRITE("%s", "\n/*** Application Parallel Function ***/\n\n");

	while(tempFunction && tempFunction->next)
		if(tempFunction->id == currentFunction)
			break;
		else
			tempFunction = tempFunction->next;
	
	
	if(tempFunction->id == currentFunction)
	{
		printInThreadsFile_ReductionVariables(tempFunction);				// Print Reduction global variables of this parallel function in [ sw_threads.c ]
		
		WRITE("%s", "\n\n");
			
		WRITE("void *parallel_function_%d(void *arg)\n", tempFunction->id);
		WRITE("%s", "{\n");
		WRITE("%s", "    long tid;\n");
		//WRITE("%s", "    long vTid = 0;\n");
		WRITE("%s", "    tid = (long)arg;\n");
		//WRITE("%s", "    bool ready = FALSE;\n");
		//WRITE("    long __sw_tasksCounter = __sw_taskCounter_Function_%d[tid];\n", tempFunction->id);
	
		/*** Print Thread local Variables ***/
		WRITE("%s", "\n    /*** Declare private/firstprivate variables ***/\n\n");
		
		// Print Function Private List
		tempList = tempFunction->privateList;
		if(tempList)
		{
			while(tempList)
			{
				WRITE("    %s %s = 0;\n", tempList->variableType, tempList->variableName);
				tempList = tempList->next;
			}
		}
        
        // Print Function Firstprivate List
		tempList = tempFunction->firstPrivateList;
		if(tempList)
		{
			while(tempList)
			{
				WRITE("    %s private_%s = %s;\n", tempList->variableType, tempList->variableName, tempList->variableName);
				WRITE("    %s %s = private_%s;\n", tempList->variableType, tempList->variableName, tempList->variableName);
				tempList = tempList->next;
			}
		}
		
		tempTask = tempFunction->tasks;
		while(tempTask)
		{
			// Print Tasks private list
			tempList = tempTask->privateList;
			if(tempList)
			{
				while(tempList)
				{
					WRITE("    %s %s = 0;\n", tempList->variableType, tempList->variableName);
					tempList = tempList->next;
				}
			}
            
            // Print Tasks Firstprivate list
			tempList = tempTask->firstPrivateList;
			if(tempList)
			{
				while(tempList)
				{
					WRITE("    %s private_%s = %s;\n", tempList->variableType, tempList->variableName, tempList->variableName);
					WRITE("    %s %s = private_%s;\n", tempList->variableType, tempList->variableName, tempList->variableName);
					tempList = tempList->next;
				}
			}
			
			// Print SW loop private variables if there is at least a task that is a loop
			if(tempTask->taskType == TASK_LOOP || tempTask->taskType == TASK_REDUCTION)
			{
                if(!loopTask)
                {
                    WRITE("%s", "    // Loop task private variables\n");
                    WRITE("%s", "    long __sw_i               = 0;\n");
                    WRITE("%s", "    long __sw_loop_start      = 0;\n");
                    WRITE("%s", "    long __sw_loop_end        = 0;\n");
                    WRITE("%s", "    long __sw_loop_chunk      = 0;\n");
                    loopTask = TRUE;
                }
				
                if(tempTask->crossLoopProducer && !crossLoopTask)
				{
					WRITE("%s", "    long __sw_loop_switches   = 0;\n");
					WRITE("%s", "    long __sw_loop_switch     = 0;\n");
					//WRITE("%s", "    long __sw_loop_kernel     = 0;\n");
                    crossLoopTask = TRUE;
				}
			}
			
			if(tempTask->taskType == TASK_REDUCTION)
			{
				WRITE("%s", "    // Reduction task private variables\n");
				tempReductionList = tempTask->reductionList;
				while(tempReductionList)
				{
					WRITE("    %s %s = ", tempReductionList->variableType, tempReductionList->variableName);
					if(!strcmp(tempReductionList->reductionType, "*") || !strcmp(tempReductionList->reductionType, "&&"))
					{
						WRITE("%s", "1;\n");
					}
					else
					{
						WRITE("%s", "0;\n");
					}
					tempReductionList = tempReductionList->next;
				}
			}
			tempTask = tempTask->next;			
		}
		
		tempSection = tempFunction->sections;
		if(tempSection)
		{
			while(tempSection)
			{
				// Print Section Private List
				tempList = tempSection->privateList;
				if(tempList)
				{
					while(tempList)
					{
						WRITE("    %s private_%s = 0;\n", tempList->variableType, tempList->variableName);
						tempList = tempList->next;
					}
				}
                
                // Print Section Firstprivate List
				tempList = tempSection->firstPrivateList;
				if(tempList)
				{
					while(tempList)
					{
						WRITE("    %s private_%s = %s;\n", tempList->variableType, tempList->variableName, tempList->variableName);
						tempList = tempList->next;
					}
				}
                
				tempSection = tempSection->next;
			}
		}
	}
	else
	{
		ERROR_IN_TRANSLATOR("Function_%d not found in SG\n", currentFunction);
		exit(-1);
	}
    
    /*** ADD THIS FOR multiple executions of the same parallel function without returning to the main program ***/
    
    //WRITE("\n\n    __sw_resetSWitches_%d(tid);\n", currentFunction);
    
    /************************************************************************************************************/
	    
	/* Print the start of the infinite loop */
	//WRITE("%s", "\n    do{\n\n");
	
}




/***************** Turn switches of a task ON after finishing execution ********************/


void printInThreadsFile_TurnSwitchesOn(parallel_function **GraphFunc, int currentFunction, int currentTask){
	
	parallel_function	*tempFunction = *GraphFunc;
	section 			*tempSection;
	task 				*tempTask;
	kernel				*tempKernel;
	dataList			*tempReductionList;
    int                 temp = 0, flag = 0;
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
	
	// Find current function
	
	while(tempFunction && tempFunction->next)
		if(tempFunction->id == currentFunction)
			break;
		else
			tempFunction = tempFunction->next;
			
	if(tempFunction && tempFunction->id == currentFunction)
	{	
		// Find the current task in the SG
		
		tempTask = tempFunction->tasks;
		while(tempTask && tempTask->next)
			if(tempTask->id == currentTask)
				break;
			else
				tempTask = tempTask->next;
		
		// If it is a task else it is a section task
		if(tempTask && tempTask->id == currentTask)
		{
			// Add code here for updating loop task dependencies if needed
			 
			 if(tempTask->taskType == TASK_LOOP || tempTask->taskType == TASK_REDUCTION)
			 {
				  if(tempTask->crossLoopProducer == TRUE)
				  {
					    WRITE("%s", "\n                // Update chunk switch & continue\n");
			            WRITE("                __sw_task_%d_chunk[vTid][__sw_i/__LOOP_%d_COUNTER] = __ON;\n", tempTask->id, tempTask->id);
						WRITE("%s", "                __sync_synchronize();\n\n");
				  }
				  WRITE("%s", "            }\n\n");
			 }
			 
			 if(tempTask->taskType == TASK_REDUCTION)
			 {
				 // Copy local reduction result to my global reduction variable
				
				tempReductionList = tempTask->reductionList;
				while(tempReductionList)
				{
					WRITE("            __sw_%s_task_%d[vTid] = %s;\n", tempReductionList->variableName, tempTask->id, tempReductionList->variableName);
					tempReductionList = tempReductionList->next;
				}		
			}	
			
			
			// Update switches
				                               
            WRITE("%s", "\n        // Update your switch && sync with main memory\n");
            WRITE("        __sw_task_%d[vTid] = __ON;\n", tempTask->id);
            WRITE("%s", "        __sw_tasksCounter--;\n");
			WRITE("%s", "\n        __sync_synchronize();\n");
			
			// Copy reduction result to the global reduction variable	
			
			 if(tempTask->taskType == TASK_REDUCTION)
			 {
				tempKernel = tempTask->kernels;
				if(tempKernel->id == 0)
				{
					WRITE("%s", "        if(tid == __MAIN_KERNEL){\n\n");
				}
				else
				{
					WRITE("        if(tid == __KERNEL_%d){\n\n", tempKernel->id);
				}
                
                // If the kernel has more chunks to execute...
                flag = 0;
                temp = tempKernel->id;
                while(tempKernel)
				{
                    if(tempKernel->id == temp)
                    {                                                
                        if(flag == 0){
                            WRITE("%s", "            if(");
                            flag = 1;
                        }
                        else{
                            WRITE("%s", " || ");
                        }
                        
                        WRITE("!__sw_task_%d[%d]", tempTask->id, tempKernel->vId);
                    }
					tempKernel = tempKernel->next;
				}
                
                if(flag == 1)
                {
                    WRITE("%s", "){\n");
                    WRITE("%s", "            __sync_synchronize();\n");
                    WRITE("%s", "            continue;\n");
                    WRITE("%s", "            }\n\n");
                }
                
				tempKernel = tempTask->kernels;
				WRITE("%s", "            while(");
				while(tempKernel)
				{						
					WRITE("!__sw_task_%d[%d]", tempTask->id, tempKernel->vId);
					if(tempKernel->next){
						WRITE("%s", " || ");
                    }
					tempKernel = tempKernel->next;
				}
				WRITE("%s", "){\n");
				WRITE("%s", "            __sync_synchronize();\n");
				WRITE("%s", "            }\n\n");
				
				
				
				tempReductionList = tempTask->reductionList;
				while(tempReductionList)
				{
					WRITE("               extern %s %s;\n", tempReductionList->variableType, tempReductionList->variableName);
					WRITE("               %s = %s %s", tempReductionList->variableName, tempReductionList->variableName, tempReductionList->reductionType);
					
					tempKernel = tempTask->kernels;
					while(tempKernel)
					{						
						WRITE(" __sw_%s_task_%d[%d]", tempReductionList->variableName, tempTask->id, tempKernel->vId);
						if(tempKernel->next){
							WRITE(" %s", tempReductionList->reductionType);
                        }
						tempKernel = tempKernel->next;
					}
					WRITE("%s", ";");
					if(tempReductionList->next){
						WRITE("%s", "\n");
                    }
					tempReductionList = tempReductionList->next;
				}
				
				WRITE("%s", "\n        }\n\n");
				
			}			
		}
		else
		{
			// If it is a section task
			
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask && tempTask->next)
					if(tempTask->id == currentTask)
						break;
					else
						tempTask = tempTask->next;
				
				if(tempTask && tempTask->id == currentTask)
				{		
					
					 if(tempTask->taskType == TASK_REDUCTION)
					 {
						 // Copy local reduction result to my global reduction variable
						
		                tempReductionList = tempTask->reductionList;
						while(tempReductionList)
						{
							WRITE("            __sw_%s_task_%d[vTid] = %s;\n", tempReductionList->variableName, tempTask->id, tempReductionList->variableName);
							tempReductionList = tempReductionList->next;
						}
					}
					
	
					// Update switches
					
		            WRITE("%s", "\n        // Update your switch && sync with main memory\n");
					WRITE("        __sw_task_%d[vTid] = __ON;\n", tempTask->id);
                    WRITE("%s", "        __sw_tasksCounter--;\n");			
					WRITE("%s", "\n        __sync_synchronize();\n\n");
					
					
					 // Copy reduction result to the global reduction variable	
					
					 if(tempTask->taskType == TASK_REDUCTION)
					 {
						tempKernel = tempTask->kernels;
						if(tempKernel->id == 0){
							WRITE("%s", "        if(tid == __MAIN_KERNEL){\n\n");
						}
						else{
							WRITE("        if(tid == __KERNEL_%d){\n\n", tempKernel->id);
						}
                        
                        // If the kernel has more chunks to execute...
                        flag = 0;
                        temp = tempKernel->id;
                        while(tempKernel)
                        {
                            if(tempKernel->id == temp)
                            {                                                
                                if(flag == 0){
                                    WRITE("%s", "            if(");
                                    flag = 1;
                                }
                                else{
                                    WRITE("%s", " || ");
                                }
                                
                                WRITE("!__sw_task_%d[%d]", tempTask->id, tempKernel->vId);
                            }
                            tempKernel = tempKernel->next;
                        }
                        
                        if(flag == 1)
                        {
                            WRITE("%s", "){\n");
                            WRITE("%s", "            __sync_synchronize();\n");
                            WRITE("%s", "            continue;\n");
                            WRITE("%s", "            }\n\n");
                        }
                        
						tempKernel = tempTask->kernels;
                        WRITE("%s", "            while(");
						while(tempKernel)
						{						
							WRITE("!__sw_task_%d[%d]", tempTask->id, tempKernel->vId);
							if(tempKernel->next){
								WRITE("%s", " || ");
                            }
							tempKernel = tempKernel->next;
						}
						WRITE("%s", "){\n");
						WRITE("%s", "            __sync_synchronize();\n");
						WRITE("%s", "            continue;\n");
						WRITE("%s", "            }\n\n");
						
						tempReductionList = tempTask->reductionList;
						while(tempReductionList)
						{
							WRITE("               extern %s %s;\n", tempReductionList->variableType, tempReductionList->variableName);
							WRITE("               %s = %s %s", tempReductionList->variableName, tempReductionList->variableName, tempReductionList->reductionType);
							
							tempKernel = tempTask->kernels;
							while(tempKernel)
							{						
								WRITE(" __sw_%s_task_%d_kernel_%d", tempReductionList->variableName, tempTask->id, tempKernel->id);
								if(tempKernel->next){
									WRITE(" %s", tempReductionList->reductionType);
                                }
								tempKernel = tempKernel->next;
							}
							WRITE("%s", ";");
							if(tempReductionList->next){
								WRITE("%s", "\n");
                            }
							tempReductionList = tempReductionList->next;
						}
						
						WRITE("%s", "\n        }\n\n");
						
					}	
					break;
				}
				tempSection = tempSection->next;
			}
		}
	}
}



/***************** Print source code of tasks in a Parallel Function in [ sw_threads.c ] file ********************/


void printInThreadsFile_TaskSourceCode(parallel_function **GraphFunc, int currentFunction, int currentTask){
	
	parallel_function	*tempFunction = *GraphFunc;
	section 			*tempSection;
	task 				*tempTask;
	dataList			*tempList;
	kernel				*tempKernel;
	kernel				*tempKernel2;
	kernel				*tempKernel3;
	producer			*tempProducer;
	int 				i = 0, temp_i = 0, cv_i = 0, flagK = 0;
	bool				flag = FALSE;
	bool				flag2 = FALSE;
	crossConsumer		*tempCrossConsumer;
    dataList			*tempReductionList;
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
	/* Print Application parallel functions source code */

	while(tempFunction && tempFunction->next)
		if(tempFunction->id == currentFunction)
			break;
		else
			tempFunction = tempFunction->next;

	if(tempFunction && tempFunction->id == currentFunction)
	{	
		// Find the current task in the SG
		
		tempTask = tempFunction->tasks;
		while(tempTask && tempTask->next)
			if(tempTask->id == currentTask)
				break;
			else
				tempTask = tempTask->next;
		
		// If it is a task else it is a section task
		if(tempTask && tempTask->id == currentTask)
		{
			
			WRITE("        /******************* TASK [ %d ] *******************/\n\n", tempTask->id);
			
			WRITE("%s", "        switch(tid){\n");
			tempKernel = tempTask->kernels;
            cv_i = 0;
            
			while(tempKernel)
			{				
                flagK = 0;
                tempKernel3 = tempTask->kernels;
                while(cv_i != 0 && tempKernel3 != tempKernel)
                {
                    if(tempKernel->id == tempKernel3->id)
                    {   
                        flagK = 1; 
                        break;
                    }
                    tempKernel3 = tempKernel3->next;
                }
                
                if(flagK == 1)
                {
                    tempKernel = tempKernel->next;
                    continue;
                }
              
                if(flagK != 1)
                {
                    if(tempKernel->id == 0)
                    {
                        WRITE("%s", "            case __MAIN_KERNEL: ");
                    }
                    else
                    {
                        WRITE("            case __KERNEL_%d: ", tempKernel->id);
                    }
                }
                
                tempKernel3 = tempKernel;
                while(tempKernel3)
                {
                    if(tempKernel3->id == tempKernel->id)
                    {   
                        WRITE("vTid = %d; ", tempKernel3->vId);
                        WRITE("%s", "if(ready = (");
                        WRITE("!__sw_task_%d[vTid]", tempTask->id);
                        
                        tempProducer = tempTask->tred_producers;
                        while(tempProducer)
                        {
                            tempCrossConsumer = tempTask->crossLoopConsumerOf;
                            
                            if(!tempCrossConsumer)
                            {
                                tempKernel2 = tempProducer->kernels;
                                while(tempKernel2)
                                {
                                    WRITE(" && __sw_task_%d[%d]", tempProducer->id, tempKernel2->vId);
                                    tempKernel2 = tempKernel2->next;
                                }
                            }
                            else
                            {
                                flag2 = FALSE;
                                while(tempCrossConsumer)
                                {
                                    if(tempProducer->id == tempCrossConsumer->id)
                                    {
                                        flag2 = TRUE;
                                        break;
                                    }
                                    tempCrossConsumer = tempCrossConsumer->next;	
                                }
                                if(flag2 ==  FALSE)
                                {
                                    tempKernel2 = tempProducer->kernels;
                                    while(tempKernel2)
                                    {
                                        WRITE(" && __sw_task_%d[%d]", tempProducer->id, tempKernel2->vId);
                                        tempKernel2 = tempKernel2->next;
                                    }
                                }
                            }
                            tempProducer = tempProducer->next;
                        }
                        
                        WRITE("%s", ")) break;\n                              ");
                    }
                    
                    tempKernel3 = tempKernel3->next;
                }
                
                WRITE("%s", "break;\n");
                    
				tempKernel = tempKernel->next;
                cv_i++;
			}
			WRITE("%s", "            default:  ready = FALSE; break;\n");
			WRITE("%s", "        }\n\n");
			WRITE("%s", "    if(ready){\n\n");
            
            // Check cross-loop switches on the consumer
			if(tempTask->crossLoopConsumerOf)
			{	
				WRITE("%s", "        // Check here if the cross-loop switches of my cross-loop producer are created, otherwise this task shouldn't start execution \n");
				
				tempCrossConsumer = tempTask->crossLoopConsumerOf;
				while(tempCrossConsumer)
				{
					WRITE("        if(!__sw_task_%d_chunk[vTid]) continue;\n", tempCrossConsumer->id);
					tempCrossConsumer = tempCrossConsumer->next;
				}
			}
			
            WRITE("%s", "        ready = FALSE;\n");
		}
		else
		{
			// If it is a section task
			
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask && tempTask->next)
					if(tempTask->id == currentTask)
						break;
					else
						tempTask = tempTask->next;
				
				if(tempTask && tempTask->id == currentTask)
				{
					WRITE("        /******************* TASK [ %d ] *******************/\n\n", tempTask->id);
					
					WRITE("%s", "        switch(tid){\n");
					tempKernel = tempTask->kernels;
                    cv_i = 0;
                    
					while(tempKernel)
					{
                        flagK = 0;
                        tempKernel3 = tempTask->kernels;
                        while(cv_i != 0 && tempKernel3 != tempKernel)
                        {
                            if(tempKernel->id == tempKernel3->id)
                            {   
                                flagK = 1; 
                                break;
                            }
                            tempKernel3 = tempKernel3->next;
                        }
                        
                        if(flagK == 1)
                        {
                            tempKernel = tempKernel->next;
                            continue;
                        }
                      
                        if(flagK != 1)
                        {
                            if(tempKernel->id == 0)
                            {
                                WRITE("%s", "            case __MAIN_KERNEL: ");
                            }
                            else
                            {
                                WRITE("            case __KERNEL_%d: ", tempKernel->id);
                            }
                        }
                        
                        tempKernel3 = tempKernel;
                        while(tempKernel3)
                        {
                            if(tempKernel3->id == tempKernel->id)
                            {
                                WRITE("vTid = %d; ", tempKernel3->vId);
                                WRITE("%s", "if(ready = (");
                                WRITE("!__sw_task_%d[vTid]", tempTask->id);
						
                                tempProducer = tempTask->tred_producers;
                                while(tempProducer)
                                {
                                    tempKernel2 = tempProducer->kernels;
                                    while(tempKernel2)
                                    {
                                        WRITE(" && __sw_task_%d[%d]", tempProducer->id, tempKernel2->vId);
                                        tempKernel2 = tempKernel2->next;
                                    }
                                    tempProducer = tempProducer->next;
                                }
                                WRITE("%s", ")) break;\n                              ");
                            }
                            tempKernel3 = tempKernel3->next;
                        }
                        WRITE("%s", "break;\n");
                        tempKernel = tempKernel->next;
                        cv_i++;
                    }
					
					WRITE("%s", "            default:  ready = FALSE; break;\n");
					WRITE("%s", "        }\n\n");
					WRITE("%s", "    if(ready){\n\n");
					WRITE("%s", "        ready = FALSE;\n");
					break;
				}
				
				tempSection = tempSection->next;
			}
		}
		
		
		// If task is a loop print the extra loop source code
		 
		if(tempTask->taskType == TASK_LOOP || tempTask->taskType == TASK_REDUCTION)
		{
			/* Write the __sw_loop_start */
			
			flag = FALSE;
			WRITE("%s", "\n");
			WRITE("%s", "            __sw_loop_start   = ");
			for(i = 0; i < SIZE; i++)
			{				
				if(stringFor[currentFor][i] == ';')
					break;
				
				if(flag)
					WRITE("%c", stringFor[currentFor][i]);
				
				if(stringFor[currentFor][i] == '=')
					flag = TRUE;
			}
			
			if(flag == TRUE)
				WRITE("%s", " + ");
			
			WRITE("vTid * __LOOP_%d_CHUNK;\n", tempTask->id);
			
			
			/* Write __sw_loop_end */
			
			flag = FALSE;
			WRITE("%s", "            __sw_loop_end     = ");
			for(i = i+1; i < SIZE; i++)
			{				
				if(stringFor[currentFor][i] == ';')
					break;
				
				if(flag)
					WRITE("%c", stringFor[currentFor][i]);
				
				if(stringFor[currentFor][i] == '=' || stringFor[currentFor][i] == '>' || stringFor[currentFor][i] == '<' || stringFor[currentFor][i] == '!')
				{	
					flag = TRUE;
					if(stringFor[currentFor][i+1] == '=')
						i++;
				}
			}
			
			WRITE("%s", ";\n");
            
            // Re-initialize Reduction variables
            if(tempTask->taskType == TASK_REDUCTION)
			{
				WRITE("%s", "            // Re-initialize Reduction variables\n");
				tempReductionList = tempTask->reductionList;
				while(tempReductionList)
				{
					WRITE("             %s = ",  tempReductionList->variableName);
					if(!strcmp(tempReductionList->reductionType, "*") || !strcmp(tempReductionList->reductionType, "&&"))
					{
						WRITE("%s", "1;\n");
					}
					else
					{
						WRITE("%s", "0;\n");
					}
					tempReductionList = tempReductionList->next;
				}
			}
			
			
			// ALLOCATE MEMORY FOR CROSS-LOOP SWITCHES
			 
			if(tempTask->crossLoopProducer == TRUE)
			 {
				WRITE("            __sw_loop_switches = ceil(__sw_loop_end / __LOOP_%d_COUNTER);\n\n", tempTask->id);
				WRITE("                if(__sw_task_%d_chunk[vTid])\n", tempTask->id);
				WRITE("                    free(__sw_task_%d_chunk[vTid]);\n\n", tempTask->id);
				WRITE("                __sw_task_%d_chunk[vTid] = (bool* )malloc(sizeof(bool) * __sw_loop_switches);\n", tempTask->id);
				WRITE("%s", "                for(__sw_i = 0; __sw_i < __sw_loop_switches; __sw_i++)\n");
				WRITE("                    __sw_task_%d_chunk[vTid][__sw_i] = __OFF;\n", tempTask->id);
			 }
			
			WRITE("\n            for (__sw_i = __sw_loop_start; __sw_i < __sw_loop_end; __sw_i += __LOOP_%d_COUNTER){\n\n", tempTask->id);
			WRITE("                __sw_loop_chunk = __sw_i + (((__sw_loop_end - __sw_i) >= __LOOP_%d_CHUNK) ? __LOOP_%d_CHUNK : (__sw_loop_end - __sw_i));\n\n", tempTask->id, tempTask->id);
			
			
			// Check cross-loop switches on the consumer
			
			if(tempTask->crossLoopConsumerOf)
			{	
				WRITE("%s", "            // Check here the cross-loop switches of my cross-loop producer\n");
				
				tempCrossConsumer = tempTask->crossLoopConsumerOf;
				while(tempCrossConsumer)
				{
					//WRITE("            __sw_loop_kernel = (__sw_i %% __LOOP_%d_COUNTER) / __LOOP_%d_CHUNK;\n", tempCrossConsumer->id, tempCrossConsumer->id);
					WRITE("            __sw_loop_switch = __sw_i / __LOOP_%d_COUNTER;\n", tempCrossConsumer->id);
					WRITE("%s", "            while(1){\n");
					WRITE("%s", "                __sync_synchronize();\n");
					//WRITE("                if(!__sw_task_%d_chunk[__sw_loop_kernel]) continue;\n", tempCrossConsumer->id);
					WRITE("                if(__sw_task_%d_chunk[vTid][__sw_loop_switch] == __ON) break;\n", tempCrossConsumer->id);
					WRITE("%s", "            }\n\n");
					
					tempCrossConsumer = tempCrossConsumer->next;
				}
			}
		}		
	}
}



/***************** TAO: Print source code of tasks in a Parallel Function in [ sw_threads.c ] file ********************/


void printInThreadsFile_TaskSourceCode_TAO(parallel_function **GraphFunc, int currentFunction, int currentTask){
	
	parallel_function	*tempFunction = *GraphFunc;
	section 			*tempSection;
	task 				*tempTask;
	dataList			*tempList;
	kernel				*tempKernel;
	kernel				*tempKernel2;
	kernel				*tempKernel3;
	producer			*tempProducer;
	int 				i = 0, temp_i = 0, cv_i = 0, flagK = 0;
	bool				flag = FALSE;
	bool				flag2 = FALSE;
	crossConsumer		*tempCrossConsumer;
    dataList			*tempReductionList;
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
	/* Print Application parallel functions source code */

	while(tempFunction && tempFunction->next)
		if(tempFunction->id == currentFunction)
			break;
		else
			tempFunction = tempFunction->next;

	if(tempFunction && tempFunction->id == currentFunction)
	{	
		// Find the current task in the SG
		
		tempTask = tempFunction->tasks;
		while(tempTask && tempTask->next)
			if(tempTask->id == currentTask)
				break;
			else
				tempTask = tempTask->next;
		
		// If it is a task else it is a section task
		if(tempTask && tempTask->id == currentTask)
		{
			WRITE("        /******************* TASK [ %d ] *******************/\n\n", tempTask->id);
		}
		else
		{
			// If it is a section task
			
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				tempTask = tempSection->tasks;
				while(tempTask && tempTask->next)
					if(tempTask->id == currentTask)
						break;
					else
						tempTask = tempTask->next;
				
				if(tempTask && tempTask->id == currentTask)
				{
					WRITE("        /******************* TASK [ %d ] *******************/\n\n", tempTask->id);
					break;
				}
				
				tempSection = tempSection->next;
			}
		}
	}
}





/***************** Print source code of tasks in a Parallel Function in [ sw_threads.c ] file ********************/


void printInThreadsFile_ForStatement(){
	
	int	 i = 0;
	bool section1_flag = FALSE;
	bool section2_flag = FALSE;
	
	for(i = 0; i < SIZE; i++)
	{						
		if(stringFor[currentFor-1][i] == ')')
			break;
		
		if(stringFor[currentFor-1][i] == '=' && section1_flag == FALSE)
		{
			WRITE("%c ", stringFor[currentFor-1][i]);
			WRITE("%s", "__sw_i; ");
			while(stringFor[currentFor-1][++i] != ';');
			section1_flag = TRUE;
		}
		else if((stringFor[currentFor-1][i] == '=' || stringFor[currentFor-1][i] == '>' || stringFor[currentFor-1][i] == '<' || stringFor[currentFor-1][i] == '!')  && section2_flag == FALSE)
		{	
			WRITE("%c", stringFor[currentFor-1][i]);
			if(stringFor[currentFor-1][i+1] == '=')
			{
				i++;
				WRITE("%c", stringFor[currentFor-1][i]);
			}
			WRITE("%s", " __sw_loop_chunk; ");
			while(stringFor[currentFor-1][++i] != ';');
			section2_flag = TRUE;
		}
		else
		{
			WRITE("%c", stringFor[currentFor-1][i]);
		}
	}
	WRITE("%c", stringFor[currentFor-1][i]);	
}




/***************** Print Jobs Threads Function in [ sw_threads.c ] file ********************/

void printInThreadsFile_JobsThreadsFunction_SWITCHES(SG** Graph){
	
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
    
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
		/* Print Job Thread function */
	
	tempGraph = *Graph;
	WRITE("%s", "\n/*** Jobs Thread Function ***/\n\n");
	
	WRITE("%s", "void *thread_jobs(void *arg)\n");
	WRITE("%s", "{\n");
	WRITE("%s", "    long tid;\n");
	WRITE("%s", "    long myFunction = 0;\n");
	
	if(affinityPolicy != AFFINITY_NONE)
	{
		WRITE("%s", "    // Thread Affinity variables\n");
		WRITE("%s", "    int __affinity_out = 0;\n");
		WRITE("%s", "    cpu_set_t cpuset;\n");
	}
    
	WRITE("%s", "    __arguments *arguments;\n");
	WRITE("%s", "    arguments = (__arguments*) arg;\n\n");
	WRITE("%s", "    tid = arguments->id;\n");
	WRITE("%s", "    myFunction = arguments->function_id;\n\n");
	
	if(affinityPolicy != AFFINITY_NONE)
	{
        WRITE("%s", "    // Activate Thread Affinity\n");
        WRITE("%s", "    CPU_ZERO(&cpuset);\n");
        
        if(affinityPolicy == AFFINITY_COMPACT)
        {
            WRITE("%s", "    CPU_SET(tid+1, &cpuset);\n");              // +1 is to avoid the OS thread/core
        }
        else if(affinityPolicy == AFFINITY_SCATTER)
        {
            WRITE("    CPU_SET(((tid %% %d)*%d + %d + (tid / %d)), &cpuset);\n", maxCores, hThreads, OSthread, maxCores);
        }
        else if(affinityPolicy == AFFINITY_HYBRID)
        {
            
            if(kernels < maxCores)
            {
                WRITE("    CPU_SET(((tid %% %d)*%d + %d + (tid / %d)), &cpuset);\n", maxCores, hThreads, OSthread, maxCores);
            }            
            else if((kernels % maxCores) == 0)
            {
                WRITE("    CPU_SET((tid %% (__KERNELS / %d)) + ((tid / (__KERNELS / %d)) * %d) + %d, &cpuset);\n", maxCores, maxCores, hThreads, OSthread);
            }
            else
            {
                ERROR_COMMANDS("Threads [ %d ] must be less than or divisible by max system cores [ %d ]!", kernels, maxCores)
                exit(-1);
            }
        }
        else if(affinityPolicy == AFFINITY_RANDOM)
        {
            WRITE("%s", "    time_t t;\n");
            WRITE("%s", "    srand((unsigned) time(&t));\n");
            WRITE("    CPU_SET((rand() %% %d)+%d, &cpuset);\n", maxCores*hThreads, OSthread);
        }
        
        
		WRITE("%s", "    __affinity_out = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);\n\n");	
	}
	
	WRITE("%s", "    if(tid == __MAIN_KERNEL){\n");
	WRITE("%s", "        currentFunction = myFunction;\n");
	WRITE("%s", "        __sync_synchronize();\n");
	WRITE("%s", "     }\n");
	WRITE("%s", "\n\n");
	
	WRITE("%s", "    do{\n\n");
	WRITE("%s", "        pthread_barrier_wait(&barrier[0]);\n");
	WRITE("%s", "        myFunction = currentFunction;\n");
	WRITE("%s", "        __sync_synchronize();\n\n");
	WRITE("%s", "        switch(myFunction){\n\n");
	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
			WRITE("            case __FUNCTION_%d:\n", tempFunction->id);
			WRITE("                parallel_function_%d((void *)tid);\n", tempFunction->id);
			WRITE("                pthread_barrier_wait(&barrier[__FUNCTION_%d]);\n", tempFunction->id);
			WRITE("                __sw_resetSWitches_%d(tid);\n", tempFunction->id);
			WRITE("%s", "                break;\n\n");
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;
	}
	WRITE("%s", "        }\n\n");
	WRITE("%s", "        if(tid == __MAIN_KERNEL)\n");
	WRITE("%s", "            break;\n\n");
	WRITE("%s", "    }while(myFunction != -1);\n\n");
	WRITE("%s", "\n\n");
	
	WRITE("%s", "    if(tid != __MAIN_KERNEL)\n");
	WRITE("%s", "        pthread_exit(NULL);\n");
	WRITE("%s", "\n}\n\n");
	
	
	
}




/***************** Print TAO + TAOSW Jobs Threads Function in [ sw_threads.c ] file ********************/


void printInThreadsFile_JobsThreadsFunction_TAO(SG** Graph){
	
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
    
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
		/* Print Job Thread function */
	
	tempGraph = *Graph;
	WRITE("%s", "\n/*** Jobs Thread Function ***/\n\n");
	
	WRITE("%s", "void *thread_jobs(void *arg)\n");
	WRITE("%s", "{\n");
	WRITE("%s", "    long tid;\n");
	WRITE("%s", "    long myFunction = 0;\n");
	
	    
	WRITE("%s", "    __arguments *arguments;\n");
	WRITE("%s", "    arguments = (__arguments*) arg;\n\n");
	WRITE("%s", "    tid = arguments->id;\n");
	WRITE("%s", "    myFunction = arguments->function_id;\n\n");
	
	WRITE("%s", "\n");
	
	WRITE("%s", "    switch(myFunction){\n\n");
	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
			WRITE("            case __FUNCTION_%d:\n", tempFunction->id);
			WRITE("                parallel_function_%d((void *)tid);\n", tempFunction->id);
			WRITE("                // pthread_barrier_wait(&barrier[__FUNCTION_%d]);\n", tempFunction->id);
			WRITE("                // __sw_resetSWitches_%d(tid);\n", tempFunction->id);
			WRITE("%s", "                break;\n\n");
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;
	}
	WRITE("%s", "        }\n\n");	
	WRITE("%s", "\n}\n\n");
	
}



/***************** Print Reset Switches functions in [ sw_threads.c ] file ********************/


void printInThreadsFile_ResetSwitchesFunctions(SG** Graph){
	
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
	kernel				*tempKernel;
	int 				i = 0;
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
	
	/* Print Reset SWitches function */

	WRITE("%s", "\n\t/*** Reset Switches Functions ***/\n\n");
	tempGraph = *Graph;
	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{	
			WRITE("/* Reset the switches of Parallel Function %d */\n\n", tempFunction->id);
			WRITE("void __sw_resetSWitches_%d(int tid)\n", tempFunction->id);
			WRITE("%s", "{\n\n");
			
			WRITE("%s", "    switch(tid)\n");
			WRITE("%s", "    {\n");
			
			for(i = 0; i < kernels; i++)
			{
				if(i == 0)
				{
					WRITE("%s", "         case __MAIN_KERNEL:\n");
				}
				else
				{
					WRITE("         case __KERNEL_%d:\n", i);
				}
			
				tempTask = tempFunction->tasks;
				while(tempTask)
				{
					tempKernel = tempTask->kernels;
					while(tempKernel)				
						{
							if(tempKernel->id == i)
                            {
								WRITE("            __sw_task_%d[%d] = __OFF;\n", tempTask->id, tempKernel->vId);
                            }
							tempKernel = tempKernel->next;
						}
					tempTask = tempTask->next;
				}
			
				tempSection = tempFunction->sections;
				if(tempSection)
				{
					while(tempSection)
					{
						/* Print tasks of a section */
						tempTask = tempSection->tasks;
						while(tempTask)
						{
							tempKernel = tempTask->kernels;
							while(tempKernel)				
								{
									if(tempKernel->id == i)
										WRITE("            __sw_task_%d[%d] = __OFF;\n", tempTask->id, tempKernel->vId);
									tempKernel = tempKernel->next;
								}
							tempTask = tempTask->next;
						}
						tempSection = tempSection->next;
					}
				}
				WRITE("%s", "            break;\n\n");
			}
			WRITE("%s", "    }\n\n");
			WRITE("%s", "}\n\n");
			tempFunction = tempFunction->next; 
		}
		tempGraph = tempGraph->next;
	}
	
}




/***************** Print Global reduction variables of application in [ sw_threads.c ] file [ Reduction ] ********************/


void printInThreadsFile_ReductionVariables(parallel_function *GraphFunc){
	
	
	parallel_function 	*tempFunction = GraphFunc;
	section 			*tempSection;
	task 				*tempTask;
	kernel				*tempKernel;
	dataList			*tempReductionList;
	int					kernelCv = 0;
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
	
	/* Print Reduction Global Variables if exist */

	WRITE("%s", "\n/*** Reduction Global Variables ***/\n\n");
	

	// Check Normal Tasks
	
	tempTask = tempFunction->tasks;
	while(tempTask)
	{
		if(tempTask->taskType == TASK_REDUCTION)
		{
			tempReductionList = tempTask->reductionList;
			while(tempReductionList)
			{
				kernelCv = 0;
				tempKernel = tempTask->kernels;
				while(tempKernel)
				{
					kernelCv++;
					tempKernel = tempKernel->next;
				}	
				
				WRITE("%s __sw_%s_task_%d[%d]          = ", tempReductionList->variableType, tempReductionList->variableName, tempTask->id, kernelCv);
					
				if(!strcmp(tempReductionList->reductionType, "*") || !strcmp(tempReductionList->reductionType, "&&"))
				{
					WRITE("%s", "{1};\n");
				}
				else
				{
					WRITE("%s", "{0};\n");
				}
					
				tempReductionList = tempReductionList->next;
			}
		}
		tempTask = tempTask->next;
	}
	
	
	// Check Section Tasks
	
	tempSection = tempFunction->sections;
	if(tempSection)
	{
		while(tempSection)
		{
			/* Print tasks of a section */
			tempTask = tempSection->tasks;
			while(tempTask)
			{
				if(tempTask->taskType == TASK_REDUCTION)
				{
					tempReductionList = tempTask->reductionList;
					while(tempReductionList)
					{
						tempKernel = tempTask->kernels;
						while(tempKernel)
						{
							WRITE("%s __sw_%s_task_%d_kernel_%d          = ", tempReductionList->variableType, tempReductionList->variableName, tempTask->id, tempKernel->id);
							
							if(!strcmp(tempReductionList->reductionType, "*") || !strcmp(tempReductionList->reductionType, "&&"))
							{
								WRITE("%s", "1;\n");
							}
							else
							{
								WRITE("%s", "0;\n");
							}
							
							tempKernel = tempKernel->next;
						}		
						tempReductionList = tempReductionList->next;
					}
				}
				tempTask = tempTask->next;
			}
			tempSection = tempSection->next;
		}
	}
}



/***************** Print SWitches in [ sw_threads.c ] file ********************/


void printInThreadsFile_SwitchesDeclaration(SG** Graph){
	
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
	kernel				*tempKernel;
	producer			*tempConsumers;
	dataList			*tempDataList;
	arrayIndex			*tempIndexes;
	crossConsumer		*tempCrossConsumer;
	
	
	task 				*tempConsumerTask;
	dataList			*tempConsumerDataList;
	arrayIndex			*tempConsumerIndexes;
	bool				crossLoopFlag = FALSE;
	int					kernelCv = 0;
	
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
	/* Print all Switches */
	
	WRITE("%s", "\n/*** Declare all the SWITCHES here ***/\n\n");
	
	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{	
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				/* Print tasks of a parallel function */
				kernelCv = 0;
				tempKernel = tempTask->kernels;
				while(tempKernel)
				{
					kernelCv++;
					tempKernel = tempKernel->next;
				}
				
				WRITE("bool __sw_task_%d[%d]    = {__OFF};\n", tempTask->id, kernelCv);
				

				if(tempTask->schedulingPolicy == LOOP_SCHED_CROSS)
				{
					// Check if this task has any task loop consumers
				
					tempConsumers = tempTask->tred_consumers;
					while(tempConsumers)
					{	 
						tempConsumerTask = tempFunction->tasks;
						while(tempConsumerTask)
							if(tempConsumers->id == tempConsumerTask->id)
								break;
							else
								tempConsumerTask = tempConsumerTask->next;
											 
						
						if(tempConsumerTask->schedulingPolicy == LOOP_SCHED_CROSS)
						{
							// Check Tasks Out List
							
							tempDataList = tempTask->dependOutList;
							while(tempDataList)
							{
								// Check Consumers Depend In List
								
								tempConsumerDataList = tempConsumerTask->dependInList;
								while(tempConsumerDataList)
								{
									if(!strcmp(tempDataList->variableName, tempConsumerDataList->variableName))
									{
										tempIndexes = tempDataList->indexes;
										tempConsumerIndexes = tempConsumerDataList->indexes;
										while(tempIndexes && tempConsumerIndexes)
										{
											if((tempIndexes->start.localInt != -1 && tempIndexes->end.localInt != -1)    || 
												(tempIndexes->start.localInt != -1 && tempIndexes->end.localStr != NULL)  || 
													(tempIndexes->start.localStr != NULL && tempIndexes->end.localInt != -1)  || 
														(tempIndexes->start.localStr != NULL && tempIndexes->end.localStr != NULL))
											   {
												   if((tempConsumerIndexes->start.localInt != -1 && tempConsumerIndexes->end.localInt != -1)    || 
														(tempConsumerIndexes->start.localInt != -1 && tempConsumerIndexes->end.localStr != NULL)  || 
															(tempConsumerIndexes->start.localStr != NULL && tempConsumerIndexes->end.localInt != -1)  || 
																(tempConsumerIndexes->start.localStr != NULL && tempConsumerIndexes->end.localStr != NULL))
													{
														crossLoopFlag = TRUE;
														//break;
													}
											   }			
											   									
											tempIndexes = tempIndexes->next;
											tempConsumerIndexes = tempConsumerIndexes->next;
										}
										//if(crossLoopFlag)
										//	break;
									}
									tempConsumerDataList = tempConsumerDataList->next;
								}
								
								//if(crossLoopFlag)
								//	break;
								
								//Check Consumers Depend InOut List
								
								tempConsumerDataList = tempConsumerTask->dependInOutList;
								while(tempConsumerDataList)
								{
									if(!strcmp(tempDataList->variableName, tempConsumerDataList->variableName))
									{
										tempIndexes = tempDataList->indexes;
										tempConsumerIndexes = tempConsumerDataList->indexes;
										while(tempIndexes && tempConsumerIndexes)
										{
											if((tempIndexes->start.localInt != -1 && tempIndexes->end.localInt != -1)    || 
												(tempIndexes->start.localInt != -1 && tempIndexes->end.localStr != NULL)  || 
													(tempIndexes->start.localStr != NULL && tempIndexes->end.localInt != -1)  || 
														(tempIndexes->start.localStr != NULL && tempIndexes->end.localStr != NULL))
											   {
												   if((tempConsumerIndexes->start.localInt != -1 && tempConsumerIndexes->end.localInt != -1)    || 
														(tempConsumerIndexes->start.localInt != -1 && tempConsumerIndexes->end.localStr != NULL)  || 
															(tempConsumerIndexes->start.localStr != NULL && tempConsumerIndexes->end.localInt != -1)  || 
																(tempConsumerIndexes->start.localStr != NULL && tempConsumerIndexes->end.localStr != NULL))
													{
														crossLoopFlag = TRUE;
														//break;
													}
											   }			
											   									
											tempIndexes = tempIndexes->next;
											tempConsumerIndexes = tempConsumerIndexes->next;
										}
										//if(crossLoopFlag)
										//	break;
									}
									tempConsumerDataList = tempConsumerDataList->next;
								}
								
								//if(crossLoopFlag)
								//	break;
								
								tempDataList = tempDataList->next;
							}
							
							// Check Tasks InOut List
							 
							tempDataList = tempTask->dependInOutList;
							while(tempDataList)
							{
								// Check Consumers Depend In List
								
								tempConsumerDataList = tempConsumerTask->dependInList;
								while(tempConsumerDataList)
								{
									if(!strcmp(tempDataList->variableName, tempConsumerDataList->variableName))
									{
										tempIndexes = tempDataList->indexes;
										tempConsumerIndexes = tempConsumerDataList->indexes;
										while(tempIndexes && tempConsumerIndexes)
										{
											if((tempIndexes->start.localInt != -1 && tempIndexes->end.localInt != -1)    || 
												(tempIndexes->start.localInt != -1 && tempIndexes->end.localStr != NULL)  || 
													(tempIndexes->start.localStr != NULL && tempIndexes->end.localInt != -1)  || 
														(tempIndexes->start.localStr != NULL && tempIndexes->end.localStr != NULL))
											   {
												   if((tempConsumerIndexes->start.localInt != -1 && tempConsumerIndexes->end.localInt != -1)    || 
														(tempConsumerIndexes->start.localInt != -1 && tempConsumerIndexes->end.localStr != NULL)  || 
															(tempConsumerIndexes->start.localStr != NULL && tempConsumerIndexes->end.localInt != -1)  || 
																(tempConsumerIndexes->start.localStr != NULL && tempConsumerIndexes->end.localStr != NULL))
													{
														crossLoopFlag = TRUE;
														//break;
													}
											   }			
											   									
											tempIndexes = tempIndexes->next;
											tempConsumerIndexes = tempConsumerIndexes->next;
										}
										//if(crossLoopFlag)
										//	break;
									}
									tempConsumerDataList = tempConsumerDataList->next;
								}
								
								//if(crossLoopFlag)
								//	break;
								
								//Check Consumers Depend InOut List
								
								tempConsumerDataList = tempConsumerTask->dependInOutList;
								while(tempConsumerDataList)
								{
									if(!strcmp(tempDataList->variableName, tempConsumerDataList->variableName))
									{
										tempIndexes = tempDataList->indexes;
										tempConsumerIndexes = tempConsumerDataList->indexes;
										while(tempIndexes && tempConsumerIndexes)
										{
											if((tempIndexes->start.localInt != -1 && tempIndexes->end.localInt != -1)    || 
												(tempIndexes->start.localInt != -1 && tempIndexes->end.localStr != NULL)  || 
													(tempIndexes->start.localStr != NULL && tempIndexes->end.localInt != -1)  || 
														(tempIndexes->start.localStr != NULL && tempIndexes->end.localStr != NULL))
											   {
												   if((tempConsumerIndexes->start.localInt != -1 && tempConsumerIndexes->end.localInt != -1)    || 
														(tempConsumerIndexes->start.localInt != -1 && tempConsumerIndexes->end.localStr != NULL)  || 
															(tempConsumerIndexes->start.localStr != NULL && tempConsumerIndexes->end.localInt != -1)  || 
																(tempConsumerIndexes->start.localStr != NULL && tempConsumerIndexes->end.localStr != NULL))
													{
														crossLoopFlag = TRUE;
														//break;
													}
											   }			
											   									
											tempIndexes = tempIndexes->next;
											tempConsumerIndexes = tempConsumerIndexes->next;
										}
										//if(crossLoopFlag)
										//	break;
									}
									tempConsumerDataList = tempConsumerDataList->next;
								}
								
								//if(crossLoopFlag)
								//	break;
								
								tempDataList = tempDataList->next;
							}
							if(crossLoopFlag)
							{
								tempTask->crossLoopProducer = TRUE;
								
								if(!tempConsumerTask->crossLoopConsumerOf)
								{
									tempConsumerTask->crossLoopConsumerOf     		= (crossConsumer *)malloc(sizeof(crossConsumer));
									tempConsumerTask->crossLoopConsumerOf->id 		= tempTask->id;;		
									tempConsumerTask->crossLoopConsumerOf->next 	= NULL;
								}
								else
								{
									tempCrossConsumer = tempConsumerTask->crossLoopConsumerOf;
									
									while(tempCrossConsumer && tempCrossConsumer->next)
										tempCrossConsumer = tempCrossConsumer->next;
									
									tempCrossConsumer->next 		= (crossConsumer *)malloc(sizeof(crossConsumer));
									tempCrossConsumer->next->id		= tempTask->id;
									tempCrossConsumer->next->next 	= NULL;
								}
								
								
								//tempConsumerTask->crossLoopConsumerOf = tempTask->id;
								WRITE("bool* __sw_task_%d_chunk[%d];\n", tempTask->id, kernelCv);
								crossLoopFlag = FALSE;
								//break;		// REMOVED to check for all loops that have cross-loop-iteration dependencies
							}
						}
						 tempConsumers = tempConsumers->next;
					}
				}
				tempTask = tempTask->next;
			}
			
			
			tempSection = tempFunction->sections;
			if(tempSection)
			{
				while(tempSection)
				{
					/* Print tasks of a section */
					tempTask = tempSection->tasks;
					while(tempTask)
					{
						tempKernel = tempTask->kernels;
						while(tempKernel)
						{
							WRITE("bool __sw_task_%d[%d]    = {__OFF};\n", tempTask->id, kernelCv);
							tempKernel = tempKernel->next;
						}
						tempTask = tempTask->next;
					}
					tempSection = tempSection->next;
				}
			}	
			tempFunction = tempFunction->next; 
		}
		tempGraph = tempGraph->next;
	}
}




/***************** Declare the Task Counter of each Thread per Function ********************/


void printInThreadsFile_taskCounters(SG** Graph){
    
    int 				i = 0, j = 0;
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	task 				*tempTask;
    section             *tempSection;
    kernel              *tempKernel;
       
    
    /* Declare the Task Counter of each Thread per Function*/
	
	WRITE("%s", "\n\n/*** Declare Task Counter of each Thread & per Function ***/\n\n");
	tempGraph = *Graph;
	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{	
			WRITE("long __sw_taskCounter_Function_%d[__KERNELS]  = {", tempFunction->id);
            for(i = 0; i < kernels; i++)
			{
                j = 0;
				tempTask = tempFunction->tasks;
				while(tempTask)
				{
					tempKernel = tempTask->kernels;
					while(tempKernel)				
                    {
                        if(tempKernel->id == i)
                        {
                            j++;
                        }
                        tempKernel = tempKernel->next;
                    }
					tempTask = tempTask->next;
				}
			
				tempSection = tempFunction->sections;
				if(tempSection)
				{
					while(tempSection)
					{
						/* Print tasks of a section */
						tempTask = tempSection->tasks;
						while(tempTask)
						{
							tempKernel = tempTask->kernels;
							while(tempKernel)				
                            {
                                if(tempKernel->id == i)
                                {
                                    j++;
                                }
                                tempKernel = tempKernel->next;
                            }
							tempTask = tempTask->next;
						}
						tempSection = tempSection->next;
					}
				}
                WRITE(" %d", j);
                if(i+1 < kernels)
                {
                    WRITE("%s", ",");
                }
			}
            WRITE("%s", " };\n");
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;
	}
	WRITE("%s", "\n\n");
}



/***************** NOT USED -- Break Function Loop after tasks of the function are finished ********************/


void printInThreadsFile_BreakLoopWhenFinished(parallel_function **GraphFunc, int currentFunction){
	
	parallel_function	*tempFunction = *GraphFunc;
	section 			*tempSection;
	task 				*tempTask;
	kernel				*tempKernel;
	int 				tempKernels = 0;
	
	//Redirect output
	__OUTP_IS_NOW_THREADS_FILE
	
	
	// Find current function
	
	while(tempFunction && tempFunction->next)
		if(tempFunction->id == currentFunction)
			break;
		else
			tempFunction = tempFunction->next;
		
    if(tempFunction && tempFunction->id == currentFunction)
	{
        WRITE("%s", "\n        // Break the parallel function infinite loop\n");
        WRITE("%s", "        if(!__sw_tasksCounter) break;\n");
    }
}



/*********************** Print in [ sw_tao.cxx ] file ****************************/


void printInTaoFile(SG** Graph){
	
	int 				i = 0, j = 0;
	SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
    
	
	//Redirect output
	__OUTP_IS_NOW_SW_TAO_FILE
    
    
   
    WRITE("%s", "#include \"tao.h\"\n");
    WRITE("%s", "extern \"C\" {\n");
    WRITE("%s", "#include \"sw.h\"\n");
    WRITE("%s", "}\n");
    WRITE("%s", "using namespace std;\n\n");
    
	
	
    /* Print Parallel Functions - Classes */
    
    tempGraph = *Graph;
	while(tempGraph)
	{
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{	
             WRITE("/** Parallel Function %d **/ \n", tempFunction->id);
             
             WRITE("class __FUNC_%d : public AssemblyTask\n", tempFunction->id);
             WRITE("%s\n", "{");
             WRITE("\t%s\n", "public:");
             WRITE("\t\t__FUNC_%d(int tao, int taos, int width) : AssemblyTask(width)\n", tempFunction->id);
             WRITE("\t\t%s\n", "{");
             WRITE("\t\t\t%s\n", "#define PSLACK 8");
             WRITE("\t\t\t%s\n", "noTaos = taos;");
             WRITE("\t\t\t%s\n", "taoID = tao;");
             WRITE("\t\t%s\n\n", "}");
             
             
             WRITE("\t\t%s\n\n", "int cleanup(){}");
             
             
             WRITE("\t\t%s\n", "// this assembly can work totally asynchronously");
             WRITE("\t\t%s\n", "int execute(int threadid)");
             WRITE("\t\t%s\n", "{");
             WRITE("\t\t\t%s\n", "int tid = threadid - leader;");
             WRITE("\t\t\t%s\n", "//int tid = taoID*width + threadid - leader;");
             WRITE("\t\t\t//fprintf(stderr, \"FUNC_%d: threadid: %%d - leader: %%d - taoID: %%d - tid:%%d -- IN\\n\", threadid, leader, taoID, tid);\n", tempFunction->id);
             WRITE("\t\t\t%s\n", "__arguments *arguments;");
             WRITE("\t\t\t%s\n", "arguments = (__arguments*)malloc(sizeof(__arguments));");
             WRITE("\t\t\t%s\n", "arguments->id = tid;");
             WRITE("\t\t\targuments->function_id = %d;\n", tempFunction->id);
             WRITE("\t\t\t%s\n", "thread_jobs((void *)arguments);");
             WRITE("\t\t\t//fprintf(stderr, \"FUNC_%d: threadid: %%d - leader: %%d - taoID: %%d - tid:%%d -- OUT\\n\", threadid, leader, taoID, tid);\n", tempFunction->id);
             WRITE("\t\t%s\n", "}");
             
             
             WRITE("\t\t%s\n", "int noTaos;");
             WRITE("\t\t%s\n", "int taoID;");
             WRITE("%s\n", "};");
             WRITE("%s", "\n\n");
            
			 tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;
	}
    
    
}




