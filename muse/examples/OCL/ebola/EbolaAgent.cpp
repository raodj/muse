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

#include "EbolaAgent.h"
#include <string>

typedef std::vector<real> DblVec;
enum Values { S = 0, E = 1, I = 2, H = 3, F = 4, R = 5};

EbolaAgent::EbolaAgent(muse::AgentID id, muse::OclState* state, std::string country):
    OclAgent(id, state), compartments(6), inf(0) {
    myState = state;
    this->country = country;
}


void EbolaAgent::seir(const real* xl, real* xln) {
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

void EbolaAgent::nextODE(float* xl) {
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
            if(i == 0 && (k1[0] + 2*k2[0] + 2*k3[0] + k4[0])<0){
                inf-= (k1[0] + 2*k2[0] + 2*k3[0] + k4[0])*h/6;
            }
        }
    }
//    std::cout << xl[0] << "\t" << xl[1] << "\t" << xl[2] 
//            << "\t" << xl[3] << "\t" << xl[4] << "\t" << xl[5] << std::endl;
    std::cout << inf << std::endl;
}

void EbolaAgent::nextSSA(real* cv) {
    // seed random number creation
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rnd(seed);
    // Create model variables based on Ebola model from Rivers et. al.
    // Liberia
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
        // Sierra Leone
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
    // Create array of potential events based on Ebola model
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
    // intialize rates based on Ebola model
    real rates[] = { N * mu,
        (((Bi * cv[0] * cv[2]) + (Bh * cv[0] * cv[3]) + (Bf * cv[0] * cv[4])) / N),
        (A * cv[E]), 
        (Yh * l * cv[I]), 
        (Yd * (1 - l) * O1 * cv[I]), 
        (Yi * (1 - l) * (1-O1) * cv[I]),
        (Ydh * O2 * cv[H]), 
        (Yih * (1 - O2) * cv[H]), 
        (Yf * cv[F])};
    
    // loop based on step size
    for (int i = 0; i < static_cast<int>(1/kernel->step); i++) {
        // loop through potential events
        for (int i = 0; (i < 9); i++) {
            // create random value based upon rates and time step
            std::poisson_distribution<> pRNG(rates[i]);
            real num = (real)pRNG(rnd) * kernel->step;
            // add random value to the current value
            for (int j = 0; j < compartments; j++) {
                cv[j] = cv[j] + (EventChanges[i][j] * num);
//                if(j == 2 && EventChanges[i][j] == 1 && num > 0){
//                    inf = inf + (EventChanges[i][j] * num);
//                }
            }
        }
    }
//    std::cout << cv[0] << "\t" << cv[1] <<  "\t" << cv[2] 
//            << "\t" << cv[3] << "\t" << cv[4] << "\t" << cv[5] << std::endl;
//    std::cout << inf << std::endl;

}

std::string EbolaAgent::getKernel() {
    // check if using ssa or ode kernel
    // then kernel string is returned
    std::ostringstream oclKernel;
    if (!kernel->ode) {
        oclKernel <<  "void kernel run(global float* cv, global int* random) {\n";
        if (country == "lib") {
            //Liberia
            oclKernel << "float Bi = .160f;\n"
            "float Bh = .062f;\n"
            "float Bf = .489f;\n"
            "float A  = .08333f;\n"
            "float Yh = .3086f;\n"
            "float Ydh = .0993f;\n"
            "float Yf = .4975f;\n"
            "float Yi = .0667f;\n"
            "float Yd = .0751f;\n"
            "float Yih = .06297f;\n"
            "float O1 = .5f;\n"
            "float O2 = .5f;\n"
            "float l = .197f;\n";
        } else {
            //Sierra Leone
            oclKernel << "float Bi = .128f;\n"
            "float Bh = .080f;\n"
            "float Bf = .111f;\n"
            "float A = .1f;\n"
            "float Yh = .2427f;\n"
            "float Ydh = .1597f;\n"
            "float Yf = .238f;\n"
            "float Yi = .05f;\n"
            "float Yd = .0963f;\n"
            "float Yih = .063f;\n"
            "float O1 = .75f;\n"
            "float O2 = .75f;\n"
            "float l = .197f;\n";
        }
        oclKernel << "    int compartments = 6;\n"
        "    int id = get_global_id(0)*compartments;\n"
        "   const float mu = 5e-4f;\n"
        "   float N = cv[0+id] + cv[1+id] + cv[2+id] + cv[3+id] + cv[4+id] + cv[5+id];\n"
        "    float stp =" << kernel->step << "f;\n"        
        " const float rates[] = { N * mu,\n"
        " (((Bi * cv[0] * cv[2]) + (Bh * cv[0] * cv[3]) + (Bf * cv[0] * cv[4])) / N),\n"
        " (A * cv[1]), \n"
        " (Yh * l * cv[2]), \n"
        " (Yd * (1 - l) * O1 * cv[2]), \n"
        " (Yi * (1 - l) * (1-O1) * cv[2]),\n"
        " (Ydh * O2 * cv[3]), \n"
        " (Yih * (1 - O2) * cv[3]), \n"
        " (Yf * cv[4])};\n"

        "    float EventChanges[9][6] = {\n"
        "    //  S   E   I   H   F   R\n"
        "     {+1,  0,  0,  0,  0,  0},\n"
        "     {-1,  1,  0,  0,  0,  0},\n"
        "     {0, -1, +1,  0,  0,  0},\n"
        "     {0,  0, -1, +1,  0,  0},\n"
        "     {0,  0, -1,  0, +1,  0},\n"
        "     {0,  0, -1,  0,  0, +1},\n"
        "     {0,  0,  0, -1, +1,  0},\n"
        "     {0,  0,  0, -1,  0, +1},\n"
        "     {0,  0,  0,  0, -1, +1}\n"
        "      };\n"
        "   for (int x = 0; x < (int)1/stp; x++) {\n"
        "     for (int i = 0; (i < 9); i++) {\n"
        "       float scale = rates[i] * ((float)random[(id + x) % 100] / 100.0f) * stp;\n"
        "        for (int j = 0; j < compartments; j++) {\n"
        "           cv[j+id] = cv[j+id] + (EventChanges[i][j] * scale);\n"
        "        }\n"
        "     }\n"
        "   }\n"
        "}\n";
        
    } else {
        oclKernel << "void kernel run(global float* xl) {\n"
        "   int compartments = 6;\n"
        "   float h = " << kernel->step << "f;\n";
        if (country == "lib") {
            //Liberia
            oclKernel << "float Bi = .160f;\n"
            "float Bh = .062f;\n"
            "float Bf = .489f;\n"
            "float A  = .08333f;\n"
            "float Yh = .3086f;\n"
            "float Ydh = .0993f;\n"
            "float Yf = .4975f;\n"
            "float Yi = .0667f;\n"
            "float Yd = .0751f;\n"
            "float Yih = .06297f;\n"
            "float O1 = .5f;\n"
            "float O2 = .5f;\n"
            "float l = .197f;\n";
        } else {
            // Sierra Leone
            oclKernel << "float Bi = .128f;\n"
            "float Bh = .080f;\n"
            "float Bf = .111f;\n"
            "float A = .1f;\n"
            "float Yh = .2427f;\n"
            "float Ydh = .1597f;\n"
            "float Yf = .238f;\n"
            "float Yi = .05f;\n"
            "float Yd = .0963f;\n"
            "float Yih = .063f;\n"
            "float O1 = .75f;\n"
            "float O2 = .75f;\n"
            "float l = .197f;\n";
        }
        
        oclKernel << "   int id = get_global_id(0)*6;\n"
        "\n"
        "const float mu = 5e-4f;\n"
        "float N = xl[0+id] + xl[1+id] + xl[2+id] + xl[3+id] + xl[4+id] + xl[5+id];\n"
        "        // Use runge-kutta 4th order method here.\n"
        "   float k1[6], k2[6], k3[6], k4[6], xlt[6];\n"
        "        // Compute k1\n"
        "   for(int j = 0; j < (int)(1.0f/h); j++){\n"
        "        k1[0] = N * mu - (((Bi * xl[0+id] * xl[2+id]) + (Bh * xl[0+id] * xl[3+id]) + (Bf * xl[0+id] * xl[4+id])) / N);\n"
        "        k1[1] = (((Bi * xl[0+id] * xl[2+id]) + (Bh * xl[0+id] * xl[3+id]) + (Bf * xl[0+id] * xl[4+id])) / N) - (A * xl[1+id]);\n"
        "        k1[2] = (A * xl[1+id]) - (Yh * l * xl[2+id]) - (Yd * (1 - l) * O1 * xl[2+id]) - (Yi * (1 - l) * (1-O1) * xl[2+id]);\n"
        "        k1[3] = (Yh * l * xl[2+id]) - (Ydh * O2 * xl[3+id]) - (Yih * (1 - O2) * xl[3+id]);\n"
        "        k1[4] = (Yd * (1 - l) * O1 * xl[2+id]) + (Ydh * O2 * xl[3+id]) - (Yf * xl[4+id]);\n"
        "        k1[5] = (Yi * (1 - l) * (1-O1) * xl[2+id]) + (Yih * (1 - O2) * xl[3+id]) + (Yf * xl[4+id]);\n"
        "\n"
        "        // Compute k2 values using k1.\n"
        "        for (int i = 0; i < compartments; i++) {\n"
        "            xlt[i] = xl[i+id] + k1[i] * h / 2;\n"
        "        }\n"
        "        \n"
        "        k2[0] = N * mu - (((Bi * xlt[0] * xlt[2]) + (Bh * xlt[0] * xlt[3]) + (Bf * xlt[0] * xlt[4])) / N);\n"
        "        k2[1] = (((Bi * xlt[0] * xlt[2]) + (Bh * xlt[0] * xlt[3]) + (Bf * xlt[0] * xlt[4])) / N) - (A * xlt[1]);\n"
        "        k2[2] = (A * xlt[1]) - (Yh * l * xlt[2]) - (Yd * (1 - l) * O1 * xlt[2]) - (Yi * (1 - l) * (1-O1) * xlt[2]);\n"
        "        k2[3] = (Yh * l * xlt[2]) - (Ydh * O2 * xlt[3]) - (Yih * (1 - O2) * xlt[3]);\n"
        "        k2[4] = (Yd * (1 - l) * O1 * xlt[2]) + (Ydh * O2 * xlt[3]) - (Yf * xlt[4]);\n"
        "        k2[5] = (Yi * (1 - l) * (1-O1) * xlt[2]) + (Yih * (1 - O2) * xlt[3]) + (Yf * xlt[4]);\n"
        "\n"
        "        // Compute k3 values using k2.\n"
        "        for (int i = 0; i < compartments; i++) {\n"
        "            xlt[i] = xl[i+id] + k2[i] * h / 2;\n"
        "        }\n"
        "\n"
        "        k3[0] = N * mu - (((Bi * xlt[0] * xlt[2]) + (Bh * xlt[0] * xlt[3]) + (Bf * xlt[0] * xlt[4])) / N);\n"
        "        k3[1] = (((Bi * xlt[0] * xlt[2]) + (Bh * xlt[0] * xlt[3]) + (Bf * xlt[0] * xlt[4])) / N) - (A * xlt[1]);\n"
        "        k3[2] = (A * xlt[1]) - (Yh * l * xlt[2]) - (Yd * (1 - l) * O1 * xlt[2]) - (Yi * (1 - l) * (1-O1) * xlt[2]);\n"
        "        k3[3] = (Yh * l * xlt[2]) - (Ydh * O2 * xlt[3]) - (Yih * (1 - O2) * xlt[3]);\n"
        "        k3[4] = (Yd * (1 - l) * O1 * xlt[2]) + (Ydh * O2 * xlt[3]) - (Yf * xlt[4]);\n"
        "        k3[5] = (Yi * (1 - l) * (1-O1) * xlt[2]) + (Yih * (1 - O2) * xlt[3]) + (Yf * xlt[4]);\n"
        "\n"
        "        // Compute k4 values using k3.\n"
        "        for (int i = 0; i < compartments; i++) {\n"
        "            xlt[i] = xl[i+id] + k3[i] * h;\n"
        "        }\n"
        "\n"
        "        k4[0] = N * mu - (((Bi * xlt[0] * xlt[2]) + (Bh * xlt[0] * xlt[3]) + (Bf * xlt[0] * xlt[4])) / N);\n"
        "        k4[1] = (((Bi * xlt[0] * xlt[2]) + (Bh * xlt[0] * xlt[3]) + (Bf * xlt[0] * xlt[4])) / N) - (A * xlt[1]);\n"
        "        k4[2] = (A * xlt[1]) - (Yh * l * xlt[2]) - (Yd * (1 - l) * O1 * xlt[2]) - (Yi * (1 - l) * (1-O1) * xlt[2]);\n"
        "        k4[3] = (Yh * l * xlt[2]) - (Ydh * O2 * xlt[3]) - (Yih * (1 - O2) * xlt[3]);\n"
        "        k4[4] = (Yd * (1 - l) * O1 * xlt[2]) + (Ydh * O2 * xlt[3]) - (Yf * xlt[4]);\n"
        "        k4[5] = (Yi * (1 - l) * (1-O1) * xlt[2]) + (Yih * (1 - O2) * xlt[3]) + (Yf * xlt[4]);\n"
        "\n"
        "        // Compute the new set of values.\n"
        "        for (int i = 0; i < compartments; i++) {\n"
        "            xl[i+id] = xl[i+id] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;\n"
        "        }\n"
        "    }  \n"
        "}\n";
    }
    return oclKernel.str();
}
