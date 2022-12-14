#ifndef MUSE_OSIM_STREAM_H
#define	MUSE_OSIM_STREAM_H
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
// Authors: Meseret Gebre       gebremr@muohio.edu
//          Dhananjai M. Rao    raodm@miamiOH.edu
//          
//
//---------------------------------------------------------------------------

#include <ostream>
#include <fstream>
#include <sstream>
#include <list>
#include <string>
#include "DataTypes.h"
#include "SimStream.h"

BEGIN_NAMESPACE(muse);

// Forward declaration to use the shared output buffer
class SharedOutBuffer;

/** The oSimStream class.  This class is meant to be used for safe out
    while running a given simulation.  To use this class is simple and
    its just like std::cout. Since there is a potential for rollbacks,
    please do not use std::cout or any other stream for output,
    instead pass in the stream buffer you would like to use into this
    class and when it is safe to do so, oSimStream will push out the
    data to the stream buffer of your choosing.
*/
class oSimStream : public std::ostream, public muse::SimStream {
    // we make the Agent a friend
    friend class muse::Agent;
public:
    /** The constructor.
        
        The ctor takes the stream buffer of any class derived from
        std::ostream. Also there is an option for using a temp file
        for the storage or just memory. It is recommend to use et this
        flag to true if you are expecting large amount of output.
         
        \param the_streambuf, this is a pointer to the stream buffer,
        where you the data pushed.
         
        \param use_temp_file, flag, if true will use a temp file for
        the temp storage before the push to the choosen stream buffer
    */
    oSimStream(std::ostream* the_ostream, bool use_temp_file=false);

    /** Custom constructor to initialize with a shared output
        buffer.
        
        The ctor takes a pointer to a shared output buffer to which
        the data in this stream is written. With this constructor,
        intermediate data is stored in memory.

        \note This stream uses the supplied pointer until the end of
        simulation.  Consequently, it is important to ensure that the
        lifetime of SharedOutBuffer is beyond that of this instance.
         
        \param[in,out] sharedOutBuf The shared output buffer to which
        the data from this stream should be written.
    */
    oSimStream(muse::SharedOutBuffer* sharedOutBuf);
    
    /** The dtor.
     */
    virtual ~oSimStream();
    
protected:
    /** The default ctor.
        Sets stream buffer to std::cout.rdbuf().
    */
    oSimStream();

    /** The saveState method.
        This is a helper method that is used to save the state.
          
        \note MUSE will automatically use this to handle the data correctly.
        \param lvt, this is the local virtual time of the agent.
        \see Time
    */
    virtual void saveState(const Time& lvt) override;
    
    /** The rollback method.

        This is a helper method that is used to undo some of the
        writes that was done. Any information collected passed the
        restored time is now useless and is discarded.
          
        \note MUSE will automatically use this to handle the data
        correctly.

        \param restored_time, this is the time the agent rollback to.

        \see Time
    */
    virtual void rollback(const Time& restored_time) override;
    
    /** The garbageCollect method.
        
        This is a helper method that is used to push the safe data to
        the choosen stream buffer. When GVT is calculated, any data
        collected with a timestamp smaller than the gvt has to be push
        to the choosen stream buffer and records of it has to be
        cleaned up to make room for new data in the temp storage.
          
        \note MUSE will automatically use this to handle the data
        correctly.

        \param gvt, this is global virtual time.

        \see Time

        \see GVTManager
    */
    virtual void garbageCollect(const Time& gvt) override;
    
    /** for debugging reasons. */
    void printAllStates();

    /** The oSimStreamState class.
        
        This class is used to maintain the state of the oSimStream at
        any given time. The information from the state can tell you
        how much data was collected at a given timestamp.
    */
    class oSimStreamState {
    public:
        /** The timestamp variable of type Time.
            This is usually the time the state was saved.
        */
        Time timestamp;
        
        /** The stream_position variable of type std::streampos.  When
            we are using the temp file for storage, we use this to ID
            the position of the file write pointer of the saving
            point.
        */
        std::streampos stream_position;

        /** The size variable of type std::streamsize.  At the save
            point we need to know how much data was collected.  This
            is used to store that information.
        */
        std::streamsize size;

        /** Used for memory buffer usage. If we are using the memory
            we dont need to worry about positions in the file.
         */
        std::string content;
    };

    /** The the_original_streambuff variable of type std::streambuf.
        This is used to keep track of the choosen stream buffer and
        comes in handy when its time to push the save data into the
        stream.
    */
    std::streambuf* the_original_streambuff;

    /** Holds a pointer to the original ostream
     */
    std::ostream* the_original_ostream;

    /** The the_temp_streambuff variable of type std::streambuf.  This
        is used to keep track of the temp file stream buffer and comes
        in handy when its time to save data into the file stream.
    */
    std::streambuf* the_temp_streambuff;

    /** The the_temp_file variable of type std::fstream.  This the
        stream that is opened to have read/write access to the temp
        file.
    */
    std::fstream the_temp_file;

    /** The the_temp_file_name variable of type char.  This is the
        name of the temp file that will be created.  We need to hold
        onto the name of temporary file so that we can remove the file
        in the destructor of this class.
    */
    std::string the_temp_file_name;
    
    /** The the_last_stream_position variable of type std::streampos.
        This is used when we are saving the state of the oSimStream.
        To calculate the oSimStreamState::size, we subtract this from
        oSimStreamState::stream_position and that tells us how much
        data was collected since the last saved state.
    */
    std::streampos the_last_stream_position;

    /** The oSimStreamState_storage is of type std::list.
        This is used to store the oSimStreamStates. Comes in handy
        when we need to rollback and garbage collection.
        This list stores pointers to oSimStreamState that have been
        allocated in heap memory.
    */
    using state_storage = std::list<oSimStreamState*>;
    
    state_storage  oSimStreamState_storage;

    /** Always pointes to the ostringstream that contains the data.
        Used for the memory buffer, before the data is pushed out to
        the provided ostream
     */
    std::ostringstream* oss;

    /** So we can know if the user wants to use memory or file for temp storage.
     */
    bool use_temp_file;

    /** An optional shared output buffer to which the data is to be
        written.  This pointer is set only in case the output stream
        is associated with a shared buffer to be used for writing data.
    */
    muse::SharedOutBuffer* sharedOutBuf;
};

END_NAMESPACE(muse);

#endif	/* OSIM_STREAM_H */
