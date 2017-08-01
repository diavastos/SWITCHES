#include "sw.h"



/*** APPLICATION Definitions & Includes ****/

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#define NRA 512                 /* number of rows in matrix A */
#define NCA 512                 /* number of columns in matrix A */
#define NCB 512                  /* number of columns in matrix B */


/*** APPLICATION declarations ****/

extern double	**a;		
extern double	**b;		
extern double    **c;		
extern double 	sum;
extern long		i, j, k;

long currentFunction = 0;



/*** Declare all the SWITCHES here ***/

bool __sw_task_1[4]    = {__OFF};


/*** Declare Task Counter of each Thread & per Function ***/

long __sw_taskCounter_Function_1[__KERNELS]  = { 1, 1, 1, 1 };



	/*** Reset Switches Functions ***/

/* Reset the switches of Parallel Function 1 */

void __sw_resetSWitches_1(int tid)
{

    switch(tid)
    {
         case __MAIN_KERNEL:
            __sw_task_1[0] = __OFF;
            break;

         case __KERNEL_1:
            __sw_task_1[1] = __OFF;
            break;

         case __KERNEL_2:
            __sw_task_1[2] = __OFF;
            break;

         case __KERNEL_3:
            __sw_task_1[3] = __OFF;
            break;

    }

}


/*** Jobs Thread Function ***/

void *thread_jobs(void *arg)
{
    long tid;
    long myFunction = 0;
    __arguments *arguments;
    arguments = (__arguments*) arg;

    tid = arguments->id;
    myFunction = arguments->function_id;

    if(tid == __MAIN_KERNEL){
        currentFunction = myFunction;
        __sync_synchronize();
     }


    do{

        pthread_barrier_wait(&barrier[0]);
        myFunction = currentFunction;
        __sync_synchronize();

        switch(myFunction){

            case __FUNCTION_1:
                parallel_function_1((void *)tid);
                pthread_barrier_wait(&barrier[__FUNCTION_1]);
                __sw_resetSWitches_1(tid);
                break;

        }

        if(tid == __MAIN_KERNEL)
            break;

    }while(myFunction != -1);



    if(tid != __MAIN_KERNEL)
        pthread_exit(NULL);

}


/*** Application Parallel Function ***/


/*** Reduction Global Variables ***/



void *parallel_function_1(void *arg)
{
    long tid;
    long vTid = 0;
    tid = (long)arg;
    bool ready = FALSE;
    long __sw_tasksCounter = __sw_taskCounter_Function_1[tid];

    /*** Declare private/firstprivate variables ***/

    long i = 0;
    long j = 0;
    long k = 0;
    // Loop task private variables
    long __sw_i               = 0;
    long __sw_loop_start      = 0;
    long __sw_loop_end        = 0;
    long __sw_loop_chunk      = 0;

    do{

	
		        /******************* TASK [ 1 ] *******************/

        switch(tid){
            case __MAIN_KERNEL: vTid = 0; if(ready = (!__sw_task_1[vTid])) break;
                              break;
            case __KERNEL_1: vTid = 1; if(ready = (!__sw_task_1[vTid])) break;
                              break;
            case __KERNEL_2: vTid = 2; if(ready = (!__sw_task_1[vTid])) break;
                              break;
            case __KERNEL_3: vTid = 3; if(ready = (!__sw_task_1[vTid])) break;
                              break;
            default:  ready = FALSE; break;
        }

    if(ready){

        ready = FALSE;

            __sw_loop_start   =  0 + vTid * __LOOP_1_CHUNK;
            __sw_loop_end     =  NRA;

            for (__sw_i = __sw_loop_start; __sw_i < __sw_loop_end; __sw_i += __LOOP_1_COUNTER){

                __sw_loop_chunk = __sw_i + (((__sw_loop_end - __sw_i) >= __LOOP_1_CHUNK) ? __LOOP_1_CHUNK : (__sw_loop_end - __sw_i));

				      for(i = __sw_i;  i < __sw_loop_chunk;  i++)    
			for(j = 0; j < NCB; j++)       
				for(k = 0; k < NCA; k++)
					c[i][j] += a[i][k] * b[k][j];
		            }


        // Update your switch && sync with main memory
        __sw_task_1[vTid] = __ON;
        __sw_tasksCounter--;

        __sync_synchronize();

    }
       if(!__sw_tasksCounter) break;


	

    }while(__sw_tasksCounter);

}

