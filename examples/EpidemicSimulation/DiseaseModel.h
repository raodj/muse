/* 
 * File:   DiseaseModel.h
 * Author: Julius Higiro
 *
 * Created on March 20, 2017, 8:36 PM
 */

#ifndef DISEASEMODEL_H
#define DISEASEMODEL_H

#include <vector>
#include <unordered_map>
#include <cassert>
#include <cmath>
#include "Person.h"

class DiseaseModel {
    
public:

    /**
     * Models the evolution of the disease within an individual entity in the 
     * population and the transmission of the disease in a population.
     */
    DiseaseModel(unsigned int latent_dwell_time,
            unsigned int incubating_dwell_time,
            unsigned int infectious_dwell_time,
            unsigned int asympt_dwell_time,
            float prob_latent, float prob_incubating,
            float prob_infectious, float prob_asymptomatic,
            float latent_infectivity, float incubating_infectivity,
            float infectious_infectivity, float asympt_infectivity
            ) :
    latentDwellTime(latent_dwell_time),
    incubatingDwellTime(incubating_dwell_time),
    infectiousDwellTime(infectious_dwell_time),
    asymptomaticDwellTime(asympt_dwell_time),
    probLatent(prob_latent),
    probIncubating(prob_incubating),
    probInfectious(prob_infectious),
    probAsymptomatic(prob_asymptomatic),
    latentInfectivity(latent_infectivity),
    incubatingInfectivity(incubating_infectivity),
    infectiousInfectivity(infectious_infectivity),
    asymptInfectivity(asympt_infectivity) {}
    
    
    /**
     * The probabilistic time transition system models the progression of 
     * the infection. A PTTS reflects the reaction that changes the state 
     * of an individual within the population
     * (i.e. w/in host disease progression). Each individual infected by a 
     * disease will progress through a series of disease states.
     */
    void ptts(Person &individual, unsigned int curTime, double randNum) {

        switch (individual.getInfectionState()) {

            case UNINFECTED:
            {
               individual.setPrevChangeStateTime(0);
            }
            break;
            case LATENT:
            {
                if( (curTime - individual.getPrevChangeStateTime())
                        >= latentDwellTime &&
                        (randNum < probLatent) ) {
                    individual.setInfectionState(INFECTIOUS);
                    individual.setPrevChangeStateTime(latentDwellTime);
                }
            }
            break;
            case INCUBATING:
            {
                if( (curTime - individual.getPrevChangeStateTime())
                        >= incubatingDwellTime &&
                        (randNum < probIncubating) ) {
                    individual.setInfectionState(ASYMPTOMATIC);
                    individual.setPrevChangeStateTime(incubatingDwellTime);
                }
            }
            break;
            case INFECTIOUS:
            {
                if( (curTime - individual.getPrevChangeStateTime())
                        >= infectiousDwellTime &&
                        (randNum < probInfectious) ) {
                    individual.setInfectionState(RECOVERED);
                    individual.setPrevChangeStateTime(infectiousDwellTime);
                }
                
            }
            break;
            case ASYMPTOMATIC:
            {
                if( (curTime - individual.getPrevChangeStateTime())
                        >= asymptomaticDwellTime &&
                        (randNum < probAsymptomatic) ) {
                    individual.setInfectionState(RECOVERED);
                    individual.setPrevChangeStateTime(asymptomaticDwellTime);
                }
            }
            break;
            case RECOVERED:
            {
               individual.setPrevChangeStateTime(0);
            }
            break;
            default:
                std::cerr << "Unhandled infection state type encountered"
                        " in DiseaseModel.h\n";
                ASSERT(false);
        }
        
    }
    
    /**
     * Defines the infection behavior of co-located individuals (probability of
     * an uninfected individual becoming infected). It determines the
     * probability of transmission within the group of individuals at any
     * location (i.e. between host disease transmission). 
     */
    void reaction(std::unordered_map<unsigned long, Person> &population,
            unsigned int curTime, double randNum) {
        int uninfectedN, recoveredN, latentN = 0;
        int incubatingN, infectiousN, asymptomaticN = 0;
        std::vector<unsigned long> uninfectedPopulationIds;
        for (auto indiv : &population) {
            // Progress the infections state of every individual in the 
            // population at a given location based on the probabilistic timed
            // transition system.
            ptts(indiv.second, curTime, randNum);
            // Collect identifier of the uninfected individuals in the
            // population and count the number of uninfected.
            if (indiv.second.getInfectionState() == UNINFECTED) {
                uninfectedPopulationIds.push_back(indiv.getId());
                uninfectedN++;
            }
            // Count the number of people infected.
            else if (indiv.second.getInfectionState() == LATENT) {
                latentN++;
            }
            // Count the number of people infected.
            else if (indiv.second.getInfectionState() == INCUBATING) {
                incubatingN++;
            }
            // Count the number of people infected.
            else if (indiv.second.getInfectionState() == INFECTIOUS) {
                infectiousN++;
            }
            // Count the number of people infected.
            else if (indiv.second.getInfectionState() == ASYMPTOMATIC) {
                asymptomaticN++;
            }
            // Count the number of people recovered.
            else {
                recoveredN++;
            }
        }
        // Determine if an individual in population can transmit the disease.
        bool canTransmit = ((latentN > 0) || (incubatingN > 0) ||
                (infectiousN > 0) || (asymptomaticN > 0));
         // Update the status of uninfected persons in a local population.
        if( (uninfectedN > 0) && canTransmit ) {
            for(size_t i = 0; i < uninfectedN; i++) {
                unsigned long pid = uninfectedPopulationIds[i];
                double susceptibility = population[pid].getSusceptibilityScore();
                double transmissibility = population[pid].getTransmissiblityScore();
                bool isVaccinated = population[pid].vaccineState();
                double prob_latent, prob_incubating,
                        prob_infectious, prob_asympt = 0;
                if(latentN) {
                    prob_latent = 1 - (susceptibility*
                            transmissibility*latentInfectivity);
                    prob_latent = std::pow(prob_latent, latentN);
                }
                if(incubatingN) {
                    prob_incubating = 1 - (susceptibility*
                            transmissibility*incubatingInfectivity);
                    prob_incubating = std::pow(prob_incubating, incubatingN);
                }
                if(infectiousN) {
                    prob_infectious = 1 - (susceptibility*
                            transmissibility*infectiousInfectivity);
                    prob_infectious = std::pow(prob_infectious, infectiousN);
                }
                if(asymptomaticN) {
                    prob_asympt = 1 - (susceptibility*
                            transmissibility*asymptInfectivity);
                    prob_asympt = std::pow(prob_asympt, asymptomaticN);
                }
                double prodProb = (prob_latent * prob_incubating *
                        prob_infectious * prob_asympt);
                double prob_of_disease =
                        1 - ( std::pow(prodProb, curTime -
                        population[pid].getLocArrivalTime()) );
                // Individual becomes infected based on the given probability.
                if(randNum < prob_of_diesease) {
                    // 
                    if(isVaccinated) {
                        if(randNum < probLatent) {
                            population[pid].setInfectionState(LATENT);
                        } else if(randNum < probIncubating) {
                            population[pid].setInfectionState(INCUBATING);
                        } else {
                            population[pid].setInfectionState(RECOVERED);
                        }
                    } else {
                        if(randNum < probLatent) {
                            population[pid].setInfectionState(LATENT);
                        }
                        else {
                            population[pid].setInfectionState(INCUBATING);
                        }
                    }
                }
            }
        }
    }
    
private:
   /**
    * The dwell time for the latent state.
    */
   unsigned int latentDwellTime; 
   /**
    * The dwell time for the incubation state.
    */
   unsigned int incubatingDwellTime;
   /**
    * The dwell time for the infectious state.
    */
   unsigned int infectiousDwellTime;
   /**
    * The dwell time for the asymptomatic state.
    */
   unsigned int asymptomaticDwellTime; 
   /**
    * The probability of departing latent state.
    */
   float probLatent;
   /**
    * The probability of departing incubating state.
    */
   float probIncubating;
   /**
    * The probability of departing infectious state.
    */
   float probInfectious;
   /**
    * The probability of departing asymptomatic state.
    */
   float probAsymptomatic;
   /**
    * Rate of infectivity for a latent individual.
    */   
   float latentInfectivity;
   /**
    * Rate of infectivity for an incubating individual.
    */  
   float incubatingInfectivity;
   /**
    * Rate of infectivity for an infectious individual.
    */
   float infectiousInfectivity;
   /**
    * Rate of infectivity for an asymptomatic individual.
    */
   float asymptInfectivity;
};


#endif /* DISEASEMODEL_H */

