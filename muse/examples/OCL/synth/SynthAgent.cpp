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
    real MU = 0.011;
    real A = 0.5;
    real V = 0.3333;
    real R0 = 3;
    real B  = (R0 * ((MU+A)*(MU+V)) / A);
    real N = xl[0] + xl[1] + xl[2] + xl[3];
    // represents change from compartment to compartment based on constants above
    xln[0] = (MU * N) - (MU * xl[0]) - (B * xl[2] * xl[0] / N);
    xln[1] = (B * xl[2] * xl[0] / N) - (A * xl[1]);
    xln[2] = (A * xl[1]) - ((V + MU) * xl[2]);
    xln[3] = (V * xl[2]) - (MU * xl[3]);
}

void SynthAgent::synth(const real* xl, real* xln) {
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

void SynthAgent::nextODE(float* xl) {
    const float h = kernel->step;
    // Use runge-kutta 4th order method here.
    real k1[kernel->compartments], k2[kernel->compartments],
            k3[kernel->compartments], k4[kernel->compartments],
            xlt[kernel->compartments];
    // Compute k1
    for (real j = 0; j < 1; j += kernel->step) {
        synth(xl, k1);
        // Compute k2 values using k1.
        for (int i = 0; i < kernel->compartments; i++) {
            xlt[i] = xl[i] + k1[i] * h / 2;
        }
        synth(xlt, k2);
        // Compute k3 values using k2.
        for (int i = 0; i < kernel->compartments; i++) {
            xlt[i] = xl[i] + k2[i] * h / 2;
        }
        synth(xlt, k3);
        // Compute k4 values using k3.
        for (int i = 0; i < kernel->compartments; i++) {
            xlt[i] = xl[i] + k3[i] * h;
        }
        synth(xlt, k4);

        // Compute the new set of values.
        for (int i = 0; i < kernel->compartments; i++) {
            xl[i] = xl[i] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;
        }
    }
}

void SynthAgent::nextSSA(real* cv) {
    // seed random value creation
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rnd(seed);
    // Create potential compartment moves for synthetic testing
    real EventChanges[kernel->compartments-1][kernel->compartments];
    // Events created to move from one compartment to the next
    // No more complicated moves are added
    for (int i = 0; i < kernel->compartments-1; i++) {
        for (int j = 0; j < kernel->compartments; j++) {
            if (i == j) {
                EventChanges[i][j] = -1;
            } else if (j-i == 1) {
                EventChanges[i][j] = 1;
            } else {
                EventChanges[i][j] = 0;
            }
        }
    }
    // constant rate for synthetic testing
    const real rates = .1;
    // loop based upon time step
    for (int i = 0; i < static_cast<int>(1/kernel->step); i++) {
        // loop through all potential events
        for (int i = 0; (i < kernel->compartments-1); i++) {
            // create ramdom value based on rate and compartment that is
            // being removed from
            std::poisson_distribution<> pRNG(rates * cv[i]);
            real num = (real)pRNG(rnd) * kernel->step;
            for (int j = 0; j < kernel->compartments; j++) {
                // add that value to the current value
                cv[j] = cv[j] + (EventChanges[i][j] * num);
            }
        }
    }
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
        "    const float rate = .01f;\n"
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
        oclKernel << "    };\n"
        // loop based on time step
        "   for(int x = 0; x < (int)1/stp; x++){\n"
        // loop through all potential events
        "     for(int i = 0; (i < (compartments-1)); i++) {\n"
        // using random values passed in and current state values, create random value
        "       float scale = rate * ((float)random[(id + x) % 100] / 100.0f) * stp * cv[i];\n"
        "        for(int j = 0; j < compartments; j++){\n"
        // add random value based on event impact on compartments
        "           cv[j+id] = cv[j+id] + (EventChanges[i][j] * scale);\n"
        "        }\n"
        "     }\n"
        "   }\n"
        "}\n";
    } else {
        // set funciton definition and variables needed in function
        oclKernel << "void kernel run(global float* xl) {\n"
        "   int compartments = " << kernel->compartments << ";\n"
        "   float h = " << kernel->step << "f;\n"
        "   float A = (float)0.005f;\n"
        "   float B  = (float)0.004f;\n"
        "   int id = get_global_id(0)*compartments;\n"
        "  \n"
        "        // Use runge-kutta 4th order method here.\n"
        "   float k1[" << kernel->compartments << "], "
        "k2[" << kernel->compartments << "], "
        "k3[" << kernel->compartments << "], "
        "k4[" << kernel->compartments << "], "
        "xlt[" << kernel->compartments << "];\n"
        "        // Compute k1\n"
        // loop based on time step and run runge kutta 4th order equations
        "   for(int i = 0; i < (int)1/h; i++){\n"
            "   for(int i = 1; i < compartments; i++){\n"
            "       k1[i] = (xl[i-1] * A) - (B * xl[i]);\n"
            "    }\n"
            "    k1[0] = -k1[1];\n"
            "    for(int i = 0; i < compartments; i++){\n"
            "        xlt[i] = xl[i] + k1[i] * h / 2;\n"
            "    }\n"

            "   for(int i = 1; i < compartments; i++){\n"
            "       k2[i] = (xlt[i-1] * A) - (B * xlt[i]);\n"
            "    }\n"
            "    k2[0] = -k2[1];\n"

            "    for(int i = 0; i < compartments; i++){\n"
            "        xlt[i] = xl[i] + k2[i] * h/ 2;\n"
            "    }\n"

            "   for(int i = 1; i < compartments; i++){\n"
            "       k3[i] = (xlt[i-1] * A) - (B * xlt[i]);\n"
            "    }\n"
            "    k3[0] = -k3[1];\n"

            "    for(int i = 0; i < compartments; i++){\n"
            "        xlt[i] = xl[i] + k3[i] * h;\n"
            "    }\n"

            "   for(int i = 1; i < compartments; i++){\n"
            "       k4[i] = (xlt[i-1] * A) - (B * xlt[i]);\n"
            "    }\n"
            "    k4[0] = -k4[1];\n"
            "\n"
            // update values based on runge kutta 4th order equation results
            "    for(int i = 0; i < compartments; i++){\n"
            "        xl[i] = xl[i] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;\n"
            "    }\n"
        "    }  \n"
        "}\n";
    }
    return oclKernel.str();

}

#endif
