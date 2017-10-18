/**************************************************/
/*                                                */
/*  File:        definitions.h                    */
/*  Description: Contains the constants/defines   */
/*               of the all translator files      */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/


/* Include Libraries: Local & System  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "prototypes.h"




/* Other Declarations */

#define INPUT_SIZE		1000
#define SIZE			10000
#define REDUCTION_SIZE	200



/* Tasks types */

#define TASK_SIMPLE       101
#define TASK_LOOP         102
#define TASK_REDUCTION    103
#define TASK_SECTION      104
#define TASK_MASTER       105



/* Target Systems supported by SWitches */

#define MIC         201
#define MULTICORE   202


/* Loop Scheduling Policies */

#define LOOP_SCHED_STATIC     301
#define LOOP_SCHED_CROSS      302



/* Device that a task will execute on */

#define DEVICE_GPU     		401
#define DEVICE_XEON_PHI		402
#define DEVICE_MULTI		403


/* Variables in default(..) clause */

#define DEFAULT_SHARED		501
#define DEFAULT_NONE 	    502


/* SIMD Lengths */

#define SIMD_512     601



/* Printing SG flags */

#define __SCREEN	701
#define __FILE 		702


/* Scheduling/Assignment Policies */

#define SCHED_RR     801
#define SCHED_FILE   802
#define SCHED_RANDOM 803
#define SCHED_NSGA   804


/* NSGA-II Values */

#define NSGA_PARAMETERS     5
#define NSGA_POPULATION     1
#define NSGA_GENERATIONS    2
#define NSGA_OBJECTIVE      3
    #define NSGA_OBJECTIVE_PERFORMANCE    31
    #define NSGA_OBJECTIVE_POWER          32
    #define NSGA_OBJECTIVE_THERMAL        33
#define NSGA_MUTATION       4
#define NSGA_CROSSOVER      5
    
#define RANDOM_NUM          55
#define SEED                0.000001
#define INF                 1.0e14
#define EPS                 1.0e-14




/* Affinity Policies */

#define AFFINITY_NONE     901
#define AFFINITY_COMPACT  902
#define AFFINITY_SCATTER  903
#define AFFINITY_HYBRID   904
#define AFFINITY_RANDOM   905


/* Runtime Systems */

#define RUNTIME_STATIC    1001
#define RUNTIME_TAO       1002
#define RUNTIME_TAOSW     1003



/* Data Lists flags */

#define  NUM_LISTS			20

#define  IN_LINEAR			0
#define  IN_ALIGNED			1
#define  IN_IS_DEVICE_PTR	2
#define  IN_USE_DEVICE_PTR	3
#define  IN_THREAD_PRIVATE	4
#define  IN_FLUSH			5
#define  IN_COPY_PRIVATE	6
#define  IN_COPY_IN			7
#define  IN_PRIVATE			8
#define  IN_FIRST_PRIVATE	9
#define  IN_LAST_PRIVATE	10
#define  IN_SHARED			11
#define  IN_REDUCTION		12
#define  IN_DEPEND_IN		13
#define  IN_DEPEND_OUT		14
#define  IN_DEPEND_INOUT	15
#define  IN_TO_DEVICE		16
#define  IN_FROM_DEVICE		17


/* Parsing signals */

#define TRUE         1
#define FALSE        0
#define PARSES		 2


/* Error Printing Colos */

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1B[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* How to use these codes */
/* Example:
 * 
 *    printf(ANSI_COLOR_RED     "This text is RED!"     ANSI_COLOR_RESET "\n");
 *    printf(ANSI_COLOR_GREEN   "This text is GREEN!"   ANSI_COLOR_RESET "\n");
 * 
 */
 
 
 /* Write to output functions */

#define __OUTP_IS_NOW_STDOUT outp = stdout;
#define __OUTP_IS_NOW_SW_FILE outp = outp_sw_h;
#define __OUTP_IS_NOW_SW_TAO_FILE outp = outp_sw_tao_h;
#define __OUTP_IS_NOW_MAIN_FILE outp = outp_sw_main;
#define __OUTP_IS_NOW_THREADS_FILE outp = outp_sw_threads;
#define __OUTP_IS_NOW_THREADPOOL_FILE outp = outp_sw_threadpool;


#define WRITE(FormalLiteral, ...)   fprintf(outp, FormalLiteral, __VA_ARGS__);fflush(outp);


 
 
 /* Error Message Definition */

#define ERROR(FormalLiteral, ...)   fprintf(stderr, ANSI_COLOR_RED "%s:%d: ERROR: " FormalLiteral ANSI_COLOR_RESET "\n", inputFiles[currentFile], __VA_ARGS__);
#define WARNING(FormalLiteral, ...) fprintf(stderr, ANSI_COLOR_YELLOW "%s:%d: WARNING: " FormalLiteral ANSI_COLOR_RESET "\n", inputFiles[currentFile], __VA_ARGS__);

#define SYNTAX_ERROR(FormalLiteral, ...)   fprintf(stderr, ANSI_COLOR_CYAN "\t\t  " FormalLiteral ANSI_COLOR_RESET "\n", __VA_ARGS__);
#define ERROR_COMMANDS(FormalLiteral, ...)   fprintf(stderr, ANSI_COLOR_RED "\nERROR: " FormalLiteral ANSI_COLOR_RESET "\n\n", __VA_ARGS__);

#define ERROR_IN_TRANSLATOR(FormalLiteral, ...)   fprintf(stderr, ANSI_COLOR_RED "%s:%d: ERROR: " FormalLiteral ANSI_COLOR_RESET "\n", __FILE__, __LINE__, __VA_ARGS__);

#define LOG_FILE log_file
#define LOG(FormalLiteral, ...) fprintf(LOG_FILE, "%s(%d):" FormalLiteral "\n", __FILE__, __LINE__, __VA_ARGS__);
