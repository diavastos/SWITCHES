/**************************************************/
/*                                                */
/*  File:        SG_Structs.h                     */
/*  Description: Declare the scheduling structures*/
/*               that will hold the information   */
/*               about the runtime, the threads   */
/*               and the dependencies             */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/



/* Structure for a linked list of all consumers of a cross-loop -- 1 cross-loop consumer can habe multiple cross-loop producers */

typedef struct crossConsumer{
	
	int 					id;							// Task ID
	struct crossConsumer* 	next;
	
}crossConsumer;

/* Structure to store either an integer value or a string */

typedef struct intStr{
	
	int 			localInt;
	char* 			localStr;
	
}intStr;


/* Structure to store either an integer value or a string */

typedef struct arrayIndex{
	
	intStr				start;
	intStr				end;
		
	struct arrayIndex* 	next;
	
}arrayIndex;


/* Structure for a linked list of all the kernels in the execution */

typedef struct dataList{
	
	char* 				variableName;
	char* 				variableType;
	char*				reductionType;
	arrayIndex* 		indexes;
	
	struct dataList* 	next;
	
}dataList;


/* Structure for a linked list of all the kernels in the execution */

typedef struct kernel{
	
	int 			id;
	int 			vId;
	struct kernel* 	next;
	
}kernel;

/* Structure for a linked list of all the producers/consumers of a task */

typedef struct producer{
	
	int 				id;							// Task ID
	kernel* 			kernels;					// ID of the kernels that will execute it
	struct producer* 	next;
	
}producer;



/* Structure of a Task */

typedef struct task{

	int 				id;							// The ID of the task
	int 				priority;					// Priority of the task
	int 				taskType;					// The type of the task (TASK_SIMPLE | TASK_LOOP | TASK_SECTION | TASK_REDUCTION | TASK_MASTER)
	int					defaultVarsState;			// The default clause of OpenMP - SHARED | NONE
	
	int 				schedulingPolicy;			// The scheduling policy to be used -- LOOP tasks only
	bool 				crossLoopProducer;			// Is this task a producer of cross-loop switches? -- (0 : NO | 1 : YES)
	crossConsumer*		crossLoopConsumerOf;		// Is this task a cross-loop consumer of cross-loop producer? -- (IDs OF ITS CROSS-LOOP PRODUCER)
	intStr				chunkSize;					// The number of iterations to split the loop to each task (INT | STRING)
	
	int					device;						// The device that this task will execute on (GPU | MULTICORE | XEON_PHI, etc.)
	int					simdLength;					// The size of SIMD instructions
	
	int					number_of_kernels;			// The total number of kernels to use for the execution of a task
	kernel* 			kernels;					// The kernels that the task will execute on
	
	producer* 			producers;					// The original producers of a task
	producer* 			consumers;					// The original consumers of a task
	
	producer* 			tred_producers;				// The transitive recuction producers of a task
	producer* 			tred_consumers;				// The transitive recuction consumers of a task
	
	dataList*			sharedList;					// A list of all shared varialbes
	dataList*			privateList;				// A list of all private varialbes
	dataList*			firstPrivateList;			// A list of all firstprivate varialbes
	dataList*			lastPrivateList;			// A list of all lastprivate varialbes
	dataList*			copyInList;					// A list of all copyin varialbes
	dataList*			copyPrivateList;			// A list of all copyPrivate varialbes
	dataList*			dependInList;				// A list of all depend-in varialbes
	dataList*			dependOutList;				// A list of all depend-out varialbes
	dataList*			dependInOutList;			// A list of all depend-inout varialbes
	dataList*			toDeviceList;				// A list of all variables to be sent to a device
	dataList*			fromDeviceList;				// A list of all variables to be returned from a device
	dataList*			reductionList;				// A list of all variables that are found in a reduction task
	dataList*			linearList;					// A list of all variables that are found in a linear expression
	dataList*			alignedList;				// A list of all variables that are found in an aligned expression
	dataList*			flushList;					// A list of all variables that are to be flushed at the end of the task
	
	struct task* 		next;
	
}task;


/* Structure of Sections */

typedef struct section{
	
	int 						id;
	task* 						tasks;				// Each separate section in a sections statement
	
	dataList*					privateList;
	dataList*					firstPrivateList;
	dataList*					lastPrivateList;
	dataList*					reductionList;
	
	struct section*				next;
	
}section;

/* Structure of a parallel function that consists of multiple tasks */

typedef struct parallel_function{
	
	int 						id;
	int							number_of_kernels;
	int							defaultVarsState;
	task* 						tasks;
	kernel* 					kernels;
	section*					sections;
	
	dataList*					privateList;
	dataList*					firstPrivateList;
	dataList*					sharedList;
	dataList*					copyInList;
	dataList*					flushList;					// A list of all variables that are to be flushed at the end of the parallel function
    
    dataList*			        dependInList;			    // A list of all depend-in varialbes -- TAOSW additions
	dataList*			        dependOutList;				// A list of all depend-out varialbes -- TAOSW additions
	dataList*			        dependInOutList;			// A list of all depend-inout varialbes -- TAOSW additions
    
    producer* 			        producers;					// The original producers of a parallel function -- TAOSW additions
	producer* 			        consumers;					// The original consumers of a parallel function -- TAOSW additions
	producer* 			        tred_producers;				// The transitive recuction producers of a parallel function -- TAOSW additions
	producer* 			        tred_consumers;				// The transitive recuction consumers of a parallel function -- TAOSW additions
	
	struct parallel_function* 	next;
	
}parallel_function;

/* Structure of the Synchronization Graph that consists of all tasks
 * and parallel functions
 */

typedef struct SG{
	
	int 						id;
	dataList*					threadPrivateList;
	parallel_function* 			parallel_functions;
	
	struct SG* 					next;
		
}SG;
