# This file is processed by automake to generate Makefile.in

#---------------------------------------------------------------------------
#
# Copyright (c) Miami University, Oxford, OH.
# All rights reserved.
#
# Miami University (MU) makes no representations or warranties about
# the suitability of the software, either express or implied,
# including but not limited to the implied warranties of
# merchantability, fitness for a particular purpose, or
# non-infringement.  MU shall not be liable for any damages suffered
# by licensee as a result of using, result of using, modifying or
# distributing this software or its derivatives.
#
# By using or copying this Software, Licensee agrees to abide by the
# intellectual property laws, and all other applicable laws of the
# U.S., and the terms of this license.
#
# Authors: Dhananjai M. Rao       raodm@miamiOH.edu
#
#---------------------------------------------------------------------------

include $(top_srcdir)/Makefile.global.am

if COND_USE_OPENCL
    OPT_OCL = OCL
endif

SUBDIRS = PingPongSimulation LonelySimulation RollbackHeavySimulation\
	RoundRobinSimulation PHOLDSimulation BugLife ClockExample\
	RescueSim PCS_Simulation $(OPT_OCL)
