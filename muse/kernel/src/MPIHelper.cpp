#ifndef MPI_HELPER_CPP
#define MPI_HELPER_CPP

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
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "MPIHelper.h"
#include <cstring>

#ifndef HAVE_LIBMPI

#ifndef _WINDOWS
// A simple implementation for MPI_WTIME on linux
#include <sys/time.h>
double MPI_WTIME() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1e6);
}

#else
// A simple implementation for MPI_WTIME on Windows
#include <windows.h>

double MPI_WTIME() {
    FILETIME st;
    GetSystemTimeAsFileTime(&st);
    long long time = st.dwHighDateTime;
    time <<= 32;
    time |= st.dwLowDateTime;
    return (double) time;
}


#endif  // _Windows

// Dummy MPI_INIT when we don't have MPI to keep code base streamlined
void MPI_INIT(int argc, char* argv[]) {
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);
}


bool MPI_IPROBE(int src, int tag, MPI_STATUS status) {
    UNUSED_PARAM(src);
    UNUSED_PARAM(tag);
    UNUSED_PARAM(status);
    return false;
}

int MPI_SEND(const void* data, int count, int type, int rank, int tag) {
    UNUSED_PARAM(data);
    UNUSED_PARAM(count);
    UNUSED_PARAM(type);
    UNUSED_PARAM(rank);
    UNUSED_PARAM(tag);
    return -1;
}

#endif  // Don't have MPI

#endif // MPI_HELPER_CPP
