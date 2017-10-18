# SWITCHES
SWITCHES is a Parallel Runtime system for Task-based Data-flow Execution. It is implemented for multi- and many-core processors that support a global address space (shared memory) and uses OpenMP v4.5 compiler directives for writting parallel applications.  
It consists a preprocessing tool (called Translator) that translates C/C++ code embedded with OpenMP directives to C/C++ pthread-based code.


__Preface__

In case you have any publications resulting from SWITCHES, please cite the following paper(s):

[1] Andreas Diavastos and Pedro Trancoso. "SWITCHES: A Lightweight Runtime for Data-flow Execution of Tasks on Many-cores" Accepted for publication on ACM Transactions on Architecture and Code Optimization (TACO) 2017.



__Installation__

$ cd Translator  
$ make  
$ export PATH=$PATH:/full/path/to/current/directory  
 

__Usage__

Usage:  ./switches -s _System_ -i _inputFiles_ -t _numberOfThreads_ [-a _Option_] [-p _Option_] ...

     Example: ./switches -s mic 60 240 -i main.c functions.c -t 4 -p screen -a compact

	 [-] Required:
	 -------------

	   * -s <System>          : mic <MAX_CORES HARDWARE_THREADS> 
	                            multicore <MAX_CORES HARDWARE_THREADS> 
	   * -i <InputFiles>      : Input files that contain SW directives
	   * -t <NumberOfThreads> : Number of threads to use for the execution

	 [-] Optional Runtime Optimizations ( Scheduling(-sched), Affinity(-a), Runtime(-r), GA(-nsga) ):
	 ------------------------------------------------------------------------------------------------

	   * [-sched <Option>]   : roundRobin (default)
	                           random 
	                           file <File with Scheduling Policy>

	   * [-a <Option>]       : none (default)
	                           compact 
	                           scatter 
	                           hybrid 
	                           random 

	   * [-r <Option>]       : static (default)
	                           tao                  (NOT YET IMPLEMENTED)
	                           taosw 

	   * [-nsga <Option>]    : -f <fileName>
	                           <population : generations : [NUM : objectives] : mutation : crossover> 
	                           Population      : INT (must be multiple of 4, default: 12)
	                           Generations     : INT (default: 50)
	                           NUM             : INT (number of objectives to use)
	                           Objectives      : STR (choices: performance | power | thermal, default: performance)
	                           Mutation        : FLT (choices: 0.0-1.0, default: 0.1)
	                           Crossover       : FLT (choices: 0.6-1.0, default: 0.6)

	 [-] Optional:
	 -------------

	   * [-tm]              : Activate Transactional Memory   (NOT YET IMPLEMENTED)
	   * [-p <Option>]      : Print the Synchronization Graph
	                              Options: screen
	                                       file
	   * [-v] | [--version] : Print the current version of the Translator
