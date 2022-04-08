#ifndef MUSE_SHARED_OUT_BUFFER_CPP
#define MUSE_SHARED_OUT_BUFFER_CPP

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

#include <numeric>
#include "SharedOutBuffer.h"
#include "SpinLock.h"
#include "Simulation.h"

// Switch to muse namespace, for convenience
using namespace muse;

SharedOutBuffer::SharedOutBuffer(const std::string& filePath,
                                 const std::string& indexPath) :
    filePath(filePath), indexPath(indexPath) {
    // Setup the spin lock for use in garbageCollect method
    comBufMutex = new SpinLock();
    // Register this stream with the Simulation kernel
    Simulation::registerSharedIOBuffer(this);
}

void
SharedOutBuffer::setFilePath(const std::string& filePath,
                                   const std::string& indexPath) {
    this->filePath  = filePath;
    this->indexPath = indexPath;
}

bool
SharedOutBuffer::openFile() {
    // Setup the output stream depending on whether MPI is available
    // or not.
#ifdef HAVE_LIBMPI
    if (MPI_File_open(MPI_COMM_WORLD, filePath.c_str(),
                      MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL,
                      &mpiFile) != MPI_SUCCESS) {
        std::cerr << "Error opening SharedOutBuffer " << filePath << std::endl;
        return false;
    }
#else
    // Regular file I/O
    mpiFile.open(filePath);
    if (!mpiFile.good()) {
        std::cerr << "Error opening SharedOutBuffer " << filePath << std::endl;
        return false;
    }
#endif
    // Open the index file as well
    if (!indexPath.empty()) {
        mpiIndexFile.open(indexPath);
        if (!mpiIndexFile.good()) {
            std::cerr << "Error opening SharedOutBuffer " << indexPath
                      << std::endl;
            return false;
        }
    }
    // File(s) opened successfully
    return true;
}

void
SharedOutBuffer::closeFile() {
    // Appropriately close the output streams
#ifdef HAVE_LIBMPI
    MPI_File_close(&mpiFile);
#else
    // Regular file I/O
    mpiFile.close();
#endif
    //Close the index file as well
    if (!indexPath.empty()) {
        mpiIndexFile.close();
    }
}

SharedOutBuffer::~SharedOutBuffer() {
    delete comBufMutex;
}

// This method can be called from multiple threads.  On each thread,
// it is called multiple times -- once for each agent that uses this
// shared stream.
void
SharedOutBuffer::write(const Time time, const std::string& data) {
    // Ensure MT-safe access here
    std::lock_guard<SpinLock> locker(*comBufMutex);
    // Add entry to commit buffer
    commitBuffer[time] += data;
}

// This method is called from commit method.
void
SharedOutBuffer::writeIndex(const Time& time) {
    size_t offset = 0;
    // Get current offset in file
    if (!indexPath.empty() && (MPI_GET_RANK() == 0)) {
#ifdef HAVE_LIBMPI
        MPI_Offset fileOffset;
        MPI_File_get_position_shared(mpiFile, &fileOffset);
        offset = fileOffset;
#else
        offset = mpiFile.tellp();
#endif
        // Create index entry.
        mpiIndexFile << time << " " << offset << "\n";
    }
}

// This method is called once on each MPI-process, after all the
// threads have garbage collected their respective agents.
void
SharedOutBuffer::commit(const Time& gvt) {
    UNUSED_PARAM(gvt);
    // Create a vector of unique time stamps
    std::vector<Time> uniqTimes;
    for (const auto& entry : commitBuffer) {
        uniqTimes.push_back(entry.first);  // Store uniq times.
    }
    // First collect the number of unique timestamps from each process.
    const int NumRanks = MPI_GET_SIZE();
    std::vector<int> numUniqTimes(NumRanks);
    const int uniqCount = commitBuffer.size();
    MPI_ALL_GATHER(&uniqCount, 1, MPI_INT, &numUniqTimes[0], 1, MPI_INT);
    // Now, we have number of unique times each process needs to
    // process. The next step is to actually get the times to everyone
    // so we can process entries at each time step.  For this we need
    // a suitable buffer to receive all the times.
    const int totUniqCount = std::accumulate(numUniqTimes.begin(),
                                             numUniqTimes.end(), 0);
    // We also need to create a vector of displacements where MPI
    // should copy times from various processes.
    std::vector<int> storeOffsets(NumRanks + 1);
    std::partial_sum(numUniqTimes.begin(), numUniqTimes.end(),
                     storeOffsets.begin() + 1);
    // Now get the unique times from all ranks
    std::vector<Time> allUniqTimes(totUniqCount);
    MPI_ALL_GATHERV(&uniqTimes[0], uniqCount, MPI_DOUBLE, &allUniqTimes[0],
                    &numUniqTimes[0], &storeOffsets[0], MPI_DOUBLE);
    // Remove duplicates to get globally unique list of times to commit
    std::set<Time> globalUniqTimes(allUniqTimes.begin(), allUniqTimes.end());
    // Now for each time, collectively write outputs
    for (Time outTime : globalUniqTimes) {
        // First write index on rank #0
        writeIndex(outTime);
        // Get the data at given time to be written if any.
        auto output = commitBuffer.find(outTime);
        if (output != commitBuffer.end()) {
            // Have some data to write at given time
#ifdef HAVE_LIBMPI
            MPI_File_write_shared(mpiFile, output->second.c_str(),
                                  output->second.size(), MPI_CHAR, NULL);
            std::cout << "Wrote output\n";
            MPI_BARRIER();
#else
            mpiFile.write(&output->second[0], output->second.size());
#endif
        }
    }
}

#endif
