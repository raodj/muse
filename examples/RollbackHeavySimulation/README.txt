
Overview:
--------------------------------------------------
This is the rollback heavy simulation.  The objective of this
simulation model is to increment simulation-time slowly in the hopes
of creating many rollback scenarios when simulation in run in
parallel.  Note that when a single process is used for simulation,
rollbacks do not (or cannot) occur as there are no asynchronous
operations performed in one process.  Nevertheless, agents are not
coerced to synchronize with each other.  Instead, the
Least-Timestamp-First (LTSF) scheduler schedules events in the correct
causal order thereby eliminating rollbacks in the simulation.

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

$ cd ~/research/muse/examples/RollbackHeavySimulation
$ make

Simulation execution:
--------------------------------------------------
The Rollback Heavy simulation (rollbackSim) takes three optional
command-line arguments as documented by the --help option:

$ ./rollbackSim  --help
A simulation that tends to be rollback heavy.
Copyright (C) PC2Lab (http://pc2lab.ece.miamiOH.edu) 2012-
Usage: rollbackSim [options]
The options are (default values are shown within []):
  --agents   The number of agents in the simulation. [2]
  --nodes    The max numbers of nodes used for this simulation. [1]
  --end      The end time for the simulation. [10]
  --help     Print this message [true]


It maybe executed as shown below:

$ mpiexec -n 2 ./rollbackSim --agents 2 --nodes 2 --end 100

The aforementioned options indicate the simulation rollbackSim will
run with two agents, on two nodes and simualtion to end at time
100.

NOTE: It is important to ensure the following command-line arguments
      are correctly supplied:

      * Ensure the number of agents is at least as many as the number
        of compute nodes used for simulation.

      * Ensure the value for '--nodes' and '-n' command-line arguments
        are the same.

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

