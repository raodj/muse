
Overview:
--------------------------------------------------
This directory contains the source for a round robin simulation in
which each agent forwards a token (single event) to its adjacent
neighboring agent.  Agents are assumed to be organized in a logical
circle.  Agent number 0 (zero) initiates the token/event that is
circulated in a cycle.  A the end of each cycle, agent 0 prints a
message indicating the token has completed another round.

Compiling:
--------------------------------------------------
The model is integrated with the MUSE build system. Prior to compiling
the model the MUSE simulation kernel must be compiled first using the
following operations:


$ cd ~/research/muse    # Change to top-level MUSE directory
$ autoreconf -i -v      # Create configure script
$ ./configure           # Configure and generate site-specific Make files
$ make                  # Build MUSE kernel

Once the MUSE kernel has been compiled, this example can be compiled
in the following manner:

$ cd ~/research/muse/examples/RoundRobinSimulation
$ make

Simulation execution:
--------------------------------------------------

The round robin simulation (roundRobinSim) takes three optional
command-line arguments as documented by the --help option:

$ ./roundRobinSim --help
A simulation in which a token is exchanged in a round-robin manner.
Copyright (C) PC2Lab (http://pc2lab.ece.miamiOH.edu) 2012-
Usage: roundRobinSim [options]
The options are (default values are shown within []):
  --agents    Number of agents in simulation [3]
  --nodes     The  numbers of compute nodes used for simulation. [1]
  --endTime   The end time for the simulation. [10]
  --help      Print this message [true]

It maybe executed as shown below:

$ mpiexec -n 2 ./roundRobinSim --agents 20 --nodes 2 --end 100

The aforementioned options indicate the simulation will run with 20
agents, on two compute nodes and simualtion to end at time 100.

NOTE: Ensure the value for '--nodes' (command-line argument to the
        simulation) and '-n' command-line argument to MPI are the same
        value.

Licence and Disclaimers:
--------------------------------------------------
    ___
   /\__\    This file is part of MUSE    <http:www.muse-tools.org/>
  /::L_L_
 /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
 \/_/:/  /  free software: you can  redistribute it and/or  modify it
   /:/  /   under the terms of the GNU  General Public License  (GPL)
   \/__/    as published  by  the   Free  Software Foundation, either
            version 3 (GPL v3), or  (at your option) a later version.
    ___
   /\__\    MUSE  is distributed in the hope that it will  be useful,
  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
 /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
 \:\/:/  /  PURPOSE.
  \::/  /
   \/__/    Miami University  and  the MUSE  development team make no
            representations  or  warranties  about the suitability of
    ___     the software,  either  express  or implied, including but
   /\  \    not limited to the implied warranties of merchantability,
  /::\  \   fitness  for a  particular  purpose, or non-infringement.
 /\:\:\__\  Miami  University and  its affiliates shall not be liable
 \:\:\/__/  for any damages  suffered by the  licensee as a result of
  \::/  /   using, modifying,  or distributing  this software  or its
   \/__/    derivatives.

    ___     By using or  copying  this  Software,  Licensee  agree to
   /\  \    abide  by the intellectual  property laws,  and all other
  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
 /::\:\__\  General  Public  License  (version 3).  You  should  have
 \:\:\/  /  received a  copy of the  GNU General Public License along
  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
   \/__/    from <http:www.gnu.org/licenses/>.


