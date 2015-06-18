#ifndef XML_ELEMENT_H
#define XML_ELEMENT_H

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

#include "XMLElementInfo.h"
#include <QMetaType>
#include <QVector>
#include <QXmlStreamReader>

/**
 * @brief SubElementList Convenience typedef to provide shortcut to
 * a vector of XMLElementInfo objects.
 *
 * Provides a shortcut for storing information about the list of
 * subelements in this XML element to be marshalled and unmarshalled.
 */
typedef QVector<XMLElementInfo> SubElementList;

// Forward declarations to keep compilation fast
class QXmlStreamWriter;

/**
 * @brief The XMLElement class represents an element in an XML file and
 * provides convenient API for marshalling and unmarshalling the element.
 *
 * <p>This class must be the base class of all elements in an XML file
 * (except for the root element for which use XMLRootElement class as the
 * parent instead of this class). The primary objective of the XMLElement
 * class to is to provide a convenient API for marshalling and unmarshalling
 * XML data into instance variables associated with a given class.  </p>
 *
 * <p>This class is designed to be used in the following manner:
 * <ol>
 *   <li>A class must be derived from this class and its default metadata
 *       information must be declared using the Q_DECLARE_METATYPE macro.</li>
 *   <li>The class must have a default constructor (so that objects can be
 *       created to load XML data from a file)</li>
 *   <li>In the constructor of the object, the object must be registered with
 *       Qt's meta type system via qRegisterMetaType() method call.</li>
 *   <li>Each sub-element to be marshalled and unmarshalled must be registered
 *       via call to the appropriate addElement method in this class.</li>
 * </ol>
 * </p>
 *
 * An example of such an operation is illustrated by the class below
 * (and it is shows the minimum required code to enable reading and
 * writing following XML data using the XMLParser class).
 *
 * \code
 *
 * #include "XMLRootElement.h"
 * #include "Address.h"
 * #include <QStringList>
 *
 * class Person : public XMLElement {
 * public:
 *   Person() : XMLElement("Person") {
 *       qRegisterMetaType<Person>("Person");
 *       addElement(XMLElementInfo("name",    &name));
 *       addElement(XMLElementInfo("email",   &emails));
 *       addElement(XMLElementInfo("Address", &addresses));
 *   }
 *
 *   ~Person() {}
 *
 * protected:
 *    QString name;
 *    QStringList emails;
 *    QList<XMLElement*> addresses;
 * };
 *
 * Q_DECLARE_METATYPE(Person)
 *
 * \endcode
 *
 * The XML data being read by the above class looks like:
 *
 * \code
 * <?xml version="1.0"?>
 * <Person xmlns="http://pc2lab.cec.miamiOH.edu/"
 *        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
 *        xsi:schemaLocation="http://www.peace-tools.org/ MUSE_GUI.xsd"
 *        Version="0.2">
 *    <name>testing</name>
 *    <email>test@gmail.com</email>
 *    <email>test@yahoo.com</email>
 *    <email>test@hotmail.com</email>
 *    <Address>
 *        <number>1600</number>
 *        <street>Pennsylvania Ave</street>
 *        <zip>45040</zip>
 *    </Address>
 *    <Address>
 *        <number>205</number>
 *        <street>510 E. High Street</street>
 *        <zip>45056</zip>
 *    </Address>
 * </Person>
 *
 * \endcode
 */
class XMLElement : public QObject {
    Q_OBJECT
    // Let the known (safe) derived class call private virtual methods
    friend class XMLRootElement;
public:
    /**
     * @brief XMLElement The default (and only) constructor.
     *
     * The constructor initializes the objects of the class. Currently,
     * it is does not have much operations to perform.
     *
     * @param name The element name for the root element in the XML document.
     * The name set for the root should be consistent with any XML schema
     * (to be) associated with the element. Note that this parameter is
     * required. The default value is sepcified just as a convenience so
     * that lists of XMLRootElement objects can be easily created.
     */
    XMLElement(const QString& name = "");

    /**
     * @brief ~XMLElement The destructor.
     *
     * The destructor is merely a place holder as this class currently does
     * not have any dynamically allocated objects.
     */
    virtual ~XMLElement();

protected:
    /**
     * @brief addElement Method to add sub-elements constituting this complex
     * XML element.
     *
     * This method must be used to add information about the sub-elements
     * constituting this complex XML element. See documentation at the class
     * level for an example. If the derived class contains transient
     * (information not to be persisted) instance variables, then those should
     * not to be added.
     *
     * \note The elements must be added in the order in which they occurr
     * in the XML document.
     *
     * @param subElemInfo The collective information about the element
     * encapsulated in a XMLElementInfo.
     *
     * \see XMLElementInfo
     */
    void addElement(const XMLElementInfo& subElemInfo);

    /**
     * @brief readNext Helper method to read the next useful token from a
     * given XML stream.
     *
     * This method provides a convenience wrapper to ignore whitespaces
     * and comment tokens in the input XML stream. This method is used
     * by various method in this class and the derived classes.
     *
     * @param xmlReader The XML stream from where whitespace and comment
     * tokens are to be skipped.
     *
     * @return The next useful token type to be processed.
     */
    static QXmlStreamReader::TokenType readNext(QXmlStreamReader& xmlReader);

private:
    /**
     * @brief checkStream Helper method to check to ensure that the given
     * input stream is in a sane state.
     *
     * This method checks to ensure that the given input stream is error free.
     * In addition, if a valid XMLElementInfo is supplied, this method also
     * verifies that the current element being processed matches the value
     * in the supplied element info.
     *
     * @param in The input stream to be validated.
     *
     * @param elemInfo Optional information about the current element being
     * processed (for sanity checking).
     *
     * @return This method returns true if the input stream and element
     * checks pass. Otherwise this method returns false indicating error.
     */
    bool checkStream(QXmlStreamReader& in,
                     const XMLElementInfo& elemInfo = XMLElementInfo::Invalid);

    /**
     * @brief checkSkipStartElement Check stream is at start of element and
     * optionally skip the token.
     *
     * This method is a refactored utility method that is used to process
     * start element tags. This method ensures that the current element
     * is indeed a start tag (otherwise it raises an error), checks to
     * ensure the element name is the same as the one in the given
     * element info (if it is valid), and skips to the next token (if
     * the skipToken flag is true).
     *
     * @param in The input XML stream to be processed.
     *
     * @param elemInfo An optional element information for validation. If
     * this element is invalid, then this method skips this check.
     *
     * @param skipToken A flag to indicate if the token should be ignored
     * and the input stream should be skipped to the next token.
     *
     * @return This method returns true if the input stream and element
     * checks pass. Otherwise this method returns false indicating error.
     */
    bool checkSkipStartElement(QXmlStreamReader& in,
                               const XMLElementInfo& elemInfo,
                               const bool skipToken = false);

    /**
     * @brief checkSkipEndElement Check stream is at end of element and
     * optionally skip the token.
     *
     * This method is a refactored utility method that is used to process
     * end element tags. This method ensures that the current element
     * is indeed a end tag (otherwise it raises an error), checks to
     * ensure the element name is the same as the one in the given
     * element info (if it is valid), and skips to the next token (if
     * the skipToken flag is true).
     *
     * @param in The input XML stream to be processed.
     *
     * @param elemInfo An optional element information for validation. If
     * this element is invalid, then this method skips this check.
     *
     * @param skipToken A flag to indicate if the token should be ignored
     * and the input stream should be skipped to the next token.
     *
     * @return This method returns true if the input stream and element
     * checks pass. Otherwise this method returns false indicating error.
     */
    bool checkSkipEndElement(QXmlStreamReader& in,
                             const XMLElementInfo& elemInfo,
                             const bool skipToken = false);

    /**
     * @brief write Utility method to marshall the contents of this object
     * to a given XML file.
     *
     * This method must be used to marshal the data in the instance
     * variables (registered via the addElement method) to the given
     * XML stream. If this element is the root element, then the necessary
     * namespace and attributes are approrpiately unmarshalled.
     *
     * \note Tyipcally, this method must not be directly invoked. Instead
     * use XMLParser class to indirectly invoke this method.
     *
     * @param out The output XML stream to which this element is to be
     * unmarshalled.
     *
     * @return This method returns true if the necessary data was successfully
     * saved. On errors this method returns false (but does not report an
     * explicit error message because unlike the QXmlStreamReader, the
     * QXmlStreamWriter does not seem to have a method to report error messages)
     */
    virtual bool write(QXmlStreamWriter& out);

    /**
     * @brief write Utility method to unmarshall the contents of corresponding
     * XML element (from a file) into instance variables.
     *
     * This method must be used to unmarshall the data from an XML element
     * into in the instance variables (registered via the addElement method)
     * from a given XML stream. If this element is the root element, then
     * the necessary namespace and attributes are approrpiately unmarshalled.
     *
     * \note Tyipcally, this method must not be directly invoked. Instead
     * use XMLParser class to indirectly invoke this method.
     *
     * @param out The int XML stream from where this element is to be
     * unmarshalled.
     *
     * @return This method returns true if the necessary data was successfully
     * saved. On errors this method returns false after reporting the error
     * with suitable message via QXmlStreamReader::raiseError() method.
     */
    virtual bool read(QXmlStreamReader& in);

    /**
     * @brief readAttributes Optional method to facilitate loading
     * optional namespaces and attributes.
     *
     * This method is invoked when the start tag for the element is being
     * read.  This method is merely a place holder in this class
     * and does not perform any operation other than reporting an error if
     * a non-root element has attributes.  However, the XMLRootElement
     * derived class overrides this method to load namespace declearations
     * and attributes.
     *
     * @param in The convenience XML parser stream from where the namespace
     * and attributes (if any) for the element are to be loaded.
     *
     * @return This method returns true if the necessary data was successfully
     * read. On errors this method returns false after reporting the error
     * with suitable message via QXmlStreamReader::raiseError() method.
     */
    virtual bool readAttributes(QXmlStreamReader& in);

    /**
     * @brief writeAttributes Optional method to facilitate unmarshalling
     * optional namespaces and attributes.
     *
     * This method is invoked when the start tag for the element is being
     * written.  This method is merely a place holder in this class
     * and does not perform any operations. However, the XMLRootElement
     * derived class overrides this method to write namespace declearations
     * and attributes.
     *
     * @param in The convenience XML parser stream from where the namespace
     * and attributes (if any) for the element are to be unmarshalled.
     *
     * @return This method returns true if the necessary data was successfully
     * written. On errors this method returns false.
     */
    virtual bool writeAttributes(QXmlStreamWriter& out);

    /**
     * @brief readSimpleType Helper method to unmarshal a simple element
     * (i.e., not an instance variable derived from XMLElement class).
     *
     * This method is invoked from the read() method in this class to
     * read a simple element (example: "<name>Testing</name>"). This method
     * extracts the text for the element and updates the corresponding
     * instance variable via the supplied info object.
     *
     * @param in The input XML stream from where the element is to be read.
     *
     * @param info The information about the element to be used to appropriately
     * store the information for the element.
     *
     * @return This method returns true if the operations succeeded. Otherwise
     * this method returns false.
     */
    bool readSimpleType(QXmlStreamReader& in, XMLElementInfo& info);

    /**
     * @brief readComplexType Helper method to unmarshal a complex element
     * (i.e., an instance variable derived from XMLElement class).
     *
     * This method is invoked from the read() method in this class to
     * read a complex element (that can have other complex elements or
     * simple elements nested). This method handles list of complex
     * elements or a single complex element differently. If it is single
     * complex element, then its read method is used for further processing.
     * However, if the storage is for a list of values, then this method
     * creates a new object (via call to QMetaType::construct() method),
     * populates the new object (by calling its read() method), and then
     * adds it to the list (instance variable).
     *
     * @param in The input XML stream from where the element is to be read.
     *
     * @param info The information about the element to be used to appropriately
     * store the information for the element.
     *
     * @return This method returns true if the operations succeeded. Otherwise
     * this method returns false.
     */
    bool readComplexType(QXmlStreamReader &in, XMLElementInfo &info);

    /**
     * @brief writeSimpleType Helper method to marshal a simple element
     * (i.e., not an instance variable derived from XMLElement class).
     *
     * This method is invoked from the write() method in this class to
     * write a simple element (example: "<name>Testing</name>"). This method
     * handles both a single simple element or a list of simple elements.
     *
     * @param out The out XML stream where the element is to be written.
     *
     * @param info The information about hte element to be written.
     *
     * @return This method returns true if the operations succeeded. Otherwise
     * this method returns false.
     */
    bool writeSimpleType(QXmlStreamWriter& out, XMLElementInfo& info);

    /**
     * @brief writeSimpleType Helper method to marshal a simple element
     * (i.e., not an instance variable derived from XMLElement class).
     *
     * This method is invoked from the write() method in this class to
     * write complex element(s) (objects derived from XMLElement class).
     * This method handles both a single complex-element or a list of
     * complex-elements. This method essentially recursively calls the
     * write() method on one of the complex elements to fully marhsal
     * to a complete XML document.
     *
     * @param out The out XML stream where the element is to be written.
     *
     * @param info The information about the element to be written.
     *
     * @return This method returns true if the operations succeeded. Otherwise
     * this method returns false.
     */
    bool writeComplexType(QXmlStreamWriter &out, XMLElementInfo &info);

private:
    /**
     * @brief info The information about the XML element represented by this
     * object. This value is typically initialized in the constructor and is
     * never changed during the life time of this object.
     */
    XMLElementInfo info;

    /**
     * @brief subElemList The list of sub-elements for this XML element. Each
     * entry in this list provides information about the corresponding
     * XML element. The information is used to suitably marshal and unmarshal
     * the data to-and-from an XML source. Entries are added to this list
     * via the addElement() method.
     */
    SubElementList subElemList;
};

#endif // XMLELEMENT_H
