#ifndef VERSION_H
#define VERSION_H

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

/** \file Verison.h
 *
 * \brief A file containing global defines for version and copyright
 * information.
 *
 * This file provides some of the basic information about the
 * version, copyright message, and licese information regarding
 * MUSE-GUI.
*/

/** \def MUSE_GUI_VERSION The current version information of the
 * software.
 *
 * This define provides a convenient way to track and log the version
 * information of the software for tracking version changes.
*/
#define MUSE_GUI_VERSION "Version 0.1-Alpha"

/** \def MUSE_GUI_RELEASE_DATE The date when this version of MUSE was
 * released.
 *
 * This define provides a convenient way to track and log the dat
 * when this version of software was released for public use.
*/
#define MUSE_GUI_RELEASE_DATE "August 15 2013"

       
/** \def MUSE_GUI_COPYRIGHT A simple copyright message.
 *
 * This constant provides a convenient way to display a consistent
 * copyright message in the software.
*/
#define MUSE_GUI_COPYRIGHT  "Copyright (C) Miami University, Oxford, OH, USA (2013-)"

/** \def MUSE_GUI_LICENSE The standard disclaimer and license message.
 *
 * This define aims to centralize the disclaimer for the software an
 * keeps the code clutter to a minimum in other source files.
*/
#define MUSE_GUI_LICENSE                                                          \
    "    ___\n"                                                                   \
    "   /\\__\\    This file is part of MUSE    <http://www.muse-tools.org/>\n"   \
    "  /::L_L_\n"                                                                 \
    " /:/L:\\__\\  Miami   University  Simulation  Environment    (MUSE)  is\n"   \
    " \\/_/:/  /  free software: you can  redistribute it and/or  modify it\n"    \
    "   /:/  /   under the terms of the GNU  General Public License  (GPL)\n"     \
    "   \\/__/    as published  by  the   Free  Software Foundation, either\n"    \
    "            version 3 (GPL v3), or  (at your option) a later version.\n"     \
    "    ___\n"                                                                   \
    "   /\\__\\    MUSE  is distributed in the hope that it will  be useful,\n"   \
    "  /:/ _/_   but   WITHOUT  ANY  WARRANTY\n  without  even  the IMPLIED\n"    \
    " /:/_/\\__\\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR\n"   \
    " \\:\\/:/  /  PURPOSE.\n"                                                    \
    "  \\::/  /\n"                                                                \
    "   \\/__/    Miami University  and  the MUSE  development team make no\n"    \
    "            representations  or  warranties  about the suitability of\n"     \
    "    ___     the software,  either  express  or implied, including but\n"     \
    "   /\\  \\    not limited to the implied warranties of merchantability,\n"   \
    "  /::\\  \\   fitness  for a  particular  purpose, or non-infringement.\n"   \
    " /\\:\\:\\__\\  Miami  University and  its affiliates shall not be liable\n" \
    " \\:\\:\\/__/  for any damages  suffered by the  licensee as a result of\n"  \
    "  \\::/  /   using, modifying,  or distributing  this software  or its\n"    \
    "   \\/__/    derivatives.\n"                                                 \
    "\n"                                                                          \
    "    ___     By using or  copying  this  Software,  Licensee  agree to\n"     \
    "   /\\  \\    abide  by the intellectual  property laws,  and all other\n"   \
    "  /::\\  \\   applicable  laws of  the U.S.,  and the terms of the  GNU\n"   \
    " /::\\:\\__\\  General  Public  License  (version 3).  You  should  have\n"  \
    " \\:\\:\\/  /  received a  copy of the  GNU General Public License along\n"  \
    "  \\:\\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3\n"   \
    "   \\/__/    from <http://www.gnu.org/licenses/>.\n"

/** \def FULL_TITLE The full expanded title for MUSE.
 *
 * This constant provides a convenient way to display the full title
 * of the software, namely: <b>Miami University Simulation Environment
 * (MUSE) </b>
*/
#define FULL_TITLE "Miami University Simulation Environment (MUSE)"

/** \def LAB_INFO_HTML Information about lab in HTML format.
 *
 * This constant provides a convenient way to display PC2Lab information
 * and such using a QLabel.
*/
#define LAB_INFO_HTML "<font size=\"-1\"><b>Parallel & Cloud Computing (PC2) Lab</b><br/>" \
    "Miami University, Oxford, OHIO 45056<br/>" \
    "Copyright (C) 2013-</font>"

#endif // VERSION_H
