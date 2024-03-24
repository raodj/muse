#ifndef CONSERVATIVE_SIMULATION_H
#define CONSERVATIVE_SIMULATION_H

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
// Authors: Jingbin Yu             yuj53@miamioh.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "Simulation.h"

BEGIN_NAMESPACE(muse);
/** \brief Class for simulation using the conservative simulation strategy

 */
class ConservativeSimulation : public Simulation {
public:
    /** \brief Default Constructor
      Constructs a ConservativeSimulation object
     */
    ConservativeSimulation();
    ~ConservativeSimulation();

    void initialize(int &argc, char *argv[], bool initMPI) override;
    void parseCommandLineArgs(int &argc, char *argv[]) override;

    inline bool isConservative() const { return lookAhead > 0; }

  private:
    int lookAhead;
};

END_NAMESPACE(muse);

#endif