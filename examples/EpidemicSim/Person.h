/* 
 * File:   Person.h
 * Author: Julius Higiro
 *
 * Created on March 20, 2017, 8:35 PM
 */

#ifndef PERSON_H
#define PERSON_H

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
    Person(unsigned long PID, int suscept_score, bool isVaxed,
            InfectionState infection_state): pid(PID),
            susceptibility(suscept_score), isVaccinated(isVaxed),
            infectionState(infection_state) {}
private:
    
    /**
     * Global identifier to distinguish an individual within a population.
     */
    unsigned long pid;
    /**
     * Score that indicates level of susceptibility to an infectious disease.
     */
    int susceptibility;
    
    /**
     * Boolean value that indicates the vaccination status of an individual.
     */
    bool isVaccinated;
    
    /**
     * The different types of infectious states.
     */
    InfectionState infectionState;
    
};


#endif /* PERSON_H */

