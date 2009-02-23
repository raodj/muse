#!/bin/bash
mpicxx -DHAVE_CONFIG_H -DDEVELOPER_ASSERTIONS  -I../../include  -g -O0 -L../../kernel -lmuse -lstdc++ RoundRobinAgent.cpp RoundRobinSimulation.cpp -o roundrobinTest
