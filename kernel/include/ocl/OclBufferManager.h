#ifndef MUSE_OCL_BUFFER_MANAGER_H
#define MUSE_OCL_BUFFER_MANAGER_H

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

// Enable OpenCL C++ exceptions
#define __CL_ENABLE_EXCEPTIONS
// Currently we have access only upto OpenCL 1.2 version. nVidia
// refuses to support 2.0 and higher versions to try and keep CUDA
// alive.
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include "CL/cl.hpp"
#endif

#include "Utilities.h"

BEGIN_NAMESPACE(muse);

/** A class to manage a given OpenCL Buffer.

    This is a helper class that has been introduced to streamline
    operations with an OpenCL buffer.  This class adds the following
    two key functionalities:

    <ol>

    <li>It provides additional feature to grow the size of an OpenCL
    buffer by recreating a new buffer</li>

    <li>It provides a exception-safe layer for mapping/unmapping
    buffers using RTTI style operation -- that is, if a buffer has
    been mapped, when this object goes out the destructor unmaps the
    buffer to ensure that read/write operations are committed.</li>
    
    </ol>
*/
class OclBufferManager {
public:
    /** The constructor to take ownership of a buffer.

        The constructor takes ownership of an unmapped buffer.

        \param[in,out] buffer The OpenCL buffer to managed by this class.
        It is assumed that the buffer is currently unmapped.
        
        \param[in,out] queue The queue to be used for performing any
        mapping/unmapping operations.
    */
    OclBufferManager(cl::Buffer& buffer, cl::CommandQueue& queue) :
        buffer(buffer), queue(queue), mappedPtr(nullptr) {
        // Nothing else to be done in the constructor.
    }

    /** The destructor

        Using an RTTI style, the destructor unmaps the buffer, if it
        is already mapped.
    */
    ~OclBufferManager() {
        // Unmap the buffer (if it is currently mapped)
        unmap();
    }
    
    /** Obtain a reference to the buffer managed by this class
        
        \return Reference to the buffer managed by this class.
    */
    cl::Buffer& get() { return buffer; }
    
    /** Obtain the current size of this buffer.

        \return The current size (in bytes) of the buffer managed by
        this class.
    */
    inline int size() const {
        return buffer.getInfo<CL_MEM_SIZE>();
    }
    
    /** Helper method to map a given OpenCL buffer for
        reading/writing.

        \param[in] write If this flag is true the buffer is mapped for
        writing.  Otherwise it is mapped for reading.

        \param[in] bufSize The size (in bytes) of memory to be mapped.
        If this value is -1 then the whole buffer (size obtained via
        call to size method in this class) is mapped for
        reading/writing.  If this bufSize is smaller than the current
        buffer size, then the buffer is resized prior to mapping.

        \param[in] offset The starting offset from where the mapping
        is to be done.
    */
    template<typename T>
    T* map(bool write = false, const int bufSz = -1,
           const size_t offset = 0);

    /** Unmap (if mapped) the current buffer.

        This is the dual to the map method in this class.  This method
        unmaps the currently mapped region (if any).  Calling this
        method on an unmapped buffer has no effect (and should not
        cause any issues).
    */
    void unmap();

    /** Convenience method to resize the buffer (but not when mapped
        for reading) managed by this class.

        This method is a convenience method that can be used to resize
        this buffer.  Note that resizing is accomplished by creating a
        new buffer with the desired size and copying the data over.
        This is the only approach that can be used with OpenCL.  So
        resizing is an expensive operation and it would be best to
        minimize number of calls to resize by doubling the capacity of
        the buffer.

        \note If the buffer is currently mapped for reading then this
        method will not perform any operation and silently return the
        currently mapped pointer.  If the buffer is mapped for
        writing, then this method returns a new pointer to the newly
        mapped region starting at offset zero.
        
        \param[in] newSize The new size of the desired buffer.

        \param[in] type The type of buffer to be created.
    */
    template<typename T>
    T* resize(const int newSize, int type = CL_MEM_READ_WRITE);
    
private:
    /** Reference to the buffer currently managed by this class. */
    cl::Buffer& buffer;

    /** The OpenCL queue to be used for mapping/unmapping operations */
    cl::CommandQueue& queue;

    /** Pointer to currently mapped portion of the buffer.  If the
        buffer is not mapped then this poitner is nullptr.
    */
    void* mappedPtr;

    /** Flag to indicate if the buffer is currently mapped for
        writing. This is used in the resize method to remap the new
        buffer in the correct mode.
    */
    bool isWrite;
};

END_NAMESPACE(muse);

// Include source file with templatized-method implementations
#include "ocl/OclBufferManager.cpp"

#endif
