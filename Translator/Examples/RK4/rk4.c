#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>


#define SIZE 9600

struct timeval startTime;
struct timeval finishTime;
double timeIntervalLength;


__sw_global__ double *yt;
__sw_global__ double *k1;
__sw_global__ double *k2;
__sw_global__ double *k3;
__sw_global__ double *k4;
__sw_global__ double *yout;
__sw_global__ double totalSum;

__sw_global__ double*  y;
__sw_global__ double*  power;
__sw_global__ double** c;

__sw_global__ double h;
__sw_global__ double sum;

__sw_global__ int i,j;

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

int main(int argc, char* argv[])
{
	
	h=0.3154;
	sum=0;

	//
	//MEMORY ALLOCATION
	//
	y    = (double* )myMalloc(SIZE*sizeof(double) ,1);
	power  = (double* )myMalloc(SIZE*sizeof(double) ,2);
	c    = (double**)myMalloc(SIZE*sizeof(double*),3);
	for (i=0;i<SIZE;i++) 
	{
		c[i]=(double*)myMalloc(SIZE*sizeof(double),4);
	}
	
	yt    = (double*)myMalloc(SIZE*sizeof(double*),4);
	k1    = (double*)myMalloc(SIZE*sizeof(double*),5);
	k2    = (double*)myMalloc(SIZE*sizeof(double*),6);
	k3    = (double*)myMalloc(SIZE*sizeof(double*),7);
	k4    = (double*)myMalloc(SIZE*sizeof(double*),8);
	yout  = (double*)myMalloc(SIZE*sizeof(double*),9);

	

	
	//
	//INITIALIZATION
	//
	for (i = 0; i < SIZE; i++) 
	{
		y[i]=i*i;
		power[i]=i+i;
		for (j = 0; j < SIZE; j++)
		{
			c[i][j]=i*i+j;
		}
	}

	

	// Start timers
	gettimeofday(&startTime, NULL);


	#pragma omp parallel for schedule(static, 32) default(shared) private(i,j)
	{
		for (i = 0; i < SIZE; i++) 
		{ 
			yt[i] = 0.0;
			for (j = 0; j < SIZE; j++) 
				yt[i] += c[i][j]*y[j];
			k1[i] = h*(power[i]-yt[i]);
		}
	}
	
	#pragma omp parallel for schedule(static, 32) default(shared) private(i,j)
	{
		for (i = 0; i < SIZE; i++) 
		{
			yt[i] = 0.0;
			for (j = 0; j < SIZE; j++) 
				yt[i] += c[i][j]*(y[j]+0.5*k1[j]);
			k2[i] = h*(power[i]-yt[i]);
		}
	}


	#pragma omp parallel for schedule(static, 32) default(shared) private(i,j)
	{
		for (i = 0; i < SIZE; i++) 
		{
			yt[i] = 0.0;
			for (j = 0; j < SIZE; j++) 
				yt[i] += c[i][j]*(y[j]+0.5*k2[j]);
			k3[i] = h*(power[i]-yt[i]);
		}
	}


	#pragma omp parallel for schedule(static, 32) default(shared) private(i,j) reduction(+:sum)
	{
		for (i =0; i < SIZE; i++) 
		{
			yt[i]=0.0;
			for (j = 0; j < SIZE; j++) 
				yt[i] += c[i][j]*(y[j]+k3[j]);
			k4[i] = h*(power[i]-yt[i]);

			yout[i] = y[i] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i])/6.0;
			sum+=yout[i];
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
	

	printf("\n\nTotalSum=%g\n\n",sum);
	fflush(stdout);

	

	return 0;
}
