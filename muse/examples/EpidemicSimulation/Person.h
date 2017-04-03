/* 
 * File:   Person.h
 * Author: Julius Higiro
 *
 * Created on March 20, 2017, 8:35 PM
 */

#ifndef PERSON_H
#define PERSON_H

#include "DataTypes.h"

/**
 *  Predefined enumerations for the different types of infectious states.
 */
enum InfectionState { UNINFECTED, LATENT, INCUBATING, INFECTIOUS, ASYMPTOMATIC,
                      RECOVERED };

/**
 * An individual person with a set of epidemic related characteristics with 
 * assignment to a location in the system.
 */
class Person {
public:

    Person(unsigned long PID, double transmit_score, double suscept_score,
            bool isVaxed, InfectionState infection_state,
            muse::Time prevChangeStateTime, muse::Time loc_arrv_time) :
    pid(PID), transmissibility(transmit_score), susceptibility(suscept_score),
    isVaccinated(isVaxed), infectionState(infection_state),
    prevStateChangeTime(prevChangeStateTime), localArrivalTime(loc_arrv_time) {}

    void setInfectionState(InfectionState infection_state) {
        infectionState = infection_state;
    }

    InfectionState getInfectionState() { return infectionState; }

    void setPrevChangeStateTime(muse::Time curTime) {
        prevStateChangeTime += curTime;
    }
    
    muse::Time getPrevChangeStateTime() { return prevStateChangeTime; }
    
    muse::Time getLocArrivalTime() { return localArrivalTime; }
    
    double getTransmissiblityScore() { return transmissibility; }
    
    double getSusceptibilityScore() { return susceptibility; }
    
    bool vaccineState() { return isVaccinated; }
    
    unsigned long getId() { return pid; }
    
private:
    /**
     * Global identifier to distinguish an individual within a population.
     */
    unsigned long pid;
    /**
     * Score that indicates level of susceptibility to an infectious disease.
     */
    double susceptibility;
    /**
     * Score that indicates level of transmissibility of an infectious disease.
     */
    double transmissibility;
    /**
     * Boolean value that indicates the vaccination status of an individual.
     */
    bool isVaccinated;
    /**
     * 
     */
    muse::Time prevStateChangeTime;
    /**
     * 
     */
    muse::Time localArrivalTime;
    /**
     * The different types of infectious states.
     */
    InfectionState infectionState;
};

#endif /* PERSON_H */

