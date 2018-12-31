#ifndef RES_CHANNEL_CPP
#define RES_CHANNEL_CPP

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

#include "ResChannel.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <cmath>

// Setup muse as the default namespace
using namespace muse;

std::string
Epoch::toString() const {
    std::string retVal = "Epoch(begTS=" + std::to_string(begTS) +
        ", endTS=" + std::to_string(endTS) + ", advTS=" +
        std::to_string(advTS) + ", exeTime=" + std::to_string(exeTime) + ")";
    return retVal;
}

ResChannel::ResChannel() : fedIndex(-1), simComplete(false) {
    fqrClock      = 0;
    minEpochCount = 3;
    socketHandle  = -1;
}

ResChannel::~ResChannel() {
    if (socketHandle != -1) {
        close(socketHandle);
        socketHandle = -1;
    }
}

void
ResChannel::connect(const int fedIndex, const std::string& ctrlInfo,
                    const int port) {
    // Parse of the host and port number from host:port_number format.
    const int colonPos = ctrlInfo.find(':');
    std::string domain = ctrlInfo.substr(0, colonPos);
    const int portNum  = (port != -1 ? port : std::stoi(ctrlInfo.substr(colonPos + 1)));
    // Resolve the host name to an IP (as needed).
    const struct hostent *serverInfo = gethostbyname(domain.c_str());
    if (serverInfo == NULL) {
        std::cerr << "ResChannel::connect() - Unable to resolve domain host "
                  << domain << ".\n Error message: "
                  << getErrorString(errno) << std::endl;
        throw std::runtime_error("Unable to resolve domain host name");
    }
    // Ok, now create a socket handle to work with.
    if ((socketHandle = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "ResChannel::connect() unable to create a socket handle.\n"
                  << "Error: " << getErrorString(errno) << std::endl;
        throw std::runtime_error("Unable to create a socket");
    }
    // Setup server address information.
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = PF_INET;
    serv_addr.sin_addr.s_addr = *((long *) serverInfo->h_addr_list[0]);
    serv_addr.sin_port        = htons(portNum);
    // Try and connect to the domain controller...
    if (::connect(socketHandle,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ResChannel::connect() - Error connecting to domain host "
                  << domain << ", port " << portNum << ".\nError message: "
                  << getErrorString(errno) << std::endl;
        throw std::runtime_error("Unable to connect to HRM domain controller");
    }
    // Socket established. Save values for future reference.
    this->domain   = domain;
    this->fedIndex = fedIndex;
}

void
ResChannel::informStart() {
    writeLine("Start " + std::to_string(fedIndex));
    simComplete = false;
    domainThread = std::thread(&ResChannel::threadMain, this);
}

void
ResChannel::informTerminate() {
    writeLine("Terminate");
    simComplete = true;
    DEBUG(std::cout << "Waiting for read thread to finish.\n");
    domainThread.join();
}

void
ResChannel::add(const Epoch& epoch, const Time& currSimTime) {
    std::lock_guard<std::mutex> guard(listMutex);
    ASSERT( (epochList.empty()) || (epochList.back() < epoch ) );
    epochList.push_back(epoch);
    fqrClock = currSimTime;
}

void
ResChannel::rollback(const Time& time) {
    std::lock_guard<std::mutex> guard(listMutex);
    while (!epochList.empty() && !(epochList.back() < time)) {
        std::cout << "rollback to " << time << " -- removing: "
                  << epochList.back().toString() << std::endl;
        epochList.pop_back();
    }
    // Either the epoch list is empty or the last entry's end time
    // stamp is less than the rollback time.  Note that, we do loose
    // some epochs if rollbacks occur to the middle of an Epoch entry
    // in the current scheme.
    ASSERT (epochList.empty() || (epochList.back() < time));
}

void
ResChannel::garbageCollect(const Time& gvt) {
    UNUSED_PARAM(gvt);
    /*
    std::lock_guard<std::mutex> guard(listMutex);
    // Find the last epoch that is safe to erase.
    EpochList::iterator upto = epochList.begin();
    while ((upto != epochList.end()) && (*upto < gvt)) {
        upto++;
    }
    // Erase the epochs upto the point it is safe.
    epochList.erase(epochList.begin(), upto);
    DEBUG(std::cout << "Epochs garbage collected upto: " << gvt << std::endl);
    */
}

void
ResChannel::threadMain() {
    DEBUG(std::cout << "Thread connected to domain controller is running.\n");
    std::string inputLine;    
    while (!simComplete && readLine(inputLine)) {
        if (inputLine.find("MeasureSpeed") == 0) {
            // Report execution speed measurement to hypervisor.
            reportSpeedMeasurement();
        } else if (inputLine.find("migrateOut") == 0) {
            // Handle migration to a different host
            migrateOut(inputLine.substr(inputLine.find(' ') + 1));
        }
    }
    DEBUG(std::cout << "Thread connected to domain controller is done running.\n");    
}

void
ResChannel::reportSpeedMeasurement() {
    std::string speed = "NaN";  // Updated below.
    // Lock the Epoch list to ensure thread safe access
    std::lock_guard<std::mutex> guard(listMutex);
    const int epochCount = epochList.size();
    std::cout << "EpochCount = " << epochCount << ", minEpochCount = "
              << minEpochCount   << std::endl;
    if (epochCount > minEpochCount) {
        // Accumulate the time advances from all the epochs in the list
        double processedTime = 0, advancedTime = 0;
        for(int i = 0; (i < epochCount); i++) {
            epochList[i].accumulate(advancedTime, processedTime);
        }
        // Update speed
        speed = std::to_string(advancedTime / processedTime);
    }
    // Clear out the epoch list
    epochList.clear();
    // Report it to domain controller
    if (!writeLine("ExecutionSpeed " + speed + " " + std::to_string(fqrClock) +
                   " " + std::to_string(epochCount))) {
        std::cerr << "Error reporting execution speed to domain controller.\n";
    }
}

void
ResChannel::migrateOut(const std::string& destInfo) {
    // Lock socket to prevent operations on it during migration
    // migrate to avoid conflicts with the socket handle.
    std::lock_guard<std::recursive_mutex> guard(socketMutex);
    // Extract destination information out of the string.
    const int spcPos           = destInfo.find(' ');
    const std::string destHost = destInfo.substr(0, spcPos);
    const int         migCap   = std::stoi(destInfo.substr(spcPos + 1));
    // Close the current socket
    close(socketHandle);
    socketHandle = -1;
    // Create a new connection to the destination. If port is present
    // in destHost then use it. Otherwise use port 10000 as default.
    const int port = (destHost.find(':') == std::string::npos) ? 10000 : -1;
    connect(fedIndex, destHost, port);
    // Report that the sim has migrated in.
    writeLine("MigrateIn " + std::to_string(fedIndex) +
              " " + std::to_string(migCap));
}

std::string
ResChannel::getErrorString(int errorNumber) {
    return strerror(errorNumber);
}

bool
ResChannel::readLine(std::string& line) {
    // Read charachter by charachter looking for '\n'. We don't lock
    // socket here because migration and aother activites cannot
    // happen until this read complete successfully.  Furthermore, if
    // we lock socket here, asynchronous wries will be blocked.
    line = "";
    char in;
    ssize_t bytesRecvd;
    while (((bytesRecvd = recv(socketHandle, &in, 1, MSG_WAITALL)) == 1)
           && (in != '\n')) {
        line += in;
    }
    if (bytesRecvd == -1) {
        std::cerr << "ResChannel::readLine() - Error reading data to socket.\n"
                  << "Error message: " << getErrorString(errno) << std::endl;
    }
    DEBUG(std::cerr << "ResChannel::readLine() - " << line << std::endl);
    // Return true if no error occurred and bytes were successfully read
    return (bytesRecvd == 1);
}

bool
ResChannel::writeLine(const std::string& line) {
    std::lock_guard<std::recursive_mutex> guard(socketMutex);    
    // Write the bytes in the string out with a trailing newline
    const std::string data = line + '\n';
    std::cerr << "Sending data to HRM controller: " + data;
    DEBUG(std::cerr << "Sending data to HRM controller: " + data);
    // Write the data out to socket
    const ssize_t bytesWritten = write(socketHandle, data.c_str(), data.size());
    if (bytesWritten == -1) {
        std::cerr << "ResChannel::writeLine() - Error writing data to socket.\n"
                  << "Error message: " << getErrorString(errno) << std::endl;
    }
    // Return true if no error occurred
    return (bytesWritten == (ssize_t) data.size());
}

#endif
