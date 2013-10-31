#ifndef SIMULATION_LISTENER_H
#define SIMULATION_LISTENER_H

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Meseret Gebre       gebremr@muohio.edu
//          Dhananjai M. Rao    raodm@muohio.edu
//
//---------------------------------------------------------------------------

/** The muse namespace.
    
    Everything in the api is in the muse namespace.
*/
BEGIN_NAMESPACE(muse);

/** Class to provide hooks to listen to various activites occuring
    within the simulator.

    This class can be extended by the simulation developer to listen
    and intercept some of the major activites occuring within the
    simulator.  The callbacks can be used by the model to perform any
    related activites. Some of the common uses is to coordinate or
    synchronize other devices or subsystems with the simulator.

    \note The the default base class does not perform any tasks and
    all the methods have empty bodies.
*/
class SimulationListener {
public:
    /** The default constructor.

        This class is meant to act purely as an interface class for
        MUSE. Consequently, this class does not have any instance
        variables. Therefore, the constructor has no specific tasks to
        perform.
    */
    SimulationListener() {}

    /** The destructor.

        The destructor for this class does not have any tasks to
        perform. It is here merely to adhere to coding conventions.
    */
    virtual ~SimulationListener() {}

    /** Callback to report completion of a phase of garbage collection.

        This method is invoked by the core simulator whenever a phase
        of garbage collection is completed by the simulator.  This
        method may be called a large number of times during
        simulation.  
        
        \param[in] gvt The Global Virtual Time for which the garbage
        collection has been completed.  There is a guarantee that GVT
        will monotonically increase each time this method is invoked.
    */
    virtual void garbageCollectionDone(const muse::Time& gvt) {
	UNUSED_PARAM(gvt);
    }
};

END_NAMESPACE(muse); //end namespace declaration

#endif
