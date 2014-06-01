
-----------------------------------------------------------------------
      ___
     /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
    /::L_L_
   /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
   \/_/:/  /  free software: you can  redistribute it and/or  modify it
     /:/  /   under the terms of the GNU  General Public License  (GPL)
     \/__/    as published  by  the   Free  Software Foundation, either
              version 3 (GPL v3), or  (at your option) a later version.
      ___
     /\__\    MUSE  is distributed in the hope that it will  be useful,
    /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
   /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
   \:\/:/  /  PURPOSE.
    \::/  /
     \/__/    Miami University  and  the MUSE  development team make no
              representations  or  warranties  about the suitability of
      ___     the software,  either  express  or implied, including but
     /\  \    not limited to the implied warranties of merchantability,
    /::\  \   fitness  for a  particular  purpose, or non-infringement.
   /\:\:\__\  Miami  University and  its affiliates shall not be liable
   \:\:\/__/  for any damages  suffered by the  licensee as a result of
    \::/  /   using, modifying,  or distributing  this software  or its
     \/__/    derivatives.
  
      ___     By using or  copying  this  Software,  Licensee  agree to
     /\  \    abide  by the intellectual  property laws,  and all other
    /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
   /::\:\__\  General  Public  License  (version 3).  You  should  have
   \:\:\/  /  received a  copy of the  GNU General Public License along
    \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
     \/__/    from <http://www.gnu.org/licenses/>.
  
-----------------------------------------------------------------------

File/Directory Structure & Organization:
----------------------------------------

The source files, images, and resources constituting PEACE-GUI have
been organized in the following manner:

* infra: Folder contains general purpose infrastructure components.

  The "infra" directory contains files that are generic Graphical User
  Interface (GUI) and other utility classes that have been used to
  develop MUSE-GUI.  These classes are generic in the sense that they
  can be readily reused in another GUI project that is not related to
  MUSE.  This directory should not contain MUSE-GUI specific code.
  Instead more generic, "library" type classes belong in the "infra"
  directory.  For example, a generic text file processing helper class
  should be in the "infra" directory while class to process MUSE logs
  would not belong in the "infra" folder.
         
* core: Folder contains core components of MUSE-GUI

  The "core" folder (and its sub-folder) contain some of the core
  components constituting MUSE-GUI.  These components essentially tie
  together the various subsystems constituting MUSE-GUI.  In addition,
  these components are typically shared or used by various subsystems
  in MUSE-GUI.

* images: Folder with various images used in MUSE-GUI

  As the name suggests the "images" folder contains various images and
  icons used by MUSE-GUI.  Non-image files that are used by MUSE-GUI
  belong in the "resource" directory.

* resources: Non-image files and data used by MUSE-GUI

  This folder contains various non-image files, text files, HTML
  files, and other data files used by MUSE-GUI.  This directory must
  not contain images.

* workspace: Files dealing with in-memory meta data for workspace artifacts

  This folder contains various classes that contain information about
  various artifacts on a workspace. This includes meta data about
  projects, servers, simulation-jobs etc.  The classes also facilitate
  marshalling (writing XML) and unmarshalling (reading XML)
  workspace information for persistence.

