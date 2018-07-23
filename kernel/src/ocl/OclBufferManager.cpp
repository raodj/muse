#ifndef MUSE_OCL_BUFFER_MANAGER_CPP
#define MUSE_OCL_BUFFER_MANAGER_CPP

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
// Authors: Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include "ocl/OclBufferManager.h"
#include <iostream>

template<typename T>
T*
muse::OclBufferManager::map(bool write, const int bufSz, const size_t offset) {
    // Determine the desired buffer size.
    const size_t bufferSize = (bufSz == -1 ? size() : bufSz);
    // Check and resize the buffer, as needed
    if (bufferSize > size()) {
        resize<T>(bufferSize);  // resize to enlarge buffer
    }
    // Perform the mappings.
    mappedPtr = queue.enqueueMapBuffer(buffer, CL_TRUE,
                                       (write ? CL_MAP_WRITE : CL_MAP_READ),
                                       offset, bufferSize);
    ASSERT(mappedPtr != NULL);
    // Return the mapped buffer
    return reinterpret_cast<T*>(mappedPtr);
}

void
muse::OclBufferManager::unmap() {
    if (mappedPtr != nullptr) {
        queue.enqueueUnmapMemObject(buffer, mappedPtr);
        mappedPtr = nullptr;
    }
}

template<typename T>
T*
muse::OclBufferManager::resize(const int newSize, int type) {
    if ((size() == newSize) || ((mappedPtr != nullptr) && !isWrite)) {
        // Buffer is mapped for reading or is of correct size
        // already. Cannot be re-sized.
        return reinterpret_cast<T*>(mappedPtr);
    }
    // Create a new buffer of the desired size.
    cl::Buffer newBuf(queue.getInfo<CL_QUEUE_CONTEXT>(), type, newSize);
    void *newMapPtr = nullptr;
    if (mappedPtr != nullptr) {
        // Map the new buffer onto the device to maintain status
        newMapPtr = queue.enqueueMapBuffer(buffer, CL_TRUE, CL_MAP_WRITE,
                                           0, newSize);
        // Copy data from old buffer to the new buffer if mapped.
        const int minSize = std::min(newSize, size());
        memcpy(newMapPtr, mappedPtr, minSize);
        // Unmap the old buffer
        unmap();
    }
    // Discard current buffer in favor of the new one
    buffer = std::move(newBuf);
    // Update mapped pointer.
    mappedPtr = newMapPtr;
    return reinterpret_cast<T*>(mappedPtr);
}

#endif
