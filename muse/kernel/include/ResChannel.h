#ifndef RES_CHANNEL_H
#define RES_CHANNEL_H

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
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "DataTypes.h"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>


BEGIN_NAMESPACE(muse);

/** A simple class to encapsulate timing information about the ephocs
    being recorded for measure performance.
*/
class Epoch {
    friend class ResChannel;
public:
    /** A default constructor to make creation of containers with
        epocs easier.
    */
    Epoch() : begTS(-1), endTS(-1), advTS(-1), exeTime(-1) {}

    /** Convenience constructor to initialize all the instance
        variables encapsulated by this class.
    */
    Epoch(double begTS, double endTS, double advTS, double exeTime) :
        begTS(begTS), endTS(endTS), advTS(advTS), exeTime(exeTime) {
        // Nothing else to be done in the constructor.
    }

    /** Convenience method to add advTS and exeTime to advanceTime and
        processedTime values.  This method is used in
        ResChannel::reportSpeedMeasurement() to compute speed
        measurements.
    */
    void accumulate(double& advanceTime, double& processedTime) {
        advanceTime   += advTS;
        processedTime += exeTime;
    }

    /** Convenience method to obtain a human readable string
        representation of this Epoch.

        \return A human readable string representation of this epoch.
    */
    std::string toString() const;

    /** Convenience operator to determine if an epoch is less than
        another epoch.

        \param[in] other The other epoch to compare against.

        \return This method returns true if the this->endTS is less
        than other.endTS
    */
    bool operator<(const Epoch& other) const { return endTS < other.endTS; }

    /** Convenience operator (non-const version) to determine if an
        epoch is less than another epoch.
        
        \param[in] other The other epoch to compare against.
        
        \return This method returns true if the this->endTS is less
        than other.endTS
    */    
    bool operator<(const Epoch& other) { return endTS < other.endTS; }    
    
    /** Convenience operator to determine if an epoch is less than a
        given virtual time.

        \param[in] endTS The virtual time to compare with this epoch.

        \return This method returns true if the this->endTS is less
        than endTS
    */
    bool operator<(const Time& endTS) const { return this->endTS < endTS;  }

    /** Convenience operator (non-const version) to determine if an
        epoch is less than a given virtual time.

        \param[in] endTS The virtual time to compare with this epoch.

        \return This method returns true if the this->endTS is less
        than endTS
    */
    bool operator<(const Time& endTS) { return this->endTS < endTS;  }
    
private:
    double begTS, endTS, advTS, exeTime;    
};

/**
   A convenience shortcut to refer to a list of Epochs used for timing
   measurements in the ResChannel class.
*/
typedef std::vector<Epoch> EpochList;

/**
   ResChannel is used to interact with the hypervisor and respond to
   its measurement requests.
*/
class ResChannel {
public:
    /** The default constructor creates an unconnected channel. The
        ResChannel::connect() method must be used to establish a
        connection.
    */
    ResChannel();

    /** The destructor.  It winds up the operation of this ResChannel
        object.
    */
    ~ResChannel();

    /** Connects to the hypervizor running on the specified domain
        with the given fedIndex.

        \note This method throws an std::runtime_exception if a
        connection could not be established to the specified HRM
        domain controller.
        
        @param fedIndex The MPI rank of the process.

        @param ctrlInfo The host name of IP address of the HRM domain
        controller in the format host:port_number (example: \c
        node001:3000 or \c 192.168.0.100:444).
        
        @param port The port number at which to connect to the
        hypervizor.  If the port number is not -1 then preference is
        given to this parameter over the port number specified in the
        domainInfo
    */
    void connect(const int fedIndex, const std::string& ctrlInfo, 
                 const int port = -1) throw (std::exception);

    /**
       Method to start background thread and let the hypervizor know
       that the simulation has actually started.  This method starts a
       separate thread to interact with the the domain controller.
       This method simply invokes the threadMain() method from a
       separate thread.
    */
    void informStart();

    /**
       Method to let the hypervizor know that the simulation has
       completed. This method also disconnects from the hypervizor and
       closes the socket after waiting for the background thread to
       finish.
    */
    void informTerminate();

    /** Add a new Epoch to the list of epochs managed by this resource
        channel.

        \param[in] epoch The new epoch to be added to the list of
        epochs managed by this class.

        \param[in] fqrClock The next simulation time value that caused
        the epoch to be created.
        
        \note Under correct operating conditions, the timestamp of the
        new epoch must be greater than the last one in the list.
    */
    void add(const Epoch& epoch, const Time& fqrClock);

    /** Inform the resource channel about a rollback.

        This method must be used to report rollbacks that occur during
        simulation.  This information is used by the resource channel
        to prune previously added eopchs based on the given time.

        \param[in] time The virtual time to which a rollback has
        occurred.  This method removes all epochs that were created at
        or after this virtual time.
    */
    void rollback(const Time& time);

    /** Garbage collect older epochs (to which rollback can never
       happen).

       This method is called when it is safe to garbage collect for a
       given Global Virtual Time (GVT).  This method removes all
       epochs that were created before the given GVT time.

       \param[in] gvt The current Global Virtual Time (GVT) of the
       simulation.
    */
    void garbageCollect(const Time& gvt);

    /** Set the minimum number of epochs that must be recorded prior
        to reporting speed measurements to domain controller.

        This method must be used to set the value that indicates the
        minimum number of epochs that must be present in the epochList
        before a timing measurement is reported to the HRM domain
        controller in response to a "MesureSpeed" request.  The
        default value for this parameter is 3.  This value can be
        overridden by specifying the \c --min-epoch-count command-line
        argument that is processed by the HRMScheduler.

        \param[in] count The number of epochs that must be recorded in
        the epochList prior to reporting speed measurements.  If the
        number of epochs is fewer than this value then the speed is
        reported as "NaN" (not a number).
    */
    void setMinEpochCount(int count) { minEpochCount = count; }

    /** Obtain the current number of Epochs in this resource channel.

        This method can be used to determine the current number of
        Epochs associated with this resource channel.

        \return The number of epochs in this resource channel. If no
        Epochs are present this method returns 0 (zero).
    */
    int getEpochCount() const { return epochList.size(); }
    
protected:
    /** This method is invoked from a separate thread and handles
        asynchronous interactions with the domain controller.  Do not
        call this method directly.  Instead call
        ResChannel::informStart() method instead.
    */
    void threadMain();

    /**
       Refactored utility method that is called from threadMain() to
       report performance measurements to the hypervisor.  The
       operation of this method is controlled by the value of
       minEpochCount configuration parameter.
    */
    void reportSpeedMeasurement();

    /**
       Refactored utility method that is called from threadMain() to
       handle migration to a different host.
       
       This method report the migration to the domain controller,
       closes the current socket, and open's a new connection in the
       destination.

       \param[in] the destination information as reported by the HRM
       domain controler.  It is assumed the string is in the form
       "destHost migCap".
    */
    void migrateOut(const std::string& destInfo);

    /** Convenience method to obtain the OS error message for a given
        error number.

        This method just returns the error message provided by the
        operating system.

        \param[in] errorNumber The error number reported by the
        operating system.

        \return The error message corresponding to the supplied
        errorNumber.
    */
    std::string getErrorString(int errorNumber);

    /**
       Convenience method to read a line from the socket.
       
       \note This method reads data in a blocking manner.  The
       ResChannel::connect() method must be called to establish a
       connection first.
       
       @param [out] line The string to be updated with a line of data
       read from the socket.  Note that the trailing '\n' is not
       included in the string returned by this method.
       
       \return Returns true if a line was successfully read. If the
       socket is invalid or is closed, this method returns false.
    */
    bool readLine(std::string& line);

    /** Write a line of data to the socket.

       \note This method writes data in a blocking manner.  The
       ResChannel::connect() method must be called to establish a
       connection first.
        
       This method writes the data in line to the socket.  Note that
       this method automatically writes a '\n' to the socket.

       \param[in] line The line of string to be written to the socket.

       \return Returns true if a line was successfully written. If the
       socket is invalid or is closed, this method returns false.
    */
    bool writeLine(const std::string& line);

private:
    /** The host name of IP address of the hypervizor. See
        ResChannel::connect() method.
    */
    std::string domain;

    /** The rank of the MPI process. See ResChannel::connect() method.
     */    
    int fedIndex;

    /**
       A mutex that is used to arbitrate access to the socket handle
       to ensure safe transfer when the domain is migrated.
    */
    std::recursive_mutex socketMutex;
    
    /**
       The handle to the socket connection used to interact with the
       hypervizor.  The socket is created via the
       ResChannel::connect() method.
    */
    int socketHandle;

    /** Flag to indicate that the simulation is complete. This flag is
        used by the threadMain() method to decide when it is time to
        exit out of the thread.
    */
    bool simComplete;

    /**
       This is the thread object that is used to interact with the
       domain hypervizor.  The thread is created in the startThread()
       method.
    */
    std::thread domainThread;

    /**
       A list of epochs that have been recorded in this class as part
       of timing measurements.  Note that by the virtue of how
       simulation proceeds using a Least-Timestamp-First scheduling
       approach, the epochs in this list are naturally ordered based
       on virtual time, specifically begTS.
    */
    EpochList epochList;

    /**
       A mutex that is used to arbitrate access to epoch list from
       domainThread and the main simulation thread.
    */
    std::mutex listMutex;

    /** The minimum number of epochs to be collected prior to
        reporting timing measurements.

        This value indicates the minimum number of epochs that must be
        present in the epochList before a timing measurement is
        reported to the HRM domain controller in response to a
        "MesureSpeed" request.  The default value for this parameter
        is 3.  This value can be overridden by specifying the \c
        --min-epoch-count command-line argument.
    */
    int minEpochCount;

    /** Instance variable to track the next virtual simulation time as
        epochs are created.

        This instance variable is used to track the next virtual
        simulation time currently progressing on this process.  This
        value is updated in the add() method and is used in the
        reportSpeedMeasurement() method for speed measurement
        reporting.
    */
    Time fqrClock;
};

END_NAMESPACE(muse);

#endif
