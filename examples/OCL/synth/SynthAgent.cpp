#ifndef SYNTHAGENT_CPP
#define SYNTHAGENT_CPP
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
// Authors: Harrison Roth          rothhl@miamioh.edu
//          Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include "SynthAgent.h"
#include <string>

SynthAgent::SynthAgent(muse::AgentID id, muse::OclState* state):
    OclAgent(id, state) {
    myState = state;
}

void SynthAgent::seir(const real* xl, real* xln) {
    // values for the disease being modeled
    real A = 0.0005f;
    real B  = 0.00004f;
    // represents change from compartment to compartment
    // for experimenting synthetic epidemic
    for (int i = 1; i < kernel->compartments; i++) {
        xln[i] = (xl[i-1] * A) - (B * xl[i]);
    }
    xln[0] = -xln[1];
}

void SynthAgent::nextSSA(real* cv, std::vector<std::vector<int>> EventChanges, std::vector<real> rates) {
    // Create potential compartment moves for synthetic testing
    std::vector<std::vector<int>> EventChange;
    // Events created to move from one compartment to the next
    // No more complicated moves are added
    for (int i = 0; i < kernel->compartments-1; i++) {
        EventChange.push_back(std::vector<int>());
        for (int j = 0; j < kernel->compartments; j++) {
            if (i == j) {
                EventChange[i].push_back(-1);
            } else if (j-i == 1) {
                EventChange[i].push_back(1);
            } else {
                EventChange[i].push_back(0);
            }
        }
    }
    // constant rate for synthetic testing
    std::vector<real> rate;
    for (int i = 0; i < kernel->compartments-1; i++) {
        rate.push_back(cv[i] * .01);
    }
    // since events and rates are created, we pass them to the base class 
    // to process this agent
    muse::OclAgent::nextSSA(cv, EventChange, rate);
}

std::string SynthAgent::getKernel() {
    // check if using ssa or ode kernel
    // then kernel string is returned
    std::ostringstream oclKernel;
    if (!kernel->ode) {
        // Set function definition and variables needed in function
        oclKernel << "void kernel run(global float* cv, global int* random) {\n"
        "    int compartments = " << kernel->compartments << ";\n"
        "    int id = get_global_id(0)*compartments;\n"
        "    float stp =" << kernel->step << "f;\n"
        "    float rates["<< kernel->compartments << "];\n"
        "for (int i = 0; i < compartments; i++) {\n"
        "   rates[i] = cv[i] * 0.01f;\n"
        "}\n"
        // create possible transition events
        "    const float EventChanges[" << kernel->compartments << "-1]"
        "[" << kernel->compartments << "] = {\n";

        for (int i = 0; i < kernel->compartments-1; i++) {
            oclKernel << "{";
            for (int j = 0; j < kernel->compartments; j++) {
                if (j != 0) {
                    oclKernel << ", ";
                }
                if (i == j) {
                    oclKernel << "-1";
                } else if (j-i == 1) {
                    oclKernel << "1";
                } else {
                    oclKernel << "0";
                }
            }
            if (i+1 == kernel->compartments-1) {
                oclKernel << "}\n";
            } else {
                oclKernel << "},\n";
            }
        }
        oclKernel << "    };\n";
        // remaining OpenCL kernel code comes from calling the base class
    } else {
        // create seir function definition which will be called later
        oclKernel << "void kernel seir(local float* xl, local float* xln){\n"
        "   float A = (float)0.005f;\n"
        "   float B  = (float)0.004f;\n"
        "   int compartments = " << kernel->compartments << ";\n"
        "   int id = get_global_id(0)*compartments;\n"
        "    for (int i = 1; i < compartments; i++) {\n"
        "        xln[i] = (xl[i-1 + id] * A) - (B * xl[i + id]);\n"
        "   }\n"
        "    xln[0] = -xln[1];\n"
        "}\n";
        
        // set funciton definition and variables needed in function
        oclKernel << "void kernel run(global float* x) {\n"
        "   int compartments = " << kernel->compartments << ";\n"
        "   float h = " << kernel->step << "f;\n"
        "   int id = get_global_id(0)*compartments;\n"
        "  \n"
        "        // Use runge-kutta 4th order method here.\n"
        "   local float xl[" << kernel->compartments << "], "
        "k1[" << kernel->compartments << "], "
        "k2[" << kernel->compartments << "], "
        "k3[" << kernel->compartments << "], "
        "k4[" << kernel->compartments << "], "
        "xlt[" << kernel->compartments << "];\n"
        "   for (int i = 0; i < compartments; i++) {\n"
        "       xl[i] = x[i];\n"
        "   }\n";        
        // remaining OpenCL kernel code comes from calling the base class
    }
    oclKernel << muse::OclAgent::getKernel();
    return oclKernel.str();

}

#endif
