This is the rollback heavy simulation.
To run this simply:

!!!Assuming you already compiled muse!!!

1. run the build.sh file ----> ./build
-----this creates and executable called rollbackTest

2. rollbackTest takes three params
-----USAGE: ./rollbackTest <number of agents> <number of nodes> <simulation end time>
-----EXAMPLE below:

---------------------mpiexec -n 2 ./rollbackTest 2 2 100

-------this means that you want to run rollbackTest with two agents, two nodes and simualtion to end at time 100. Make sure the number of agents evenly divids into the number of nodes, because 
I have not put in the checks to handle uneven number of agents.