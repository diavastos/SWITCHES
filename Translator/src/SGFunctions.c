/**************************************************/
/*                                                */
/*  File:        SGFunctions.c                    */
/*  Description: - Add data to SG lists           */
/*               - Print SG lists                 */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/


#include "definitions.h"



extern int 	line;
extern int 	pass;
extern bool firstPass;
extern int 	targetSystem;
extern int  currentFile;
extern bool transactions;
extern char **inputFiles;
extern int 	totalInputFiles;
extern char variableType[SIZE];
extern FILE *outp;
extern FILE *inp, *outp_sw_main, *outp_sw_h, *outp_sw_threadpool, *outp_sw_threads, *outp_sw_tao_h;



/****************** Print SG to Terminal for Debugginh ******************/


void printSG(SG** Graph, int printSGFlag){
	
	SG 				  	*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
	kernel 				*tempKernel;
	kernel 				*tempKernel2;
	producer 			*tempProducer;
	producer 			*tempConsumer;
	dataList 			*tempList;
	arrayIndex			*tempIndexes;
	crossConsumer		*tempCrossConsumer;
	
	FILE *SGlogFile;
	
	if(printSGFlag == __SCREEN)
		SGlogFile = stderr;
	else if(printSGFlag == __FILE)
		SGlogFile = fopen("SGlogFile.log", "w");
	else
		return;
		
	if(printSGFlag == __FILE && !SGlogFile){
	   ERROR_COMMANDS("File [ %s ] not opened!", "SGlogFile.log")
	   exit(-1);
	}
	
	
	
	while(tempGraph)
	{
		// Print Graph
		fprintf(SGlogFile, "Graph_%d:\n", tempGraph->id);
		
		
		// Print [threadprivate] list of the graph
		tempList = tempGraph->threadPrivateList;
		if(tempList)
		{
			fprintf(SGlogFile, "\t[*] Thread Private List: \n");
			fprintf(SGlogFile, "\t\tName \t\tType \t\tIndexes \n");
			while(tempList)
			{
				fprintf(SGlogFile, "\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
				
				tempIndexes = tempList->indexes;
				if(!tempIndexes)
					fprintf(SGlogFile, "(none)");
					
				while(tempIndexes)
				{
					if(tempIndexes->start.localInt != -1)
						fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
					else if(tempIndexes->start.localStr)
						fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
					
					if(tempIndexes->end.localInt != -1)
						fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
					else if(tempIndexes->end.localStr)
						fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
					else
						fprintf(SGlogFile, "]");
						
					tempIndexes = tempIndexes->next;
				}
				fprintf(SGlogFile, "\n");
				
				tempList = tempList->next;
			}
			fprintf(SGlogFile, "\n");
		}
		
		
		// Print parallel functions of the graph
		fprintf(SGlogFile, "\t[*] Parallel Functions: \n");
		
		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
	
			fprintf(SGlogFile, "\t    [-] Parallel Function %d: [kernels: %d (", tempFunction->id, tempFunction->number_of_kernels);
			tempKernel = tempFunction->kernels;
			if(!tempKernel)
			{
				fprintf(SGlogFile, "none");
			}
			else
			{
				while(tempKernel)
				{
					if(tempKernel->next)
						fprintf(SGlogFile, "%d ", tempKernel->id);
					else
						fprintf(SGlogFile, "%d", tempKernel->id);
					tempKernel = tempKernel->next;
				}
				fprintf(SGlogFile, ") V-Kernels: (");
				
				tempKernel = tempFunction->kernels;
				while(tempKernel)
				{
					if(tempKernel->next)
						fprintf(SGlogFile, "%d ", tempKernel->vId);
					else
						fprintf(SGlogFile, "%d", tempKernel->vId);
					tempKernel = tempKernel->next;
				}
				
			}
			
			fprintf(SGlogFile, "), Default: ");
			if(tempFunction->defaultVarsState == DEFAULT_NONE)
				fprintf(SGlogFile, "%s]\n", "none");
			else if(tempFunction->defaultVarsState == DEFAULT_SHARED)
				fprintf(SGlogFile, "%s]\n", "shared");
                
                
            
            /* Print Producers/Consumers of a Parallel Function */
            
            fprintf(SGlogFile, "\t\t   [+] Producers: ( ");
            tempProducer = tempFunction->producers;
            while(tempProducer)
            {
                fprintf(SGlogFile, "p%d", tempProducer->id);
                tempProducer = tempProducer->next;
                if(tempProducer)
                    fprintf(SGlogFile, ", ");
            }
            fprintf(SGlogFile, " )\n");
            
            
            fprintf(SGlogFile, "\t\t   [+] Consumers: ( ");
            tempConsumer = tempFunction->consumers;
            while(tempConsumer)
            {
                fprintf(SGlogFile, "p%d", tempConsumer->id);
                tempConsumer = tempConsumer->next;
                if(tempConsumer)
                    fprintf(SGlogFile, ", ");
            }
            fprintf(SGlogFile, " )\n");
            
            
            fprintf(SGlogFile, "\t\t   [+] tred_producers: ( ");
            tempProducer = tempFunction->tred_producers;
            while(tempProducer)
            {
                fprintf(SGlogFile, "p%d", tempProducer->id);
                tempProducer = tempProducer->next;
                if(tempProducer)
                    fprintf(SGlogFile, ", ");
            }
            fprintf(SGlogFile, " )\n");
            
            
            fprintf(SGlogFile, "\t\t   [+] tred_consumers: ( ");
            tempConsumer = tempFunction->tred_consumers;
            while(tempConsumer)
            {
                fprintf(SGlogFile, "p%d", tempConsumer->id);
                tempConsumer = tempConsumer->next;
                if(tempConsumer)
                    fprintf(SGlogFile, ", ");
            }
            fprintf(SGlogFile, " )\n");            
        
                
                
			
			/* Print Variables lists of a function */
			
			fprintf(SGlogFile, "\n\t\t   [+] Function Variables Lists: \n");
			
			// Print Function Private List
			tempList = tempFunction->privateList;
			if(tempList)
			{
				fprintf(SGlogFile, "\t\t\t[*] Private List: \n");
				fprintf(SGlogFile, "\t\t\t\tName \t\tType \t\tIndexes \n");
				while(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
					
					tempIndexes = tempList->indexes;
					if(!tempIndexes)
						fprintf(SGlogFile, "(none)");
						
					while(tempIndexes)
					{
						if(tempIndexes->start.localInt != -1)
							fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
						else if(tempIndexes->start.localStr)
							fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
						
						if(tempIndexes->end.localInt != -1)
							fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
						else if(tempIndexes->end.localStr)
							fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
						else
							fprintf(SGlogFile, "]");
							
						tempIndexes = tempIndexes->next;
					}
					fprintf(SGlogFile, "\n");
					
					tempList = tempList->next;
				}
			}
			
			// Print Function First Private List
			tempList = tempFunction->firstPrivateList;
			if(tempList)
			{
				fprintf(SGlogFile, "\t\t\t[*] First Private List: \n");
				fprintf(SGlogFile, "\t\t\t\tName \t\tType \t\tIndexes \n");
				while(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
					
					tempIndexes = tempList->indexes;
					if(!tempIndexes)
						fprintf(SGlogFile, "(none)");
						
					while(tempIndexes)
					{
						if(tempIndexes->start.localInt != -1)
							fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
						else if(tempIndexes->start.localStr)
							fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
						
						if(tempIndexes->end.localInt != -1)
							fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
						else if(tempIndexes->end.localStr)
							fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
						else
							fprintf(SGlogFile, "]");
							
						tempIndexes = tempIndexes->next;
					}
					fprintf(SGlogFile, "\n");
					
					tempList = tempList->next;
				}
			}
			
			// Print Function Shared List
			tempList = tempFunction->sharedList;
			if(tempList)
			{
				fprintf(SGlogFile, "\t\t\t[*] Shared List: \n");
				fprintf(SGlogFile, "\t\t\t\tName \t\tType \t\tIndexes \n");
				while(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
					
					tempIndexes = tempList->indexes;
					if(!tempIndexes)
						fprintf(SGlogFile, "(none)");
						
					while(tempIndexes)
					{
						if(tempIndexes->start.localInt != -1)
							fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
						else if(tempIndexes->start.localStr)
							fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
						
						if(tempIndexes->end.localInt != -1)
							fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
						else if(tempIndexes->end.localStr)
							fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
						else
							fprintf(SGlogFile, "]");
							
						tempIndexes = tempIndexes->next;
					}
					fprintf(SGlogFile, "\n");			
					
					tempList = tempList->next;
				}
			}
			
			// Print Function CopyIn List
			tempList = tempFunction->copyInList;
			if(tempList)
			{
				fprintf(SGlogFile, "\t\t\t[*] CopyIn List: \n");
				fprintf(SGlogFile, "\t\t\t\tName \t\tType \t\tIndexes \n");
				while(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);

					tempIndexes = tempList->indexes;
					if(!tempIndexes)
						fprintf(SGlogFile, "(none)");
						
					while(tempIndexes)
					{
						if(tempIndexes->start.localInt != -1)
							fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
						else if(tempIndexes->start.localStr)
							fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
						
						if(tempIndexes->end.localInt != -1)
							fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
						else if(tempIndexes->end.localStr)
							fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
						else
							fprintf(SGlogFile, "]");
							
						tempIndexes = tempIndexes->next;
					}
					fprintf(SGlogFile, "\n");					
					
					tempList = tempList->next;
				}
			}
			
			// Print Function Flush List
			tempList = tempFunction->flushList;
			if(tempList)
			{
				fprintf(SGlogFile, "\t\t\t[*] Flush List: \n");
				fprintf(SGlogFile, "\t\t\t\tName \t\tType \t\tIndexes \n");
				while(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
					
					tempIndexes = tempList->indexes;
					if(!tempIndexes)
						fprintf(SGlogFile, "(none)");
						
					while(tempIndexes)
					{
						if(tempIndexes->start.localInt != -1)
							fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
						else if(tempIndexes->start.localStr)
							fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
						
						if(tempIndexes->end.localInt != -1)
							fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
						else if(tempIndexes->end.localStr)
							fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
						else
							fprintf(SGlogFile, "]");
							
						tempIndexes = tempIndexes->next;
					}
					fprintf(SGlogFile, "\n");					
					
					tempList = tempList->next;
				}
			}
            
            /* Print Parallel Function Dependencies  */
            
            fprintf(SGlogFile, "\n\t\t   [+] Function Dependencies: \n");
            
            
            // Print Task DependIn List
            tempList = tempFunction->dependInList;
            if(tempList)
            {
                fprintf(SGlogFile, "\t\t\t[*] DependIn List: \n");
                fprintf(SGlogFile, "\t\t\t\tName \t\tType \t\tIndexes \n");
                while(tempList)
                {
                    fprintf(SGlogFile, "\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
                    
                    tempIndexes = tempList->indexes;
                    if(!tempIndexes)
                        fprintf(SGlogFile, "(none)");
                        
                    while(tempIndexes)
                    {
                        if(tempIndexes->start.localInt != -1)
                            fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
                        else if(tempIndexes->start.localStr)
                            fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
                        
                        if(tempIndexes->end.localInt != -1)
                            fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
                        else if(tempIndexes->end.localStr)
                            fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
                        else
                            fprintf(SGlogFile, "]");
                            
                        tempIndexes = tempIndexes->next;
                    }
                    fprintf(SGlogFile, "\n");							
                    
                    tempList = tempList->next;
                }
            }
            
            // Print Task DependOut List
            tempList = tempFunction->dependOutList;
            if(tempList)
            {
                fprintf(SGlogFile, "\t\t\t[*] DependOut List: \n");
                fprintf(SGlogFile, "\t\t\t\tName \t\tType \t\tIndexes \n");
                while(tempList)
                {
                    fprintf(SGlogFile, "\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
                    
                    tempIndexes = tempList->indexes;
                    if(!tempIndexes)
                        fprintf(SGlogFile, "(none)");
                        
                    while(tempIndexes)
                    {
                        if(tempIndexes->start.localInt != -1)
                            fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
                        else if(tempIndexes->start.localStr)
                            fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
                        
                        if(tempIndexes->end.localInt != -1)
                            fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
                        else if(tempIndexes->end.localStr)
                            fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
                        else
                            fprintf(SGlogFile, "]");
                            
                        tempIndexes = tempIndexes->next;
                    }
                    fprintf(SGlogFile, "\n");							
                    
                    tempList = tempList->next;
                }
            }
            
            // Print Task DependInOut List
            tempList = tempFunction->dependInOutList;
            if(tempList)
            {
                fprintf(SGlogFile, "\t\t\t[*] DependInOut List: \n");
                fprintf(SGlogFile, "\t\t\t\tName \t\tType \t\tIndexes \n");
                while(tempList)
                {
                    fprintf(SGlogFile, "\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
                    
                    tempIndexes = tempList->indexes;
                    if(!tempIndexes)
                        fprintf(SGlogFile, "(none)");
                        
                    while(tempIndexes)
                    {
                        if(tempIndexes->start.localInt != -1)
                            fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
                        else if(tempIndexes->start.localStr)
                            fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
                        
                        if(tempIndexes->end.localInt != -1)
                            fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
                        else if(tempIndexes->end.localStr)
                            fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
                        else
                            fprintf(SGlogFile, "]");
                            
                        tempIndexes = tempIndexes->next;
                    }
                    fprintf(SGlogFile, "\n");					
                    
                    tempList = tempList->next;
                }
            }
			
			
			
			/* Print tasks of a parallel function */
					
			fprintf(SGlogFile, "\n\t\t   [+] Tasks: \n");
		
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				fprintf(SGlogFile, "\t\t\t   [$$] Task %d: [ Type: ", tempTask->id);
				if(tempTask->taskType == TASK_SIMPLE)
					fprintf(SGlogFile, "%s, ", "simple");
				else if(tempTask->taskType == TASK_LOOP)
					fprintf(SGlogFile, "%s, ", "loop");
				else if(tempTask->taskType == TASK_REDUCTION)
					fprintf(SGlogFile, "%s, ", "reduction");
				else if(tempTask->taskType == TASK_SECTION)
					fprintf(SGlogFile, "%s, ", "section");
				else if(tempTask->taskType == TASK_MASTER)
					fprintf(SGlogFile, "%s, ", "master");
				else
					fprintf(SGlogFile, "%s, ", "other");
								
								
				fprintf(SGlogFile, "Priority: %d, ", tempTask->priority);
				
				
				fprintf(SGlogFile, "Default: ");
				if(tempTask->defaultVarsState == DEFAULT_SHARED)
					fprintf(SGlogFile, "%s, ", "shared");
				else if(tempTask->defaultVarsState == DEFAULT_NONE)
					fprintf(SGlogFile, "%s, ", "none");
										
				
				fprintf(SGlogFile, "Device: ");
				if(tempTask->device == DEVICE_GPU)
					fprintf(SGlogFile, "%s, ", "gpu");
				else if(tempTask->device == DEVICE_XEON_PHI)
					fprintf(SGlogFile, "%s, ", "mic");
				else if(tempTask->device == DEVICE_MULTI)
					fprintf(SGlogFile, "%s, ", "multicore");
					
					
				fprintf(SGlogFile, "SIMD: ");
				if(tempTask->simdLength == SIMD_512)
					fprintf(SGlogFile, "%s, ", "512");
				else
					fprintf(SGlogFile, "%s, ", "none");
				fprintf(SGlogFile, "\n");
				
				
				fprintf(SGlogFile, "\t\t\t\t   Cross-Loop-Producer: %s,\n", tempTask->crossLoopProducer ? "true" : "false");
				
				
				tempCrossConsumer = tempTask->crossLoopConsumerOf;
				fprintf(SGlogFile, "\t\t\t\t   Cross-Loop-Consumer-Of: ( ");
				if(!tempCrossConsumer)
					fprintf(SGlogFile, "none");
					
				while(tempCrossConsumer)
				{
					fprintf(SGlogFile, "%d", tempCrossConsumer->id);
					tempCrossConsumer = tempCrossConsumer->next;
					if(tempCrossConsumer)
						fprintf(SGlogFile, " ");
				}
				fprintf(SGlogFile, " ),\n");
				
				
				fprintf(SGlogFile, "\t\t\t\t   Sche_Policy: ");
				if(tempTask->schedulingPolicy == LOOP_SCHED_STATIC)
				{
					fprintf(SGlogFile, "%s ", "static");
					if(tempTask->chunkSize.localInt)
						fprintf(SGlogFile, "(%d), ", tempTask->chunkSize.localInt);
					else
						fprintf(SGlogFile, "(%s), ", tempTask->chunkSize.localStr);
				}
				else if(tempTask->schedulingPolicy == LOOP_SCHED_CROSS)
				{
					fprintf(SGlogFile, "%s ", "cross");
					if(tempTask->chunkSize.localInt)
						fprintf(SGlogFile, "(%d), ", tempTask->chunkSize.localInt);
					else
						fprintf(SGlogFile, "(%s), ", tempTask->chunkSize.localStr);
				}
				else
					fprintf(SGlogFile, "%s, ", "none");
				fprintf(SGlogFile, "\n");
				
				
				fprintf(SGlogFile, "\t\t\t\t   Kernels  : %d ( ", tempTask->number_of_kernels);
				tempKernel = tempTask->kernels;
				while(tempKernel)
				{
					fprintf(SGlogFile, "%d ", tempKernel->id);
					tempKernel = tempKernel->next;
				}
				fprintf(SGlogFile, "), \n");
				
				fprintf(SGlogFile, "\t\t\t\t   V-Kernels: %d ( ", tempTask->number_of_kernels);
				tempKernel = tempTask->kernels;
				while(tempKernel)
				{
					fprintf(SGlogFile, "%d ", tempKernel->vId);
					tempKernel = tempKernel->next;
				}
				fprintf(SGlogFile, "), \n");
					
				
				fprintf(SGlogFile, "\t\t\t\t   Producers: ( ");
				tempProducer = tempTask->producers;
				while(tempProducer)
					{
						tempKernel2 = tempProducer->kernels;
						while(tempKernel2)
						{
							fprintf(SGlogFile, "t%d-k%d[%d]", tempProducer->id, tempKernel2->id, tempKernel2->vId);
							tempKernel2 = tempKernel2->next;
							if(tempKernel2)
								fprintf(SGlogFile, ", ");
						}
						tempProducer = tempProducer->next;
						if(tempProducer)
							fprintf(SGlogFile, ", ");
					}
				fprintf(SGlogFile, " )\n");
				
				
				fprintf(SGlogFile, "\t\t\t\t   Consumers: ( ");
				tempConsumer = tempTask->consumers;
				while(tempConsumer)
					{
						tempKernel2 = tempConsumer->kernels;
						while(tempKernel2)
						{
							fprintf(SGlogFile, "t%d-k%d[%d]", tempConsumer->id, tempKernel2->id, tempKernel2->vId);
							tempKernel2 = tempKernel2->next;
							if(tempKernel2)
								fprintf(SGlogFile, ", ");
						}
						tempConsumer = tempConsumer->next;
						if(tempConsumer)
							fprintf(SGlogFile, ", ");
					}
				fprintf(SGlogFile, ")\n");
				
				
				fprintf(SGlogFile, "\t\t\t\t   tred_producers: ( ");
				tempProducer = tempTask->tred_producers;
				while(tempProducer)
					{
						tempKernel2 = tempProducer->kernels;
						while(tempKernel2)
						{
							fprintf(SGlogFile, "t%d-k%d[%d]", tempProducer->id, tempKernel2->id, tempKernel2->vId);
							tempKernel2 = tempKernel2->next;
							if(tempKernel2)
								fprintf(SGlogFile, ", ");
						}
						tempProducer = tempProducer->next;
						if(tempProducer)
							fprintf(SGlogFile, ", ");
					}
				fprintf(SGlogFile, " )\n");
				
				
				fprintf(SGlogFile, "\t\t\t\t   tred_consumers: ( ");
				tempConsumer = tempTask->tred_consumers;
				while(tempConsumer)
					{
						tempKernel2 = tempConsumer->kernels;
						while(tempKernel2)
						{
							fprintf(SGlogFile, "t%d-k%d[%d]", tempConsumer->id, tempKernel2->id, tempKernel2->vId);
							tempKernel2 = tempKernel2->next;
							if(tempKernel2)
								fprintf(SGlogFile, ", ");
						}
						tempConsumer = tempConsumer->next;
						if(tempConsumer)
							fprintf(SGlogFile, ", ");
					}
				fprintf(SGlogFile, ") ]\n");
				
				
				// Print Task Shared List
				tempList = tempTask->sharedList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] Shared List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
						
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");
						
						tempList = tempList->next;
					}
				}
				
				// Print Task Private List
				tempList = tempTask->privateList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] Private List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");
						
						tempList = tempList->next;
					}
				}
				
				// Print Task First-Private List
				tempList = tempTask->firstPrivateList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] First-Private List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");
						
						tempList = tempList->next;
					}
				}
				
				// Print Task Last-Private List
				tempList = tempTask->lastPrivateList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] Last-Private List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");			
						
						tempList = tempList->next;
					}
				}
				
				// Print Task CopyIn List
				tempList = tempTask->copyInList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] CopyIn List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");							
						
						tempList = tempList->next;
					}
				}
				
				// Print Task Copy-Private List
				tempList = tempTask->copyPrivateList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] CopyPrivate List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");						
						
						tempList = tempList->next;
					}
				}
				
				// Print Task DependIn List
				tempList = tempTask->dependInList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] DependIn List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");							
						
						tempList = tempList->next;
					}
				}
				
				// Print Task DependOut List
				tempList = tempTask->dependOutList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] DependOut List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");							
						
						tempList = tempList->next;
					}
				}
				
				// Print Task DependInOut List
				tempList = tempTask->dependInOutList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] DependInOut List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");					
						
						tempList = tempList->next;
					}
				}
				
				// Print Task toDevice List
				tempList = tempTask->toDeviceList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] toDevice List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");							
						
						tempList = tempList->next;
					}
				}
				
				// Print Task fromDevice List
				tempList = tempTask->fromDeviceList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] fromDevice List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");					
						
						tempList = tempList->next;
					}
				}
				
				// Print Task Reduction List
				tempList = tempTask->reductionList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] Reduction List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \t\tReduction Type \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						
						fprintf(SGlogFile, "\t\t\t%s", tempList->reductionType);
						fprintf(SGlogFile, "\n");					
						
						tempList = tempList->next;
					}
				}
				
				// Print Task Linear List
				tempList = tempTask->linearList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] Linear List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);

						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");							
						
						tempList = tempList->next;
					}
				}
				
				// Print Task Aligned List
				tempList = tempTask->alignedList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] Aligned List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");					
						
						tempList = tempList->next;
					}
				}
				
				// Print Task Flush List
				tempList = tempTask->flushList;
				if(tempList)
				{
					fprintf(SGlogFile, "\t\t\t\t\t[*] Flush List: \n");
					fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
					while(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
						
						tempIndexes = tempList->indexes;
						if(!tempIndexes)
							fprintf(SGlogFile, "(none)");
							
						while(tempIndexes)
						{
							if(tempIndexes->start.localInt != -1)
								fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
							else if(tempIndexes->start.localStr)
								fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
							
							if(tempIndexes->end.localInt != -1)
								fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
							else if(tempIndexes->end.localStr)
								fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
							else
								fprintf(SGlogFile, "]");
								
							tempIndexes = tempIndexes->next;
						}
						fprintf(SGlogFile, "\n");					
						
						tempList = tempList->next;
					}
				}
				
				fprintf(SGlogFile, "\n");
				
				tempTask = tempTask->next;
			}
					
			
			
			/* Print Sections of a function */
			tempSection = tempFunction->sections;
			if(tempSection)
			{
				fprintf(SGlogFile, "\n\t\t   [+] Sections: \n");
				
				while(tempSection)
				{
					
					fprintf(SGlogFile, "\t\t\t[#] Section %d: \n", tempSection->id);
					
				
					// Print Section Private List
					tempList = tempSection->privateList;
					if(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t[*] Private List: \n");
						fprintf(SGlogFile, "\t\t\t\t\tName \t\tType \t\tIndexes \n");
						while(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
							
							tempIndexes = tempList->indexes;
							if(!tempIndexes)
								fprintf(SGlogFile, "(none)");
							
							while(tempIndexes)
							{
								if(tempIndexes->start.localInt != -1)
									fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
								else if(tempIndexes->start.localStr)
									fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
								
								if(tempIndexes->end.localInt != -1)
									fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
								else if(tempIndexes->end.localStr)
									fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
								else
									fprintf(SGlogFile, "]");
									
								tempIndexes = tempIndexes->next;
							}
							fprintf(SGlogFile, "\n");			
							
							tempList = tempList->next;
						}
					}
					
					// Print Section First Private List
					tempList = tempSection->firstPrivateList;
					if(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t[*] First Private List: \n");
						fprintf(SGlogFile, "\t\t\t\t\tName \t\tType \t\tIndexes \n");
						while(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
							
							tempIndexes = tempList->indexes;
							if(!tempIndexes)
								fprintf(SGlogFile, "(none)");
								
							while(tempIndexes)
							{
								if(tempIndexes->start.localInt != -1)
									fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
								else if(tempIndexes->start.localStr)
									fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
								
								if(tempIndexes->end.localInt != -1)
									fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
								else if(tempIndexes->end.localStr)
									fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
								else
									fprintf(SGlogFile, "]");
									
								tempIndexes = tempIndexes->next;
							}
							fprintf(SGlogFile, "\n");			
							
							tempList = tempList->next;
						}
					}
					
					// Print Section Last Private List
					tempList = tempSection->lastPrivateList;
					if(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t[*] Last Private List: \n");
						fprintf(SGlogFile, "\t\t\t\t\tName \t\tType \t\tIndexes \n");
						while(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
							
							tempIndexes = tempList->indexes;
							if(!tempIndexes)
								fprintf(SGlogFile, "(none)");
								
							while(tempIndexes)
							{
								if(tempIndexes->start.localInt != -1)
									fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
								else if(tempIndexes->start.localStr)
									fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
								
								if(tempIndexes->end.localInt != -1)
									fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
								else if(tempIndexes->end.localStr)
									fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
								else
									fprintf(SGlogFile, "]");
									
								tempIndexes = tempIndexes->next;
							}
							fprintf(SGlogFile, "\n");							
							
							tempList = tempList->next;
						}
					}
					
					// Print Section Reduction List
					tempList = tempSection->reductionList;
					if(tempList)
					{
						fprintf(SGlogFile, "\t\t\t\t[*] Reduction List: \n");
						fprintf(SGlogFile, "\t\t\t\t\tName \t\tType \t\tIndexes \t\tReduction Type\n");
						while(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
							
							tempIndexes = tempList->indexes;
							if(!tempIndexes)
								fprintf(SGlogFile, "(none)");
								
							while(tempIndexes)
							{
								if(tempIndexes->start.localInt != -1)
									fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
								else if(tempIndexes->start.localStr)
									fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
								
								if(tempIndexes->end.localInt != -1)
									fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
								else if(tempIndexes->end.localStr)
									fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
								else
									fprintf(SGlogFile, "]");
									
								tempIndexes = tempIndexes->next;
							}
							
							fprintf(SGlogFile, "\t\t\t%s", tempList->reductionType);
							fprintf(SGlogFile, "\n");							
							
							tempList = tempList->next;
						}
					}
					
					
					
					/* Print tasks of a section */
					
					fprintf(SGlogFile, "\t\t\t\t[*] Tasks: \n");
					
					tempTask = tempSection->tasks;
					while(tempTask)
					{
						fprintf(SGlogFile, "\t\t\t\t   [$$] Task %d: [ Type: ", tempTask->id);
						if(tempTask->taskType == TASK_SIMPLE)
							fprintf(SGlogFile, "%s, ", "simple");
						else if(tempTask->taskType == TASK_LOOP)
							fprintf(SGlogFile, "%s, ", "loop");
						else if(tempTask->taskType == TASK_REDUCTION)
							fprintf(SGlogFile, "%s, ", "reduction");
						else if(tempTask->taskType == TASK_SECTION)
							fprintf(SGlogFile, "%s, ", "section");
						else if(tempTask->taskType == TASK_MASTER)
							fprintf(SGlogFile, "%s, ", "master");
						else
							fprintf(SGlogFile, "%s, ", "other");
						
						
						fprintf(SGlogFile, "Priority: %d, ", tempTask->priority);
						
						
						fprintf(SGlogFile, "Default: ");
						if(tempTask->defaultVarsState == DEFAULT_SHARED)
							fprintf(SGlogFile, "%s, ", "shared");
						else if(tempTask->defaultVarsState == DEFAULT_NONE)
							fprintf(SGlogFile, "%s, ", "none");
												
						
						fprintf(SGlogFile, "Device: ");
						if(tempTask->device == DEVICE_GPU)
							fprintf(SGlogFile, "%s, ", "gpu");
						else if(tempTask->device == DEVICE_XEON_PHI)
							fprintf(SGlogFile, "%s, ", "mic");
						else if(tempTask->device == DEVICE_MULTI)
							fprintf(SGlogFile, "%s, ", "multicore");
							
							
						fprintf(SGlogFile, "SIMD: ");
						if(tempTask->simdLength == SIMD_512)
							fprintf(SGlogFile, "%s, ", "512");
						else
							fprintf(SGlogFile, "%s, ", "none");
						fprintf(SGlogFile, "\n");
						
						
						fprintf(SGlogFile, "\t\t\t\t\t\t   Cross-Loop-Producer: %s, \n", tempTask->crossLoopProducer ? "true" : "false");
						
						
						tempCrossConsumer = tempTask->crossLoopConsumerOf;
						fprintf(SGlogFile, "\t\t\t\t\t\t   Cross-Loop-Consumer-Of: ( ");
						if(!tempCrossConsumer)
							fprintf(SGlogFile, "none");
							
						while(tempCrossConsumer)
						{
							fprintf(SGlogFile, "%d", tempCrossConsumer->id);
							tempCrossConsumer = tempCrossConsumer->next;
							if(tempCrossConsumer)
								fprintf(SGlogFile, " ");
						}
						fprintf(SGlogFile, " ),\n");
						
						
						fprintf(SGlogFile, "\t\t\t\t\t\t   Sche_Policy: ");
						if(tempTask->schedulingPolicy == LOOP_SCHED_STATIC)
						{
							fprintf(SGlogFile, "%s ", "static");
							if(tempTask->chunkSize.localInt)
								fprintf(SGlogFile, "(%d), ", tempTask->chunkSize.localInt);
							else
								fprintf(SGlogFile, "(%s), ", tempTask->chunkSize.localStr);
						}
						else if(tempTask->schedulingPolicy == LOOP_SCHED_CROSS)
						{
							fprintf(SGlogFile, "%s ", "cross");
							if(tempTask->chunkSize.localInt)
								fprintf(SGlogFile, "(%d), ", tempTask->chunkSize.localInt);
							else
								fprintf(SGlogFile, "(%s), ", tempTask->chunkSize.localStr);
						}
						else
							fprintf(SGlogFile, "%s", "none");
						fprintf(SGlogFile, ",\n");
						
						
						fprintf(SGlogFile, "\t\t\t\t\t\t   Kernels  : %d ( ", tempTask->number_of_kernels);
						tempKernel = tempTask->kernels;
						while(tempKernel)
						{
							fprintf(SGlogFile, "%d ", tempKernel->id);
							tempKernel = tempKernel->next;
						}
						fprintf(SGlogFile, "),\n");
						
						fprintf(SGlogFile, "\t\t\t\t\t\t   V-Kernels: %d ( ", tempTask->number_of_kernels);
						tempKernel = tempTask->kernels;
						while(tempKernel)
						{
							fprintf(SGlogFile, "%d ", tempKernel->vId);
							tempKernel = tempKernel->next;
						}
						fprintf(SGlogFile, "),\n");
						
						
						fprintf(SGlogFile, "\t\t\t\t\t\t   Producers: ( ");
						tempProducer = tempTask->producers;
						while(tempProducer)
							{
								tempKernel2 = tempProducer->kernels;
								while(tempKernel2)
								{
									fprintf(SGlogFile, "t%d-k%d[%d]", tempProducer->id, tempKernel2->id, tempKernel2->vId);
									tempKernel2 = tempKernel2->next;
									if(tempKernel2)
										fprintf(SGlogFile, ", ");
								}
								tempProducer = tempProducer->next;
								if(tempProducer)
									fprintf(SGlogFile, ", ");
							}
						fprintf(SGlogFile, " ),\n");
						
						
						fprintf(SGlogFile, "\t\t\t\t\t\t   Consumers: ( ");
						tempConsumer = tempTask->consumers;
						while(tempConsumer)
							{
								tempKernel2 = tempConsumer->kernels;
								while(tempKernel2)
								{
									fprintf(SGlogFile, "t%d-k%d[%d]", tempConsumer->id, tempKernel2->id, tempKernel2->vId);
									tempKernel2 = tempKernel2->next;
									if(tempKernel2)
										fprintf(SGlogFile, ", ");
								}
								tempConsumer = tempConsumer->next;
								if(tempConsumer)
									fprintf(SGlogFile, ", ");
							}
						fprintf(SGlogFile, ")\n");
						
						fprintf(SGlogFile, "\t\t\t\t\t\t   tred_producers: ( ");
						tempProducer = tempTask->tred_producers;
						while(tempProducer)
							{
								tempKernel2 = tempProducer->kernels;
								while(tempKernel2)
								{
									fprintf(SGlogFile, "t%d-k%d[%d]", tempProducer->id, tempKernel2->id, tempKernel2->vId);
									tempKernel2 = tempKernel2->next;
									if(tempKernel2)
										fprintf(SGlogFile, ", ");
								}
								tempProducer = tempProducer->next;
								if(tempProducer)
									fprintf(SGlogFile, ", ");
							}
						fprintf(SGlogFile, " ),\n");
						
						
						fprintf(SGlogFile, "\t\t\t\t\t\t   tred_consumers: ( ");
						tempConsumer = tempTask->tred_consumers;
						while(tempConsumer)
							{
								tempKernel2 = tempConsumer->kernels;
								while(tempKernel2)
								{
									fprintf(SGlogFile, "t%d-k%d[%d]", tempConsumer->id, tempKernel2->id, tempKernel2->vId);
									tempKernel2 = tempKernel2->next;
									if(tempKernel2)
										fprintf(SGlogFile, ", ");
								}
								tempConsumer = tempConsumer->next;
								if(tempConsumer)
									fprintf(SGlogFile, ", ");
							}
						fprintf(SGlogFile, ") ]\n");
						
						
						// Print Task Shared List
						tempList = tempTask->sharedList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] Shared List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
								
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");			
								
								tempList = tempList->next;
							}
						}
						
						// Print Task Private List
						tempList = tempTask->privateList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] Private List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");								
								
								tempList = tempList->next;
							}
						}
						
						// Print Task First-Private List
						tempList = tempTask->firstPrivateList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] First-Private List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");							
								
								tempList = tempList->next;
							}
						}
						
						// Print Task Last-Private List
						tempList = tempTask->lastPrivateList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] Last-Private List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");								
								
								tempList = tempList->next;
							}
						}
						
						// Print Task CopyIn List
						tempList = tempTask->copyInList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] CopyIn List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");							
								
								tempList = tempList->next;
							}
						}
						
						// Print Task Copy-Private List
						tempList = tempTask->copyPrivateList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] CopyPrivate List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");								
								
								tempList = tempList->next;
							}
						}
						
						// Print Task DependIn List
						tempList = tempTask->dependInList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] DependIn List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
							
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");							
								
								tempList = tempList->next;
							}
						}
						
						// Print Task DependOut List
						tempList = tempTask->dependOutList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] DependOut List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");							
								
								tempList = tempList->next;
							}
						}
						
						// Print Task DependInOut List
						tempList = tempTask->dependInOutList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] DependInOut List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");							
								
								tempList = tempList->next;
							}
						}
						
						// Print Task toDevice List
						tempList = tempTask->toDeviceList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] toDevice List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");				
								
								tempList = tempList->next;
							}
						}
						
						// Print Task fromDevice List
						tempList = tempTask->fromDeviceList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] fromDevice List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");								
								
								tempList = tempList->next;
							}
						}
						
						// Print Task Reduction List
						tempList = tempTask->reductionList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] Reduction List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \t\tReduction Type\n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\t\t\t%s", tempList->reductionType);
								fprintf(SGlogFile, "\n");							
								
								tempList = tempList->next;
							}
						}
						
						// Print Task Linear List
						tempList = tempTask->linearList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] Linear List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempGraph->parallel_functions->sections->tasks->linearList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
									
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");							
								
								tempList = tempList->next;
							}
						}
						
						// Print Task Aligned List
						tempList = tempTask->alignedList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] Aligned List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
								
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");						
								
								tempList = tempList->next;
							}
						}
						
						// Print Task Flush List
						tempList = tempTask->flushList;
						if(tempList)
						{
							fprintf(SGlogFile, "\t\t\t\t\t[*] Flush List: \n");
							fprintf(SGlogFile, "\t\t\t\t\t\tName \t\tType \t\tIndexes \n");
							while(tempList)
							{
								fprintf(SGlogFile, "\t\t\t\t\t\t%s \t\t%s \t\t", tempList->variableName, tempList->variableType);
								
								tempIndexes = tempList->indexes;
								if(!tempIndexes)
									fprintf(SGlogFile, "(none)");
								
								while(tempIndexes)
								{
									if(tempIndexes->start.localInt != -1)
										fprintf(SGlogFile, "[%d", tempIndexes->start.localInt);
									else if(tempIndexes->start.localStr)
										fprintf(SGlogFile, "[%s", tempIndexes->start.localStr);
									
									if(tempIndexes->end.localInt != -1)
										fprintf(SGlogFile, ":%d]", tempIndexes->end.localInt);
									else if(tempIndexes->end.localStr)
										fprintf(SGlogFile, ":%s]", tempIndexes->end.localStr);
									else
										fprintf(SGlogFile, "]");
										
									tempIndexes = tempIndexes->next;
								}
								fprintf(SGlogFile, "\n");							
								
								tempList = tempList->next;
							}
						}
						
						fprintf(SGlogFile, "\n");
						
						tempTask = tempTask->next;
					}	
					tempSection = tempSection->next;
				}	
			}
			fprintf(SGlogFile, "\n");
			tempFunction = tempFunction->next; 
		}
		fprintf(SGlogFile, "--------------------------------------------------------------------------------------------------------------------------\n\n");		
		tempGraph = tempGraph->next;
	}
	
	if(printSGFlag == __SCREEN)
		SGlogFile = NULL;
	else if(printSGFlag == __FILE)
		fclose(SGlogFile);
	
}



/******************** Add a new Synchronization Graph ********************/


void addSynchronizationGraph(SG** Graph){
	
	SG* tempGraph = *Graph;
	
	if(!(*Graph))
	{
		(*Graph) = (SG *)malloc(sizeof(SG));
		(*Graph)->id 				  = 1;
		(*Graph)->parallel_functions  = NULL;
		(*Graph)->next 			 	  = NULL;
	}
	else
	{
		while(tempGraph && tempGraph->next)
			tempGraph = tempGraph->next;
		
		tempGraph->next = (SG *)malloc(sizeof(SG));
		tempGraph->next->id 				= tempGraph->id+1;
		tempGraph->next->parallel_functions = NULL;
		tempGraph->next->next 		  		= NULL;
	}	
}



/*************** Add a new [ parallel function ] in the Synchronization Graph ***************/


void addParallelFunctionToGraph(parallel_function **GraphFunc, int numKernels, int stateOfDefault, dataList *lists[]){
		
	parallel_function* tempParallelFunctions = *GraphFunc;
	dataList **tempDataList;
	dataList **tempList;
	
	
	
	if(!(*GraphFunc))
	{
		(*GraphFunc) = (parallel_function *)malloc(sizeof(parallel_function));
		(*GraphFunc)->id 				  = 1;
		(*GraphFunc)->number_of_kernels   = numKernels;
		(*GraphFunc)->defaultVarsState    = stateOfDefault;
		(*GraphFunc)->tasks 			  = NULL;
		(*GraphFunc)->kernels 			  = NULL;
		(*GraphFunc)->sections 			  = NULL;
        (*GraphFunc)->producers	      	  = NULL;
		(*GraphFunc)->consumers	      	  = NULL;
		(*GraphFunc)->tred_producers   	  = NULL;
		(*GraphFunc)->tred_consumers   	  = NULL;
		
		tempList = &(*GraphFunc)->privateList;
		tempDataList = &lists[IN_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*GraphFunc)->firstPrivateList;
		tempDataList = &lists[IN_FIRST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*GraphFunc)->sharedList;
		tempDataList = &lists[IN_SHARED];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*GraphFunc)->copyInList;
		tempDataList = &lists[IN_COPY_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*GraphFunc)->flushList;
		tempDataList = &lists[IN_FLUSH];
		copyDataListToSG(tempList, tempDataList);
        
        tempList = &(*GraphFunc)->dependInList;
		tempDataList = &lists[IN_DEPEND_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*GraphFunc)->dependOutList;
		tempDataList = &lists[IN_DEPEND_OUT];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*GraphFunc)->dependInOutList;
		tempDataList = &lists[IN_DEPEND_INOUT];
		copyDataListToSG(tempList, tempDataList);
		
		(*GraphFunc)->next 			 	  = NULL;
	}
	else
	{
		while(tempParallelFunctions && tempParallelFunctions->next)
			tempParallelFunctions = tempParallelFunctions->next;
		
		tempParallelFunctions->next = (parallel_function *)malloc(sizeof(parallel_function));
		tempParallelFunctions->next->id 				    = tempParallelFunctions->id+1;
		tempParallelFunctions->next->number_of_kernels   = numKernels;
		tempParallelFunctions->next->defaultVarsState    = stateOfDefault;
		tempParallelFunctions->next->tasks 			     = NULL;
		tempParallelFunctions->next->kernels		     = NULL;
		tempParallelFunctions->next->sections		     = NULL;
		
		tempList = &tempParallelFunctions->next->privateList;
		tempDataList = &lists[IN_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempParallelFunctions->next->firstPrivateList;
		tempDataList = &lists[IN_FIRST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempParallelFunctions->next->sharedList;
		tempDataList = &lists[IN_SHARED];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempParallelFunctions->next->copyInList;
		tempDataList = &lists[IN_COPY_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempParallelFunctions->next->flushList;
		tempDataList = &lists[IN_FLUSH];
		copyDataListToSG(tempList, tempDataList);
        
        tempList = &tempParallelFunctions->next->dependInList;
		tempDataList = &lists[IN_DEPEND_IN];
		copyDataListToSG(tempList, tempDataList);
        
        tempList = &tempParallelFunctions->next->dependOutList;
		tempDataList = &lists[IN_DEPEND_OUT];
		copyDataListToSG(tempList, tempDataList);
        
        tempList = &tempParallelFunctions->next->dependInOutList;
		tempDataList = &lists[IN_DEPEND_INOUT];
		copyDataListToSG(tempList, tempDataList);        
		
		tempParallelFunctions->next->next 			 	= NULL;
	}		
}



/********************* Add new data to [ depend indexes ]  *********************/


void addToIndexes(arrayIndex** indexes, int indexStartInt, int indexEndInt, char* indexStartStr, char* indexEndStr){

	arrayIndex* tempIndexes = *indexes;
	
	if(!(*indexes))
	{
		(*indexes) = (arrayIndex *)malloc(sizeof(arrayIndex));
		(*indexes)->start.localInt		= indexStartInt;
		(*indexes)->end.localInt		= indexEndInt;
		
		if(indexStartStr)
		{
			(*indexes)->start.localStr = (char *)malloc(sizeof(char)*strlen(indexStartStr));
			strcpy((*indexes)->start.localStr, indexStartStr);
		}
		else
			(*indexes)->start.localStr = NULL;
		
		if(indexEndStr)
		{
			(*indexes)->end.localStr = (char *)malloc(sizeof(char)*strlen(indexEndStr));
			strcpy((*indexes)->end.localStr, indexEndStr);
		}
		else
			(*indexes)->end.localStr = NULL;
		
		(*indexes)->next = NULL;
	}
	else
	{
		while(tempIndexes && tempIndexes->next)
			tempIndexes = tempIndexes->next;
		
		tempIndexes->next = (arrayIndex *)malloc(sizeof(arrayIndex));
		tempIndexes->next->start.localInt		= indexStartInt;
		tempIndexes->next->end.localInt			= indexEndInt;
		
		if(indexStartStr)
		{
			tempIndexes->next->start.localStr = (char *)malloc(sizeof(char)*strlen(indexStartStr));
			strcpy(tempIndexes->next->start.localStr, indexStartStr);
		}
		else
			tempIndexes->next->start.localStr = NULL;
		
		if(indexEndStr)
		{
			tempIndexes->next->end.localStr = (char *)malloc(sizeof(char)*strlen(indexEndStr));
			strcpy(tempIndexes->next->end.localStr, indexEndStr);
		}
		else
			tempIndexes->next->end.localStr = NULL;
		
		tempIndexes->next->next = NULL;
	}
}


/********************* Create a new [ dataList ]  *********************/


void addToDataList(dataList** list, char *variableName, char *variableType, char *reductionType, arrayIndex** indexes){
	
	dataList   *tempDataList = *list;
	arrayIndex *tempIndexes; 
	
	if(!(*list))
	{
		(*list) = (dataList *)malloc(sizeof(dataList));
		(*list)->variableName = (char *)malloc(sizeof(char)*strlen(variableName));
		strcpy((*list)->variableName, variableName);
		
		if(variableType)
			{
				(*list)->variableType = (char *)malloc(sizeof(char)*strlen(variableType));
				strcpy((*list)->variableType, variableType);
			}
		else
			(*list)->variableType = NULL;
		
		if(reductionType)
		{
			(*list)->reductionType = (char *)malloc(sizeof(char)*strlen(reductionType));
			strcpy((*list)->reductionType, reductionType);
		}
		else
			(*list)->reductionType = NULL;
			
		if(indexes)
		{
			while(*indexes)
			{
				if(!(*list)->indexes)
				{
					(*list)->indexes = (arrayIndex *)malloc(sizeof(arrayIndex));
					(*list)->indexes->start.localInt = (*indexes)->start.localInt;
					(*list)->indexes->end.localInt = (*indexes)->end.localInt;
					
					if((*indexes)->start.localStr)
					{
						(*list)->indexes->start.localStr = (char *)malloc(sizeof(char)*strlen((*indexes)->start.localStr));
						strcpy((*list)->indexes->start.localStr, (*indexes)->start.localStr);
					}
					else
						(*list)->indexes->start.localStr = NULL;
					
					if((*indexes)->end.localStr)
					{
						(*list)->indexes->end.localStr = (char *)malloc(sizeof(char)*strlen((*indexes)->end.localStr));
						strcpy((*list)->indexes->end.localStr, (*indexes)->end.localStr);
					}
					else
						(*list)->indexes->end.localStr = NULL;
						
					(*list)->indexes->next = NULL;
				}
				else
				{
					tempIndexes = (*list)->indexes;
					while(tempIndexes && tempIndexes->next)
						tempIndexes = tempIndexes->next;
						
					tempIndexes->next = (arrayIndex *)malloc(sizeof(arrayIndex));
					tempIndexes->next->start.localInt = (*indexes)->start.localInt;
					tempIndexes->next->end.localInt = (*indexes)->end.localInt;
					
					if((*indexes)->start.localStr)
					{
						tempIndexes->next->start.localStr = (char *)malloc(sizeof(char)*strlen((*indexes)->start.localStr));
						strcpy(tempIndexes->next->start.localStr, (*indexes)->start.localStr);
					}
					else
						tempIndexes->next->start.localStr = NULL;
					
					if((*indexes)->end.localStr)
					{
						tempIndexes->next->end.localStr = (char *)malloc(sizeof(char)*strlen((*indexes)->end.localStr));
						strcpy(tempIndexes->next->end.localStr, (*indexes)->end.localStr);
					}
					else
						tempIndexes->next->end.localStr = NULL;

					tempIndexes->next->next = NULL;
				}
				(*indexes) = (*indexes)->next;
			}
		}
		(*list)->next = NULL;
	}
	else
	{
		while(tempDataList && tempDataList->next)
			tempDataList = tempDataList->next;
		
		tempDataList->next = (dataList *)malloc(sizeof(dataList));
		tempDataList->next->variableName = (char *)malloc(sizeof(char)*strlen(variableName));
		strcpy(tempDataList->next->variableName, variableName);
		
		if(variableType)
		{
			tempDataList->next->variableType = (char *)malloc(sizeof(char)*strlen(variableType));
			strcpy(tempDataList->next->variableType, variableType);
		}
		else
			tempDataList->next->variableType = NULL;
			
		if(reductionType)
		{
			tempDataList->next->reductionType = (char *)malloc(sizeof(char)*strlen(reductionType));
			strcpy(tempDataList->next->reductionType, reductionType);
		}
		else
			tempDataList->next->reductionType = NULL;
		
		if(indexes)
		{
			while(*indexes)
			{
				
				if(!tempDataList->next->indexes)
				{
					tempDataList->next->indexes = (arrayIndex *)malloc(sizeof(arrayIndex));
					tempDataList->next->indexes->start.localInt = (*indexes)->start.localInt;
					tempDataList->next->indexes->end.localInt = (*indexes)->end.localInt;
					
					if((*indexes)->start.localStr)
					{
						tempDataList->next->indexes->start.localStr = (char *)malloc(sizeof(char)*strlen((*indexes)->start.localStr));
						strcpy(tempDataList->next->indexes->start.localStr, (*indexes)->start.localStr);
					}
					else
						tempDataList->next->indexes->start.localStr = NULL;
					
					if((*indexes)->end.localStr)
					{
						tempDataList->next->indexes->end.localStr = (char *)malloc(sizeof(char)*strlen((*indexes)->end.localStr));
						strcpy(tempDataList->next->indexes->end.localStr, (*indexes)->end.localStr);
					}
					else
						tempDataList->next->indexes->end.localStr = NULL;

					tempDataList->next->indexes->next = NULL;
				}
				else
				{
					tempIndexes = tempDataList->next->indexes;
					while(tempIndexes && tempIndexes->next)
						tempIndexes = tempIndexes->next;
					
					tempIndexes->next = (arrayIndex *)malloc(sizeof(arrayIndex));
					tempIndexes->next->start.localInt = (*indexes)->start.localInt;
					tempIndexes->next->end.localInt = (*indexes)->end.localInt;
					
					if((*indexes)->start.localStr)
					{
						tempIndexes->next->start.localStr = (char *)malloc(sizeof(char)*strlen((*indexes)->start.localStr));
						strcpy(tempIndexes->next->start.localStr, (*indexes)->start.localStr);
					}
					else
						tempIndexes->next->start.localStr = NULL;
					
					if((*indexes)->end.localStr)
					{
						tempIndexes->next->end.localStr = (char *)malloc(sizeof(char)*strlen((*indexes)->end.localStr));
						strcpy(tempIndexes->next->end.localStr, (*indexes)->end.localStr);
					}
					else
						tempIndexes->next->end.localStr = NULL;
						
					tempIndexes->next->next = NULL;
				}
				(*indexes) = (*indexes)->next;
			}
		}
		tempDataList->next->next = NULL;
	}

	if(indexes)
		bzero(indexes, sizeof(indexes));
}



/********************* Copy temp data lists to SG data lists  *********************/


void copyDataListToSG(dataList **SGList, dataList** tempList){
	
	dataList* tempSGList = *SGList;
	arrayIndex* tempIndexes;
	arrayIndex* tempIndexesSG;

	while(*tempList)
	{
		if(!(*SGList))
		{
			(*SGList) = (dataList *)malloc(sizeof(dataList));
			(*SGList)->variableName = (char *)malloc(sizeof(char)*strlen((*tempList)->variableName));
			strcpy((*SGList)->variableName, (*tempList)->variableName);
		
			if((*tempList)->variableType)
				{
					(*SGList)->variableType = (char *)malloc(sizeof(char)*strlen((*tempList)->variableType));
					strcpy((*SGList)->variableType, (*tempList)->variableType);
				}
			else
				(*SGList)->variableType = NULL;
				
			if((*tempList)->reductionType)
				{
					(*SGList)->reductionType = (char *)malloc(sizeof(char)*strlen((*tempList)->reductionType));
					strcpy((*SGList)->reductionType, (*tempList)->reductionType);
				}
			else
				(*SGList)->reductionType = NULL;
				
			tempIndexes = (*tempList)->indexes;
			while(tempIndexes)
			{
				if(!(*SGList)->indexes)
				{
					(*SGList)->indexes = (arrayIndex *)malloc(sizeof(arrayIndex));
					(*SGList)->indexes->start.localInt = tempIndexes->start.localInt;
					(*SGList)->indexes->end.localInt = tempIndexes->end.localInt;
					
					if(tempIndexes->start.localStr)
					{
						(*SGList)->indexes->start.localStr = (char *)malloc(sizeof(char)*strlen(tempIndexes->start.localStr));
						strcpy((*SGList)->indexes->start.localStr, tempIndexes->start.localStr);
					}
					else
						(*SGList)->indexes->start.localStr = NULL;
					
					if(tempIndexes->end.localStr)
					{
						(*SGList)->indexes->end.localStr = (char *)malloc(sizeof(char)*strlen(tempIndexes->end.localStr));
						strcpy((*SGList)->indexes->end.localStr, tempIndexes->end.localStr);
					}
					else
						(*SGList)->indexes->end.localStr = NULL;

					(*SGList)->indexes->next = NULL;
				}
				else
				{
					tempIndexesSG = (*SGList)->indexes;
					while(tempIndexesSG && tempIndexesSG->next)
						tempIndexesSG = tempIndexesSG->next;
						
					tempIndexesSG->next = (arrayIndex *)malloc(sizeof(arrayIndex));
					tempIndexesSG->next->start.localInt = tempIndexes->start.localInt;
					tempIndexesSG->next->end.localInt = tempIndexes->end.localInt;
					
					if(tempIndexes->start.localStr)
					{
						tempIndexesSG->next->start.localStr = (char *)malloc(sizeof(char)*strlen(tempIndexes->start.localStr));
						strcpy(tempIndexesSG->next->start.localStr, tempIndexes->start.localStr);
					}
					else
						tempIndexesSG->next->start.localStr = NULL;
					
					if(tempIndexes->end.localStr)
					{
						tempIndexesSG->next->end.localStr = (char *)malloc(sizeof(char)*strlen(tempIndexes->end.localStr));
						strcpy(tempIndexesSG->next->end.localStr, tempIndexes->end.localStr);
					}
					else
						tempIndexesSG->next->end.localStr = NULL;

					tempIndexesSG->next->next = NULL;
				}				
				tempIndexes = tempIndexes->next;
			}			
			(*SGList)->next = NULL;
		}
		else
		{
			tempSGList = *SGList;
			while(tempSGList && tempSGList->next)
				tempSGList = tempSGList->next;
			
			tempSGList->next = (dataList *)malloc(sizeof(dataList));
			tempSGList->next->variableName = (char *)malloc(sizeof(char)*strlen((*tempList)->variableName));
			strcpy(tempSGList->next->variableName, (*tempList)->variableName);
		
			if((*tempList)->variableType)
				{
					tempSGList->next->variableType = (char *)malloc(sizeof(char)*strlen((*tempList)->variableType));
					strcpy(tempSGList->next->variableType, (*tempList)->variableType);
				}
			else
				tempSGList->next->variableType = NULL;
				
			
			if((*tempList)->reductionType)
				{
					tempSGList->next->reductionType = (char *)malloc(sizeof(char)*strlen((*tempList)->reductionType));
					strcpy(tempSGList->next->reductionType, (*tempList)->reductionType);
				}
			else
				tempSGList->next->reductionType = NULL;			
			
			
			tempIndexes = (*tempList)->indexes;
			while(tempIndexes)
			{
				if(!tempSGList->next->indexes)
				{
					tempSGList->next->indexes = (arrayIndex *)malloc(sizeof(arrayIndex));
					tempSGList->next->indexes->start.localInt = tempIndexes->start.localInt;
					tempSGList->next->indexes->end.localInt = tempIndexes->end.localInt;
					
					if(tempIndexes->start.localStr)
					{
						tempSGList->next->indexes->start.localStr = (char *)malloc(sizeof(char)*strlen(tempIndexes->start.localStr));
						strcpy(tempSGList->next->indexes->start.localStr, tempIndexes->start.localStr);
					}
					else
						tempSGList->next->indexes->start.localStr = NULL;
					
					if(tempIndexes->end.localStr)
					{
						tempSGList->next->indexes->end.localStr = (char *)malloc(sizeof(char)*strlen(tempIndexes->end.localStr));
						strcpy(tempSGList->next->indexes->end.localStr, tempIndexes->end.localStr);
					}
					else
						tempSGList->next->indexes->end.localStr = NULL;
											
					tempSGList->next->indexes->next = NULL;
				}
				else
				{
					tempIndexesSG = tempSGList->next->indexes;
					while(tempIndexesSG && tempIndexesSG->next)
						tempIndexesSG = tempIndexesSG->next;
						
					tempIndexesSG->next = (arrayIndex *)malloc(sizeof(arrayIndex));
					tempIndexesSG->next->start.localInt = tempIndexes->start.localInt;
					tempIndexesSG->next->end.localInt = tempIndexes->end.localInt;
					
					if(tempIndexes->start.localStr)
					{
						tempIndexesSG->next->start.localStr = (char *)malloc(sizeof(char)*strlen(tempIndexes->start.localStr));
						strcpy(tempIndexesSG->next->start.localStr, tempIndexes->start.localStr);
					}
					else
						tempIndexesSG->next->start.localStr = NULL;
					
					if(tempIndexes->end.localStr)
					{
						tempIndexesSG->next->end.localStr = (char *)malloc(sizeof(char)*strlen(tempIndexes->end.localStr));
						strcpy(tempIndexesSG->next->end.localStr, tempIndexes->end.localStr);
					}
					else
						tempIndexesSG->next->end.localStr = NULL;
					
					tempIndexesSG->next->next = NULL;
				}				
				tempIndexes = tempIndexes->next;
			}		
			tempSGList->next->next = NULL;
		}
		(*tempList) = (*tempList)->next;
	}		
	
	// As soon as all data lists are copied to the SG, empty the temporary data list	
	bzero(tempList, sizeof(tempList));
	
}



/********** Add a new [ section ] in the Synchronization Graph ***********/


void addSectionToParallelFunction(parallel_function **GraphFunc, dataList *lists[]){
	
	parallel_function* tempParallelFunctions = *GraphFunc;
	section  **tempSections;
	section   *tempFuncSections;
	dataList **tempDataList;
	dataList **tempList;
	
	// Find the last inserted parallel function (this is the correct function to add the new section)
	
	while(tempParallelFunctions && tempParallelFunctions->next)
			tempParallelFunctions = tempParallelFunctions->next;
	
	tempSections = &(tempParallelFunctions->sections);
	
	if(!(*tempSections))
	{
		(*tempSections) = (section *)malloc(sizeof(section));
		(*tempSections)->id 		      = 1;
		(*tempSections)->tasks 			  = NULL;
		
		tempList = &(*tempSections)->privateList;
		tempDataList = &lists[IN_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempSections)->firstPrivateList;
		tempDataList = &lists[IN_FIRST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempSections)->lastPrivateList;
		tempDataList = &lists[IN_LAST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempSections)->reductionList;
		tempDataList = &lists[IN_REDUCTION];
		copyDataListToSG(tempList, tempDataList);
		
		(*tempSections)->next 			 	  = NULL;
	}
	else
	{
		tempFuncSections = *tempSections;
		
		while(tempFuncSections && tempFuncSections->next)
			tempFuncSections = tempFuncSections->next;
		
		tempFuncSections->next = (section *)malloc(sizeof(section));
		tempFuncSections->next->id 				= tempFuncSections->id+1;
		tempFuncSections->next->tasks 			= NULL;
		
		tempList = &tempFuncSections->next->privateList;
		tempDataList = &lists[IN_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempFuncSections->next->firstPrivateList;
		tempDataList = &lists[IN_FIRST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempFuncSections->next->lastPrivateList;
		tempDataList = &lists[IN_LAST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempFuncSections->next->reductionList;
		tempDataList = &lists[IN_REDUCTION];
		copyDataListToSG(tempList, tempDataList);
		
		tempFuncSections->next->next 			 	= NULL;
	}	
	

	
}



/********** Add a new [ task ] to a Section ***********/

void addTaskToSection(parallel_function **GraphFunc, dataList *lists[], int currentTask){
	
	parallel_function* tempParallelFunctions = *GraphFunc;
	section  *tempSections;
	task	 **tempTasks;
	task	  *tempSectionTasks;
	dataList **tempDataList;
	dataList **tempList;
	
	// Find the last inserted parallel function (this is the correct function to add the new section)
	
	while(tempParallelFunctions && tempParallelFunctions->next)
		tempParallelFunctions = tempParallelFunctions->next;
	
	tempSections = tempParallelFunctions->sections;
	
	
	// Find the last section inserted (this is the correct section to add the new task)
	
	while(tempSections && tempSections->next)
		tempSections = tempSections->next;
		
	tempTasks = &tempSections->tasks;
	
	if(!(*tempTasks))
	{
		(*tempTasks) = (task *)malloc(sizeof(task));
		(*tempTasks)->id 		      	  = currentTask;		
		(*tempTasks)->priority 		      = 0;
		(*tempTasks)->taskType 		      = TASK_SECTION;
		(*tempTasks)->defaultVarsState    = DEFAULT_SHARED;							
		(*tempTasks)->crossLoopProducer   = FALSE;
		(*tempTasks)->crossLoopConsumerOf = NULL;
		(*tempTasks)->schedulingPolicy    = 0;								
		(*tempTasks)->chunkSize.localInt  = 0;		
		(*tempTasks)->chunkSize.localStr  = NULL;
		(*tempTasks)->device 		      = DEVICE_MULTI;
		(*tempTasks)->simdLength	      = 0;
		(*tempTasks)->number_of_kernels   = 1;
		(*tempTasks)->kernels = (kernel *)malloc(sizeof(kernel));
		(*tempTasks)->kernels->id 		  = -1;
		(*tempTasks)->kernels->next		  = NULL;	
		(*tempTasks)->producers	      	  = NULL;
		(*tempTasks)->consumers	      	  = NULL;
		(*tempTasks)->tred_producers  	  = NULL;
		(*tempTasks)->tred_consumers  	  = NULL;
	
		tempList = &(*tempTasks)->sharedList;
		tempDataList = &lists[IN_SHARED];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->privateList;
		tempDataList = &lists[IN_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->firstPrivateList;
		tempDataList = &lists[IN_FIRST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->lastPrivateList;
		tempDataList = &lists[IN_LAST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->copyInList;
		tempDataList = &lists[IN_COPY_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->copyPrivateList;
		tempDataList = &lists[IN_COPY_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->dependInList;
		tempDataList = &lists[IN_DEPEND_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->dependOutList;
		tempDataList = &lists[IN_DEPEND_OUT];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->dependInOutList;
		tempDataList = &lists[IN_DEPEND_INOUT];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->toDeviceList;
		tempDataList = &lists[IN_TO_DEVICE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->fromDeviceList;
		tempDataList = &lists[IN_FROM_DEVICE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->reductionList;
		tempDataList = &lists[IN_REDUCTION];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->linearList;
		tempDataList = &lists[IN_LINEAR];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->alignedList;
		tempDataList = &lists[IN_ALIGNED];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->flushList;
		tempDataList = &lists[IN_FLUSH];
		copyDataListToSG(tempList, tempDataList);
		
		(*tempTasks)->next 			 	  = NULL;
	}
	else
	{
		tempSectionTasks = *tempTasks;
		
		while(tempSectionTasks && tempSectionTasks->next)
			tempSectionTasks = tempSectionTasks->next;
		
		tempSectionTasks->next = (task *)malloc(sizeof(task));
		tempSectionTasks->next->id 		  	 		 = currentTask;
		tempSectionTasks->next->priority 		     = 0;
		tempSectionTasks->next->taskType 		     = TASK_SECTION;
		tempSectionTasks->next->defaultVarsState     = DEFAULT_SHARED;
		tempSectionTasks->next->crossLoopProducer    = FALSE;
		tempSectionTasks->next->crossLoopConsumerOf  = NULL;
		tempSectionTasks->next->schedulingPolicy     = 0;
		tempSectionTasks->next->chunkSize.localInt 	 = 0;
		tempSectionTasks->next->chunkSize.localStr   = NULL;
		tempSectionTasks->next->device 		      	 = DEVICE_MULTI;
		tempSectionTasks->next->simdLength	      	 = 0;
		tempSectionTasks->next->number_of_kernels    = 1;
		tempSectionTasks->next->kernels = (kernel *)malloc(sizeof(kernel));
		tempSectionTasks->next->kernels->id 	 	 = -1;
		tempSectionTasks->next->kernels->next  	  	 = NULL;	
		tempSectionTasks->next->producers      		 = NULL;
		tempSectionTasks->next->consumers      		 = NULL;
		tempSectionTasks->next->tred_producers		 = NULL;
		tempSectionTasks->next->tred_consumers 		 = NULL;
		
		tempList = &tempSectionTasks->next->sharedList;
		tempDataList = &lists[IN_SHARED];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->privateList;
		tempDataList = &lists[IN_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->firstPrivateList;
		tempDataList = &lists[IN_FIRST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->lastPrivateList;
		tempDataList = &lists[IN_LAST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->copyInList;
		tempDataList = &lists[IN_COPY_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->copyPrivateList;
		tempDataList = &lists[IN_COPY_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->dependInList;
		tempDataList = &lists[IN_DEPEND_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->dependOutList;
		tempDataList = &lists[IN_DEPEND_OUT];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->dependInOutList;
		tempDataList = &lists[IN_DEPEND_INOUT];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->toDeviceList;
		tempDataList = &lists[IN_TO_DEVICE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->fromDeviceList;
		tempDataList = &lists[IN_FROM_DEVICE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->reductionList;
		tempDataList = &lists[IN_REDUCTION];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->linearList;
		tempDataList = &lists[IN_LINEAR];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->alignedList;
		tempDataList = &lists[IN_ALIGNED];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempSectionTasks->next->flushList;
		tempDataList = &lists[IN_FLUSH];
		copyDataListToSG(tempList, tempDataList);
		
		tempSectionTasks->next->next 	     	 	 = NULL;
	}	
}



/********** Add a new [ task ] in the Synchronization Graph ***********/


void addTaskToParallelFunction(parallel_function **GraphFunc, dataList *lists[], int type, int numKernels, int stateOfDefault, int priority, int schedulingPolicy, intStr schedulingPolicyChunk, int simdLength, int currentTask){
	
	parallel_function* tempParallelFunctions = *GraphFunc;
	task	 **tempTasks;
	task	  *tempTasksPtr;
	dataList **tempDataList;
	dataList **tempList;
	
	
	// Find the last inserted parallel function (this is the correct function to add the new section)
	
	while(tempParallelFunctions && tempParallelFunctions->next)
		tempParallelFunctions = tempParallelFunctions->next;
			
	
	if(numKernels == 0) numKernels = tempParallelFunctions->number_of_kernels;
	
	if(numKernels > tempParallelFunctions->number_of_kernels)
		{
			WARNING("Number of kernels [ %d ] declared for Task is more than number of kernels allocated for the parallel function!\n\t\t--> [ %d ] kernels will be used instead!", line, numKernels, tempParallelFunctions->number_of_kernels);
			numKernels = tempParallelFunctions->number_of_kernels;
		}
	
	tempTasks = &tempParallelFunctions->tasks;
	
	if(!(*tempTasks))
	{
		(*tempTasks) = (task *)malloc(sizeof(task));
		(*tempTasks)->id 		      	  = currentTask;		
		(*tempTasks)->priority 		      = priority;
		(*tempTasks)->taskType 		      = type;
		(*tempTasks)->defaultVarsState    = stateOfDefault;
		(*tempTasks)->crossLoopProducer   = FALSE;
		(*tempTasks)->crossLoopConsumerOf = NULL;
		(*tempTasks)->schedulingPolicy    = schedulingPolicy;
		(*tempTasks)->chunkSize.localInt  = schedulingPolicyChunk.localInt;
		
		if(schedulingPolicyChunk.localStr)
		{
			(*tempTasks)->chunkSize.localStr = (char *)malloc(sizeof(char)*strlen(schedulingPolicyChunk.localStr));
			strcpy((*tempTasks)->chunkSize.localStr, schedulingPolicyChunk.localStr);
		}
		else
			(*tempTasks)->chunkSize.localStr = NULL;
			
		(*tempTasks)->device 		      = DEVICE_MULTI;
		(*tempTasks)->simdLength	      = simdLength;
		(*tempTasks)->number_of_kernels   = numKernels;
		(*tempTasks)->kernels = (kernel *)malloc(sizeof(kernel));
		(*tempTasks)->kernels->id 		  = -1;
		(*tempTasks)->kernels->next		  = NULL;	
		(*tempTasks)->producers	      	  = NULL;
		(*tempTasks)->consumers	      	  = NULL;
		(*tempTasks)->tred_producers   	  = NULL;
		(*tempTasks)->tred_consumers   	  = NULL;
		
		tempList = &(*tempTasks)->sharedList;
		tempDataList = &lists[IN_SHARED];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->privateList;
		tempDataList = &lists[IN_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->firstPrivateList;
		tempDataList = &lists[IN_FIRST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->lastPrivateList;
		tempDataList = &lists[IN_LAST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->copyInList;
		tempDataList = &lists[IN_COPY_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->copyPrivateList;
		tempDataList = &lists[IN_COPY_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->dependInList;
		tempDataList = &lists[IN_DEPEND_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->dependOutList;
		tempDataList = &lists[IN_DEPEND_OUT];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->dependInOutList;
		tempDataList = &lists[IN_DEPEND_INOUT];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->toDeviceList;
		tempDataList = &lists[IN_TO_DEVICE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->fromDeviceList;
		tempDataList = &lists[IN_FROM_DEVICE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->reductionList;
		tempDataList = &lists[IN_REDUCTION];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->linearList;
		tempDataList = &lists[IN_LINEAR];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->alignedList;
		tempDataList = &lists[IN_ALIGNED];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &(*tempTasks)->flushList;
		tempDataList = &lists[IN_FLUSH];
		copyDataListToSG(tempList, tempDataList);
	
		(*tempTasks)->next 			 	  = NULL;
	}
	else
	{
		tempTasksPtr = *tempTasks;
		
		while(tempTasksPtr && tempTasksPtr->next)
			tempTasksPtr = tempTasksPtr->next;
		
		tempTasksPtr->next = (task *)malloc(sizeof(task));
		tempTasksPtr->next->id 		  	 		 = currentTask;	
		tempTasksPtr->next->priority 		     = priority;
		tempTasksPtr->next->taskType 		     = type;
		tempTasksPtr->next->defaultVarsState     = stateOfDefault;
		tempTasksPtr->next->crossLoopProducer    = FALSE;
		tempTasksPtr->next->crossLoopConsumerOf  = NULL;
		tempTasksPtr->next->schedulingPolicy     = schedulingPolicy;
		tempTasksPtr->next->chunkSize.localInt   = schedulingPolicyChunk.localInt;
		
		if(schedulingPolicyChunk.localStr)
		{
			tempTasksPtr->next->chunkSize.localStr   = (char *)malloc(sizeof(char)*strlen(schedulingPolicyChunk.localStr));
			strcpy(tempTasksPtr->next->chunkSize.localStr, schedulingPolicyChunk.localStr);
		}
		else
			tempTasksPtr->next->chunkSize.localStr = NULL;
			
		tempTasksPtr->next->device 		      	 = DEVICE_MULTI;
		tempTasksPtr->next->simdLength	      	 = simdLength;
		tempTasksPtr->next->number_of_kernels    = numKernels;
		tempTasksPtr->next->kernels = (kernel *)malloc(sizeof(kernel));
		tempTasksPtr->next->kernels->id 	 	 = -1;
		tempTasksPtr->next->kernels->next  	  	 = NULL;	
		tempTasksPtr->next->producers      		 = NULL;
		tempTasksPtr->next->consumers      		 = NULL;
		tempTasksPtr->next->tred_producers 		 = NULL;
		tempTasksPtr->next->tred_consumers 		 = NULL;
		
		tempList = &tempTasksPtr->next->sharedList;
		tempDataList = &lists[IN_SHARED];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->privateList;
		tempDataList = &lists[IN_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->firstPrivateList;
		tempDataList = &lists[IN_FIRST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->lastPrivateList;
		tempDataList = &lists[IN_LAST_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->copyInList;
		tempDataList = &lists[IN_COPY_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->copyPrivateList;
		tempDataList = &lists[IN_COPY_PRIVATE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->dependInList;
		tempDataList = &lists[IN_DEPEND_IN];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->dependOutList;
		tempDataList = &lists[IN_DEPEND_OUT];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->dependInOutList;
		tempDataList = &lists[IN_DEPEND_INOUT];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->toDeviceList;
		tempDataList = &lists[IN_TO_DEVICE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->fromDeviceList;
		tempDataList = &lists[IN_FROM_DEVICE];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->reductionList;
		tempDataList = &lists[IN_REDUCTION];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->linearList;
		tempDataList = &lists[IN_LINEAR];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->alignedList;
		tempDataList = &lists[IN_ALIGNED];
		copyDataListToSG(tempList, tempDataList);
		
		tempList = &tempTasksPtr->next->flushList;
		tempDataList = &lists[IN_FLUSH];
		copyDataListToSG(tempList, tempDataList);
	
		tempTasksPtr->next->next 	     	 	 = NULL;
	}

}




/********** Add the type of a variable in the SG ***********/

void addTypeOfVariable(SG **Graph, char* type, char *variable){
	
	SG 				  	*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
	task 				*tempTask;
	
	
	
	while(tempGraph)
	{
		/* Check graphs thread private list */
		searchVariableList(&(tempGraph->threadPrivateList), type, variable);

		tempFunction = tempGraph->parallel_functions;
		while(tempFunction)
		{
			/* Check functions private list */
			searchVariableList(&(tempFunction->privateList), type, variable);
			
			/* Check functions first private list */
			searchVariableList(&(tempFunction->firstPrivateList), type, variable);
			
			/* Check functions shared list */
			searchVariableList(&(tempFunction->sharedList), type, variable);
			
			/* Check functions copyIn list */
			searchVariableList(&(tempFunction->copyInList), type, variable);
			
			/* Check functions flush list */
			searchVariableList(&(tempFunction->flushList), type, variable);
			
			
			// Check normal tasks
			tempTask = tempFunction->tasks;
			while(tempTask)
			{
				
				/* Check tasks shared list */
				searchVariableList(&(tempTask->sharedList), type, variable);
				
				/* Check tasks private list */
				searchVariableList(&(tempTask->privateList), type, variable);
				
				/* Check tasks first private list */
				searchVariableList(&(tempTask->firstPrivateList), type, variable);
				
				/* Check tasks last private list */
				searchVariableList(&(tempTask->lastPrivateList), type, variable);
				
				/* Check tasks copyIn list */
				searchVariableList(&(tempTask->copyInList), type, variable);
				
				/* Check tasks copyPrivate list */
				searchVariableList(&(tempTask->copyPrivateList), type, variable);
				
				/* Check tasks dependIn list */
				searchVariableList(&(tempTask->dependInList), type, variable);
				
				/* Check tasks dependOut list */
				searchVariableList(&(tempTask->dependOutList), type, variable);
				
				/* Check tasks dependInOut list */
				searchVariableList(&(tempTask->dependInOutList), type, variable);
				
				/* Check tasks toDevice list */
				searchVariableList(&(tempTask->toDeviceList), type, variable);
				
				/* Check tasks fromDevice list */
				searchVariableList(&(tempTask->fromDeviceList), type, variable);
				
				/* Check tasks reduction list */
				searchVariableList(&(tempTask->reductionList), type, variable);
				
				/* Check tasks linear list */
				searchVariableList(&(tempTask->linearList), type, variable);
				
				/* Check tasks aligned list */
				searchVariableList(&(tempTask->alignedList), type, variable);
				
				/* Check tasks flush list */
				searchVariableList(&(tempTask->flushList), type, variable);
				
				
				tempTask = tempTask->next;
			}
			
			
			
			// Check section tasks
			tempSection = tempFunction->sections;
			while(tempSection)
			{
				
				/* Check sections private list */
				searchVariableList(&(tempSection->privateList), type, variable);
				
				/* Check sections first private list */
				searchVariableList(&(tempSection->firstPrivateList), type, variable);
				
				/* Check sections last private list */
				searchVariableList(&(tempSection->lastPrivateList), type, variable);
				
				/* Check sections reduction list */
				searchVariableList(&(tempSection->reductionList), type, variable);
				
				
				tempTask = tempSection->tasks;
				while(tempTask)
				{
					/* Check tasks shared list */
					searchVariableList(&(tempTask->sharedList), type, variable);
					
					/* Check tasks private list */
					searchVariableList(&(tempTask->privateList), type, variable);
					
					/* Check tasks first private list */
					searchVariableList(&(tempTask->firstPrivateList), type, variable);
					
					/* Check tasks last private list */
					searchVariableList(&(tempTask->lastPrivateList), type, variable);
					
					/* Check tasks copyIn list */
					searchVariableList(&(tempTask->copyInList), type, variable);
					
					/* Check tasks copyPrivate list */
					searchVariableList(&(tempTask->copyPrivateList), type, variable);
					
					/* Check tasks dependIn list */
					searchVariableList(&(tempTask->dependInList), type, variable);
					
					/* Check tasks dependOut list */
					searchVariableList(&(tempTask->dependOutList), type, variable);
					
					/* Check tasks dependInOut list */
					searchVariableList(&(tempTask->dependInOutList), type, variable);
					
					/* Check tasks toDevice list */
					searchVariableList(&(tempTask->toDeviceList), type, variable);
					
					/* Check tasks fromDevice list */
					searchVariableList(&(tempTask->fromDeviceList), type, variable);
					
					/* Check tasks reduction list */
					searchVariableList(&(tempTask->reductionList), type, variable);
					
					/* Check tasks linear list */
					searchVariableList(&(tempTask->linearList), type, variable);
					
					/* Check tasks aligned list */
					searchVariableList(&(tempTask->alignedList), type, variable);
					
					/* Check tasks flush list */
					searchVariableList(&(tempTask->flushList), type, variable);
					
					
					tempTask = tempTask->next;
				}	
				tempSection = tempSection->next;
			}
			tempFunction = tempFunction->next;
		}
		tempGraph = tempGraph->next;
	}
}




/********** Search a variable list in the SG ***********/

void searchVariableList(dataList **list, char* type, char* variable){

	dataList			*tempDataList = *list;


	while(tempDataList)
	{
		if(!strcmp(tempDataList->variableName, variable))
		{
			if(!tempDataList->variableType)
			{
				tempDataList->variableType = (char *)malloc(sizeof(char)*strlen(type));
				strcpy(tempDataList->variableType, type);
				break;
			}
		}
		tempDataList = tempDataList->next;
	}
}
