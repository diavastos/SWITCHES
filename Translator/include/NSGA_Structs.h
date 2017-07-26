/**************************************************/
/*                                                */
/*  File:        NSGA_Structs.h                   */
/*  Description: Declare the structures of the    */
/*               NSGA algorithm                   */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/



/* Structure for command-line of file input data to run the GA */
typedef struct NSGA{
	
	int 	    population;			// Size of the population (default: 50)
	int 	    generations;		// Number of generations to run (default: 100)
	int 	    objectives_size;	// # of Objectives to monitor
	int* 	    objectives;			// Optimize the GA for < 31:Performance | 32:Power | 33:Thermal> (default: 31:Performance)
	double 		mutation;			// Mutation Probability (values: 0.0-1.0, default: 1/# of tasks)
	double 		crossover;			// Crossover Probability (values: 0.6-1.0, default: 0.6)
    char*       path;               // Path to the application
	
}NSGA;


/* Structure of a Child from the Genetic Algorithm */
typedef struct Child{
    
    int         rank;
    int         vector_size;
    int         *sched_vector;
    double      *objectives;
    double      constr_violation;
    double      crowd_dist;
    
    struct Child*   next;
    
}Child;


/* Strcture of a population that hold various children */
typedef struct Population{
    
    Child*      child;
    
}Population;


/* Structure for list that is used by NSGA_assign_rank_and_crowding_distance() */
typedef struct lists
{
    int             index;
    struct lists    *parent;
    struct lists    *child;
}
list;

