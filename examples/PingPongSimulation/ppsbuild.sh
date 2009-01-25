#!/bin/bash
mpicxx -DHAVE_CONFIG_H -DDEVELOPER_ASSERTIONS -I../muse/kernel/include -I../muse/include -Wall -g -L../muse/kernel -lmuse -lstdc++ PingPongAgent.cpp PingPongSimulation.cpp -o pp_sim