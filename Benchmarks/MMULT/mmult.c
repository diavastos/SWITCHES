#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#define NRA 2048                 /* number of rows in matrix A */
#define NCA 2048                 /* number of columns in matrix A */
#define NCB 2048                  /* number of columns in matrix B */


struct timeval startTime;
struct timeval finishTime;
double timeIntervalLength;

__sw_global__ double	**a;		/* [NRA][NCA] */
__sw_global__ double	**b;		/* [NCA][NCB] */
__sw_global__ double    **c;		/* [NRA][NCB] */
__sw_global__ double 	sum;

void* myMalloc(int size, int info)
{
	void* t = (void*)malloc(size);

	if(!t)
	{
		printf("\nMemory allocation error [%d]",info);
		fflush(stdout);
		exit(0);
	}
	
	return t;
}


int main (int argc, char *argv[]) 
{
	__sw_global__ long		i, j, k;
	sum = 0;
	
	
	a    = (double**)myMalloc(NRA*sizeof(double*),1);
	for (i=0;i<NCA;i++) 
		a[i]=(double*)myMalloc(NCA*sizeof(double),2);
	b    = (double**)myMalloc(NCA*sizeof(double*),3);
	for (i=0;i<NCB;i++) 
		b[i]=(double*)myMalloc(NCB*sizeof(double),4);
	c    = (double**)myMalloc(NRA*sizeof(double*),5);
	for (i=0;i<NCB;i++) 
		c[i]=(double*)myMalloc(NCB*sizeof(double),6);

  
	/*** Initialize matrices ***/
	for(i = 0; i < NRA; i++)
		for(j = 0; j < NCA; j++)
			a[i][j] = i + j;

	for(i = 0; i < NCA; i++)
		for(j = 0; j < NCB; j++)
			b[i][j] = i * j;

	for(i = 0; i < NRA; i++)
		for(j = 0; j < NCB; j++)
			c[i][j] = 0;

		
	// Start timers
	gettimeofday(&startTime, NULL);



	#pragma omp parallel private (i, j ,k) 
	{
		#pragma omp for schedule (static, 8)
		{
		  for(i = 0; i < NRA; i++)    
			for(j = 0; j < NCB; j++)       
				for(k = 0; k < NCA; k++)
					c[i][j] += a[i][k] * b[k][j];
		}
	}

	
 
	// End timers
 	gettimeofday(&finishTime, NULL);


	//Calculate the interval length 
	timeIntervalLength = (double)(finishTime.tv_sec-startTime.tv_sec) * 1000000 
	                     + (double)(finishTime.tv_usec-startTime.tv_usec);
	timeIntervalLength=timeIntervalLength/1000;

	//Print the interval lenght
	printf("__aid_Time: %g msec.\n", timeIntervalLength);

	/*** Print results ***/
	for(i = 0; i < NRA; i++)
		for(j = 0; j < NCB; j++) 
			sum += c[i][j];
  

	printf("__aid_Result: %g\n\n", sum);

	return 0;

}
