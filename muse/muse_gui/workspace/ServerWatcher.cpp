#ifndef SERVER_WATCHER_CPP
#define SERVER_WATCHER_CPP

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------


#include "ServerWatcher.h"
#include "Workspace.h"
#include "JobList.h"
#include "Job.h"

#include <iostream>
#include <chrono>

static void exec();

static void checkJob(Job &job);

static void printJobDesc(Job &job);

ServerWatcher::ServerWatcher() {

}

ServerWatcher::~ServerWatcher() {
    stop();
}

void
ServerWatcher::start() {
    thread = std::thread(exec);
}

void
ServerWatcher::stop() {
    if (thread.joinable()) {
        thread.join();
    }
}

static void exec() {
    while (true) {
        Workspace *ws = Workspace::get();
        JobList jl = ws->getJobList();

        std::cout << "----------------------" << std::endl;
        std::cout << "------ JOB LIST ------" << std::endl;
        std::cout << "----------------------" << std::endl;

        for (int i = 0; i < jl.size(); i++) {
            checkJob(jl.get(i));
        }

        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

static void checkJob(Job &job) {
    printJobDesc(job);
}

static void printJobDesc(Job &job) {
    std::cout << "***** JOB *****" << std::endl;
    std::cout << "** Job Name: " << job.getName().toStdString() << std::endl;
    std::cout << "** Job ID: " << job.getJobId() << std::endl;
    std::cout << "** Server: " << job.getServer().toStdString() << std::endl;
    std::cout << "** Status: " << job.getStatus().toStdString() << std::endl;
    std::cout << "** Date: " << job.getDateSubmitted().toString().toStdString() << std::endl;
    std::cout << "** Desc: " << job.getDescription().toStdString() << std::endl;
}

#endif // SERVER_WATCHER_CPP
