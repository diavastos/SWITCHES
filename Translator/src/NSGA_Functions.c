/**************************************************/
/*                                                */
/*  File:        NSGA_Functions.c                 */
/*  Description: All NSGA Functions               */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/


#include "definitions.h"


extern NSGA     *nsga;
double          seed = SEED;
double          oldrand[RANDOM_NUM];
int             jrand;

extern int      kernels;
extern int      totalKernels;
extern double   *min_var;
extern double   *max_var;
extern int      maxCores;
extern int      hThreads;
extern int      OSthread;


/********** Allocate memory for populations ***********/

void allocatePopulation(Population** population, int population_size){
    
    
    int i = 0, j = 0;
    Child*  tempChild = NULL;
    

    *population = (Population *)malloc(sizeof(Population));
    (*population)->child  = NULL;
    
    // Initialize with 0s the allocated values
    
    for(i = 0; i < population_size; i++)
    {        
        if(!((*population)->child))
        {
            // Allocate Child
            (*population)->child = (Child*)malloc(sizeof(Child));
            
            // Allocate Rank
            (*population)->child->rank = 0;
            
            // Allocate + Initialize Vector Size
            (*population)->child->vector_size   =   totalKernels;
            
            // Allocate Vector
            (*population)->child->sched_vector  =   (int*)malloc(sizeof(int) * totalKernels);
            for(j = 0; j < totalKernels; j++)
                (*population)->child->sched_vector[j] = -1;
            
            // Allocate Objectives
            (*population)->child->objectives    =   (double*)malloc(sizeof(double) * nsga->objectives_size);
            for(j = 0; j < nsga->objectives_size; j++)
                (*population)->child->objectives[j]   = 0;
            
            //Allocate + Initialize Constraint Violation
            (*population)->child->constr_violation   =   0.0;
            
            //Allocate + Initialize Crowding Distance
            (*population)->child->crowd_dist   =   0.0;
            
            (*population)->child->next          =   NULL;
        }
        else
        {
            tempChild = (*population)->child;
            
            while(tempChild && tempChild->next)
                tempChild = tempChild->next;
            
            // Allocate Child
            tempChild->next = (Child*)malloc(sizeof(Child));
            
            // Allocate Rank
            tempChild->next->rank = 0;
            
            // Allocate + Initialize Vector Size
            tempChild->next->vector_size   =   totalKernels;
            
            // Allocate Scheduling Vector
            tempChild->next->sched_vector  =   (int*)malloc(sizeof(int) * totalKernels);
            for(j = 0; j < totalKernels; j++)
                tempChild->next->sched_vector[j] = -1;
            
            // Allocate Objectives
            tempChild->next->objectives    =   (double*)malloc(sizeof(double) * nsga->objectives_size);
            for(j = 0; j < nsga->objectives_size; j++)
                tempChild->next->objectives[j]   = -1;
                
            //Allocate + Initialize Constraint Violation
            tempChild->next->constr_violation   =   0.0;
            
            //Allocate + Initialize Crowding Distance
            tempChild->next->crowd_dist   =   0.0;
            
            tempChild->next->next          =   NULL;
            
        }
    }
}



/********** Allocate memory of a population ***********/

void deallocatePopulation(Population** population){
    
    free(population);    
}



/********** Get seed number for random and start it up ***********/

void randomize(){
    int j = 0;
    
    for(j = 0; j < RANDOM_NUM; j++)
        oldrand[j] = 0.0;
    
    jrand = 0;
    warmup_random(seed);
}



/********** Get randomize off and running ***********/

void warmup_random(double seed){
    int j = 0, i = 0;
    double new_random, prev_random;
    
    oldrand[RANDOM_NUM-1] = seed;
    new_random            = 0.000000001;
    prev_random           = seed;
    
    for(j = 1; j < RANDOM_NUM; j++)
    {
        i           = (21 * j) % 54;
        oldrand[i]  = new_random;
        new_random  = prev_random - new_random;
        if(new_random < 0.0)
            new_random += 1.0;
        prev_random = oldrand[i];
    }
    
    advance_random();
    advance_random();
    advance_random();
    jrand = 0;
}



/********** Create next batch of RANDOM_NUM random numbers ***********/

void advance_random(){
    int j;
    double new_random;
    
    for(j = 0; j < 24; j++)
    {
        new_random = oldrand[j] - oldrand[j + 31];
        
        if(new_random < 0.0)
            new_random += 1.0;
            
        oldrand[j] = new_random;
    }
    
    for(j = 24; j < RANDOM_NUM; j++)
    {
        new_random = oldrand[j] - oldrand[j - 24];
        if(new_random < 0.0)
            new_random += 1.0;
            
        oldrand[j] = new_random;
    }
}



/********** Fetch a single random number between 0.0 and 1.0 ***********/

double randomperc(){
    jrand++;
    
    if(jrand >= (RANDOM_NUM-1))
    {
        jrand = 1;
        advance_random();
    }
    
    return((double)oldrand[jrand]);
}



/********** Fetch a single random integer between low and high including the bounds ***********/

int rnd(int low, int high){
    int res;
    if (low >= high)
        res = low;
    else
    {
        res = low + (randomperc() * (high - low + 1));
        if (res > high)
            res = high;
    }
    
    return res;
}



/********** Randomly initialize population ***********/

void NSGA_initializePopulation(SG** Graph, Population** population){
    
    int                 i = 0, j = 0, k = 0, a = 0;
    SG 					*tempGraph = *Graph;
	parallel_function 	*tempFunction;
	section 			*tempSection;
    task                *tempTask;
    kernel				**tempKernel;
    Child               *tempChild = (*population)->child;
    
    int					kernelAvailability[kernels];
    bool				reassignFlag = FALSE;
    int                 temp = -1;

    
    for(j = 0; j < nsga->population; j++)
    {
        k = 0;
        tempGraph = *Graph;
        
        if(j == 0)      /* Add in the initial population the SWITCHES Predifined scheduling policies (RoundRobin)*/
        {
            
            // Call the RR scheduling function to assign the policy to the Graph -- 
            //  This will only be used for the next step and then it will be ingored
            offlineScheduling_AssignKernelsToParallelFunctions(Graph);
            offlineScheduling_AssignKernelsToTasks_RoundRobin(Graph);
            
            // Use the Graph here to copy to the 1st child the RR policy
            
            while(tempGraph)
            {
                tempFunction = tempGraph->parallel_functions;
                while(tempFunction)
                {
                    tempTask = tempFunction->tasks;
                    while(tempTask)
                    {
                        tempKernel = &(tempTask->kernels);
                        for(i = 0; i < tempTask->number_of_kernels; i++)
                        {
                            tempChild->sched_vector[k++] = (*tempKernel)->id;
                            tempKernel = &(*tempKernel)->next;
                        }
                        tempTask = tempTask->next;
                    }
                    
                    // Check section tasks
                    tempSection = tempFunction->sections;
                    while(tempSection)
                    {
                        tempTask = tempSection->tasks;
                        while(tempTask)
                        {
                            tempKernel = &(tempTask->kernels);
                            for(i = 0; i < tempTask->number_of_kernels; i++)
                            {
                                tempChild->sched_vector[k++] = (*tempKernel)->id;
                                tempKernel = &(*tempKernel)->next;
                            }
                            tempTask = tempTask->next;
                        }
                        tempSection = tempSection->next;	
                    }						
                    tempFunction = tempFunction->next;
                }
                tempGraph = tempGraph->next;		
            }
        }
        else                        /* NSGA Population Initialization */
        {
            while(tempGraph)
            {
                tempFunction = tempGraph->parallel_functions;
                while(tempFunction)
                {
                    reassignFlag = FALSE;
                    for(a = 0; a < kernels; a++)
                        kernelAvailability[a] = TRUE;
                    
                    tempTask = tempFunction->tasks;
                    while(tempTask)
                    {
                        for(i = 0; i < tempTask->number_of_kernels; i++)
                        {
                            if(tempTask->taskType == TASK_MASTER)
                            {
                                tempChild->sched_vector[k++] = 0;
                                kernelAvailability[0] = FALSE;
                            }
                            else
                            {
                                do{
                                    temp = rnd(0, ((maxCores * hThreads) - 1));
                                    if(kernelAvailability[temp] == TRUE  || reassignFlag == TRUE)
                                    {
                                        tempChild->sched_vector[k++] = temp;
                                        kernelAvailability[temp] = FALSE;
                                        break;
                                    }
                                }while(1);
                            }
                            
                            for(a = 0; a < kernels; a++)
                            {
                                if(kernelAvailability[a] == TRUE)
                                {
                                    reassignFlag = FALSE;
                                    break;
                                }    
                                reassignFlag = TRUE;
                            }
                            
                        }
                        tempTask = tempTask->next;
                    }
                    
                    // Check section tasks
                    tempSection = tempFunction->sections;
                    while(tempSection)
                    {
                        tempTask = tempSection->tasks;
                        while(tempTask)
                        {
                            for(i = 0; i < tempTask->number_of_kernels; i++)
                            {
                                if(tempTask->taskType == TASK_MASTER)
                                {
                                    tempChild->sched_vector[k++] = 0;
                                    kernelAvailability[0] = FALSE;
                                }
                                else
                                {
                                    do{
                                        temp = rnd(0, ((maxCores * hThreads) - 1));
                                        if(kernelAvailability[temp] == TRUE || reassignFlag == TRUE)
                                        {
                                            tempChild->sched_vector[k++] = temp;
                                            kernelAvailability[temp] = FALSE;
                                            break;
                                        }
                                    }while(1);
                                }
                                
                                for(a = 0; a < kernels; a++)
                                {
                                    if(kernelAvailability[a] == TRUE)
                                    {
                                        reassignFlag = FALSE;
                                        break;
                                    }    
                                    reassignFlag = TRUE;
                                }
                            }
                            tempTask = tempTask->next;
                        }
                        tempSection = tempSection->next;	
                    }						
                    tempFunction = tempFunction->next;
                }
                tempGraph = tempGraph->next;		
            } 
        }
        tempChild = tempChild->next;
    }
     
     
    /* SOLUTION 2 BUT DOESN'T TAKE INTO ACCOUNT THE TASKS TO BE EXECUTED BY MASTER_THREAD
     * 
     for(i = 0; i < nsga->population; i++)
    {
        for (j = ; j < totalKernels; j++)
        {
            tempChild->sched_vector[j] = rnd(0, ((maxCores * hThreads) - 1));
        }
               
        tempChild = tempChild->next;
    }
    */
}



/********** Print population to scheduling files ***********/

void NSGA_printPopulationToFiles(Population** population, int generation){
    
    int    i = 0, j = 0;
    Child  *tempChild = (*population)->child;
    FILE*  outpChild;
    char   buff[1000];
    char   tempBuff[100];
    
    for(i = 1; i <= nsga->population && tempChild; i++)
    {
        outpChild = NULL;
        bzero(buff, sizeof(buff));
        strcat(buff, nsga->path);
        bzero(tempBuff, sizeof(tempBuff));
        sprintf(tempBuff, "/Generations/Gen%d/g%d-c%d", generation, generation, i);
        strcat(buff, tempBuff);
        
        outpChild = fopen(buff, "w");
        if(!outpChild){
            ERROR_COMMANDS("Child file [ %s ] not opened!", buff)
            exit(-1);
        }
        
        for (j = 0; j < totalKernels; j++)
            fprintf(outpChild, "%d\n", tempChild->sched_vector[j]);
        
        fclose(outpChild);
        tempChild = tempChild->next;
    }  
}



/********** Evaluate Population ***********/

void NSGA_evaluatePopulation(Population** population, int generation){
    
    int     i = 0, j = 0;
    Child   *tempChild = (*population)->child;
	FILE*   inpChild;
    char    buff[1000];
    char    tempBuff[100];
    int     dummy = 0;
    
        /** 
         *  RUN THE APPLICATION HERE IF YOU REMOVE THE SCRIPT AFTERALL 
         *  USE system() and pop*() to execute the Translato and the translated program respectively
         *      - Translator needed parameters are stored in the NSGA struct
         *
         **/
    
    
    
    // Read in the .res files with the objectives results
    
    for(j = 0; j < nsga->objectives_size; j++)
    {
        tempChild = (*population)->child;
        for(i = 1; i <= nsga->population && tempChild; i++)
        {
            // Load all children results (.res files)
            inpChild = NULL;
            bzero(buff, sizeof(buff));
            bzero(tempBuff, sizeof(tempBuff));
            strcat(buff, nsga->path);
            
            switch(nsga->objectives[j]){
                
                /* Update NSGA Structs objective results: TIME */
                case NSGA_OBJECTIVE_PERFORMANCE:
                            
                            sprintf(tempBuff, "/Results/Gen%d/g%d-c%d-time.res", generation, generation, i);
                            strcat(buff, tempBuff);
            
                            do{
                                inpChild = fopen(buff, "r");
                            }while(!inpChild);
            
                            dummy = fscanf(inpChild, "%lf", &tempChild->objectives[j]);
                            fclose(inpChild);
                
                            break;
                
                /* Update NSGA Structs objective results: POWER */
                case NSGA_OBJECTIVE_POWER:
                
                            sprintf(tempBuff, "/Results/Gen%d/g%d-c%d-pow.res", generation, generation, i);
                            strcat(buff, tempBuff);
                        
                            do{
                                inpChild = fopen(buff, "r");
                            }while(!inpChild);
                            
                            dummy = fscanf(inpChild, "%lf", &tempChild->objectives[j]);
                            fclose(inpChild);
                            
                            break;
                
                /* Update NSGA Structs objective results: THERMAL */
                case NSGA_OBJECTIVE_THERMAL:
                
                            sprintf(tempBuff, "/Results/Gen%d/g%d-c%d-thermal.res", generation, generation, i);
                            strcat(buff, tempBuff);
                        
                            do{
                                inpChild = fopen(buff, "r");
                            }while(!inpChild);
                            
                            dummy = fscanf(inpChild, "%lf", &tempChild->objectives[j]);
                            fclose(inpChild);
                            
                            break;                
                
                default:
                        ERROR_COMMANDS("GA Objective [ %d ] not recognized!", nsga->objectives[j])
                        exit(-1);
                
            }
            tempChild = tempChild->next;
        }
    }
    
    /* PRINT THE OBJECTIVES READ FROM THE FILES FOR DEBUGGING
     * 
    tempChild = (*population)->child;
    
     for(i = 0; i < nsga->population; i++)
     {
        for(j = 0; j < nsga->objectives_size; j++)
            fprintf(stderr, "%lf ", tempChild->objectives[j]);
     
        fprintf(stderr, "\n");
        tempChild = tempChild->next;   
     }
     *
     */
    
}



/********** Function to assign rank and crowding distance to a population ***********/

void NSGA_assign_rank_and_crowding_distance(Population** population){
    
    int flag = 0, i, end, front_size, rank = 1;
    list *orig, *cur, *temp1, *temp2;
    int popsize = nsga->population;
    
    Child *tempChild1 = (*population)->child;
    Child *tempChild2 = (*population)->child;
    
    orig = (list *)malloc(sizeof(list));
    cur  = (list *)malloc(sizeof(list));
    
    front_size      = 0;
    orig->index     = -1;
    orig->parent    = NULL;
    orig->child     = NULL;
    cur->index      = -1;
    cur->parent     = NULL;
    cur->child      = NULL;
    temp1           = orig;
    
    
    for(i = 0; i < popsize; i++)
    {
        insert(temp1, i);
        temp1 = temp1->child;
    }
    
    do{
        if(orig->child->child == NULL)
        {
            tempChild1 = (*population)->child;
            for(i = 0; i < orig->child->index; i++)
                tempChild1 = tempChild1->next;
                
            tempChild1->rank = rank;
            tempChild1->crowd_dist = INF;
            break;
        }
        
        temp1 = orig->child;
        insert(cur, temp1->index);
        front_size = 1;
        temp2 = cur->child;
        temp1 = del(temp1);
        temp1 = temp1->child;
        
        do{
            temp2 = cur->child;
            do{
                end = 0;
                
                tempChild1 = (*population)->child;
                for(i = 0; i < temp1->index; i++)
                    tempChild1 = tempChild1->next;
                    
                tempChild2 = (*population)->child;
                for(i = 0; i < temp2->index; i++)
                    tempChild2 = tempChild2->next;
                
                flag = check_dominance(tempChild1, tempChild2);
                
                if(flag == 1)
                {
                    insert(orig, temp2->index);
                    temp2 = del (temp2);
                    front_size--;
                    temp2 = temp2->child;
                }
                
                if(flag == 0)
                    temp2 = temp2->child;
                    
                if(flag == -1)
                    end = 1;
                
            }while(end != 1 && temp2 != NULL);
            
            if(flag == 0 || flag == 1)
            {
                insert(cur, temp1->index);
                front_size++;
                temp1 = del (temp1);
            }
            
            temp1 = temp1->child;
            
        }while(temp1 != NULL);
        
        temp2 = cur->child;
        do{
            tempChild2 = (*population)->child;
            for(i = 0; i < temp2->index; i++)
                tempChild2 = tempChild2->next;
            
            tempChild2->rank = rank;
            temp2 = temp2->child;
            
        }while(temp2 != NULL);
        
        assign_crowding_distance_list((*population), cur->child, front_size);
        
        temp2 = cur->child;
        
        do{
            temp2 = del (temp2);
            temp2 = temp2->child;
        }while(cur->child != NULL);
        
        rank+=1;
        
    }while(orig->child != NULL);
    
    free (orig);
    free (cur);    
}



/********** Routine for tournament selection, it creates a new_pop from old_pop by performing TOURNAMENT SELECTION and CROSSOVER ***********/

void NSGA_selection(Population *old_pop, Population *new_pop){
    
    int *a1, *a2;
    int temp;
    int i, j;
    int rand;
    Child *parent1, *parent2;
    
    Child *tempChild1 = NULL;
    Child *tempChild2 = NULL;
    Child *tempChild3 = NULL;
    Child *tempChild4 = NULL;
    
    
    a1 = (int *)malloc(nsga->population * sizeof(int));
    a2 = (int *)malloc(nsga->population * sizeof(int));
    
    for(i = 0; i < nsga->population; i++)
        a1[i] = a2[i] = i;
        
    for(i = 0; i < nsga->population; i++)
    {
        rand     = rnd(i, nsga->population-1);
        temp     = a1[rand];
        a1[rand] = a1[i];
        a1[i]    = temp;
        rand     = rnd(i, nsga->population-1);
        temp     = a2[rand];
        a2[rand] = a2[i];
        a2[i]    = temp;
    }
    
    for(i = 0; i < nsga->population; i+=4)
    {
        // Crossover Procedure #1
        
        tempChild1 = old_pop->child;
        for(j = 0; j < a1[i]; j++)
            tempChild1 = tempChild1->next;
            
        tempChild2 = old_pop->child;
        for(j = 0; j < a1[i+1]; j++)
            tempChild2 = tempChild2->next;
        
        tempChild3 = old_pop->child;
        for(j = 0; j < a1[i+2]; j++)
            tempChild3 = tempChild3->next;
            
        tempChild4 = old_pop->child;
        for(j = 0; j < a1[i+3]; j++)
            tempChild4 = tempChild4->next;
        
        parent1 = tournament(tempChild1, tempChild2);
        parent2 = tournament(tempChild3, tempChild4);
        
        tempChild1 = new_pop->child;
        for(j = 0; j < i; j++)
            tempChild1 = tempChild1->next;
            
        tempChild2 = new_pop->child;
        for(j = 0; j < i+1; j++)
            tempChild2 = tempChild2->next;
        
        crossover(parent1, parent2, tempChild1, tempChild2);
        
        
        // Crossover Procedure #2
        
        tempChild1 = old_pop->child;
        for(j = 0; j < a2[i]; j++)
            tempChild1 = tempChild1->next;
            
        tempChild2 = old_pop->child;
        for(j = 0; j < a2[i+1]; j++)
            tempChild2 = tempChild2->next;
        
        tempChild3 = old_pop->child;
        for(j = 0; j < a2[i+2]; j++)
            tempChild3 = tempChild3->next;
            
        tempChild4 = old_pop->child;
        for(j = 0; j < a2[i+3]; j++)
            tempChild4 = tempChild4->next;
        
        parent1 = tournament(tempChild1, tempChild2);
        parent2 = tournament(tempChild3, tempChild4);
        
        tempChild1 = new_pop->child;
        for(j = 0; j < i+2; j++)
            tempChild1 = tempChild1->next;
            
        tempChild2 = new_pop->child;
        for(j = 0; j < i+3; j++)
            tempChild2 = tempChild2->next;
        
        crossover(parent1, parent2, tempChild1, tempChild2);
    }
    
    free (a1);
    free (a2);
}



/********** Routine for BINARY TOURNAMENT ***********/

Child* tournament(Child *chld1, Child *chld2){
    
    int flag;
    
    flag = check_dominance(chld1, chld2);
    
    if(flag == 1)
        return chld1;
        
    if(flag == -1)
        return chld2;
        
    if(chld1->crowd_dist > chld2->crowd_dist)
        return chld1;

    if(chld2->crowd_dist > chld1->crowd_dist)
        return chld2;

    if((randomperc()) <= 0.5)
        return chld1;
    else
        return chld2;
}



/********** Function to cross two individuals ***********/

void crossover(Child *parent1, Child *parent2, Child *child1, Child *child2){
    
    int j;
    double rand;
    int temp, site1, site2;
    
    rand = randomperc();
    if(rand <= nsga->crossover)
    {
        site1 = rnd(0, totalKernels-1);
        site2 = rnd(0, totalKernels-1);
        
        if(site1 > site2)
        {
            temp = site1;
            site1 = site2;
            site2 = temp;
        }
        
        for(j = 0; j < site1; j++)
        {
            child1->sched_vector[j] = parent1->sched_vector[j];
            child2->sched_vector[j] = parent2->sched_vector[j];
        }
        
        for(j = site1; j < site2; j++)
        {
            child1->sched_vector[j] = parent2->sched_vector[j];
            child2->sched_vector[j] = parent1->sched_vector[j];
        }
        
        for(j = site2; j < totalKernels; j++)
        {
            child1->sched_vector[j] = parent1->sched_vector[j];
            child2->sched_vector[j] = parent2->sched_vector[j];
        }
    }
    else
    {
        for(j = 0; j < totalKernels; j++)
        {
            child1->sched_vector[j] = parent1->sched_vector[j];
            child2->sched_vector[j] = parent2->sched_vector[j];
        }
    }
}



/********** Function to perform mutation in a population ***********/

void NSGA_mutation(Population **population){
    
    int     i = 0, j = 0;
    double  prob;
    Child   *tempChild = (*population)->child;
    
    for(i = 0; i < nsga->population && tempChild; i++)
    {
        for(j = 0; j < totalKernels; j++)
        {
            prob = randomperc();
            if(prob <= nsga->mutation)
                tempChild->sched_vector[j] = rnd(0, ((maxCores * hThreads) - 1));
        }
        tempChild = tempChild->next;
    }
    
}



/********** Insert an element into the list at location specified by NODE ***********/

void insert(list *node, int x){
    
    list *temp;
    
    if(node == NULL)
    {
        ERROR_COMMANDS("Asked to enter a NULL pointer in [ %s ]. Exiting... !", "Crowding List")
        exit(1);
    }
    
    temp = (list *)malloc(sizeof(list));
    temp->index = x;
    temp->child = node->child;
    temp->parent = node;
    
    if(node->child != NULL)
        node->child->parent = temp;
    
    node->child = temp;
}



/********** Delete the node NODE from the list ***********/

list* del(list *node){
    
    list *temp;
    
    if(node == NULL)
    {
        ERROR_COMMANDS("Asked to delete a NULL pointer in [ %s ]. Exiting... !", "Crowding List")
        exit(1);
    }
    
    temp = node->parent;
    temp->child = node->child;
    
    if(temp->child != NULL)
        temp->child->parent = temp;
    
    free(node);
    
    return temp;
}



/********** Routine for usual non-domination checking
            It will return the following values:
                1 if a dominates b
               -1 if b dominates a
                0 if both a and b are non-dominated ***********/

int check_dominance(Child *a, Child *b){
    
    int i, flag1 = 0, flag2 = 0;
    
    if(a->constr_violation < 0 && b->constr_violation < 0)
    {
        if(a->constr_violation > b->constr_violation)
            return 1;
        else
        {
            if(a->constr_violation < b->constr_violation)
                return -1;
            else
                return 0;
        }
    }
    else
    {
        if(a->constr_violation < 0 && b->constr_violation == 0)
            return -1;
        else
        {
            if(a->constr_violation == 0 && b->constr_violation < 0)
                return 1;
            else
            {
                for(i = 0; i < nsga->objectives_size; i++)
                {
                    if(a->objectives[i] < b->objectives[i])
                    {
                        flag1 = 1;
                    }
                    else
                    {
                        if(a->objectives[i] > b->objectives[i])
                            flag2 = 1;
                    }
                }
                
                if(flag1 == 1 && flag2 == 0)
                    return 1;
                else
                {
                    if(flag1 == 0 && flag2 == 1)
                        return -1;
                    else
                        return 0;
                }
            }
        }
    }
}



/********** Routine to compute crowding distance based on ojbective function values when the population in the form of a list ***********/

void assign_crowding_distance_list(Population *population, list *lst, int front_size){
    
    int **obj_array;
    int *dist;
    int i, j;
    list *temp;
    
    Child *tempChild1 = population->child;
    Child *tempChild2 = population->child;
    
    temp = lst;
    
    if(front_size == 1)
    {
        tempChild1 = population->child;
        for(i = 0; i < lst->index; i++)
            tempChild1 = tempChild1->next;
        
        tempChild1->crowd_dist = INF;
        return;
    }
    
    if(front_size == 2)
    {
        tempChild1 = population->child;
        for(i = 0; i < lst->index; i++)
            tempChild1 = tempChild1->next;
            
        tempChild2 = population->child;
        for(i = 0; i < lst->child->index; i++)
            tempChild2 = tempChild2->next;
        
        tempChild1->crowd_dist = INF;
        tempChild2->crowd_dist = INF;
        return;
    }
    
    obj_array = (int**)malloc(nsga->objectives_size * sizeof(int*));
    dist = (int*)malloc(front_size * sizeof(int));
    
    for(i = 0; i < nsga->objectives_size; i++)
        obj_array[i] = (int*)malloc(front_size * sizeof(int));
        
    for(j = 0; j < front_size; j++)
    {
        dist[j] = temp->index;
        temp = temp->child;
    }
    
    assign_crowding_distance(population, dist, obj_array, front_size);
    free(dist);
    
    for(i = 0; i < nsga->objectives_size; i++)
        free(obj_array[i]);
        
    free(obj_array);
}



/********** Routine to compute crowding distances ***********/

void assign_crowding_distance(Population *population, int *dist, int **obj_array, int front_size){
    
    int i = 0, j = 0, k = 0;
    
    Child *tempChild1 = population->child;
    Child *tempChild2 = population->child;
    Child *tempChild3 = population->child;
    Child *tempChild4 = population->child;
    Child *tempChild5 = population->child;
    
    
    for(i = 0; i < nsga->objectives_size; i++)
    {
        for(j = 0; j < front_size; j++)
            obj_array[i][j] = dist[j];
            
        quicksort_front_obj(population, i, obj_array[i], front_size);
    }
    
    for(j = 0; j < front_size; j++)
    {
        tempChild1 = population->child;
        for(i = 0; i < dist[j]; i++)
            tempChild1 = tempChild1->next;
            
        tempChild1->crowd_dist = 0.0;
    }
        
        
    for(i = 0; i < nsga->objectives_size; i++)
    {
        tempChild1 = population->child;
        for(k = 0; k < obj_array[i][0]; k++)
            tempChild1 = tempChild1->next;
        
        tempChild1->crowd_dist = INF;
    }

    for(i = 0; i < nsga->objectives_size; i++)
    {
        for(j = 1; j < front_size-1; j++)
        {
            tempChild1 = population->child;
            for(k = 0; k < obj_array[i][j]; k++)
                tempChild1 = tempChild1->next;
            
            if (tempChild1->crowd_dist != INF)
            {
                tempChild2 = population->child;
                for(k = 0; k < obj_array[i][front_size-1]; k++)
                    tempChild2 = tempChild2->next;
                    
                tempChild3 = population->child;
                for(k = 0; k < obj_array[i][0]; k++)
                    tempChild3 = tempChild3->next;
                
                if (tempChild2->objectives[i] == tempChild3->objectives[i])
                {
                    tempChild4 = population->child;
                    for(k = 0; k < obj_array[i][j]; k++)
                        tempChild4 = tempChild4->next;
                        
                    tempChild4->crowd_dist += 0.0;
                }
                else
                {
                    tempChild1 = population->child;
                    for(k = 0; k < obj_array[i][j]; k++)
                        tempChild1 = tempChild1->next;
                    
                    tempChild2 = population->child;
                    for(k = 0; k < obj_array[i][j+1]; k++)
                        tempChild2 = tempChild2->next;
                        
                    tempChild3 = population->child;
                    for(k = 0; k < obj_array[i][j-1]; k++)
                        tempChild3 = tempChild3->next;
                        
                    tempChild4 = population->child;
                    for(k = 0; k < obj_array[i][front_size-1]; k++)
                        tempChild4 = tempChild4->next;
                        
                    tempChild5 = population->child;
                    for(k = 0; k < obj_array[i][0]; k++)
                        tempChild5 = tempChild5->next;
                    
                    tempChild1->crowd_dist += (tempChild2->objectives[i] - tempChild3->objectives[i]) / (tempChild4->objectives[i] - tempChild5->objectives[i]);
                }
            }
        }
    }
    
    for(j = 0; j < front_size; j++)
    {
        tempChild1 = population->child;
        for(i = 0; i < dist[j]; i++)
            tempChild1 = tempChild1->next;
                    
        if(tempChild1->crowd_dist != INF)
            tempChild1->crowd_dist = (tempChild1->crowd_dist)/nsga->objectives_size;
    }
}



/********** Randomized quick sort routine to sort a population based on a particular objective chosen ***********/

void quicksort_front_obj(Population *population, int objcount, int obj_array[], int obj_array_size){
    
    q_sort_front_obj(population, objcount, obj_array, 0, obj_array_size-1);
}



/********** Actual implementation of the randomized quick sort used to sort a population based on a particular objective chosen ***********/

void q_sort_front_obj(Population *population, int objcount, int obj_array[], int left, int right){
    
    int index, temp, i, j, k;
    double pivot;
    
    Child *tempChild1 = population->child;
    
    if(left < right)
    {
        index = rnd(left, right);
        temp = obj_array[right];
        obj_array[right] = obj_array[index];
        obj_array[index] = temp;
        
        tempChild1 = population->child;
        for(k = 0; k < obj_array[right]; k++)
            tempChild1 = tempChild1->next;
        
        pivot = tempChild1->objectives[objcount];
        i = left-1;
        
        for(j = left; j < right; j++)
        {
            tempChild1 = population->child;
            for(k = 0; k < obj_array[j]; k++)
                tempChild1 = tempChild1->next;
            
            if(tempChild1->objectives[objcount] <= pivot)
            {
                i+=1;
                temp = obj_array[j];
                obj_array[j] = obj_array[i];
                obj_array[i] = temp;
            }
        }
        
        index=i+1;
        temp = obj_array[index];
        obj_array[index] = obj_array[right];
        obj_array[right] = temp;
        
        q_sort_front_obj(population, objcount, obj_array, left, index-1);
        q_sort_front_obj(population, objcount, obj_array, index+1, right);
    }
}



/********** Routine to merge two populations into one ***********/

void NSGA_merge(Population **pop1, Population **pop2, Population **pop3){
    
    int i, k;
    Child   *tempChild1 = (*pop1)->child;
    Child   *tempChild2 = (*pop2)->child;
    Child   *tempChild3 = (*pop3)->child;
    
    for(i = 0; i < nsga->population; i++)
    {
        copy_child(tempChild1, tempChild3);
        tempChild1 = tempChild1->next;
        tempChild3 = tempChild3->next;
    }
        
    for(i = 0; i < nsga->population; i++)
    {
        copy_child(tempChild2, tempChild3);
        tempChild2 = tempChild2->next;
        tempChild3 = tempChild3->next;
    }
        
}



/********** Routine to copy 'child1' into 'child2' ***********/

void copy_child(Child *child1, Child *child2){
    
    int i;
    
    child2->rank             = child1->rank;
    child2->constr_violation = child1->constr_violation;
    child2->crowd_dist       = child1->crowd_dist;
 
    for(i = 0; i < totalKernels; i++)
        child2->sched_vector[i] = child1->sched_vector[i];
  
    for(i = 0; i < nsga->objectives_size; i++)
        child2->objectives[i] = child1->objectives[i];
    
}



/********** Routine to perform non-dominated sorting ***********/

void NSGA_fill_nondominated_sort(Population **mixed_pop, Population **new_pop){
    
    int flag, i, j, k, end, front_size = 0, archieve_size = 0, rank=1;
    int popsize = (int)nsga->population;
    
    list *pool;
    list *elite;
    list *temp1, *temp2;
    
    Child *tempChild1 = NULL;
    Child *tempChild2 = NULL;
    
    pool = (list*)malloc(sizeof(list));
    elite = (list*)malloc(sizeof(list));
    
    pool->index = -1;
    pool->parent = NULL;
    pool->child = NULL;
    elite->index = -1;
    elite->parent = NULL;
    elite->child = NULL;
    temp1 = pool;
    
    for(i = 0; i < 2*popsize; i++)
    {
        insert(temp1, i);
        temp1 = temp1->child;
    }
    
    i = 0;
    do{
        temp1 = pool->child;
        insert(elite, temp1->index);
        front_size = 1;
        temp2 = elite->child;
        temp1 = del (temp1);
        temp1 = temp1->child;
        
        do{
            temp2 = elite->child;
            if (temp1 == NULL)
                break;
            
            do{
                end = 0;
                tempChild1 = (*mixed_pop)->child;
                for(k = 0; k < temp1->index; k++)
                    tempChild1 = tempChild1->next;
                    
                tempChild2 = (*mixed_pop)->child;
                for(k = 0; k < temp2->index; k++)
                    tempChild2 = tempChild2->next;
                
                flag = check_dominance(tempChild1, tempChild2);
                
                if(flag == 1)
                {
                    insert(pool, temp2->index);
                    temp2 = del (temp2);
                    front_size--;
                    temp2 = temp2->child;
                }
                
                if (flag == 0)
                    temp2 = temp2->child;
                    
                if (flag == -1)
                    end = 1;
                
            }while(end != 1 && temp2 != NULL);
            
            if (flag == 0 || flag == 1)
            {
                insert(elite, temp1->index);
                front_size++;
                temp1 = del(temp1);
            }
            
            temp1 = temp1->child;
            
        }while(temp1 != NULL);
        
        temp2 = elite->child;
        j=i;
        
        if((archieve_size + front_size) <= popsize)
        {
            do{
                
                tempChild1 = (*mixed_pop)->child;
                for(k = 0; k < temp2->index; k++)
                    tempChild1 = tempChild1->next;
                    
                tempChild2 = (*new_pop)->child;
                for(k = 0; k < i; k++)
                    tempChild2 = tempChild2->next;
                
                copy_child(tempChild1, tempChild2);
                tempChild2->rank = rank;
                archieve_size+=1;
                temp2 = temp2->child;
                i+=1;
                
            }while(temp2 != NULL);
            
            assign_crowding_distance_indices(*new_pop, j, i-1);
            rank += 1;
        }
        else
        {
            crowding_fill(*mixed_pop, *new_pop, i, front_size, elite);
            archieve_size = popsize;
            
            for(j = i; j < popsize; j++)
            {
                tempChild1 = (*new_pop)->child;
                for(k = 0; k < j; k++)
                    tempChild1 = tempChild1->next;
                    
                tempChild1->rank = rank;
            }
        }
        temp2 = elite->child;
        
        do{
            temp2 = del(temp2);
            temp2 = temp2->child;
        }while(elite->child != NULL);
    
    }while(archieve_size < popsize);
    
    while (pool!=NULL)
    {
        temp1 = pool;
        pool = pool->child;
        free (temp1);
    }
    
    while (elite!=NULL)
    {
        temp1 = elite;
        elite = elite->child;
        free (temp1);
    }
    return;
    
}



/********** Routine to compute crowding distance based on objective function values when the population is in the form of an array ***********/

void assign_crowding_distance_indices(Population *pop, int c1, int c2){

    int **obj_array;
    int *dist;
    int i, j;
    int front_size;
    front_size = c2-c1+1;
    Child *tempChild1 = NULL;
    Child *tempChild2 = NULL;
    
    if(front_size == 1)
    {
        tempChild1 = pop->child;
        for(i = 0; i < c1; i++)
            tempChild1 = tempChild1->next;
            
        tempChild1->crowd_dist = INF;
        return;
    }
    
    if(front_size == 2)
    {
        tempChild1 = pop->child;
        for(i = 0; i < c1; i++)
            tempChild1 = tempChild1->next;
            
        tempChild2 = pop->child;
        for(i = 0; i < c2; i++)
            tempChild2 = tempChild2->next;
        
        tempChild1->crowd_dist = INF;
        tempChild2->crowd_dist = INF;
        return;
    }
    
    obj_array = (int**)malloc(nsga->objectives_size * sizeof(int*));
    dist = (int*)malloc(front_size * sizeof(int));
    
    for(i = 0; i < nsga->objectives_size; i++)
        obj_array[i] = (int*)malloc(front_size * sizeof(int));
        
    for(j = 0; j < front_size; j++)
        dist[j] = c1++;
        
    assign_crowding_distance(pop, dist, obj_array, front_size);
    
    free(dist);    
    for(i = 0; i < nsga->objectives_size; i++)
        free(obj_array[i]);
    free(obj_array);
}



/********** Routine to fill a population with individuals in the decreasing order of crowding distance ***********/

void crowding_fill(Population *mixed_pop, Population *new_pop, int count, int front_size, list *elite){
    
    int *dist;
    list *temp;
    int i, j, k;
    Child *tempChild1 = NULL;
    Child *tempChild2 = NULL;
    int popsize = nsga->population;
    
    assign_crowding_distance_list(mixed_pop, elite->child, front_size);
    
    dist = (int *)malloc(front_size*sizeof(int));
    temp = elite->child;
    
    for(j = 0; j < front_size; j++)
    {
        dist[j] = temp->index;
        temp = temp->child;
    }
    
    quicksort_dist(mixed_pop, dist, front_size);
    
    for(i = count, j = front_size - 1; i < popsize; i++, j--)
    {
        tempChild1 = mixed_pop->child;
        for(k = 0; k < dist[j]; k++)
            tempChild1 = tempChild1->next;
            
        tempChild2 = new_pop->child;
        for(k = 0; k < i; k++)
            tempChild2 = tempChild2->next;
            
        copy_child(tempChild1, tempChild2);
    }
       
    free (dist);
    return;
}



/********** Randomized quick sort routine to sort a population based on crowding distance ***********/

void quicksort_dist(Population *pop, int *dist, int front_size){
    
    q_sort_dist(pop, dist, 0, front_size-1);
}



/********** Actual implementation of the randomized quick sort used to sort a population based on crowding distance ***********/

void q_sort_dist(Population *pop, int *dist, int left, int right){
    
    int     index, temp, i, j, k;
    double  pivot;
    Child   *tempChild1 = NULL;
    
    if(left < right)
    {
        index = rnd (left, right);
        temp = dist[right];
        dist[right] = dist[index];
        dist[index] = temp;
        
        tempChild1 = pop->child;
        for(k = 0; k < dist[right]; k++)
            tempChild1 = tempChild1->next;
        
        pivot = tempChild1->crowd_dist;
        
        i = left-1;
        for(j = left; j < right; j++)
        {
            tempChild1 = pop->child;
            for(k = 0; k < dist[j]; k++)
                tempChild1 = tempChild1->next;
                
            if (tempChild1->crowd_dist <= pivot)
            {
                i+=1;
                temp = dist[j];
                dist[j] = dist[i];
                dist[i] = temp;
            }
        }
        
        index = i + 1;
        temp = dist[index];
        dist[index] = dist[right];
        dist[right] = temp;
        
        q_sort_dist(pop, dist, left, index-1);
        q_sort_dist(pop, dist, index+1, right);
    }
}



