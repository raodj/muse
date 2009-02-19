#!/bin/bash
mpicxx -DHAVE_CONFIG_H -DDEVELOPER_ASSERTIONS  -I../../include  -g -O0 -L../../kernel -lmuse -lstdc++ RollbackAgent.cpp RollbackSimulation.cpp -o rollbackTest
