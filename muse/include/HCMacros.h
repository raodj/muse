#ifndef MUSE_HC_MACROS_H
#define MUSE_HC_MACROS_H

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
// Authors: Dhananjai M. Rao    raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

/** \file HCMacros.h

    \brief This file provides several macros that facilitate
    heterogeneous compute model generation.  Refer to the
    documentation on the macros for further details.
*/

// Forward declaration for random number generation
struct MTrand_Info;

/** \def HC_STATE(x)
    
    \brief A convenience macro for generating subset of state
    variables to be shared with kernel running on the GPU.

    In heterogeneous computing (HC) mode, MUSE copies parts of an
    Agent (aka LP) state to the GPU and runs the kernel on the GPU.
    In order to streamline operations and code readability between GPU
    and CPU, several interface methods are required to be defined in
    muse::HCState class.  This is convenience macro that can be used
    to create a derived HCState class.  This macro enables using 1
    single definition for creating both CPU state and kernel state as
    shown in the code example below:

    \code
    
    class SiSState : muse::HCState {
    public:
        HC_STATE(
            int sus;
            int inf;
        );
    };

    \endcode

    \note This macro generates the states as a substructure with a
    fixed name "hc".

    \note It assumes that all the state variables are primitive data
    types.
*/
#define HC_STATE(x)                                             \
                                                                \
    /* Definition of sub-struct hc for use on CPU */            \
    struct hc { x } hc;                                         \
                                                                \
    /* Return structure definition for kernel code */           \
    std::string getHCStateDefinition() const override {         \
        return "struct hc_state { " #x " };";                   \
    }                                                           \
                                                                \
    /* Override method to determine size */                     \
    virtual int getHCStateSize() const override {               \
        return sizeof(struct hc);                               \
    }                                                           \
                                                                \
    /* Copy state to GPU */                                     \
    virtual void copyToDevice(void *dest,                       \
                              const int bufSz) override {       \
        ASSERT(bufSz == getHCStateSize());                      \
        UNUSED_PARAM(bufSz);                                    \
        memcpy(dest, &this->hc, getHCStateSize());              \
    }                                                           \
                                                                \
    /* Copy data from GPU into state */                         \
    virtual void copyFromDevice(const void* src,                \
                                const int bufSz) override {     \
        ASSERT(bufSz == getHCStateSize());                      \
        UNUSED_PARAM(bufSz);                                    \
        memcpy(&this->hc, src, getHCStateSize());               \
    }


/** \def HC_PARAMETERS(x)
    
    \brief A convenience macro for generating subset of parameters to
    be shared with kernel running on the GPU.

    <p>In heterogeneous computing (HC) mode, parameters (i.e.,
    instance variables in an Agent) are copied to the GPU for use on
    the GPU.  The muse::HCAgent class requires 3 interface methods to
    be included to faciltate copying parameters to the GPU.  This is
    convenience macro that can be used to create a custom
    implementation for the interface methods in a derived HCAgent
    class.  This macro enables using 1 single definition for creating
    both CPU state and kernel state as shown in the code example
    below:

    \code
    
    class SiSAgent : muse::HCAgent {
    public:
        // other methods go here.
    protected:
        HC_PARAMETERS(
            double beta;
            double gamma;
        );
    };

    \endcode

    \note This macro generates the parameters as a substructure with a
    fixed name "hc".

    \note It assumes that all the state variables are primitive data
    types.
*/
#define HC_PARAMETERS(x)                                  \
                                                          \
    /* Definition of sub-struct hc for use on CPU */      \
    struct hc { x } hc;                                   \
                                                          \
    /* Return structure definition for kernel code */     \
    std::string getHCParamDefinition() const override {   \
        return "struct hc_params {" #x "};";              \
    }                                                     \
                                                          \
    /* Override method to determine size */               \
    virtual int getHCParamSize() const override {         \
        return sizeof(struct hc);                         \
    }                                                     \
                                                          \
    /* Copy state to GPU */                               \
    virtual void copyToDevice(void *dest,                 \
                              const int bufSz) override { \
        ASSERT(bufSz == getHCParamSize());                \
        memcpy(dest, &this->hc, bufSz);                   \
    }

/** \def HC_PRE_SOURCE(AgentClass, StateClass, ...)
    
    \brief A convenience macro for generating generic support code for
    OpenCL kernel for running either on CPU or on GPU.
    
    <p>In heterogeneous computing (HC) mode, parts of a model's logic
    are executed on a different device/GPU.  In order to generate code
    to be run on a device/GPU, MUSE requires an agent to supply the
    necssary code.  This is convenience macro that can be used to
    create custom implementation for the getHCPreSupportCode API
    method in a derived HCAgent source code.  This macro is meant to
    be used in the source file (.cpp file) of an agent as shown below:

    \code

    HC_SOURCE(
       typedef double real;

       void helperMethod(int i, int j) {
       }
    );
    \endcode

    \note This macro is meant to be used only the C++ source file.
*/
#define HC_PRE_SOURCE(AgentClass, StateClass, ...)                      \
                                                                        \
    /* Definition for running on CPU if OpenCL is not used */           \
    __VA_ARGS__                                                         \
                                                                        \
    /* Return kernel body for use on GPU */                             \
    std::string AgentClass::getHCPreSupportCode() {                     \
        return #__VA_ARGS__ ;                                           \
    }

/** \def HC_POST_SOURCE(AgentClass, StateClass, ...)
    
    \brief A convenience macro for generating generic support code for
    OpenCL kernel for running either on CPU or on GPU.
    
    <p>In heterogeneous computing (HC) mode, parts of a model's logic
    are executed on a different device/GPU.  In order to generate code
    to be run on a device/GPU, MUSE requires an agent to supply the
    necssary code.  This is convenience macro that can be used to
    create custom implementation for the getHCPostSupportCode API
    method in a derived HCAgent source code.  This macro is meant to
    be used in the source file (.cpp file) of an agent as shown below:

    \code

    HC_SOURCE(
       typedef double real;

       void helperMethod(int i, int j) {
       }
    );
    \endcode

    \note This macro is meant to be used only the C++ source file.
*/
#define HC_POST_SOURCE(AgentClass, StateClass, ...)                     \
                                                                        \
    /* Add aliases to state and params to streamline code */            \
    typedef struct StateClass::hc hc_state_type;                        \
    typedef struct AgentClass::hc hc_params_type;                       \
                                                                        \
    /* Definition for running on CPU if OpenCL is not used */           \
    __VA_ARGS__                                                         \
                                                                        \
    /* Return kernel body for use on GPU */                             \
    std::string AgentClass::getHCPostSupportCode() {                    \
        return                                                          \
            "\n\n#ifndef hc_state_type\n"                               \
            "typedef struct hc_state hc_state_type;\n"                  \
            "typedef struct hc_params hc_params_type;\n"                \
            "#endif\n\n"                                                \
            #__VA_ARGS__ ;                                              \
    }


/** \def HC_KERNEL(AgentClass, StateClass)

    \breif A convenience macro to just delcare the hcKernel method in
    an agent's class definition (in a header file).
*/
#define HC_KERNEL_DECL(AgentClass, StateClass)                 \
    /* Definition for running on CPU if OpenCL is not used */  \
    void hcKernel(int, const struct AgentClass::hc*,           \
                  struct StateClass::hc*,                      \
                  struct MTrand_Info* rndInfo,                 \
                  const muse::Time lvt,                        \
                  const muse::Time gvt);                       \
                                                               \
    /* This is glue method to call the HC kernel directly */   \
    /* when OpenCL is not being used */                        \
    void executeHCkernel() override;                           \
    /* Return kernel body for use on GPU */                    \
    std::string getHCkernelDefinition() override;


/** \def HC_KERNEL(AgentClass, StateClass, ...)
    
    \brief A convenience macro for generating the OpenCL kernel for
    running either on CPU or on GPU.
    
    <p>In heterogeneous computing (HC) mode, parts of a model's logic
    are executed on a different device/GPU.  In order to generate code
    to be run on a device/GPU, MUSE requires an agent to supply the
    necssary code.  This is convenience macro that can be used to
    create custom implementation for the interface methods in a
    derived HCAgent source code.  This macro is meant to be used in
    the source file (.cpp file) of an agent as shown below:

    \code

    void
    SiSAgent::executeTask(const EventContainer& eventList) {
        // Code to process events goes here.

        // Let muse know that HC kernel needs to be run at this LVT
        runHCkernel();
    }

    HC_KERNEL("SiSAgent", "SiSState", {
        hc_state->sus -= hc_state->inf * hc_params->beta;
        hc_state->inf += hc_state->sus * hc_params->beta;
    });
    
    \endcode

    \note This macro generates the parameters as a substructure with a
    fixed name "hc" in both state and agent classes.
*/
#define HC_KERNEL(AgentClass, StateClass, ...)                          \
                                                                        \
    /* Definition for running on CPU if OpenCL is not used */           \
    void AgentClass::hcKernel(int id,                                   \
                              const struct AgentClass::hc* hc_params,   \
                              struct StateClass::hc* hc_state,          \
                              struct MTrand_Info* rndInfo,              \
                              const muse::Time lvt,                     \
                              const muse::Time gvt)  {                  \
        /* Dump all of the user's code */                               \
        __VA_ARGS__                                                     \
    }                                                                   \
                                                                        \
    /* This is glue method to call the HC kernel directly */            \
    /* when OpenCL is not being used */                                 \
    void AgentClass::executeHCkernel()  {                               \
        StateClass* state = dynamic_cast<StateClass*>(getState());      \
        ASSERT( state != NULL );                                        \
        hcKernel(getAgentID(), &this->hc, &state->hc, &rndInfo,         \
                 getTime(), getTime(GVT));                              \
    }                                                                   \
                                                                        \
    /* Return kernel body for use on GPU */                             \
    std::string AgentClass::getHCkernelDefinition()  {                  \
        return                                                          \
            "\n\n#ifndef hc_state_type\n"                               \
            "typedef struct hc_state hc_state_type;\n"                  \
            "typedef struct hc_params hc_params_type;\n"                \
            "#endif\n\n"                                                \
                                                                        \
            "void hcKernel(int id, const hc_params_type* hc_params,\n"  \
            "\thc_state_type* hc_state,\n"                              \
            "\tglobal struct MTrand_Info* rndInfo,\n"                   \
            "\tconst Time lvt, const Time gvt) {"                       \
            #__VA_ARGS__                                                \
            "\n}\n\n";                                                  \
    }

#endif
