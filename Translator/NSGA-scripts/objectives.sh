#####################################################
#                                                   #
#   Title: Objectives to optimize with the GA       #
#                                                   #
#   Author: Andreas I. Diavastos                    #
#   Email:  diavastos@cs.ucy.ac.cy                  #
#                                                   #
#   Last Update: 01-07-2017                         #
#                                                   #
#####################################################


#!/bin/bash

# COMMAND LINE ARGUMENTS
GUEST=$1
OBJECTIVE=$2

# RUN Power COMMAND
if [ "$OBJECTIVE" = "power" ]; then
    while true
    do
	    micsmc -f $GUEST
    done
fi

# RUN Thermal COMMAND
if [ "$OBJECTIVE" = "thermal" ]; then
    while true
    do
	    micsmc -t $GUEST
    done
fi
