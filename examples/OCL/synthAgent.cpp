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

#include "synthAgent.h"


synthAgent::synthAgent(muse::AgentID id, muse::oclState* state, bool ocl, float stp, int compartmentNum, bool useODE):
    oclAgent(id, state, ocl, stp, compartmentNum, useODE){
    myState = state;
    useOCL= ocl;
    step = stp;
    compartments = compartmentNum;
    ode = useODE;
}


void synthAgent::seir(const real xl[4], real xln[4]) {
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

void synthAgent::synth(const real xl[4], real xln[4]) {
    // values for the disease being modeled
    real A = 0.0005f;
    real B  = 0.00004f;
    // represents change from compartment to compartment 
    // for experimenting synthetic epidemic
    for(int i = 1; i < compartments; i++){
        xln[i] = (xl[i-1] * A) - (B * xl[i]);
    }
    xln[0] = -xln[1];
}

void synthAgent::nextODE(float* xl) {
    const float h = step;
    // Use runge-kutta 4th order method here.
    real k1[compartments], k2[compartments], k3[compartments], k4[compartments], xlt[compartments];
    // Compute k1
    for(real j = 0; j < 1; j += step){
        synth(xl, k1);
        // Compute k2 values using k1.
        for(int i = 0; i < compartments; i++){
            xlt[i] = xl[i] + k1[i] * h / 2;
        }
        synth(xlt, k2);
        // Compute k3 values using k2.
        for(int i = 0; i < compartments; i++){
            xlt[i] = xl[i] + k2[i] * h / 2;
        }
        synth(xlt, k3);
        // Compute k4 values using k3.
        for(int i = 0; i < compartments; i++){
            xlt[i] = xl[i] + k3[i] * h;
        }
        synth(xlt, k4);

        // Compute the new set of values.
        for(int i = 0; i < compartments; i++){
            xl[i] = xl[i] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;
        }
    }
}

void synthAgent::nextSSA(real* cv){
    real EventChanges[compartments-1][compartments];
    for(int i = 0; i < compartments-1; i++){
        for(int j = 0; j < compartments; j++){
            if(i==j){
                EventChanges[i][j] = -1;
            }
            else if(j-i == 1){
                EventChanges[i][j] = 1;
            }else{
                EventChanges[i][j] = 0;
            }

        }
    }
    srand ( time(NULL) );
    const real rates = .1;
    for(int i = 0; i < (int)1/step; i++){
        for(int i = 0; (i < compartments-1); i++) { 
            for(int j = 0; j < compartments; j++){
                float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                float scale = rates*step*r;
                cv[j] = cv[j] + (EventChanges[i][j] * scale); 
            }
        }
    }
}

std::string synthAgent::getKernel(){
    // check if using ssa or ode kernel
    // then kernel string is returned
    if(!ode){
        std::string s = std::to_string(step);
        const char *pchar = s.c_str();
        s = std::to_string(compartments);
        const char *comp = s.c_str();
        float rate = .0001f;
        s = std::to_string(rate);
        const char *r = s.c_str();
        std::string ssa_kernel_code = "void kernel run(global float* cv, global float* random) {\n"
        "    int compartments = ";
        ssa_kernel_code.append(comp);
        ssa_kernel_code.append(";\n"
        "    int id = get_global_id(0)*compartments;\n"
        "    float stp =");
        ssa_kernel_code.append(pchar);
        ssa_kernel_code.append("f;\n"
        "    const float rate = "); 
        ssa_kernel_code.append(r);
        ssa_kernel_code.append("f;\n"
        "    const float EventChanges[");
        ssa_kernel_code.append(comp);
        ssa_kernel_code.append("+1][");
        ssa_kernel_code.append(comp);
        ssa_kernel_code.append("] = {\n");
        
        for(int i = 0; i < compartments-1; i++){
            ssa_kernel_code.append("{");
            for(int j = 0; j < compartments; j++){
                if(j!=0){
                    ssa_kernel_code.append(", ");
                }
                if(i==j){
                    ssa_kernel_code.append("-1");
                }
                else if(j-i == 1){
                    ssa_kernel_code.append("1");
                }else{
                    ssa_kernel_code.append("0");
                }
            }
            if(i+1 == compartments-1){
                ssa_kernel_code.append("}\n");
            }else{
                ssa_kernel_code.append("},\n");
            }  
        }
        ssa_kernel_code.append("    };\n"
        "   for(int x = 0; x < (int)1/stp; x++){\n"
        "     for(int i = 0; (i < (compartments-1)); i++) {\n"
        "        for(int j = 0; j < compartments; j++){\n"
        "           float scale = rate*random[(id+x)%100]*stp;\n"
//        "           cv[j+id] = cv[j+id] + (EventChanges[i][j] * scale);\n"
        "           cv[j+id] = cv[j+id] + scale;\n"
        "        }\n"
        "     }\n"
        "   }\n"
        "}\n");
        return ssa_kernel_code;
    }else{
        std::string s = std::to_string(compartments);
        const char *pchar = s.c_str();
        s = std::to_string(step);
        const char *h = s.c_str();
        std::string ode_kernel_code = "void kernel run(global float* xl) {\n"
        "   int compartments = ";
        ode_kernel_code.append(pchar);
        ode_kernel_code.append(";\n"
        "   float h = ");
        ode_kernel_code.append(h);
        ode_kernel_code.append("f;\n"
        "   float A = (float)0.0005f;\n"
        "   float B  = (float)0.00004f;\n"
        "   int id = get_global_id(0)*compartments;\n"
        "  \n"
        "        // Use runge-kutta 4th order method here.\n"
        "   float k1[");
        ode_kernel_code.append(pchar);
        ode_kernel_code.append("], k2[");
        ode_kernel_code.append(pchar);
        ode_kernel_code.append("], k3[");
         ode_kernel_code.append(pchar);
        ode_kernel_code.append("], k4[");
         ode_kernel_code.append(pchar);
        ode_kernel_code.append("], xlt[");
         ode_kernel_code.append(pchar);
        ode_kernel_code.append("];\n");
         ode_kernel_code.append("        // Compute k1\n"
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

            "    for(int i = 0; i < compartments; i++){\n"
            "        xl[i] = xl[i] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;\n"
            "    }\n"        
                
        "    }  \n"
        "}\n");
        return ode_kernel_code;
    }
}

#endif