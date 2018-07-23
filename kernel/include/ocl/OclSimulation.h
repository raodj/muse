#ifndef MUSE_OCL_SIMULATION_H
#define MUSE_OCL_SIMULATION_H

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

#include <vector>
#include <string>
#include "Simulation.h"
#include "OclScheduler.h"
#include "OclState.h"
#include "MuseOclLibrary.h"

// Enable OpenCL C++ exceptions
#define __CL_ENABLE_EXCEPTIONS
#define CL_HPP_ENABLE_EXCEPTIONS
// Currently we have access only upto OpenCL 1.2 version. nVidia
// refuses to support 2.0 and higher versions to try and keep CUDA
// alive.
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include "CL/cl.hpp"
#endif


BEGIN_NAMESPACE(muse);

// Use real for all state data so users can redefine real as anything needed
typedef float real;

// forward declarations
class HCAgent;
class HCState;

class OclSimulation : public muse::Simulation{
public:
    /** The constructor for this class.
     * 
     * The constructor merely initializes all the instance variables
     * to default (or invalid) initial values.  The constructor does
     * not perform any further tasks.
     * 
     * \param[in] usingSharedEvents Flag to indicate if events are
     * directly shared between threads on a process.  The parameter is
     * passed onto the base class constructor.
     */
    explicit OclSimulation(const bool usingSharedEvents = false);    

    /** Utility method to determine if this simulation kernel has
        Heterogeneous Computing (HC) capabilty.

        \return This method returns true to indicate that this has HC
        capabilities.
    */
    virtual bool hasHCsupport() const override { return true; }
    
protected:
    /** \brief Interface method to enable derived classes to complete
        initialization of the simulator/kernel.

        This method is invoked just after this class is instantiated.
        Compliant with MUSE-API, this method first calls the base
        class method to perform necessary initialization.  Next it
        parses the command-line arguments upplied to it and performs
        core initialization of OpenCL.

        \param argc[in,out] The number of command line arguments.
        This is the value passed-in to main() method.  This value is
        modifed if command-line arguments are consumed.
        
        \param argv[in,out] The actual command line arguments.  This
        list is modifed if command-line arguments are consumed by the
        kernel.
        
        \param initMPI[in] Flag to indicate if MPI needs to be
        reinitialized.  This flag is set to false if the simulation is
        simply being repeated and initialization of MPI is not
        necessary.
    */
    virtual void initialize(int& argc, char* argv[], bool initMPI = true)
        throw (std::exception) override;

    /** Custom override to generate and setup OpenCL kernel after the
        agent's have been initialized.

        This method overrides the default implementation in the base
        class to enable generation of OpenCL kernel.  This method
        first calls the base class method to perform initialization.
        It then generates and builds a custom kernel for further
        operations.
    */
    virtual void initAgents() override;

    /** process the next set of events (if any) associated with 1
        agent.
     
        <p>This method is repeatedly invoked from the core simulation
        loop (in start() method) to process the next set of events for 1
        agent.</p>
        
        <p>If events for an agent was processed, then this method
        checks to see if the agent required its OpenCL kernel to to be
        run.  If so, it marks the agent for OpenCL execution.</p>

        <p>Next, if the LGVT changes, then this method arranges to
        have the kernels of the agents to be executed via OpenCL.</p>
        
        \return If event(s) were scheduled/processed this method
        returns true.  If no events were processed (either because of
        time-window restrictions or there were no events) then this
        method returns false.
     */
    virtual bool processNextEvent() override;

    /** Refactored utility method to parse command-line arguments
        pertinent to the kernel.

        This method is called just once from the initialize method to
        parse out the command-line arguments associated with the
        kernel.  This method first lets the base class consume
        command-line arguments.

	\param argc[in,out] The number of command line arguments.
	This is the value passed-in to main() method.  This value is
	modifed if command-line arguments are consumed.
        
	\param argv[in,out] The actual command line arguments.  This
	list is modifed if command-line arguments are consumed by the
	kernel.        
    */
    virtual void parseCommandLineArgs(int &argc, char* argv[]) override;

    /** \brief Convenience method to perform initialization/setup just
        before agents are initialized.

        This is a convenience method that this class needs to override
        to reigster agents with the communicator.  Prior to that it
        permits the base class to complete any final
        initializations/setup.  It then finalizes the agent map by
        calling Communicator::registerAgents(allAgents)
    */
    virtual void preStartInit() override;
    
private:
    /** Top-level method to coordinate initialization of OpenCL.

        This method uses various helper methods in this class to
        coordinate the initialization operations with OpenCL.  This
        method is called only once from the initialize() method in
        this class.

        \param[in] platformID The zero-based index of the OpenCL
        platform to be used for heterogeneous computing.

        \param[in] deviceID The zero-based index of the device on the
        specified platform to be used for heterogeneous computing.

        \param[in] bool Print brief information about list of
        platforms and devices available.
    */
    void initOpenCL(int platformID = 0, int deviceID = 0,
                    bool display = false);
    
    /*
      Helper function for initOpenCL to enumerate and return the
      specified platforms to run OpenCL kernel
      
      \return If it finds a valid platform, it returns the platform
      that will be used by the OpenCL kernel code
      
      \param[in] bool Print brief information about list of
      platforms available.
    */
    cl::Platform getPlatform(int platformID = 0,
                             bool display = false) const;

    /**
       Helper function for initOpenCL, to return information about the
       specified devices to run OpenCL kernel.

       \param[in] platform The platform on which the device is to be
       created.

       \param[in] deviceID The zero-based index of the device to use
       on the specified platform.

       \param[in] bool Print brief information about list of
       devices available.
       
       \return if it finds valid devices, it returns the device to run
       OpenCL kernel code with.
    */
    cl::Device getDevice(cl::Platform platform, int deviceID,
                         bool display) const;

    /**
       Run OpenCL kernel code to process operations associated with
       agents who have requested additional functionality.
       
       Uses the OpenCL instance variables in this class that are
       initialized in initOpenCL and other methods.

       \param[in] blocking If this flag is true then this method
       blocks and waits for the execution of the OpenCL kernel to
       finish by calling waitForOclKernel method.
    */
    void runOclKernels(bool blocking);

    /** Refactored utility method to wait for execution of currently
        scheduled OpenCL kernel (if any) to finish.

        This is a convenience method that is used to wait for
        execution of any pending OpenCL kernel to finish.  This method
        checks the isKernelRunning flag in this class.  If the flag is
        true, it waits for the oclRunEvent to finish.  In addition, it
        records the execution time reported by OpenCL via this event.
    */
    void waitForOclKernel();

    /** Primary garbage collection entry-point method.

        This method overrides the default implementation in the base
        class to ensure all running OpenCL kernels have finished (by
        calling waitForOclKernel) prior to a phase of garbage
        collection.  This is done to ensure that the memory on the CPU
        is consistent and all the information required for garbage
        collection is up to date.  This method calls the base class
        method to perform the actual garbage collection.
     */
    virtual void garbageCollect() override;

    /** Helper method to construct the full OpenCL kernel program for
        a given agent and kernel ID.

        This method assembles the source code for the parameters,
        state, and kernel from the specified agent. It then uses the
        assembled source for building an OpenCL program and stores the
        program in kernelList.
        
        \param[in] agent The agent from which the source code for the
        kernel is to be constructed.  This pointer cannot be NULL.

        \param[in] kernelID The logical kernel ID associated with this
        kernel.  This ID should not be less-than the size of
        kernelList when this method is called.
    */
    void buildOclKernel(muse::HCAgent* agent, const int kernelID);

    /** Helper method to compile and create an OpenCL kernel from a
        given source code.  This method is called from buildOclKernel.
        This method compiles the given source code and creates the
        top-level kernel and returns it back to the caller.

        \param[in] source The source code to be compiled and returned
        back to the caller.

        \param[in] name The top-level kernel's name.
    */
    cl::Kernel compileToKernel(const std::string& source,
                               const std::string& name = "muse") const
        throw (cl::Error);

    /** Helper method to copy state and parameter information to device/GPU.

        This is a refactored helper method method.  It is called from
        the runOclKernels method to copy agent IDs, state, and
        parameter information to the GPU.
    */
    void copyDataToDevice();

    /** Helper method to copy only state information from device/GPU.

        This is a refactored helper method method.  It is called from
        the waitForOclKernel method to copy agent state information
        into the specific agents.  The copy is done only if the
        current LGVT is the sam as the oclRunLGVT (indicating no
        rollbacks have occurred).
    */    
    void copyDataFromDevice();
    
private:
    /** The list of agents that have requested OpenCL processing.
        This vector is cleared each time the runOCLkernel method is
        called.  Entries are added to this vector in the
        processNextEvent() method.
    */
    std::vector<HCAgent*> oclAgents;

    /** The list of agents that are currently running on a device/GPU.
        This ifnormation is required to copy the results back from the
        device to appropriate agent's state.  This value is set in
        runOclKernels and used in copyFromDevice method.
    */
    std::vector<HCAgent*> runningAgents;

    /** The LGVT time when agents were scheduled to run on the device.
        This information is needed to handle cases were a rollback
        occurrs before an OpenCL kernel finishes (in asynchronous
        mode).
    */
    muse::Time oclRunLGVT;
    
    /** The last LGVT when the OpenCL kernels were run.  When LGVT
        changes, all the pending oclAgents (all at the same LVT) are
        scheduled for execution on the GPU.
    */
    muse::Time lastLGVT;

    /** The maximum number of agents to be scheduled.  This
        information is used to batch the agents scheduled to run the
        GPU to ensure that we don't exceed the memory limits of the
        GPU.
    */
    int maxWorkGroupSize;

    /** The OpenCL command queue used to interact with the GPU and
        send command/requests to the GPU.  The queue is setup once in
        the initOpenCL method and is never changed for the duration of
        this class.
    */
    cl::CommandQueue queue;

    /** The OpenCL device to be used for running kernels.  The device
        is setup in initOpenCL method.  It is used in the
        compileToKernel method.
     */
    cl::Device device;
    
    /** The opaque OpenCL context that is necessary for operations on
        the queue and for scheduling kernel executions.  It is setup
        once in the initOpenCL method an is never changed for the
        duration of this class.
    */
    cl::Context context;

    /** The list of kernels associated with different agents. Each
        agent can potentially use a different type of kernel.  The
        kernel's are identified based on an integer ID logically
        associated with each kernel.
    */
    std::vector<cl::Kernel> kernelList;

    /** An OpenCL buffer that is used to send IDs of the agents that
        are currently being scheduled to run.  The values in this
        buffer are set in the run OclKernels method.
    */
    cl::Buffer agentIDsBuf;

    /** An OpenCL buffer that is used to send state information of the
        agents that are currently being scheduled to run.  The values
        in this buffer are copied by the copyToDevice method.
    */
    cl::Buffer stateBuf;
    
    /** An OpenCL buffer that is used to send parameter information of
        the agents that are currently being scheduled to run.  The
        values in this buffer are copied by the copyToDevice method.
    */
    cl::Buffer paramBuf;

    /** An OpenCL buffer that is used to maintain random number
        generator information in global space.  Random number
        generation is a facility that is provided by MUSE for
        applications.  Each work-item gets an independent random
        number generator for its use.  This ensures that work-items
        can generate random numbers in parallel on the GPU.  This
        buffer is initialized to zeros and is copied only once during
        initialization.  Consequently, other than taking ~10KB of
        space on the GPU, this buffer should not introduce too much
        overhead during regular simulation.
    */
    cl::Buffer rndGenBuf;
    
    /** Instance variale to track cummulative runtime for OpenCL
        kernel.  The time here is in nanoseconds (10^-9 of a second).
        The runtime is obtained from OpenCL using the oclRunEvent
        object in this class.  This value is updated in the
        waitForOclKernel() method.
    */
    cl_ulong oclRunTime;

    /** Flag to indicate if an OCL kernel has been scheduled to run.
        This flag is used to determine if we need to wait on the event
        to finish prior to running.
    */
    bool isKernelRunning;
    
    /** Event to track if an OCL kernel is running and to measure the
        OCL execution time.  This event is used to ensure that kernel
        executions are essentially serialized prior to being run.
    */
    cl::Event oclRunEvent;

    /** The OpenCL platfrom to be used.  This value is set in the
        parseCommandLineArgs method based on the value specified by
        the user via the --platfrom command-line argument.
    */
    int platformID;

    /** The OpenCL platfrom to be used.  This value is set in the
        parseCommandLineArgs method value specified by the user via
        the --device command-line argument.
    */    
    int deviceID;

    /** Flag to indicate if the OpenCL platfrom and devices should be
        listed.  This value is set in the parseCommandLineArgs method
        value specified by the user via the --ocl-list command-line
        argument.
    */
    bool listOclDevices;

    /** A predefined string constant that holds the whole MUSE OpenCL
        support library code. This string is initialized by directly
        including MuseOclLibrary.c OpenCL kernel library into the
        source code using raw string literal support introduced in
        C++11.  This string is included at the beginning of an OpenCL
        kernel in the buildOclKernel method.
    */
    static const std::string MuseOclLibrary;
    
    /** A predefined string constant that holds MUSE's OpenCL
        interface kernel code. This string is initialized by directly
        including MuseOclKernel.c OpenCL kernel library into the
        source code using raw string literal support introduced in
        C++11.  This string is included at the end of an OpenCL kernel
        in the buildOclKernel method.
    */
    static const std::string MuseOclKernel;
};

END_NAMESPACE(muse);

#endif
