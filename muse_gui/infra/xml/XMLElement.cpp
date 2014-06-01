#ifndef XML_ELEMENT_CPP
#define XML_ELEMENT_CPP

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

#include "XMLElement.h"
#include <QXmlStreamWriter>
#include <QDebug>

QXmlStreamReader::TokenType
XMLElement::readNext(QXmlStreamReader& xmlReader) {
    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        // Read the next token from the XML stream
        xmlReader.readNext();
        if (!xmlReader.isWhitespace() && !xmlReader.isComment()) {
            // Found a non-comment and non-white space token for processing.
            break;
        }
    }
    // Return the next useful token to be processed.
    return xmlReader.tokenType();
}

XMLElement::XMLElement(const QString &name) : info(XMLElementInfo(name, this)) {
    // Nothing else to be done here in the constructor
}

XMLElement::~XMLElement() {
    // An empty constructor begets an empty destructor.
}

void
XMLElement::addElement(const XMLElementInfo &subElemInfo) {
    subElemList.push_back(subElemInfo);
}

bool
XMLElement::checkStream(QXmlStreamReader& in, const XMLElementInfo& elemInfo) {
    // Check to ensure that the XML stream is valid and is in good state
    if (in.atEnd() || in.hasError()) {
        const QString msg = "Error reading element '" +
                elemInfo.getName() + "' from XML.";
        in.raiseError(msg);
        return false;
    }
    // Check to ensure that the element name (if any) matches (insanity check)
    if (!elemInfo.getName().isEmpty()) {
        // Double check to ensure that the current element names match up!
        if (in.name() != elemInfo.getName()) {
            const QString msg = "Error reading element '" + elemInfo.getName() +
                    "' from XML (found element '" + in.name().toString() +
                    "' instead)";
            in.raiseError(msg);
            return false;
        }
    }
    // Everything seems fine.
    return true;
}

bool
XMLElement::readAttributes(QXmlStreamReader &in) {
    // Process attributes only for root elements. For other elements generate
    // an error if attributes are present (and ignore attributes).
    if (!in.attributes().empty()) {
        const QString msg = "Non-root element " + info.getName() +
                " has attributes (it is a unhandled scenario)";
        in.raiseError(msg);
        return false;
    }
    // No issues so far.
    return true;
}

bool
XMLElement::checkSkipStartElement(QXmlStreamReader& in,
                                  const XMLElementInfo& elemInfo,
                                  const bool skipToken) {
    // Check to ensure that the XML stream is valid and is in good state
    if (!checkStream(in, elemInfo)) {
        return false; // something is off with the stream.
    }
    // Ensure the stream is at start of element
    if (in.tokenType() != QXmlStreamReader::StartElement) {
        const QString msg = "XML stream is not at start of element (" +
                in.name().toString() + "). Token is: " + in.tokenString();
        in.raiseError(msg);
        return false;
    }
    // Process attributes only for root elements. For other elements generate
    // an error if attributes are present (and ignore attributes).
    if (skipToken && !readAttributes(in)) {
        return false; // Error encountered.
    }
    // Skip the start element preparing the XML stream for the next token
    if (skipToken) {
        readNext(in);
    }
    // Looks good so far.
    return !in.hasError();
}

bool
XMLElement::checkSkipEndElement(QXmlStreamReader& in,
                                const XMLElementInfo& elemInfo,
                                const bool skipToken) {
    // Check to ensure that the XML stream is valid and element name matches
    if (!checkStream(in, elemInfo)) {
        return false; // error encountered.
    }
    // Ensure the stream is at end of element
    if (in.tokenType() != QXmlStreamReader::EndElement) {
        const QString msg = "XML stream is not at end of element." +
                in.name().toString();
        in.raiseError(msg);
        return false;
    }
    // Skip the end element preparing the XML stream for the next token
    if (skipToken) {
        readNext(in);
    }
    // Looks good so far.
    return !in.hasError();
}

bool
XMLElement::read(QXmlStreamReader &in) {
    if (!checkSkipStartElement(in, info, true)) {
        // The stream is not in a consistent state!
        return false;
    }
    // Next read data for each element in the sub element list.
    int subElemIndex = 0;
    // Repeatedly process sub-elements (or an error occurrs)
    while (!in.isEndElement() && !in.atEnd() && !in.hasError() &&
           (subElemIndex < subElemList.size())) {
        // Read the next token in the input XML stream. It must be a sub-element
        if (!checkSkipStartElement(in, XMLElementInfo::Invalid)) {
            return false; // Error case!
        }
        // Find the matching element name in the sub-element list.
        while((subElemIndex < subElemList.size()) &&
              (subElemList[subElemIndex].getName() != in.name())) {
            subElemIndex++;
        }
        // If the element was not found, bail out with an error
        if (subElemIndex >= subElemList.size()) {
            // Element in XML was not found in our sub-elements! Error out
            const QString msg = "Element '" + in.name().toString() +
                    "' is not a valid sub-element of '" + info.getName() + "'.";
            in.raiseError(msg);
            return false; // Error occurred during processing.
        }
        // Have the helper method consume this element based on its type.
        XMLElementInfo& info = subElemList[subElemIndex];
        if (!(info.isUserType() ? readComplexType(in, info) :
             readSimpleType(in, info))) {
            // Error occurred during processing;
            return false;
        }
    }
    // Everything went well so far. Here we should be at end of this element.
    return checkSkipEndElement(in, info, true);
}

bool
XMLElement::readComplexType(QXmlStreamReader &in, XMLElementInfo &info) {
    Q_ASSERT(info.isUserType());
    if (!info.isListType()) {
        // The sub-element is a complex data type. Here we assume the
        // pointer is of XMLElement type.
        return static_cast<XMLElement*>(info.getPointer())->read(in);
    }
    // This is a list of complex types. This case is different.
    // Create an object via the type system for this element.
    const int type = XMLElementInfo::getType(info.getName());
    void *object   = QMetaType::create(type);
    if (object == NULL) {
        // The expected user-type was not found!
        const QString msg = "Element '" + info.getName() +
                "' does not seem to have been registered with Qt's "
                "type system!. Cannot load element into the list.";
        in.raiseError(msg);
        return false; // Error occurred during processing.
    }
    XMLElement* subElem = reinterpret_cast<XMLElement*>(object);
    if (!subElem->read(in)) {
        // Error reading sub element (error already reported) bail out!
        return false;
    }
    // Add sub-element to the list.
    info.addValue(subElem);
    // Everything went well
    return true;
}

bool
XMLElement::readSimpleType(QXmlStreamReader& in, XMLElementInfo& info) {
    Q_ASSERT(!info.isUserType());
    // A simple element may have ocurrence of 1 (individual element)
    // or more (list of elements)
    if (!checkSkipStartElement(in, info)) {
        // The stream is not in a consistent state!
        return false;
    }
    // Obtain the next token (should be Characters)
    if (readNext(in) != QXmlStreamReader::Characters) {
        const QString msg = "Expecting data for element " + info.getName() +
                ". But got XML token " + in.tokenString() + " instead.";
        in.raiseError(msg);
        return false;
    }
    // Extract value for the element.
    const QString value = in.text().toString();
    // Appropriately set/add the value depending on the element's data type.
    info.setValue(value);
    // Onto the next token.
    readNext(in);
    // Everything went well so far. Here we should be at end of this element.
    return checkSkipEndElement(in, info, true);
}

bool
XMLElement::writeAttributes(QXmlStreamWriter &out) {
    Q_UNUSED(out);
    // In the XMLElement base class this method has nothing to do as
    // non-root elements do not currently have attributes.
    return true;
}

bool
XMLElement::write(QXmlStreamWriter& out) {
    // Write the starting element for the top-level element.
    out.writeStartElement(info.getName());
    // Write out attributes (only for root element at this time)
    writeAttributes(out);
    // Write out the sub-elements (if any)
    for(int i = 0; (!out.hasError() && (i < subElemList.size())); i++) {
        XMLElementInfo& subInfo = subElemList[i];
        if (subInfo.getPointer() == NULL) {
            // No data to write. Skip this entry.
            continue;
        }
        // If it is a complex element let the sub-element handle writing.
        const bool result =
                (subInfo.isUserType() ? writeComplexType(out, subInfo) :
                                        writeSimpleType (out, subInfo));
        if (!result) {
            // Error occurred when writing sub-element. Bail out.
            return false;
        }
    }
    // Wrap up the element.
    out.writeEndElement();
    // Return reporting any potential errors.
    return !out.hasError();
}

bool
XMLElement::writeComplexType(QXmlStreamWriter &out, XMLElementInfo &info) {
    Q_ASSERT(info.isUserType());
    if (!info.isListType()) {
        // This is a simple subelement.
        return reinterpret_cast<XMLElement*>(info.getPointer())->write(out);
    }
    // The information is a list type. The operations are different.
    bool result = true;
    for(int i = 0; (result && (i < info.getListSize())); i++) {
        result = info.getSubElement(i)->write(out);
    }
    return result;
}

bool
XMLElement::writeSimpleType(QXmlStreamWriter &out, XMLElementInfo &info) {
    Q_ASSERT(!info.isUserType());
    if (!info.isListType()) {
        // If it is not a list type, then the just write the value out.
        out.writeTextElement(info.getName(), info.getValue());
    } else {
        // The information is a list type. The operations are different.
        for(int i = 0; (i < info.getListSize()); i++) {
            out.writeTextElement(info.getName(), info.getValue(i));
        }
    }
    // Return reporting any potential errors.
    return !out.hasError();
}

#endif
