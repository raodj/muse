#ifndef _OSIMSTREAM_CPP
#define	_OSIMSTREAM_CPP
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
// Authors: Meseret Gebre          gebremr@muohio.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//
//
//---------------------------------------------------------------------------
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <string.h>

#include "oSimStream.h"
#include "SharedOutBuffer.h"

using namespace muse;

oSimStream::oSimStream() : std::ostream(std::cout.rdbuf()),
                           the_original_ostream(&std::cout),
                           use_temp_file(false), sharedOutBuf(NULL) {
    // by default we use memory instead of a temp file for storage.
    // heap allocate the ostringstream
    oss = new std::ostringstream;
    //lets grab the stream buffer from the temp and save it.
    the_temp_streambuff = oss->rdbuf();
    // lets save our original stream buffer for later use.  and at the
    // same time set stream buffer to temp file stream buffer.
    the_original_streambuff = rdbuf(the_temp_streambuff);
    // we clear our flags
    clear();
}

oSimStream::oSimStream(std::ostream* the_ostream, bool use_tf) :
    std::ostream(the_ostream->rdbuf()), the_original_ostream(the_ostream),
    use_temp_file(use_tf), sharedOutBuf(NULL) {
    // lets see what they want the temp storage to be.
    if (use_temp_file) {
        // first we create a temp file name
        std::string tmpFileName = (std::getenv("TMPDIR") != NULL ?
                                   std::getenv("TMPDIR") : "/tmp");
        tmpFileName += "/oSimStreamTempFile.XXXXXXXXXX";
        mkstemp(&tmpFileName[0]);
        the_temp_file_name = tmpFileName;
        // now we create the file to grab its stream buffer
        the_temp_file.open(tmpFileName, std::ios::out); the_temp_file.close();
        the_temp_file.open(tmpFileName);
        if (!the_temp_file.good()) {
            std::cout << "ERROR: Temp file was not open" << std::endl;
            exit(1);
        }
        // lets grab the stream buffer from the temp file and save it.
        the_temp_streambuff = the_temp_file.rdbuf();
        // lets save our original stream buffer for later use.  and at
        // the same time set stream buffer to temp file stream buffer.
        the_original_streambuff = rdbuf(the_temp_streambuff);
        // save the last stream position so we can calculate how much
        // data was stored in each oSimStreamState in the
        // storage. will only be used for the first saveState() call.
        the_last_stream_position = the_temp_file.tellp();
    } else {
        // here we do things for stream buffer to memory.
        // heap allocate the ostringstream
        oss = new std::ostringstream;
        //lets grab the stream buffer from the temp and save it.
        the_temp_streambuff = oss->rdbuf();
        // lets save our original stream buffer for later use.  and at
        // the same time set stream buffer to temp file stream buffer.
        the_original_streambuff = rdbuf(the_temp_streambuff);
    }
    // we clear our state bits just in case there was any sort of error
    clear();
}

oSimStream::oSimStream(muse::SharedOutBuffer* sharedOutBuf) :
    the_original_streambuff(NULL), the_original_ostream(NULL),
    use_temp_file(false), sharedOutBuf(sharedOutBuf) {
    // by default we use memory instead of a temp file for storage.
    // heap allocate the ostringstream
    oss = new std::ostringstream;
    //lets grab the stream buffer from the temp and save it.
    the_temp_streambuff = oss->rdbuf();
    // lets save our original stream buffer for later use.  and at the
    // same time set stream buffer to temp file stream buffer.
    the_original_streambuff = rdbuf(the_temp_streambuff);
    // we clear our flags
    clear();
}

oSimStream::~oSimStream() {
    // lets make sure we dont distroy the orginial stream buffer
    rdbuf(the_temp_streambuff);

    if (use_temp_file) {
        // lets destroy the temp file that was created
        the_temp_file.close();
        remove(the_temp_file_name.c_str());
    } else {
        delete oss;
    }
    // lets make sure we dont leak and memory from the storage of states
    while (!oSimStreamState_storage.empty()) {
        delete oSimStreamState_storage.front();
        oSimStreamState_storage.pop_front();
    }
}

void
oSimStream::saveState(const Time& lvt) {
    if (use_temp_file) {
        // first we grab the current get pointer
        std::streampos current_streampos = the_temp_file.tellp();
        //now we check if there was any data collect since the last save point
        if (current_streampos - the_last_stream_position == 0) {
            return;  // no change. Nothing further to do.
        }
        // if control drops here then we have collect data since last
        // save.  we need to save the state of the newly collected
        // data.
        oSimStreamState * new_state  = new oSimStreamState;
        new_state->timestamp         = lvt;
        new_state->stream_position   = the_last_stream_position;
        new_state->size              = (current_streampos -
                                        the_last_stream_position);
        // now we push this to our collection of states in storage
        oSimStreamState_storage.push_back(new_state);
        // lets update the_last_stream_position for the next time
        the_last_stream_position = current_streampos;
    } else {
        //first we check if there is anything in the memory buffer
        if (oss->str().length() > 0) {
            // We create a state to hold the content
            oSimStreamState* new_state = new oSimStreamState;
            new_state->timestamp       = lvt;
            new_state->content         = oss->str();
            // now we push this to our collection of states in storage
            oSimStreamState_storage.push_back(new_state);

            // now we delete and create a new ostringstream
            delete oss;
            oss = new std::ostringstream;
            rdbuf(oss->rdbuf());
        }
    }
}

void
oSimStream::garbageCollect(const Time& gvt) {
    if (use_temp_file) {
        // save our put pointer because garbage collect will change
        // and we need to put back
        std::streampos current_streampos = the_temp_file.tellp();
        // now we need to loop through our storage and send the stuff
        // into the original ostream until one timstamp before gvt
        std::vector<char> buffer;
        while (!oSimStreamState_storage.empty() &&
               oSimStreamState_storage.front()->timestamp < gvt) {
            // we need to get the oSimStreamState and write out its content
            oSimStreamState * current_state = oSimStreamState_storage.front();
            //lets seek our temp file to the stream position.
            the_temp_file.seekg(current_state->stream_position);
            // now we read in the data into a buffer
            buffer.resize(current_state->size);
            the_temp_file.read(&buffer[0], current_state->size);
            // here we finally push the data into the the orginal stream buffer
            the_original_ostream->write(&buffer[0], current_state->size);
            // now we delete the state and pop from storage
            delete current_state;
            oSimStreamState_storage.pop_front();
        }
        // now that we are done garbage collection lets restore the
        // put pointer position
        the_temp_file.seekg(current_streampos);
    } else {
        // now we need to loop through our storage and send the stuff
        // into the original ostream until one timstamp before gvt
        while((!oSimStreamState_storage.empty()) &&
              (oSimStreamState_storage.front()->timestamp < gvt)) {
            // we need to get the oSimStreamState and write out its content
            oSimStreamState* current_state = oSimStreamState_storage.front();
            // here we finally push the data into the the orginal
            // stream buffer prefer to write the data than to insert
            // it so that binary information is handled correctly.
            if (sharedOutBuf != NULL ) {
                sharedOutBuf->write(current_state->timestamp,
                                    current_state->content);
            } else {
                the_original_ostream->write(current_state->content.c_str(),
                                            current_state->content.size());
            }
            // now we delete the state and pop from storage
            delete current_state;
            oSimStreamState_storage.pop_front();
        }
        // Finally dump anything in our intermediate oss buffer if
        // time is infinity (that is end of simulation)
        if (gvt == TIME_INFINITY) {
            if (sharedOutBuf != NULL) {
                sharedOutBuf->write(TIME_INFINITY, oss->str());
            } else {
                ASSERT(the_original_ostream != NULL);
                the_original_ostream->write(oss->str().c_str(),
                                            oss->str().length());
            }
        }
    }
}

void
oSimStream::rollback(const Time& restored_time) {
    // better to do if statement once and suplicate the while loop,
    // instead of the if statement inside
    if (use_temp_file) {
        // we start from the back and delete anything with a bigger
        // timestamp than restored time.
        while (!oSimStreamState_storage.empty() &&
               oSimStreamState_storage.back()->timestamp > restored_time) {
            oSimStreamState* invalid_state = oSimStreamState_storage.back();
            // first we need to set our put pointer back
            the_temp_file.seekp(-invalid_state->size, std::fstream::cur);
            // delete the invalid state
            delete invalid_state;
            // finally pop the state
            oSimStreamState_storage.pop_back();
        }
    } else {
        // we start from the back and delete anything with a bigger
        // timestamp than restored time.
        while (!oSimStreamState_storage.empty() &&
               oSimStreamState_storage.back()->timestamp > restored_time) {
            delete oSimStreamState_storage.back();
            // finally pop the state
            oSimStreamState_storage.pop_back();
        }
    }
}

void
oSimStream::printAllStates(){
    state_storage::iterator it = oSimStreamState_storage.begin();
    std::cout << "*******************" << std::endl;
    if (use_temp_file){
        for (; (it != oSimStreamState_storage.end()); it++) {
          
            std::cout << "TimeStamp: " << (*it)->timestamp << std::endl;
            std::cout << "Stream position: " << (*it)->stream_position
                      << std::endl;
            std::cout << "Size: " << (*it)->size << std::endl;
            std::cout << "*******************" << std::endl;
        }
    }else{
        for (; (it != oSimStreamState_storage.end()); it++) {
            std::cout << "TimeStamp: " << (*it)->timestamp << std::endl;
            std::cout << "Content:   " << (*it)->content   << std::endl;
            std::cout << "*******************" << std::endl;
        }
    }
}

#endif
