#ifndef CORE_H
#define CORE_H

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

/** \file Core.h
 *
 * @brief The header file to be included in all .cpp files that are
 * associated with the core subsystem of MUSE.
 *
 * MUSE-GUI is organized as a coordinated collection of subsystems.
 * Each subsystem performs a specific set of functionality associated
 * with MUSE-GUI.
 *
 * This header file must be included in each of the .cpp files that       <Redundant?
 * constitute the "core" subsystem of MUSE-GUI. The
 * infrastructure subsystem provides a core features, including:          <"a core features"
 *
 *   - The main method that triggers all operations
 *   - The main window that houses all the GUI components
 *   - The main tool bar and menuing operations of the GUI
 */

/** \def MUSE_SUBSYS "GUI-Core"
 *
 * @brief Sets the current MUSE subsystem to be "GUI-Core" for
 * generating logging information.
 *
 */
#define MUSE_SUBSYS "GUI-Core"

#endif // CORE_H
