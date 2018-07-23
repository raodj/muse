R"(

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OH.
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
// Authors: Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

/** The top-level interface kernel that calls the user-defined OpenCL
    code for each agent it is scheduled to run.  This code is included
    at the end of the OpenCL kernel.  So all the user-defined
    structures for state and parameters are readily usable here.

    \note The params buf is read-only. But we don't make it __constant
    because constant buffer size is limited to 64K on nVidia. But applications
    may require larger set of parameters to be passed-in.
*/
kernel void muse(const int agentCount, __constant AgentID* agentIDList,
                 __global const hc_params_type* paramsBuf,
                 const int hostParamsSize,
                 __global hc_state_type* stateBuf, const int hostStateSize,
                 const Time lvt, const Time gvt,
                 __global struct MTrand_Info* rndGenInfo,
                 const int hostEndianCheck) {
    // Sanity check to ensure Endianness of host and device are same.
    // This is a fundamental underlying assumption in this implementation.
    if (hostEndianCheck != 0xaabbcc) {
        printf("Interfacing error: Endianness mismatch.\n"
               "Host checksum: %x. Device checksum: %x\n", hostEndianCheck,
               0xaabbcc);
    }
    // Just a sanity check to ensure that the state and parameter
    // structures are somewhat consistent between host and device.
    if ((sizeof(hc_state_type) != hostStateSize) ||
        (sizeof(hc_params_type) != hostParamsSize)) {
        printf("Interfacing error: State or parameter size mismatch.\n"
               "Host state size: %d. Device state  size: %ld\n"
               "Host params size: %d. Device params size: %ld\n",
               hostStateSize, sizeof(hc_state_type),
               hostParamsSize, sizeof(hc_params_type));
    }

    // Call the agent's OpenCL kernel
    const int id = get_global_id(0);
    // Make local copy of parameters and state to keep access fast
    hc_state_type state   = stateBuf[id];
    hc_params_type params = paramsBuf[id];
    // Run the user's OpenCL kernel
    // printf("Running agent %d at lvt: %lf.\n", id, lvt);
    hcKernel(id, &params, &state, rndGenInfo + get_local_id(0), lvt, gvt);
    // Copy the updated state back to global memory to reflect back to host
    stateBuf[id] = state;
}

)"
