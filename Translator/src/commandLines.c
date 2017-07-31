/**************************************************/
/*                                                */
/*  File:        commandLines.c                   */
/*  Description: Contains the functions that      */
/*               process the Translator           */
/*               command line arguments           */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/


#include "definitions.h"


char **inputFiles;
char *schedulingInputFile;
int totalInputFiles = 0;
int kernels = 0;

extern int targetSystem;
extern int printSGFlag;
extern int assignmentPolicy;
extern int affinityPolicy;
extern int dataSchedPolicy;
extern int line;
extern int pass;
extern bool firstPass;
extern bool transactions;
extern bool tao;
extern bool taosw;
extern FILE *inp, *outp_sw_main, *outp_sw_h, *outp_sw_threadpool, *outp_sw_threads;
extern FILE *outp;
extern NSGA *nsga;
extern int  maxCores;
extern int  hThreads;
extern int  OSthread;




/************************* Print command line help ****************************/

void printVersion(){
    
    printf("\n SWITCHES Translator Version 1.6 (31-07-2017)\n");
    printf(" Copyright (c) 2017 Andreas Diavastos\n");
    printf(" Download: https://github.com/diavastos/SWITCHES\n");
    printf(" Contact : diavastos@cs.ucy.ac.cy\n\n");
    exit(-1);
}




/************************* Print command line help ****************************/

void printHelp(){
    
    printf(ANSI_COLOR_MAGENTA "\n Usage:" ANSI_COLOR_GREEN "  ./switches -s <System> -i <inputFiles> -t <numberOfThreads> [-a <Option>] [-p <Option>]\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\n     Example:" ANSI_COLOR_BLUE " ./switches -s phi -i main.c functions.c -t 4 -p screen -a compact\n\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\t [-] Required:\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\t -------------\n\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\t   * -s <System>          :" ANSI_COLOR_CYAN " phi \n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                            amd \n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                            office \n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\t   * -i <InputFiles>      :" ANSI_COLOR_BLUE " Input files that contain SW directives\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\t   * -t <NumberOfThreads> :" ANSI_COLOR_BLUE " Number of threads to use for the execution\n" ANSI_COLOR_RESET);
    printf("\n");
    printf(ANSI_COLOR_MAGENTA "\t [-] Scheduling (-sched)/Affinity (-a) Optimizations (Optional):\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\t ---------------------------------------------------------------\n\n" ANSI_COLOR_RESET);    
    printf(ANSI_COLOR_MAGENTA "\t   * [-sched <Option>]   :" ANSI_COLOR_CYAN " roundRobin (default)\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                           random \n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                           file <File with Scheduling Policy>\n" ANSI_COLOR_RESET);
    printf("\n");
    printf(ANSI_COLOR_MAGENTA "\t   * [-a <Option>]       :" ANSI_COLOR_CYAN " none (default)\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                           compact \n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                           scatter \n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                           hybrid \n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                           random \n" ANSI_COLOR_RESET);
    printf("\n");
    
    printf(ANSI_COLOR_MAGENTA "\t   * [-nsga <Option>]    :" ANSI_COLOR_CYAN " -f <fileName>\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                           <population : generations : [NUM : objectives] : mutation : crossover> \n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_BLUE "\t                           Population      : INT (must be multiple of 4, default: 12)\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_BLUE "\t                           Generations     : INT (default: 50)\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_BLUE "\t                           NUM             : INT (number of objectives to use)\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_BLUE "\t                           Objectives      : STR (choices: performance | power | thermal, default: performance)\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_BLUE "\t                           Mutation        : FLT (choices: 0.0-1.0, default: 0.1)\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_BLUE "\t                           Crossover       : FLT (choices: 0.6-1.0, default: 0.6)\n" ANSI_COLOR_RESET);
    printf("\n");
    
    printf(ANSI_COLOR_MAGENTA "\t   * [-tao]              :" ANSI_COLOR_BLUE " Produce code for the TAO runtime\n" ANSI_COLOR_RESET);
    printf("\n");
    
    printf(ANSI_COLOR_MAGENTA "\t   * [-taosw]            :" ANSI_COLOR_BLUE " Produce code for the TAO+SW runtime\n" ANSI_COLOR_RESET);
    printf("\n");
    
    printf(ANSI_COLOR_MAGENTA "\t [-] Optional:\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\t -------------\n\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\t   * [-tm]              :" ANSI_COLOR_BLUE " Activate Transactional Memory   (NOT YET IMPLEMENTED)\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\t   * [-p <Option>]      :" ANSI_COLOR_BLUE " Print the Synchronization Graph\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                              Options: screen\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\t                                       file\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_MAGENTA "\t   * [-v] | [--version] :" ANSI_COLOR_BLUE " Print the current version of the Translator\n" ANSI_COLOR_RESET);
    printf("\n");
    
    printf("\n");
        
    exit(-1);
}


/*************** Recognize and store command line arguments *******************/

void recognizeCommandlineArguments(int argc, char **argv){
    
    int i = 0, j = 0;
    int temp_i = 0;
    int temp_objectives = 1;
    int temp_j = 0;
    int countInputFiles = 0;
    bool targetSystemFound = FALSE;
    bool inputFilesFound = FALSE;
    bool transactionsFound = FALSE;
    bool printSGFound = FALSE;
    bool kernelsFound = FALSE;
    bool affinityFound = FALSE;
    bool nsgaFound = FALSE;
    bool policyFound = FALSE;
    bool dataSchedPolicyFound = FALSE;
    bool taoFound = FALSE;
    bool taoswFound = FALSE;
    FILE*   nsgaInputs;
    char    buff[1000];
    int     res;

    
    // Print help message to explain command line arguments
    for(i = 1; i < argc; i++)
        if(!strcmp(argv[i], "-h"))
            printHelp();
    
    // Print version message
    for(i = 1; i < argc; i++)
        if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version"))
            printVersion();
        
    for(i = 1; i < argc; i++)
    {
        /** If command line argument  is for target system **/
        if(!strcmp(argv[i], "-s") && !targetSystemFound)
        {   
            i++;
            if(!argv[i])
            {
                ERROR_COMMANDS("%s not declared!", "System")
                printHelp();
            }
                
            if(!strcmp(argv[i], "phi"))
                targetSystem = PHI;
            else if(!strcmp(argv[i], "amd"))
                targetSystem = AMD;
            else if(!strcmp(argv[i], "office"))
                targetSystem = OFFICE_PC;
            else
            {
                ERROR_COMMANDS("System [ %s ] not recognized!", argv[i])
                printHelp();
            }
            
            
            switch(targetSystem){
        
                case PHI:
                    maxCores = PHI_MAX_CORES;
                    hThreads = PHI_H_THREADS;
                    OSthread = PHI_OS_THREAD;
                    break;
                    
                case AMD:
                    maxCores = AMD_MAX_CORES;
                    hThreads = AMD_H_THREADS;
                    OSthread = AMD_OS_THREAD;
                    break;
                    
                case OFFICE_PC:
                    maxCores = OFFICE_MAX_CORES;
                    hThreads = OFFICE_H_THREADS;
                    OSthread = OFFICE_OS_THREAD;
                    break;
                    
            
                default:
                    // nothing
                    break;
        }
            
            
            targetSystemFound = TRUE;
            continue;
        }

        /** If command line argument is for input files **/
        if(!strcmp(argv[i], "-i") && !inputFilesFound)
        {
            i++;
            
            if(!argv[i])
            {
                ERROR_COMMANDS("No %s given!", "Input Files")
                printHelp();
            }
            
            // Count the number of input files given
            temp_i = i;
            countInputFiles = 0;
            while(argv[temp_i] && strcmp(argv[temp_i], "-s")
                               && strcmp(argv[temp_i], "-t")
                               && strcmp(argv[temp_i], "-tm")
                               && strcmp(argv[temp_i], "-p")
                               && strcmp(argv[temp_i], "-a")
                               && strcmp(argv[temp_i], "-nsga")
                               && strcmp(argv[temp_i], "-sched"))
            {
                countInputFiles++;
                temp_i++;
            }
            
            if(!countInputFiles)
            {
                ERROR_COMMANDS("No %s given!", "Input Files")
                printHelp();
            }
            
            // Store input files
            j = 0;
            totalInputFiles = countInputFiles;
            inputFiles = malloc(sizeof(char*) * countInputFiles);
            
            while(argv[i] && strcmp(argv[i], "-s")
                          && strcmp(argv[i], "-t")
                          && strcmp(argv[i], "-tm")
                          && strcmp(argv[i], "-p")
                          && strcmp(argv[i], "-a")
                          && strcmp(argv[i], "-nsga")
                          && strcmp(argv[i], "-sched")
                          && strcmp(argv[i], "-tao")
                          && strcmp(argv[i], "-taosw"))
            {
                inputFiles[j] = malloc(sizeof(char) * strlen(argv[i]));
                strcpy(inputFiles[j], argv[i]);
                j++;
                i++;
            }
            
            i--;
            inputFilesFound = TRUE;
            continue;
        }
        
        /** If command line argument  is for TAO **/
        if(!strcmp(argv[i], "-tao") && !taoFound)
        {   
            tao      = TRUE;
            taoFound = TRUE;
            continue;
        }
        
        /** If command line argument  is for TAO+SW **/
        if(!strcmp(argv[i], "-taosw") && !taoswFound)
        {   
            taosw      = TRUE;
            taoswFound = TRUE;
            continue;
        }
        
        /** If command line argument  is for Transactional Memory **/
        if(!strcmp(argv[i], "-tm") && !transactionsFound)
        {   
            transactions = TRUE;
            transactionsFound = TRUE;
            
            ERROR_COMMANDS("%s not implemented yet!", "TM Version")
            printHelp();
            
            continue;
        }
        
        /** If command line argument  is for number of Kernels **/
        if(!strcmp(argv[i], "-t") && !kernelsFound)
        {   
            i++;
            if(!argv[i])
            {
                ERROR_COMMANDS("%s not given!", "Number of threads")
                printHelp();
            }
            kernels = atoi(argv[i]);
            kernelsFound = TRUE;
            continue;
        }
        
        
        /** If command line argument  is for printing the SG **/
        if(!strcmp(argv[i], "-p") && !printSGFound)
        {   
            i++;
            if(!argv[i])
            {
                ERROR_COMMANDS("%s not declared!", "Printing screen or file")
                printHelp();
            }
                
            if(!strcmp(argv[i], "screen"))
                printSGFlag = __SCREEN;
            else if(!strcmp(argv[i], "file"))
                printSGFlag = __FILE;
            else
            {
                ERROR_COMMANDS("Print output [ %s ] not recognized!", argv[i])
                printHelp();
            }
            
            printSGFound = TRUE;
            continue;
        }
        
        /** If command line argument  is for affinity policy **/
        if(!strcmp(argv[i], "-a") && !affinityFound)
        {   
            i++;
            if(!argv[i])
            {
                ERROR_COMMANDS("%s not declared!", "Affinity Policy")
                printHelp();
            }
                
            if(!strcmp(argv[i], "none"))
                affinityPolicy = AFFINITY_NONE;
            else if(!strcmp(argv[i], "compact"))
                affinityPolicy = AFFINITY_COMPACT;
            else if(!strcmp(argv[i], "scatter"))
                affinityPolicy = AFFINITY_SCATTER;
            else if(!strcmp(argv[i], "hybrid"))
                affinityPolicy = AFFINITY_HYBRID;
            else if(!strcmp(argv[i], "random"))
                affinityPolicy = AFFINITY_RANDOM;
            else
            {
                ERROR_COMMANDS("Print output [ %s ] not recognized!", argv[i])
                printHelp();
            }
            
            affinityFound = TRUE;
            continue;
        }
        
        /** If command line argument  is for NSGA algorithm **/
        if(!strcmp(argv[i], "-nsga") && !nsgaFound)
        {   
            i++;
            if(!argv[i])
            {
                ERROR_COMMANDS("%s not given!", "NSGA parameters")
                printHelp();
            }
            
            // Allocate memory for NSGA input data structure
            nsga = (NSGA*)malloc(sizeof(NSGA));
                
            if(!strcmp(argv[i], "-f"))
            {
                i++;
                if(!argv[i])
                {
                    ERROR_COMMANDS("%s not given!", "NSGA Input File")
                    printHelp();
                }
                nsgaInputs = fopen(argv[i], "r");
                if(!nsgaInputs){
                    ERROR_COMMANDS("NSGA File [ %s ] not opened!", argv[i])
                    exit(-1);
                }
                
                for(temp_i = 1; temp_i <= NSGA_PARAMETERS; temp_i++)
                {
                    bzero(buff, sizeof(buff));
                    res = fscanf(nsgaInputs, "%s", buff);
                    
                    switch(temp_i){
                        
                        case NSGA_POPULATION:
                            if(!strcmp(buff, "default"))
                                nsga->population = 12;
                            else
                            {
                                nsga->population = atoi(buff);
                                if((nsga->population % 4) != 0)
                                {
                                    ERROR_COMMANDS("Population size must be multiple of [ %s ]", "4")
                                    exit(-1);
                                }
                            }
                            
                            break;
                            
                        case NSGA_GENERATIONS:
                            if(!strcmp(buff, "default"))
                                nsga->generations = 50;
                            else
                                nsga->generations = atoi(buff);
                            
                            break;
                            
                        case NSGA_OBJECTIVE:
                        
                            // First read the number of objectives
                            temp_objectives = atoi(buff);
                            nsga->objectives_size = temp_objectives;
                            nsga->objectives = (int *)malloc(sizeof(int) * temp_objectives);
                            temp_j = 0;
                        
                            // Now read the Objectives
                            do
                            {
                                bzero(buff, sizeof(buff));
                                res = fscanf(nsgaInputs, "%s", buff);
                            
                                if(!strcmp(buff, "default"))
                                    nsga->objectives[temp_j] = NSGA_OBJECTIVE_PERFORMANCE;
                                else
                                {
                                    if(!strcmp(buff, "performance"))
                                        nsga->objectives[temp_j] = NSGA_OBJECTIVE_PERFORMANCE;
                                    else if(!strcmp(buff, "power"))
                                        nsga->objectives[temp_j] = NSGA_OBJECTIVE_POWER;
                                    else if(!strcmp(buff, "thermal"))
                                        nsga->objectives[temp_j] = NSGA_OBJECTIVE_THERMAL;
                                    else
                                    {
                                        ERROR_COMMANDS("Unrecognized NSGA Objective [ %s ]!", buff)
                                        printHelp();
                                    }
                                }
                                
                                temp_j++;
                                temp_objectives--;
                            }while(temp_objectives);
                            
                            break;
                            
                        case NSGA_MUTATION:
                            if(!strcmp(buff, "default"))
                                nsga->mutation = 0.1;
                            else
                            {
                                if(atof(buff) < 0.0 || atof(buff) > 1.0)
                                {
                                    ERROR_COMMANDS("NSGA Mutation Value must be between [ %s ]!", "0.0-1.0")
                                    printHelp();
                                }                                
                                nsga->mutation = atof(buff);
                            }
                            
                            break;
                            
                        case NSGA_CROSSOVER:
                            if(!strcmp(buff, "default"))
                                nsga->crossover = 0.6;
                            else
                            {
                                if(atof(buff) < 0.0 || atof(buff) > 1.0)
                                {
                                    ERROR_COMMANDS("NSGA Crossover Value must be between [ %s ]!", "0.0-1.0")
                                    printHelp();
                                }                                
                                nsga->crossover = atof(buff);
                            }
                            
                            break;
                            
                        default:
                            ERROR_COMMANDS("Unrecognized NSGA parameter in file [ %s ]!", argv[i])
                            exit(-1);
                    }
                }
            }
            else
            {
                for(temp_i = 1; temp_i <= NSGA_PARAMETERS; temp_i++)
                {
                    bzero(buff, sizeof(buff));
                    strcpy(buff, argv[i++]);
                    
                    switch(temp_i){
                        
                        case NSGA_POPULATION:
                            if(!strcmp(buff, "default"))
                                nsga->population = 10;
                            else
                                nsga->population = atoi(buff);
                            
                            break;
                            
                        case NSGA_GENERATIONS:
                            if(!strcmp(buff, "default"))
                                nsga->generations = 50;
                            else
                                nsga->generations = atoi(buff);
                            
                            break;
                            
                        case NSGA_OBJECTIVE:
                        
                            // First read the number of objectives
                            temp_objectives = atoi(buff);
                            nsga->objectives_size = temp_objectives;
                            nsga->objectives = (int *)malloc(sizeof(int) * temp_objectives);
                            temp_j = 0;
                        
                            // Now read the Objectives
                            do
                            {
                                bzero(buff, sizeof(buff));
                                strcpy(buff, argv[i++]);
                            
                                if(!strcmp(buff, "default"))
                                    nsga->objectives[temp_j] = NSGA_OBJECTIVE_PERFORMANCE;
                                else
                                {
                                    if(!strcmp(buff, "performance"))
                                        nsga->objectives[temp_j] = NSGA_OBJECTIVE_PERFORMANCE;
                                    else if(!strcmp(buff, "power"))
                                        nsga->objectives[temp_j] = NSGA_OBJECTIVE_POWER;
                                    else if(!strcmp(buff, "thermal"))
                                        nsga->objectives[temp_j] = NSGA_OBJECTIVE_THERMAL;
                                    else
                                    {
                                        ERROR_COMMANDS("Unrecognized NSGA Objective [ %s ]!", buff)
                                        printHelp();
                                    }
                                }
                                
                                temp_j++;
                                temp_objectives--;
                            }while(temp_objectives);
                            
                            break;
                            
                        case NSGA_MUTATION:
                            if(!strcmp(buff, "default"))
                                nsga->mutation = 0.1;
                            else
                            {
                                if(atof(buff) < 0.0 || atof(buff) > 1.0)
                                {
                                    ERROR_COMMANDS("NSGA Mutation Value must be between [ %s ]!", "0.0-1.0")
                                    printHelp();
                                }                                
                                nsga->mutation = atof(buff);
                            }
                            
                            break;
                            
                        case NSGA_CROSSOVER:
                            if(!strcmp(buff, "default"))
                                nsga->crossover = 0.6;
                            else
                            {
                                if(atof(buff) < 0.0 || atof(buff) > 1.0)
                                {
                                    ERROR_COMMANDS("NSGA Crossover Value must be between [ %s ]!", "0.0-1.0")
                                    printHelp();
                                }                                
                                nsga->crossover = atof(buff);
                            }
                            
                            break;
                        
                        default:
                            ERROR_COMMANDS("Unrecognized NSGA parameter in file [ %s ]!", argv[i])
                            exit(-1);
                    }

                }
            }
            
            // Read the path of the application
            bzero(buff, sizeof(buff));
            strcpy(buff, argv[++i]);
            nsga->path = (char*)malloc(sizeof(char) * strlen(buff));
            strcpy(nsga->path, buff);
            
            assignmentPolicy = SCHED_NSGA;
            nsgaFound = TRUE;
            policyFound = TRUE;
            continue;
        }
        
        
        /** If command line argument  is for scheduling policy **/
        if(!strcmp(argv[i], "-sched") && !policyFound)
        {   
            i++;
            if(!argv[i])
            {
                ERROR_COMMANDS("%s not declared!", "Scheduling Policy")
                printHelp();
            }
                
            if(!strcmp(argv[i], "roundRobin"))
                assignmentPolicy = SCHED_RR;
            else if(!strcmp(argv[i], "file"))
            {
                i++;
                assignmentPolicy = SCHED_FILE;
                
                // Count the number of FILE parameters (MUST BE 1)
                countInputFiles = 0;
                if(argv[i] && strcmp(argv[i], "-s")
                           && strcmp(argv[i], "-t")
                           && strcmp(argv[i], "-tm")
                           && strcmp(argv[i], "-p")
                           && strcmp(argv[i], "-a")
                           && strcmp(argv[i], "-nsga")
                           && strcmp(argv[i], "-sched"))
                {
                    countInputFiles++;
                }
                
                if(countInputFiles != 1)
                {
                    ERROR_COMMANDS("No %s given!", "Scheduling file given for FILE scheduling policy")
                    printHelp();
                }
                
                // Store input files
                schedulingInputFile = malloc(sizeof(char) * strlen(argv[i])*2);
                bzero(schedulingInputFile, sizeof(schedulingInputFile));
                strcpy(schedulingInputFile, argv[i]);
            }
            else if(!strcmp(argv[i], "random"))
                assignmentPolicy = SCHED_RANDOM;
            else
            {
                ERROR_COMMANDS("Print output [ %s ] not recognized!", argv[i])
                printHelp();
            }
            
            policyFound = TRUE;
            continue;
        }
        
        ERROR_COMMANDS("Command line argument [ %s ] not recognized!", argv[i])
        printHelp();
    
    }
    
    // Check if all required parameters were given
    
    if(!targetSystemFound)
    {
        ERROR_COMMANDS("%s not declared!", "System")
        printHelp();
    }
    if(!inputFilesFound)
    {
        ERROR_COMMANDS("No %s given!", "Input Files")
        printHelp();
    }
    if(!kernelsFound)
    {
        ERROR_COMMANDS("%s not declared!", "Number of Threads")
        printHelp();
    }
    
    // Check that affinity scheduling matches with the target system processor
    if(targetSystem != PHI && (affinityPolicy == AFFINITY_SCATTER || affinityPolicy == AFFINITY_HYBRID))
    {
        ERROR_COMMANDS("Affinity policies [%s] and [%s] is only supported by Intel Xeon Phi", "Stack", "Hybrid")
        printHelp();
    }
    

}


