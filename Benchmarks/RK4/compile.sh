#!/bin/bash

# Translate Application
switches -s multicore -i rk4.c -t 4 -sched roundRobin -a compact -p screen

# Compile Application
make -f Makefile

