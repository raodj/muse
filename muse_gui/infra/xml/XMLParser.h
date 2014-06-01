#ifndef XML_PARSER_H
#define XML_PARSER_H

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

#include <QObject>
#include <QCoreApplication>
#include <QAbstractMessageHandler>

// A forward declaration for XMLElement(s)
class XMLRootElement;
class XMLElement;

/**
 * @brief The XMLParser class to ease validation, reading, and writing of
 * instance variables to XML files.
 *
 * This class serves as the top-level API for validation, marshaling and
 * unmarshalling XML content to-and-from in memory class hierarchies (that
 * ease processing). The class is essentially a thin wrapper around the
 * QXmlStreamWriter, QXmlStreamReader, and associated classes from Qt's
 * \c xmlpatterns library.  Ensure that the project (.pro) file contains:
 *
 * \code
 *
 * QT += xmlpatterns
 *
 * \endcode
 *
 * The classes relies on the features provided by XMLElement, XMLRootElement,
 * and XMLElementInfo classes for its operations.  Refer to documentation on
 * these classes for additional information.
 */
class XMLParser : public QAbstractMessageHandler {
    Q_OBJECT
public:
    /**
     * @brief XMLParser The only (and default) constructor to create an XML Parser.
     *
     * @param parent The parent component (if any) with which this parser is
     * logically associated.  This value is not really used other than to pass on
     * to the base class.
     */
    explicit XMLParser(QObject* parent = NULL);

    /**
     * @brief ~XMLParser The destructor.
     *
     * This class does not explicitly use any dynamic memory and consequenlty the
     * destructor does not have any specific functionality. It is present as
     * a plase holder (for any future extensions) and to adhere to coding
     * conventions.
     */
    virtual ~XMLParser() {}

    /**
     * @brief validate Validate if a given XML file conforms to an XML schema
     * in a given schema file.
     *
     * This method (and class) provides a light weight wrapper around the
     * QXmlSchemaValidator class to ease validation of XML files.
     *
     * @param xmlFileName Path to the XML file to be validated.
     *
     * @param schemaFileName Path to the schema file to be used for validation.
     *
     * @return This method returns an empty string ("") upon successful
     * validation. However, on errors, this method returns a string with
     * error message(s) reported during validation.
     */
    QString validate(const QString& xmlFileName, const QString &schemaFileName);

    /**
     * @brief loadXML Unmarshal XML data from a given file into memory via
     * a given root XML element.
     *
     * This method internally invokes the validate() method to ensure that
     * the XML file conforms to the given schema before loading it.
     *
     * @param xmlFileName Path to the XML file to be loaded.
     *
     * @param schemaFileName Path to the schema file to be used for validation.
     *
     * @param root The root object corresponding to the top-level XML
     * element expected in the XML file.
     *
     * @return This method returns an empty string ("") upon successful
     * validation. However, on errors, this method returns a string with
     * error message(s) reported during validation.
   */
    QString loadXML(const QString& xmlFileName, const QString &schemaFileName,
                 XMLRootElement &root);

    /**
     * @brief saveXML Marshal contents from an in memory data structure to
     * an XML file.
     *
     * @param xmlFileName The path to the file to which the XML data is to be
     * unmarshalled. Existing data in the file is overwritten.
     *
     * @param root The top-level XML element from where the unmarshalling
     * commences.
     *
     * @return This method returns an empty string ("") upon successful
     * validation. However, on errors, this method returns a string with
     * error message(s) reported during validation.
     */
    QString saveXML(const QString& xmlFileName, XMLRootElement& root);

    /**
     * @brief saveXML Marshal contents from an in memory data structure in
     * an XML format to a given string.
     *
     * @param output The string to which the given root element information
     * is to be marshalled to.
     *
     * @param root The top-level XML element from where the unmarshalling
     * commences.
     *
     * @return This method returns an empty string ("") upon successful
     * validation. However, on errors, this method returns a string with
     * error message(s) reported during validation.
     */
    QString saveXML(QString& output, XMLRootElement& root);

protected:
    /**
     * @brief handleMessage Implementation for API method in the
     * QAbstractMessageHandler class.
     *
     * This method intercepts warnings and errors reported during XML
     * validation and stores it in the parserErrMsg instance variable.
     * This method strips out any HTML decoration that may be present
     * in the description, easing display and reporting to the user.
     *
     * @param type The type of message. Other than QtDebugMsg type
     * messages, all others are considered as errors (including warnings)
     * providing a stringent validation requirement.
     *
     * @param description The description about the error. Any HTML
     * decoration in the message is stripped.
     *
     * @param identifier Currently this identifier is ignored.
     *
     * @param srcLoc The location in the XML file where the error ocurred.
     * This information is used to generate a comprehensive error message
     * to be reported to the user.
     */
    virtual void handleMessage (QtMsgType type, const QString& description,
                                const QUrl& identifier,
                                const QSourceLocation & srcLoc);

private:
    QString xmlFileName;
    /**
     * @brief parserErrMsg Error message(s) (if any) reported during XML
     * validation. This instance variable is updated by the handleMessage
     * method to track any errors/warnings reported during XML validation.
     */
    QString parserErrMsg;
};

#endif // XMLPARSER_H
