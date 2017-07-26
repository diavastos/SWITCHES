#####################################################
#                                                   #
#   Title: NSGA algorithm execution with SWITCHES   #
#          parallel programming system              #
#                                                   #
#   Author: Andreas I. Diavastos                    #
#   Email:  diavastos@cs.ucy.ac.cy                  #
#                                                   #
#   Last Update: 02-03-2017                         #
#                                                   #
#####################################################

#!/bin/bash


# GLOBAL PARAMETERS
ARRAY=()
REPEATS=3
AFFINITY=compact

# COMMAND LINE ARGUMENTS
APP=$1
GUEST=$2
SYSTEM=$3
NSGA_PARAMS=$4

# CHECK COMMAND-LINE PARAMETERS
if [ "$#" -ne 4 ]; then
    echo "ERROR: Illegal number of parameters."
    echo "   Correct Format:"
    echo "    ./run_nsga.sh <application.c> <guest> <system> <nsga_parameters_file>"
    echo "    ./run_nsga.sh <application.c> <guest> <system>"
    echo ""
    echo "   Correct Format:"
    echo "    ./run_nsga.sh test.c mic0 phi nsga.in"
    exit 1
fi

# CHECK IF APPLICATION SOURCE FILE EXISTS
if [[ ! -f "$APP" ]]; then
    echo "ERROR: File [ $APP ] doesn't exist!"
    exit 1
fi

                
if [ "$SYSTEM" = "phi" ]; then
    THREADS=240
fi

if [ "$SYSTEM" = "amd" ]; then
    THREADS=12
fi

if [ "$SYSTEM" = "office" ]; then
    THREADS=4
fi
           

############################## STEP 1 ##############################
# Start NSGA SWITCHES process

echo -n "Initializing NSGA data structures... "

# Initialize NSGA data
if [[ -f "$NSGA_PARAMS" ]]; then
    
    # Read from nsga.in file
    while IFS='' read -r line || [[ -n "$line" ]]; do
        ARRAY+=("$line")
    done < "$NSGA_PARAMS"

    # Store file data to variables
    POPULATION=${ARRAY[0]}
    OBJECTIVES=${ARRAY[2]}
    GENERATIONS=${ARRAY[1]}

    if [[ "$OBJECTIVES" -gt 1 ]]; then
        OBJECTIVE_1=${ARRAY[3]}
        OBJECTIVE_2=${ARRAY[4]}
        MUTATION=${ARRAY[5]}
        CROSSOVER=${ARRAY[6]}
    else
        OBJECTIVE_1=${ARRAY[3]}
        MUTATION=${ARRAY[4]}
        CROSSOVER=${ARRAY[5]}
    fi
    
    echo "DONE"
    
    # Create Generations Folder
    echo -n "Creating Generations Folders........ "
    mkdir -p Generations
    mkdir -p Results
    
    # Create a Folder for every Generation
    CV_GEN=1
    while [  $CV_GEN -le $GENERATIONS ]; 
    do
        mkdir -p Generations/Gen$CV_GEN
        mkdir -p Results/Gen$CV_GEN
        let CV_GEN=CV_GEN+1 
    done
    echo "DONE"
    
    CMD_PWD="pwd"
    PWD=`eval $CMD_PWD`
    
    # Start SWITCHES process in the background with -nsga option
    echo -n "Start NSGA SWITCHES process......... "
    switches -s $SYSTEM -i $APP -t $THREADS -nsga -f $NSGA_PARAMS $PWD &
    echo "DONE"
    
else
    # Manually give the NSGA values -- CHANGE THESE VALUES IF YOU DONT GIVE THEM AS INPUT FILE
    POPULATION=-1
    GENERATIONS=
    OBJECTIVES=
    OBJECTIVE_1=
    OBJECTIVE_2=
    MUTATION=
    CROSSOVER=

    if [[ POPULATION -eq -1 ]]; then
        echo "ERROR: NSGA data not initialized!"
        exit 1
    fi
    
    echo "DONE"
    
    # Create Generations Folder
    echo -n "Creating Generations Folders........ "
    mkdir -p Generations
    mkdir -p Results
    
    # Create a Folder for every Generation
    CV_GEN=1
    while [  $CV_GEN -le $GENERATIONS ]; 
    do
        mkdir -p Generations/Gen$CV_GEN
        mkdir -p Results/Gen$CV_GEN
        let CV_GEN=CV_GEN+1 
    done
    echo "DONE"
    
    CMD_PWD="pwd"
    PWD=`eval $CMD_PWD`
        
    # Start SWITCHES process in the background with -nsga option
    echo -n "Start NSGA SWITCHES process......... "
    switches -s $SYSTEM -i $APP -t $THREADS -nsga $POPULATION $GENERATIONS $OBJECTIVES $OBJECTIVE_1 $OBJECTIVE_2 $MUTATION $CROSSOVER $PWD &
    echo "DONE"
    
fi



############################## STEP 2 ##############################
# Translating, Producing & Executing children


if [[ $GENERATIONS -eq "default" ]]; then
    GENERATIONS=50
fi

if [[ $POPULATION -eq "default" ]]; then
    POPULATION=12
fi

echo "Starting Evaluation Fitness:"
echo "----------------------------"

CV_GEN=1
while [  $CV_GEN -le $GENERATIONS ]; 
do
    echo -n "Generation $CV_GEN: "
    
    CV_POP=1
    while [ $CV_POP -le $POPULATION ];
    do

        # Wait for NSGA process to create the scheduling file
        if [[ ! -f "Generations/Gen$CV_GEN/g$CV_GEN-c$CV_POP" ]]; then
            continue
        fi
        
        echo -n "$CV_POP "
        
        # Clean folder from previous translations
        make -f Makefile_mic clean > /dev/null
        
        # Translate child
        switches -s $SYSTEM -i $APP -t $THREADS -sched file "Generations/Gen$CV_GEN/g$CV_GEN-c$CV_POP" -a $AFFINITY
        
        # Compile child on HOST CPU
        make -f Makefile_mic > /dev/null
        
        # Execute child on PHI
        CMD_PWD="pwd"
        PWD=`eval $CMD_PWD`
        CV_REPEATS=0
        while [ $CV_REPEATS -lt $REPEATS ];
        do
            if [ "$OBJECTIVE_1" = "power" ] || [ "$OBJECTIVE_2" = "power" ]; then
                ./objectives.sh $GUEST power >> Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-pow.out &
            fi
            
            if [ "$OBJECTIVE_1" = "thermal" ] || [ "$OBJECTIVE_2" = "thermal" ]; then
                ./objectives.sh $GUEST thermal >> Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-thermal.out &
            fi
            
            ssh -o StrictHostKeyChecking=no $GUEST "cd $PWD; ./run.sh >> Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-time.out"
            
            if [ "$OBJECTIVE_1" = "power" ] || [ "$OBJECTIVE_2" = "power" ]; then
                PID=`pgrep -U adiavastos -n bash`
                kill $PID
                wait $PID 2>/dev/null
            fi
            
            if [ "$OBJECTIVE_1" = "thermal" ] || [ "$OBJECTIVE_2" = "thermal" ]; then
                PID=`pgrep -U adiavastos -n bash`
                kill $PID
                wait $PID 2>/dev/null
            fi
                        
            let CV_REPEATS=CV_REPEATS+1
        done
        
        # Record & Average objectives
        
        # PERFORMANCE
    	fileIn="Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-time.out"
	    fileOut="Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-time.tmp"
	    flag=0
	    AVERAGE=(0.0)
        while IFS= read -r line
        do
            for word in $line; 
            do   
        		if [ "$flag" = "1" ]; then
	        		AVER="echo "$AVERAGE+$word" | bc -l"
	        		AVERAGE=`eval $AVER`	    			
	        	    flag=0
	        	fi
        							
	        	if [ "$word" = "__aid_Time:" ]; then
	        		flag=1
   	        	fi
	        done
   	    done <"$fileIn"
        AVER="echo "$AVERAGE/$REPEATS" | bc -l"
	    AVERAGE=`eval $AVER`	    			
        echo "$AVERAGE" > $fileOut
        # !! VERY IMPORTANT TO CALL move, TO MAKE SURE THAT THE WRITE HAS FIRST FINISHED BEFORE THE switches PROCESS ATTEMPTS READING IT !!
        mv $fileOut Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-time.res
        
        # POWER
        if [ "$OBJECTIVE_1" = "power" ] || [ "$OBJECTIVE_2" = "power" ]; then
    		fileIn="Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-pow.out"
	    	fileOut="Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-pow.tmp"
	    	flag=0
	    	AVERAGE=(0.0)
	    	COUNT=0
            while IFS= read -r line
            do
                for word in $line; 
                do   
        			if [ $flag = 3 ]; then
	        			AVER="echo "$AVERAGE+$word" | bc -l"
	        			AVERAGE=`eval $AVER`    			
	        		    flag=0
	        		    let COUNT=COUNT+1
	        		fi
        							
	        		if [ "$word" = "Total" ]; then
	        			let flag=flag+1
   	        		fi
   	        		
   	        		if [ "$word" = "Power:" ]; then
	        			let flag=flag+1
   	        		fi
   	        		
   	        		if [ "$word" = "............." ]; then
	        			let flag=flag+1
   	        		fi
	        	done
   	        done <"$fileIn"
            AVER="echo "$AVERAGE/$COUNT" | bc -l"
	    	AVERAGE=`eval $AVER`	    			
            echo "$AVERAGE" > $fileOut
            # !! VERY IMPORTANT TO CALL move, TO MAKE SURE THAT THE WRITE HAS FIRST FINISHED BEFORE THE switches PROCESS ATTEMPTS READING IT !!
            mv $fileOut Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-pow.res
        fi
        
        # THERMAL
        if [ "$OBJECTIVE_1" = "thermal" ] || [ "$OBJECTIVE_2" = "thermal" ]; then
    		fileIn="Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-thermal.out"
	    	fileOut="Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-thermal.tmp"
	    	flag=0
	    	AVERAGE=(0.0)
	    	COUNT=0
            while IFS= read -r line
            do
                for word in $line; 
                do   
        			if [ $flag = 3 ]; then
	        			AVER="echo "$AVERAGE+$word" | bc -l"
	        			AVERAGE=`eval $AVER`    			
	        		    flag=0
	        		    let COUNT=COUNT+1
	        		fi
        							
	        		if [ "$word" = "Cpu" ]; then
	        			let flag=flag+1
   	        		fi
   	        		
   	        		if [ $flag = 1 ] && [ "$word" = "Temp:" ]; then
	        			let flag=flag+1
   	        		fi
   	        		
   	        		if [ $flag = 2 ] && [ "$word" = "................" ]; then
	        			let flag=flag+1
   	        		fi
	        	done
   	        done <"$fileIn"
            AVER="echo "$AVERAGE/$COUNT" | bc -l"
	    	AVERAGE=`eval $AVER`	    			
            echo "$AVERAGE" > $fileOut
            # !! VERY IMPORTANT TO CALL move, TO MAKE SURE THAT THE WRITE HAS FIRST FINISHED BEFORE THE switches PROCESS ATTEMPTS READING IT !!
            mv $fileOut Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-thermal.res
        fi
        
        # IF NEED BE, ADD NEW OBJECTIVE BY COPYING PREVIOUS (THERMAL) IF STATEMENT
        
        let CV_POP=CV_POP+1 
    done
    let CV_GEN=CV_GEN+1 
    echo "DONE"
done

############################## STEP 3 ##############################
# SUM-UP ALL GENERATIONS OBJECTIVES RESULTS INTO A SINGLE FILE

echo -n "Sum-up all results.................. "

CV_GEN=1
while [  $CV_GEN -le $GENERATIONS ]; 
do   
    CV_POP=1
    while [ $CV_POP -le $POPULATION ];
    do
        # PERFORMANCE
    	fileIn="Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-time.res"
	    fileOut="time.res"
        while IFS= read -r line
        do
            for word in $line; 
            do   
        		echo -n "$word " >> $fileOut
	        done
   	    done <"$fileIn"
   	    
   	    # POWER
   	    if [ "$OBJECTIVE_1" = "power" ] || [ "$OBJECTIVE_2" = "power" ]; then
        	fileIn="Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-pow.res"
	        fileOut="power.res"
            while IFS= read -r line
            do
                for word in $line; 
                do   
            		echo -n "$word " >> $fileOut
	            done
   	        done <"$fileIn"
   	    fi
   	        
   	    # THERMAL
   	    if [ "$OBJECTIVE_1" = "thermal" ] || [ "$OBJECTIVE_2" = "thermal" ]; then
        	fileIn="Results/Gen$CV_GEN/g$CV_GEN-c$CV_POP-thermal.res"
	        fileOut="thermal.res"
            while IFS= read -r line
            do
                for word in $line; 
                do   
            		echo -n "$word " >> $fileOut
	            done
   	        done <"$fileIn"
   	    fi
   	    
   	    # IF YOU ADDED A NEW OBJECTIVE THEN COPY PREVIOUS (THERMAL) IF STATEMENT TO SUM-UP THE NEW RESULTS
   	    
        let CV_POP=CV_POP+1 
    done
    fileOut="time.res"
    echo "" >> $fileOut
    fileOut="power.res"
    echo "" >> $fileOut
    fileOut="thermal.res"
    echo "" >> $fileOut
    let CV_GEN=CV_GEN+1 
done

echo "DONE"
