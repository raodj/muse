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
    
    if(country == "lib") {
        Bi = .160;
        Bh = .062;
        Bf = .489;
        A  = .08333;
        Yh = .3086;
        Ydh = .0993;
        Yf = .4975;
        Yi = .0667;
        Yd = .0751;
        Yih = .06297;
        O1 = .5;
        O2 = .5;
        l = .197;
    } else {
        //Sierra Leone
        Bi = .128;
        Bh = .080;
        Bf = .111;
        A = .1;
        Yh = .2427;
        Ydh = .1597;
        Yf = .238;
        Yi = .05;
        Yd = .0963;
        Yih = .063;
        O1 = .75;
        O2 = .75;
        l = .197;
    }
}


void EbolaAgent::seir(const real* xl, real* xln) {
    // values for the disease being modeled    
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

void EbolaAgent::nextSSA(real* cv, std::vector<std::vector<int>> EventChanges, std::vector<real> rates) {
    // seed random number creation
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rnd(seed);
    // Create model variables based on Ebola model from Rivers et. al.
    const real beta = 1, mu = 5e-4, psi= 0.1;
    real N = cv[0] + cv[1] + cv[2] + cv[3] + cv[4] + cv[5];
    // Create array of potential events based on Ebola model
    const std::vector<std::vector<int>> EventChange = {
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
    std::vector<real> rate = { N * mu,
        (((Bi * cv[0] * cv[2]) + (Bh * cv[0] * cv[3]) + (Bf * cv[0] * cv[4])) / N),
        (A * cv[E]), 
        (Yh * l * cv[I]), 
        (Yd * (1 - l) * O1 * cv[I]), 
        (Yi * (1 - l) * (1-O1) * cv[I]),
        (Ydh * O2 * cv[H]), 
        (Yih * (1 - O2) * cv[H]), 
        (Yf * cv[F])};
    
    muse::OclAgent::nextSSA(cv, EventChange, rate);
//    std::cout << cv[0] << "\t" << cv[1] <<  "\t" << cv[2] 
//            << "\t" << cv[3] << "\t" << cv[4] << "\t" << cv[5] << std::endl;
//    std::cout << inf << std::endl;

}

std::string EbolaAgent::getKernel() {
    // create ostream to build OpenCL kernel code
    std::ostringstream oclKernel;
    // check if running ode or ssa
    if (!kernel->ode) {
        // set function definition
        oclKernel <<  "void kernel run(global float* cv, global int* random) {\n";
    } else {
        // create seir function definition to be called in main ode loop
        oclKernel << "void kernel seir(local float* xl, local float xln[]){\n";
    }
    // create variables needed in ebola model
    oclKernel << "float Bi = "<< Bi << ";\n"
    "float Bh = "<< Bh << ";\n"
    "float Bf = "<< Bf << ";\n"
    "float A  = "<< A << ";\n"
    "float Yh = "<< Yh << ";\n"
    "float Ydh = "<< Ydh << ";\n"
    "float Yf = "<< Yf << ";\n"
    "float Yi = "<< Yi << ";\n"
    "float Yd = "<< Yd << ";\n"
    "float Yih = "<< Yih << ";\n"
    "float O1 = "<< O1 << ";\n"
    "float O2 = "<< O2 << ";\n"
    "float l = "<< l << ";\n";
    
    if (!kernel->ode) {
        // using ssa
        // create remaining variables needed
        oclKernel << "    int compartments = 6;\n"
        "    int id = get_global_id(0)*compartments;\n"
        "   const float mu = 5e-4f;\n"
        "   float N = cv[0+id] + cv[1+id] + cv[2+id] + cv[3+id] + cv[4+id] + cv[5+id];\n"
        "    float stp =" << kernel->step << "f;\n"   
        // set transition rates from ebola model
        " const float rates[] = { N * mu,\n"
        " (((Bi * cv[0] * cv[2]) + (Bh * cv[0] * cv[3]) + (Bf * cv[0] * cv[4])) / N),\n"
        " (A * cv[1]), \n"
        " (Yh * l * cv[2]), \n"
        " (Yd * (1 - l) * O1 * cv[2]), \n"
        " (Yi * (1 - l) * (1-O1) * cv[2]),\n"
        " (Ydh * O2 * cv[3]), \n"
        " (Yih * (1 - O2) * cv[3]), \n"
        " (Yf * cv[4])};\n"
         // create potential events from ebola model
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
        "      };\n";
        // rates and event changes used in code 
        // that will come from the base class
        
    } else {
        // using ode
        // complete the seir function
        oclKernel << "   int id = get_global_id(0)*6;\n"
            "const float mu = 5e-4;\n"
            "float N = xl[id + 0] + xl[id + 1] + xl[id + 2] + xl[id + 3] + xl[id + 4] + xl[id + 5];\n"
            // represents change from compartment to compartment based on constants above
            "xln[0] = N * mu - (((Bi * xl[id + 0] * xl[id + 2]) + (Bh * xl[id + 0] * xl[id + 3]) + (Bf * xl[id + 0] * xl[id + 4])) / N);\n"
            "xln[1] = (((Bi * xl[id + 0] * xl[id + 2]) + (Bh * xl[id + 0] * xl[id + 3]) + (Bf * xl[id + 0] * xl[id + 4])) / N) - (A * xl[id + 1]);\n"
            "xln[2] = (A * xl[id + 1]) - (Yh * l * xl[id + 2]) - (Yd * (1 - l) * O1 * xl[id + 2]) - (Yi * (1 - l) * (1-O1) * xl[id + 2]);\n"
            "xln[3] = (Yh * l * xl[id + 2]) - (Ydh * O2 * xl[id + 3]) - (Yih * (1 - O2) * xl[id + 3]);\n"
            "xln[4] = (Yd * (1 - l) * O1 * xl[id + 2]) + (Ydh * O2 * xl[id + 3]) - (Yf * xl[id + 4]);\n"
            "xln[5] = (Yi * (1 - l) * (1-O1) * xl[id + 2]) + (Yih * (1 - O2) * xl[id + 3]) + (Yf * xl[id + 4]);\n"
        "}";
        
        // set funcion definition and create initial variables needed
        oclKernel << "void kernel run(global float* x) {\n"
        "   int compartments = 6;\n"
        "   float h = " << kernel->step << "f;\n"
        "   int id = get_global_id(0)*6;\n"
        "\n"
        "   local float xl[6], k1[6], k2[6], k3[6], k4[6], xlt[6];\n"
        "   for (int i = 0; i < 6; i++) {\n"
        "       xl[i] = x[i];\n"
        "   }\n";
        }
    // remaining OpenCL kernel code will come from the base class
    oclKernel << muse::OclAgent::getKernel();
    return oclKernel.str();
}
