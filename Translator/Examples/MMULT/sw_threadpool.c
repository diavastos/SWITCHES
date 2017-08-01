#include "sw.h"


bool __threads_created = FALSE;


/*** Initialize Threadpool ***/

void __threadpool_initialize(int __currentFunction)
{
    int         i = 0;
    __arguments *arguments;

    if(!__threads_created)
    {
        /* Initialize the function barriers */

        pthread_barrier_init(&barrier[0], NULL, __KERNELS);
        pthread_barrier_init(&barrier[__FUNCTION_1], NULL, __KERNELS);


        /* Initialize the kernels in the threadpool */

        for(i = 1; i <= __PTHREADS; i++)
        {
            int rc = 0;
            arguments = (__arguments*)malloc(sizeof(__arguments));
            arguments->id = i;
            arguments->function_id = 0;  // Only Main Kernel defines the currently executed parallel function

            /* Create Pthread and assign its ID */
            rc = pthread_create(&__kernels[i-1], NULL, (void *)thread_jobs, (void *)arguments);
            if(rc)
            {
                ERROR("__threadpool_initialize(): Return code from pthread_create is [ %d ]", rc);
                exit(-1);
            }
        }
        __threads_created = TRUE;
        arguments = (__arguments*)malloc(sizeof(__arguments));
        arguments->id = __MAIN_KERNEL;
        arguments->function_id = __currentFunction;
        thread_jobs((void *)arguments);
    }
    else
    {
        arguments = (__arguments*)malloc(sizeof(__arguments));
        arguments->id = __MAIN_KERNEL;
        arguments->function_id = __currentFunction;
        thread_jobs((void *)arguments);
    }
}


/*** Destroy Threadpool ***/

void __threadpool_destroy()
{
    int i = 0;

    /* Wait for all kernels to finish */

    for(i = 0; i < __PTHREADS; i++)
    {
        pthread_join(__kernels[i], NULL);
    }
}


