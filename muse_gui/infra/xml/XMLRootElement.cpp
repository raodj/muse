#ifndef XML_ROOT_ELEMENT_CPP
#define XML_ROOT_ELEMENT_CPP

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

#include "XMLRootElement.h"

XMLRootElement::XMLRootElement(const QString &name) : XMLElement(name) {
    // Nothing else to be done here.
}

XMLRootElement::~XMLRootElement() {
    // An empty constructor begets an empty destructor
}

// This method is meanigful only for root element.
bool
XMLRootElement::readAttributes(QXmlStreamReader &in) {
    // Process attributes only for root elements and store it for
    // use when writing data back in the writeAttributes() method.
    attributes = in.attributes();
    namespaces = in.namespaceDeclarations();
    // Looks good so far.
    return !in.hasError();
}

// This method is meanigful only for root element.
bool
XMLRootElement::writeAttributes(QXmlStreamWriter &out) {
    // Setup the namespaces using same suffixes read from XML file.
    for(int i = 0; (i < namespaces.size()); i++) {
        const QXmlStreamNamespaceDeclaration& ns = namespaces[i];
        // Assuming that no prefix implies a default namspace...
        if (ns.prefix().toString().isEmpty()) {
            out.writeDefaultNamespace(ns.namespaceUri().toString());
        } else {
            out.writeNamespace(ns.namespaceUri().toString(),
                               ns.prefix().toString());
        }
    }
    // Write any additional attributes for the root element.
    if (!out.hasError() && !attributes.empty()) {
        out.writeAttributes(attributes);
    }
    // Return reporting any potential errors.
    return !out.hasError();
}

bool
XMLRootElement::read(QXmlStreamReader &in) {
    // Process the start document information in XML (insanity check)
    if (XMLElement::readNext(in) != QXmlStreamReader::StartDocument) {
        in.raiseError("Did not find start document in XML file!");
        return false;
    }
    // Read next token (and it should be a start element for the root element)
    XMLElement::readNext(in);
    // Now load the data into the top-level root element using base class.
    return XMLElement::read(in);
}

bool
XMLRootElement::write(QXmlStreamWriter &out) {
    // Start the XML document generation.
    out.writeStartDocument();
    // Have base class write the necessary information
    return XMLElement::write(out);
}

#endif
