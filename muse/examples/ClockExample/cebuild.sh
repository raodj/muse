#!/bin/bash
mpicxx -DHAVE_CONFIG_H -DDEVELOPER_ASSERTIONS  -I../../include -I include
-Wall -g -L../../kernel -lmuse -lstdc++ src/ClockExample.cpp src/ClockEvent.cpp src/ClockAgent.cpp src/ClockState.cpp -o clock_example

