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

#include "ebolaAgent.h"
#include <string>

typedef std::vector<real> DblVec;
enum Values { S = 0, E = 1, I = 2, H = 3, F = 4, R = 5};

ebolaAgent::ebolaAgent(muse::AgentID id, muse::oclState* state, std::string country):
    oclAgent(id, state), compartments(6), inf(0) {
    myState = state;
    this->country = country;
}


void ebolaAgent::seir(const real* xl, real* xln) {
    // values for the disease being modeled
   //Liberia
    real Bi = .160;
    real Bh = .062;
    real Bf = .489;
    real A  = .08333;
    real Yh = .3086;
    real Ydh = .0993;
    real Yf = .4975;
    real Yi = .0667;
    real Yd = .0751;
    real Yih = .06297;
    real O1 = .5;
    real O2 = .5;
    real l = .197;
    if(country != "lib"){
        //Sierra Leone
        real Bi = .128;
        real Bh = .080;
        real Bf = .111;
        real A = .1;
        real Yh = .2427;
        real Ydh = .1597;
        real Yf = .238;
        real Yi = .05;
        real Yd = .0963;
        real Yih = .063;
        real O1 = .75;
        real O2 = .75;
        real l = .197;
    }
    const real beta = 1, mu = 5e-4, psi= 0.1;
    real N = xl[0] + xl[1] + xl[2] + xl[3] + xl[4] + xl[5];
    // represents change from compartment to compartment based on constants above
    xln[0] = N * mu - (((Bi * xl[0] * xl[2]) + (Bh * xl[0] * xl[3]) + (Bf * xl[0] * xl[4])) / N);
    xln[1] = (((Bi * xl[0] * xl[2]) + (Bh * xl[0] * xl[3]) + (Bf * xl[0] * xl[4])) / N) - (A * xl[E]);
    xln[2] = (A * xl[E]) - (Yh * l * xl[I]) - (Yd * (1 - l) * O1 * xl[I]) - (Yi * (1 - l) * (1-O1) * xl[I]);
    xln[3] = (Yh * l * xl[I]) - (Ydh * O2 * xl[H]) - (Yih * (1 - O2) * xl[H]);
    xln[4] = (Yd * (1 - l) * O1 * xl[I]) + (Ydh * O2 * xl[H]) - (Yf * xl[F]);
    xln[5] = (Yi * (1 - l) * (1-O1) * xl[I]) + (Yih * (1 - O2) * xl[H]) + (Yf * xl[F]);
//    std::cout << xln[0] << "\t" << xln[1] << "\t" << xln[2] 
//            << "\t" << xln[3] << "\t" << xln[4] << "\t" << xln[5] << std::endl;
}

void ebolaAgent::nextODE(float* xl) {
    const float h = kernel->step;
    // Use runge-kutta 4th order method here.
    real k1[compartments], k2[compartments],
            k3[compartments], k4[compartments],
            xlt[compartments];
    // Compute k1
    for (int i = 0; i < static_cast<int>(1/h); i++) {
        seir(xl, k1);
        // Compute k2 values using k1.
        for (int i = 0; i < compartments; i++) {
            xlt[i] = xl[i] + k1[i] * h / 2;
        }
        seir(xlt, k2);
        // Compute k3 values using k2.
        for (int i = 0; i < compartments; i++) {
            xlt[i] = xl[i] + k2[i] * h / 2;
        }
        seir(xlt, k3);
        // Compute k4 values using k3.
        for (int i = 0; i < compartments; i++) {
            xlt[i] = xl[i] + k3[i] * h;
        }
        seir(xlt, k4);

        // Compute the new set of values.
        for (int i = 0; i < compartments; i++) {
            xl[i] = xl[i] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;
//            if(i == I && (k1[i] + 2*k2[i] + 2*k3[i] + k4[i])* h / 6<0){
//                inf += (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;
//            }

        }
    }
    std::cout << xl[0] << "\t" << xl[1] << "\t" << xl[2] 
            << "\t" << xl[3] << "\t" << xl[4] << "\t" << xl[5] << std::endl;
//    std::cout << inf << std::endl;
}

void ebolaAgent::nextSSA(real* cv) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rnd(seed);
    //Liberia
    real Bi = .160;
    real Bh = .062;
    real Bf = .489;
    real A  = .08333;
    real Yh = .3086;
    real Ydh = .0993;
    real Yf = .4975;
    real Yi = .0667;
    real Yd = .0751;
    real Yih = .06297;
    real O1 = .5;
    real O2 = .5;
    real l = .197;
    if(country != "lib"){
        //Sierra Leone
        real Bi = .128;
        real Bh = .080;
        real Bf = .111;
        real A = .1;
        real Yh = .2427;
        real Ydh = .1597;
        real Yf = .238;
        real Yi = .05;
        real Yd = .0963;
        real Yih = .063;
        real O1 = .75;
        real O2 = .75;
        real l = .197;
    }
    const real beta = 1, mu = 5e-4, psi= 0.1;
    real N = cv[0] + cv[1] + cv[2] + cv[3] + cv[4] + cv[5];
    
    const DblVec EventChanges[] = {
 //  S   E   I   H   F   R
    {+1,  0,  0,  0,  0,  0},  // S = S + 1
    {-1,  1,  0,  0,  0,  0},  // I = I + 1, S = S - 1
    {0, -1, +1,  0,  0,  0},  // R = R + 1, I = I - 1
    {0,  0, -1, +1,  0,  0},
    {0,  0, -1,  0, +1,  0},
    {0,  0, -1,  0,  0, +1},
    {0,  0,  0, -1, +1,  0},
    {0,  0,  0, -1,  0, +1},
    {0,  0,  0,  0, -1, +1}
     };
    srand(time(NULL));
    real rates[] = { N * mu,
        (((Bi * cv[0] * cv[2]) + (Bh * cv[0] * cv[3]) + (Bf * cv[0] * cv[4])) / N),
        (A * cv[E]), 
        (Yh * l * cv[I]), 
        (Yd * (1 - l) * O1 * cv[I]), 
        (Yi * (1 - l) * (1-O1) * cv[I]),
        (Ydh * O2 * cv[H]), 
        (Yih * (1 - O2) * cv[H]), 
        (Yf * cv[F])};
    
    for (int i = 0; i < static_cast<int>(1/kernel->step); i++) {
        for (int i = 0; (i < 9); i++) {
            std::poisson_distribution<> pRNG(rates[i]);
            real num = (real)pRNG(rnd) * kernel->step;
            for (int j = 0; j < compartments; j++) {
                cv[j] = cv[j] + (EventChanges[i][j] * num);
//                if(j == 2 && EventChanges[i][j] == 1){
//                    inf = inf + (EventChanges[i][j] * num);
//                }
            }
        }
    }
    std::cout << cv[0] << "\t" << cv[1] <<  "\t" << cv[2] 
            << "\t" << cv[3] << "\t" << cv[4] << "\t" << cv[5] << std::endl;
//    std::cout << inf << std::endl;

}

std::string ebolaAgent::getKernel() {
    // check if using ssa or ode kernel
    // then kernel string is returned
    if (!kernel->ode) {
        std::string s = std::to_string(kernel->step);
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

        for (int i = 0; i < compartments-1; i++) {
            ssa_kernel_code.append("{");
            for (int j = 0; j < compartments; j++) {
                if (j != 0) {
                    ssa_kernel_code.append(", ");
                }
                if (i == j) {
                    ssa_kernel_code.append("-1");
                } else if (j-i == 1) {
                    ssa_kernel_code.append("1");
                } else {
                    ssa_kernel_code.append("0");
                }
            }
            if (i+1 == compartments-1) {
                ssa_kernel_code.append("}\n");
            } else {
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
    } else {
        std::string s = std::to_string(compartments);
        const char *pchar = s.c_str();
        s = std::to_string(kernel->step);
        const char *h = s.c_str();
        std::string ode_kernel_code = "void kernel run(global float* xl) {\n"
        "   int compartments = ";
        ode_kernel_code.append(pchar);
        ode_kernel_code.append(";\n"
        "   float h = ");
        ode_kernel_code.append(h);
        ode_kernel_code.append("f;\n"
        "   float A = (float)0.005f;\n"
        "   float B  = (float)0.004f;\n"
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
