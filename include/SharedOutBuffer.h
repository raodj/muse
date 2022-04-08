#ifndef MUSE_SHARED_OUT_BUFFER_H
#define MUSE_SHARED_OUT_BUFFER_H

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
// Authors:  Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "MPIHelper.h"
#include "DataTypes.h"

BEGIN_NAMESPACE(muse);

// Forward declaration for SpinLock in MUSE kernel
class SpinLock;

/** A shared output buffer for MPI-based parallel output.

    As the name suggests, a shared output buffer is designed to be
    used in conjunction with muse::oSimStream to write data to a file
    logically shared by all agents (who choose to write to it).  An
    important aspect of this design is that the same shared buffer
    must be instantiated on all parallel processes.

    <p>This shared buffer operates in conjunction with oSimStream to
    to commit output is a thread-safe and parallel manner.  Methods in
    this class are periodically invoked from multiple agents, whenever
    GVT advances.  Any output written at a timestamp below the GVT is
    deemed final and is committed as the actual output from an
    agent.</p>
    
    <p>Note that the actual output operation occurs in the
    following 2 phases:
    
    <ol>
    
    <li>First, the write method is invoked from multiple threads, for
    each agent that has registered this shared buffer with an output
    stream.  In this method, the outputs to write are collected in the
    commitBuffer.  The commitBuffer holds blocks of output, ordered
    based on their timestamps.</li>
    
    <li>Once all threads have completed garbage collection, the
    commit method is invoked by the Simulation kernel.  The commit
    method first aggregates the timestamps to all outputs at all
    MPI-processes. Then they use MPI_File_Write_all function to
    collectively write output at each timestamp in order.</li>
    
    </ol>
    
    </p>
    
    \note A shared buffer must be instantiated on all MPI-processes.
    To ensure this, it is best to use this SharedOutBuf as a
    <b>static</b> instance variable in your agent class.
*/
class SharedOutBuffer {
    // Simulation needs access to open/close files
    friend class Simulation;
public:
    /** Constructor to create a shared output buffer.

        \param[in] fileName The destination file name to where the
        data is to be written.

        \param[in] indexFilePath An optional file to where index data
        data is to be written.
    */
    SharedOutBuffer(const std::string& filePath = "",
                     const std::string& indexFilePath = "");

    /** Method to set path for shared output file(s).

        \note This method must be invoked before the simulation
        commences.

        \param[in] fileName The destination file name to where the
        data is to be written.

        \param[in] indexFilePath An optional file to where index data
        data is to be written.
    */
    void setFilePath(const std::string& filePath,
                     const std::string& indexFilePath = "");
    
    /** The destructor.

        The destructor currently does not have much work to do because
        the base class performs necessary cleanup.
    */
    virtual ~SharedOutBuffer();

    /** Handle shared, parallel writes during garbage collection.

        This method is invoked from multiple threads, for each agent
        that has registered this shared buffer with an output stream.
        In this method, the outputs to write are collected in the
        commitBuffer.  The commitBuffer holds blocks of output,
        ordered based on their timestamps.

        \param[in] time The time at which the output was logically
        generated.

        \param[in] data The actual data written.
    */
    virtual void write(const muse::Time time, const std::string& data);
    
protected:
    /** Handle second phase of output across all MPI-processes.

        This method is invoked from Simulation::garbageCollect, affter
        all threads have completed garbage collection This method
        first aggregates the timestamps (in the commitBuffer) to all
        outputs at all MPI-processes. Then they use MPI_File_Write_all
        function to collectively write output at each timestamp in
        order.
    */
    void commit(const muse::Time& gvt);

    /** Method to actually open the file.

        This method is invoked from Simulation::initSharedIOBuffers()
        method.  This method is invoked after MPI has been
        initialized.  This method opens the actual file stream for I/O
        operations.

        \return This method returns true if the file was successfully
        opened. On errors, it returns false.
    */
    bool openFile();

    /** Method to close all  open file(s).

        This method is invoked from Simulation::closeSharedIOBuffers()
        method.  This method is invoked after the simulation has
        finished. This method closes the file stream(s).
    */
    void closeFile();

    /** Helper method to check and write an index only on Rank 0

        This is a convenience method that is used to check and write
        the index file entries. This method performs operations only
        on rank 0.  On all other ranks, calling this method has no
        side effects.  This method is called from the commit method
        for unique simulation time for which index is to be created.

        \param[in] time The simulation time for which entries are
        about to be written.
    */
    void writeIndex(const muse::Time& time);
    
private:
    /** A shortcut to manage blocks of data at various timestamps in
        order of timestamps.
    */
    using CommitBuffer = std::map<muse::Time, std::string>;

    /** The commit buffer that temporarily holds batches of output to
        be written by this MPI-process.  Entries are added by the
        garbageCollect method.  The commit method removes entries.
    */
    CommitBuffer commitBuffer;

    /** A mutex to ensure MT-safe access to the commit buffer, for
        MT-safe operations in the garbageCollect method.
    */
    SpinLock* comBufMutex;

#ifdef HAVE_LIBMPI
    /** The final MPI file handle to which the data is written as part
        of parallel I/O operations
    */
    MPI_File mpiFile;
#else
    /** If MPI is not used, then the data is finally written to a
        regular output file.
    */
    std::ofstream mpiFile;
#endif
    
    /** If MPI is not used, then the index data is written as a
        regular output file.
    */
    std::ofstream mpiIndexFile;

    /** Path to the file where the data is to be written.  This value
        is set in the constructor and is never changed.  The value is
        used in the openFile() method.
    */
    std::string filePath;

    /** Optional path to an file where the index data is to be
        written.  This value is set in the constructor and is never
        changed.  The value is used in the openFile() method.
    */
    std::string indexPath;
};

END_NAMESPACE(muse);

#endif
