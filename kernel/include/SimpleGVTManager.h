#ifndef SIMPLE_GVT_MANAGER_H
#define SIMPLE_GVT_MANAGER_H

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
// Authors: Jingbin Yu       yuj53@miamioh.edu
//
//---------------------------------------------------------------------------

#include "DataTypes.h"
#include "GVTManagerBase.h"
#include "ConservativeSimulation.h"

BEGIN_NAMESPACE(muse)

class SimpleGVTManager: public GVTManagerBase {
    // Allow ConservativeSimulation to call protected members
    friend class ConservativeSimulation;
public:
    void initialize(const Time &startTime, Communicator *comm) override;
    bool sendRemoteEvent(Event *event) override;
    inline Time getGVT() override;
    void forceUpdateGVT() override;
    void inspectRemoteEvent(Event *event) override;
protected:
    SimpleGVTManager(Simulation *sim);
private:
    bool hasMessageToProcess = false;
    void allReduceLGVTAndUpdateGVT();
};

END_NAMESPACE(muse)

#endif