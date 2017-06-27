#ifndef PCS_STATE_H
#define PCS_STATE_H

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

/* 
 * File:   PCS_State.h
 * Author: Julius Higiro
 *
 * Created on September 1, 2016, 3:05 PM
 */

#include "State.h"

/** The state associated with the PCSAgent or "Cell".  PCSAgent models
    a PCS "Cell". A Cell represents a cellular receiver/transmitter
    (or a cell tower) that has some fixed number of "channels"
    allocated to it and is the central entity (LP) object type for the
    simulation. A "channel" is a wireless channel via which a
    "portable" (or cellular device) can send/receive information from
    a Cell.  Portables are modeled using different values in the
    PCSEvent.
 */
class PCS_State : public muse::State {
public:
    State* getClone();
    PCS_State();

    inline int getIndex() const {
        return index;
    }

    inline void setIndex(int new_index) {
        index = new_index;
    }

    inline unsigned int getBlockedChannels() const {
        return blockedChannels;
    }

    inline void setBlockedChannels(unsigned int value) {
        blockedChannels = value;
    }

    inline unsigned int getCallAttempts() const {
        return callAttempts;
    }

    inline void setCallAttempts(unsigned int value) {
        callAttempts = value;
    }

    inline unsigned int getHandOffBlocks() const {
        return handOffBlocks;
    }

    inline void setHandOffBlocks(unsigned int value) {
        handOffBlocks = value;
    }

    inline unsigned int getIdleChannels() const {
        return idleChannels;
    }

    inline void setIdleChannels(unsigned int value) {
        idleChannels = value;
    }

private:

    int index;

    /** 
     * The number of blocked calls that occur because channels
     * are not available.
     */
    unsigned int blockedChannels;

    /** 
     * The number of calls attempted.
     */
    unsigned int callAttempts;

    /** 
     * The number of blocked calls that occur because channels
     * are not available when engaged PCSEvent/Portables are in transit to 
     * a new destination PCSAgent/Cell. 
     */
    unsigned int handOffBlocks;

    /**
     * The number of channels available on this Cell to be potentially
     * used for a call by a portable.
     */
    unsigned int idleChannels;
};


#endif /* PCS_STATE_H */
