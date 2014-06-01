#ifndef XML_ROOT_ELEMENT_H
#define XML_ROOT_ELEMENT_H

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

/**
 * @brief The XMLRootElement class that represents the root element in an
 * XML document.
 *
 * This class customizes the XMLElement base class and represents the root
 * element in an XML document. Currently, it is assumed that an XML document
 * has only a single root element that is represented by this class.
 * The root element follows all the semantics and features of the underlying
 * XMLElement class and provides convenient interface for marshalling and
 * unmarshalling XML data into instance variables of appropriate type. In
 * addition, his class also provides the following two additional features:
 *
 * <ol>
 *
 *  <li>The root element can have specific namespaces associated with it.
 *      The namespaces (if present) are loaded from a given file and written
 *      to disk.</li>
 *
 *  <li>The root element can have additional attributes associated with it.
 *      The attributes are automatically loaded from a given file and are
 *      written to disk (when the XML data is written). </li>
 *
 * </ol>
 *
 * Refer to the documentation in the base class for additional information.
 *
 * \note The XMLParser class must be used for marhsalling and unmarshalling
 * this class.
 *
 * \note In the current design, complex sub-elements that occurr more than
 * once must be stored as a QList<XMLElement*>.
 */
class XMLRootElement : public XMLElement {
    // Let XMLParser call private methods for marhsalling & unmarshalling.
    friend class XMLParser;
public:
    /**
     * @brief XMLRootElement The default (and only) constructor.
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
    XMLRootElement(const QString& name = "");

    /**
     * @brief ~XMLRootElement The destructor.
     *
     * The destructor is merely a place holder as this class currently does
     * not have any dynamically allocated objects.
     */
    virtual ~XMLRootElement();

protected:
    /**
     * @brief attributes The list of attributes associated with the root
     * element.
     *
     * The values in this list are updated when an XML file is loaded. The
     * values are written when the element is saved to an XML file. The
     * attributes can be empty (and when empty, they have no negative
     * side effects).
     */
    QXmlStreamAttributes attributes;

    /**
     * @brief namespaces The namespace delcarations associated with the root
     * element.
     *
     * The values in this list are updated when an XML file is loaded. The
     * values are written when the element is saved to an XML file. The
     * namespaces can be empty (and when empty, they have no negative
     * side effects).
     */
    QXmlStreamNamespaceDeclarations namespaces;

private:
    /**
     * @brief readAttributes Override base class implementation to actually
     * read namespaces and attributes.
     *
     * This method is invoked from the base class when a root element is
     * being read from an XML file. This method loads any attributes and
     * namespace values specified for the root element from the given input
     * XML file.
     *
     * @param in The convenience XML parser stream from where the namespace
     * and attributes (if any) for the root element are to be loaded.
     *
     * @return This method returns true if the necessary data was successfully
     * read. On errors this method returns false after reporting the error
     * with suitable message via QXmlStreamReader::raiseError() method.
     */
    virtual bool readAttributes(QXmlStreamReader& in);

    /**
     * @brief writeAttributes Override base class implementation to actually
     * write namespaces and attributes (if any) for the root element.
     *
     * This method is invoked from the base class when a root element is
     * being written to an XML file. This method writes any attributes and
     * namespace values specified for the root element to the given output
     * XML file.
     *
     * @param in The convenience XML writer stream to where the namespace
     * and attributes (if any) for the root element are to be written.
     *
     * @return This method returns true if the necessary data was successfully
     * saved. On errors this method returns false (but does not report an
     * explicit error message because unlike the QXmlStreamReader, the
     * QXmlStreamWriter does not seem to have a method to report error messages)
     */
    virtual bool writeAttributes(QXmlStreamWriter& out);

    /**
     * @brief write Utility method to start marshalling the contents
     * of this XML document (and underlying elements) to a given XML file.
     *
     * This method is used by the XMLParser to commence marshalling the
     * data in the instance variables (registered via the addElement method)
     * to the given XML stream. This method starts the document and then
     * uses the base class method to perform the actual operations.
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
     * XML element and underlying elements into instance variables.
     *
     * This method must be used to commence unmarshaling the data from an XML
     * element into in the instance variables (registered via the addElement
     * method) from a given XML stream. If this element is the root element,
     * then the necessary namespace and attributes are approrpiately
     * unmarshalled.
     *
     * \note Tyipcally, this method must not be directly invoked. Instead
     * use XMLParser class to indirectly invoke this method.
     *
     * @param out The int XML stream from where this element is to be
     * unmarshalled. Typically no additional operation is performed on
     * the XML stream as this method perofrms some sanity checks to ensure
     * consistent starting state.
     *
     * @return This method returns true if the necessary data was successfully
     * saved. On errors this method returns false after reporting the error
     * with suitable message via QXmlStreamReader::raiseError() method.
     */
    virtual bool read(QXmlStreamReader& in);
};

#endif // XML_ROOT_ELEMENT_H
