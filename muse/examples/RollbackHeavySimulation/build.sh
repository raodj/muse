#!/bin/bash
mpicxx -DHAVE_CONFIG_H -DDEVELOPER_ASSERTIONS  -I../../include  -g -O0 -L../../kernel  -lstdc++ RollbackAgent.cpp RollbackSimulation.cpp -lmuse -o rollbackTest
