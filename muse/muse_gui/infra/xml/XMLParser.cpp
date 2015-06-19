#ifndef XML_PARSER_CPP
#define XML_PARSER_CPP

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

#include "XMLParser.h"

#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QRegExp>
#include "XMLRootElement.h"
#include <QXmlStreamReader>
#include <QXmlSchema>
#include <QXmlSchemaValidator>

XMLParser::XMLParser(QObject *parent) : QAbstractMessageHandler(parent) {
    // Register meta type about the XMLElement common base class.
    qRegisterMetaType<XMLElement>("XMLElement");
}

void
XMLParser::handleMessage(QtMsgType type, const QString& description,
                         const QUrl& identifier,
                         const QSourceLocation& srcLoc) {
    Q_UNUSED(identifier);
    // Note that description has HTML annotations in certain cases.
    // So the HTML tags have to be stripped out of description to
    // retain just the text part of the message.
    QString message(description);
    message.remove(QRegExp("<[^>]*>"));
    // Track only non-debug messages (warning are treated as errors)
    if (type != QtDebugMsg) {
        parserErrMsg += srcLoc.uri().toString() + " [line: " +
                QString::number(srcLoc.line()) + ", col: " +
                QString::number(srcLoc.column()) + "]: " + message;
    }
}

QString
XMLParser::validate(const QString& xmlFileName, const QString &schemaFileName) {
    // Clear out the parser error message.
    parserErrMsg = "";
    // Open the schema file for reading.
    QFile schemaFile(schemaFileName);
    if (!schemaFile.open(QFile::ReadOnly)) {
        QString errMsg = "Error opening schema file: " + schemaFileName;
        return errMsg;
    }
    // Load the schema from the given file and ensure it is valid.
    QXmlSchema schema;
    schema.setMessageHandler(this);
    if (!schema.load(&schemaFile, QUrl::fromLocalFile(schemaFileName)) ||
        (!schema.isValid())) {
        QString errMsg = "Invalid schema or error reading from file: " +
                schemaFileName + ":\n" + parserErrMsg;
        return errMsg;
    }
    // Next, open the XML file to be validated.
    QFile xmlFile(xmlFileName);
    if (!xmlFile.open(QFile::ReadOnly)) {
        QString errMsg = "Error opening XML file: " + xmlFileName;
        return errMsg;
    }
    // Validate the XML file against the schema and return result
    QXmlSchemaValidator validator(schema);
    validator.setMessageHandler(this);
    if (validator.validate(&xmlFile, QUrl::fromLocalFile(xmlFileName))) {
        return ""; // success!
    }
    // Return error from validation
    return parserErrMsg;
}

QString
XMLParser::loadXML(const QString& xmlFileName, const QString& schemaFileName,
                   XMLRootElement& root) {
    // Track error messages (if any)
    QString errMsg = "";
    // Validate the XML file against the supplied schema
    if ((errMsg = validate(xmlFileName, schemaFileName)) != "") {
        // Validation failed. Don't load the XML
        return errMsg;
    }
    // Now that the XML is actually valid, load it.
    QFile xmlFile(xmlFileName);
    if (!xmlFile.open(QFile::ReadOnly)) {
        return (errMsg + "Error opening XML file: " + xmlFileName);
    }
    // Create the XML reader to load the data.
    QXmlStreamReader xmlReader(&xmlFile);
    xmlReader.setNamespaceProcessing(true);
    // Now load the data into the top-level root element
    root.read(xmlReader);
    // Check and report any errors
    if (xmlReader.hasError()) {
        return (errMsg + "Error ocurred when loading XML data from " +
                xmlFileName + ": " + xmlReader.errorString());
    }
    // Everything went well.
    return "";
}

QString
XMLParser::saveXML(const QString &xmlFileName, XMLRootElement &root) {
    QFile xmlFile(xmlFileName);
    if (!xmlFile.open(QIODevice::WriteOnly)) {
        QString errMsg = "Error opening XML file for writing: " + xmlFileName;
        return errMsg;
    }
    // Create XML writer and setup its properties.
    QXmlStreamWriter xmlWriter(&xmlFile);
    xmlWriter.setAutoFormatting(true);
    // Have the root element write itself and all underlying elements
    if (!root.write(xmlWriter)) {
        return QString("Error writing XML to file: " + xmlFileName);
    }
    // Everything went well.
    return "";
}

QString
XMLParser::saveXML(QString &output, XMLRootElement &root) {
    // Create XML writer and setup its properties.
    QXmlStreamWriter xmlWriter(&output);
    xmlWriter.setAutoFormatting(true);
    // Have the root element write itself and all underlying elements
    if (!root.write(xmlWriter)) {
        return QString("Error writing XML to file: " + xmlFileName);
    }
    // Everything went well.
    return "";
}

#endif
