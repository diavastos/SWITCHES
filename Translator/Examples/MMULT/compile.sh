#!/bin/bash

# Translate Application
switches -s multicore -i mmult.c -t 4 -sched roundRobin -a compact -p screen

# Compile Application
make -f Makefile

