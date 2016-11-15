/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PCS_State.h
 * Author: Julius Higiro
 *
 * Created on September 1, 2016, 3:05 PM
 */

#ifndef PCS_STATE_H
#define PCS_STATE_H

#include "State.h"

class PCS_State : public muse::State {
public:
    State * getClone();
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

protected:

    /** The number of blocked calls that occur because channels
     *  are not available.
     */
    unsigned int blockedChannels;

    /** The number of calls attempted.
     */
    unsigned int callAttempts;

    /** The number of blocked calls that occur because channels
     *  are not available when engaged PCSEvent/Portables are in transit to 
     *  a new destination PCSAgent/Cell. 
     */
    unsigned int handOffBlocks;

    /** The number of channels available
     * 
     */
    unsigned int idleChannels;
};


#endif /* PCS_STATE_H */
