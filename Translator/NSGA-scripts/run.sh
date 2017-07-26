#####################################################
#                                                   #
#   Title: Execution script to run the application  #
#          on an Intel Xeon Phi from the            #
#          run_nsga.sh script                       #
#                                                   #
#   Author: Andreas I. Diavastos                    #
#   Email:  diavastos@cs.ucy.ac.cy                  #
#                                                   #
#   Last Update: 02-03-2017                         #
#                                                   #
#####################################################

#!/bin/bash

# GLOBAL VARIABLES
APP="sw_taskShared_L"

# EXPORT MIC ENVIROMENTAL VARIABLES
export LD_LIBRARY_PATH=/home/buildsets/eb141118/software/icc/2015.0.090/lib/mic/:$LD_LIBRARY_PATH
export PATH=/home/buildsets/eb141118/software/impi/5.0.1.035-iccifort-2015.0.090/mic/bin:$PATH

# EXECUTE APPLICATION
./$APP

# Exit the ssh command
exit

