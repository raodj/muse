#!/bin/bash
mpicxx -DHAVE_CONFIG_H -DDEVELOPER_ASSERTIONS -I../muse/kernel/include -I../muse/include -I include  -Wall -g -L../muse/kernel -lmuse -lstdc++ src/ClockExample.cpp src/ClockEvent.cpp src/ClockAgent.cpp src/ClockState.cpp -o clock_example

