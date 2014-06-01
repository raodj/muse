#ifndef XML_ELEMENT_INFO_H
#define XML_ELEMENT_INFO_H

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

#include <QMetaType>
#include <typeinfo>
#include <QString>
#include <QHash>

// Forward declarations to keep compile fast
class XMLElement;

/**
 * @brief The XMLElementInfo class A convenience class to wrap information about
 * various sub-elements in an XML document to ease marshaling and unmarshaling.
 *
 * <p>This class provides a type-agnostic wrapper to ease reading and writing of
 * various types of commonly used XML elements.  In the current design,
 * complex sub-elements that occurr more than once must be stored as a
 * QList<XMLElement*>.  This class provides the XMLElement class a convenient
 * API to handle simple, complex, and list of various types of elements.
 * Refer to class-level documentation for the XMLElement class for an
 * operational example.</p>
 *
 * Currently this class handles the following data types and
 * corresponding list of elements (represented as QList<T> objects):
 *
 * <table>
 *   <th><td>XML datatype</td> <td>C++ datatype</td></th>
 *   <tr><td>xsd:string</td>   <td>QString</td></tr>
 *   <tr><td>xsd:integer</td>  <td>int</td></tr>
 *   <tr><td>xsd:long</td>     <td>long</td></tr>
 *   <tr><td>xsd:decimal</td>  <td>double</td></tr>
 *   <tr><td>xsd:date</td>     <td>QDate</td></tr>
 *   <tr><td>xsd:time</td>     <td>QTime</td></tr>
 *   <tr><td>xsd:dateTime</td> <td>QDateTime</td></tr>
 * </table>
 *
 * \note All complex sub-elements must be derived from XMLElement object.
 * Furthermore, list of complex sub-elements are handled as QList<XMLElement*>.
 */
class XMLElementInfo {
public:
    /**
     * @brief XMLElementInfo This constructor should never be used but is
     * present merely to ease creating vectors of this object.
     *
     * This constructor is merely present to serve as a convenience place
     * holder. In addition, it populates the TypeNameMap (needed only
     * for compatibility with non c++11 compilers) the first time it is
     * called.
     */
    XMLElementInfo();

    /**
     * @brief XMLElementInfo Create an element information for a standard
     * boolean element.
     *
     * @param name The name of the XML simple element associated with this
     * element information.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the data associated
     * with this XML element. This pointer is used during marshaling and
     * unmarshalling operations. If the pointer is NULL, then this element
     * is ignored during read/write operations.
     */
    XMLElementInfo(const QString &name, bool* pointer);

    /**
     * @brief XMLElementInfo Create an element information for a standard
     * string element.
     *
     * @param name The name of the XML simple element associated with this
     * element information.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the data associated
     * with this XML element. This pointer is used during marshaling and
     * unmarshalling operations. If the pointer is NULL, then this element
     * is ignored during read/write operations.
     */
    XMLElementInfo(const QString &name, QString* pointer);

    /**
     * @brief XMLElementInfo Create an element information for a standard
     * integer element.
     *
     * @param name The name of the XML simple element associated with this
     * element information.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the data associated
     * with this XML element. This pointer is used during marshaling and
     * unmarshalling operations. If the pointer is NULL, then this element
     * is ignored during read/write operations.
     */
    XMLElementInfo(const QString &name, int* pointer);

    /**
     * @brief XMLElementInfo Create an element information for a standard
     * long element.
     *
     * @param name The name of the XML simple element associated with this
     * element information.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the data associated
     * with this XML element. This pointer is used during marshaling and
     * unmarshalling operations. If the pointer is NULL, then this element
     * is ignored during read/write operations.
     */
    XMLElementInfo(const QString &name, long* pointer);

    /**
     * @brief XMLElementInfo Create an element information for a standard
     * decimal (double) element.
     *
     * @param name The name of the XML simple element associated with this
     * element information.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the data associated
     * with this XML element. This pointer is used during marshaling and
     * unmarshalling operations. If the pointer is NULL, then this element
     * is ignored during read/write operations.
     */
    XMLElementInfo(const QString &name, double* pointer);

    /**
     * @brief XMLElementInfo Create an element information for a standard
     * date element.
     *
     * @param name The name of the XML simple element associated with this
     * element information.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the data associated
     * with this XML element. This pointer is used during marshaling and
     * unmarshalling operations. If the pointer is NULL, then this element
     * is ignored during read/write operations.
     */
    XMLElementInfo(const QString &name, QDate* pointer);

    /**
     * @brief XMLElementInfo Create an element information for a standard
     * time element.
     *
     * @param name The name of the XML simple element associated with this
     * element information.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the data associated
     * with this XML element. This pointer is used during marshaling and
     * unmarshalling operations. If the pointer is NULL, then this element
     * is ignored during read/write operations.
     */
    XMLElementInfo(const QString &name, QTime* pointer);

    /**
     * @brief XMLElementInfo Create an element information for a standard
     * DateTime element.
     *
     * @param name The name of the XML simple element associated with this
     * element information.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the data associated
     * with this XML element. This pointer is used during marshaling and
     * unmarshalling operations. If the pointer is NULL, then this element
     * is ignored during read/write operations.
     */
    XMLElementInfo(const QString &name, QDateTime* pointer);

    /**
     * @brief XMLElementInfo Create an element information for a complex
     * sub-element element.
     *
     * @param name The name of the XML simple element associated with this
     * element information.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the data associated
     * with this XML element. This pointer is used during marshaling and
     * unmarshalling operations. If the pointer is NULL, then this element
     * is ignored during read/write operations.
     */
    XMLElementInfo(const QString &name, XMLElement* pointer);

    /**
     * @brief XMLElementInfo Create an element information for a list
     * (immaterial of occurrence count) of strings.
     *
     * @param name The name of the XML simple element associated with each
     * occurrence of this XML element.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the QStringList
     * that contains the data associated with this XML element. This
     * pointer is used during marshaling and unmarshalling operations.
     * If the pointer is NULL, then this element is ignored during
     * read/write operations.
     */
    XMLElementInfo(const QString &name, QStringList* pointer);

    /**
     * @brief XMLElementInfo Create an element information for various
     * lists of elements (both simple and complex).
     *
     * @param name The name of the XML element associated with each
     * occurrence of this element.
     *
     * @param pointer Pointer to the instance variable (typically in a
     * class derived from XMLElement) which contains the QList
     * that contains the data associated with this XML element. This
     * pointer is used during marshaling and unmarshalling operations.
     * If the pointer is NULL, then this element is ignored during
     * read/write operations.
     *
     * \note In the current design, complex sub-elements that occurr
     * more than once must be stored as a QList<XMLElement*>.
     */
    template<typename T>
    XMLElementInfo(const QString &name, QList<T>* pointer) : name(name),
        type(getType(typeid(T).name())), isList(true), pointer(pointer) {
        // Nothing else to be done here.
    }

    /**
     * @brief XMLElementInfo The copy constructor.
     *
     * The copy constructor needs to be explicitly defined to facilitate
     * management of arrays of objects.
     *
     * @param src The source object from where the data is to be copied.
     */
    XMLElementInfo(const XMLElementInfo& src);

    /**
     * @brief ~XMLElement The destructor.
     *
     * The destructor deletes underlying complex sub-element objects if this
     * element contains a list of such sub-elements (that is, isListType() and
     * isUserType() methods return true).
     */
    ~XMLElementInfo();

    /**
     * @brief getType Convenience method to wrap calls to QMetaType::type()
     * method.
     *
     * This method enables mapping of custom type names generated by
     * different compilers to a consistent type name used by Qt's
     * QMetaType class. For example given the type name "i" (generated by GCC),
     * this method converts it to "int" (by looking-up TypeNameMap) and then
     * uses the mapped type name to determine the type handle.  This method
     * is used to instantiate derived classes when handling lists of complex
     * XML elements.
     *
     * @param typeName The type name whose integer handle is to be determined.
     *
     * @return The integer handle to the given type.
     */
    static int getType(const QString& typeName);

    /**
     * @brief getName Returns the name set for the XML element.
     *
     * The name corresponds to both the XML element and the C++ class that
     * is used for its in-memory representation. This value is set when the
     * XMLElementInfo is created (as is never changed).
     *
     * @return This method returns the name associated with the given XML
     * element.
     */
    const QString& getName() const { return name; }

    /**
     * @brief getPointer Obtain the uninterpreted pointer set for storing the
     * data associated with the XML element.
     *
     * This method returns the raw/uninterpreted pointer associated with
     * the given XML document. The pointer was the value set when this object
     * was instantiated.
     *
     * @return The uninterpreted pointer for storing data. This value can be
     * NULL.
     */
    void* getPointer() { return pointer; }

    /**
     * @brief getType Returns the type handle value associated with the data
     * type of the XML element.
     *
     * This method returns the type handle value set for the data type of
     * the XML element. This method essentially returns the value set for the
     * type of the XML element when this object was intantiated. If the
     * element is a list of values, then this method returns the type of
     * individual elements in the list.
     *
     * @return The type handle associated with the data type of the XML element.
     */
    int getType() const { return type; }

    /**
     * @brief isListType Determine if the storage for this element is a list of
     * values (and not just a single valued element).
     *
     * This method essentially returns the value set when the object was
     * instantiated.  This flag is used to determine if the storage for the
     * element is a QList<T> as the element has zero or more occurrence in the
     * XML file.
     *
     * @return This method returns true if the storage is a list. That is the
     * value returned by getPointer() must be interpreted as a QList<T>.
     */
    bool isListType() const { return isList; }

    /**
     * @brief isUserType Convenience method to determine if the data type of
     * element(s) is derived from XMLElement.
     *
     * This method provides a streamlined interface to determine how the
     * datatype of the element is to be interpreted.
     *
     * @return Returns true if the type is not one of the default/standard
     * Qt types, indicating that the type is a class derived from XMLElement.
     */
    bool isUserType() const { return type >= QMetaType::User; }

    /**
     * @brief getListSize Convenience method to determine the number of elements
     * in the list associated with this element.
     *
     * This method provides meanigful result only if isListType() method
     * returns true for this object.
     *
     * @return Returns the number of elements in the list. If the element is
     * not a list of values, then this method returns -1.
     */
    int  getListSize() const;

    /**
     * @brief setValue Interpret and set the value for element.
     *
     * This method appropriately interprets the contents of the given string
     * value (read from an XML source) associated with this element and
     * stores it in the location specified by getPointer() method. The datatype
     * of the storage location is suitably interpreted based on the value
     * returned by getType().a string representation of the data associated with
     * the element at a given index position.
     *
     * \note If isListType() method returns true, then this method internally
     * calls the addValue() method.
     *
     * \note If the pointer is NULL, then this method does not perform any
     * operations.
     *
     * @param value The textual form of data from the XML source.
     */
    void setValue(const QString& value);

    /**
     * @brief addValue Interpret and add the value to list of values.
     *
     * This method appropriately interprets the contents of the given string
     * value (read from an XML source) associated with this element and
     * adds it to the QList<T> specified by getPointer() method. The datatype
     * of the elements in the QList is suitably interpreted based on the value
     * returned by getType().
     *
     * \note This method does not perform any operations if the data type is
     * not a list of values or if the pointer is NULL.
     *
     * @param value The value to be interpreted and added to the list of
     * values.
     */
    void addValue(const QString& value);

    /**
     * @brief addValue Convenience method to add a complex element to the list
     * of values.
     *
     * This method provides a convenience interface to add a complex element
     * to a QList<XMLElement*> pointed by pointer (instance variable).
     *
     * \note After call to this method, this object takes ownership of the
     * supplied object and handles deletion of the objects in the list.
     *
     * @param value The value to be added to the list of elements.
     */
    void addValue(XMLElement* value);

    /**
     * @brief getValue Obtain string representation of value associated with
     * this element.
     *
     * This method appropriately reinterprets the contents of the pointer
     * associated with this element and returns a string representation
     * of the data to ease marshalling to XML.
     *
     * \note This method currently handles only simple/known element types
     * listed in the class-level documentation. Specifically, this method
     * must not be used if isUserType() method returns true.
     *
     * @return Returns a string representation of the data associated with
     * this element. If the data type could not be determined then this
     * method returns an empty string.
     */
    QString getValue() const;

    /**
     * @brief getValue Obtain string representation of value associated with
     * an element in a list.
     *
     * This method is applicable only if isListType() method returns true
     * (indicating that the pointer is a QList).  This method interprets
     * the contents of the list based on the datatype and returns the string
     * implementation of the element at the given index location.
     *
     * @param index The value must be in the range 0 <= index < getListSize().
     *
     * @return Returns a string representation of the data associated with
     * the element at a given index position. If the data type could not be
     * determined then this method returns an empty string.
     */
    QString getValue(const int index) const;

    /**
     * @brief getValue Obtain a complex sub-element from a list of values.
     *
     * This method is applicable only if isListType() method returns true
     * (indicating that the pointer is a QList) and isUserType() method returns
     * true (indicating it is list of XMLElement*).  This method returns the
     * XMLElement* object at the given index location.
     *
     * @param index The value must be in the range 0 <= index < getListSize().
     *
     * @return Returns the XMLElement* (previously added) at the given index
     * position. If this object is not a list of complex elements, then this
     * method returns NULL.
     */
    const XMLElement* getSubElement(const int index) const;

    /**
     * @brief getValue Obtain a complex sub-element from a list of values.
     *
     * This method is applicable only if isListType() method returns true
     * (indicating that the pointer is a QList) and isUserType() method returns
     * true (indicating it is list of XMLElement*).  This method returns the
     * XMLElement* object at the given index location.
     *
     * @param index The value must be in the range 0 <= index < getListSize().
     *
     * @return Returns the XMLElement* (previously added) at the given index
     * position. If this object is not a list of complex elements, then this
     * method returns NULL.
     */
    XMLElement* getSubElement(const int index);

public:
    /**
     * @brief Invalid A convenience static constant to refer to an invalid
     * element information object.  This object is used by various methods
     * in XMLElement object hierarcy as a convenience.
     */
    static const XMLElementInfo Invalid;

protected:
    /**
     * @brief toBool Convert a string value to boolean.
     *
     * Just a convenience method for converting to native boolean.
     *
     * @param value The string to be converted. The string must be either "true"
     * or "false".
     *
     * @return This method returns the native boolean value for the given
     * string.
     */
    static bool toBool(const QString& value);

    /**
     * @brief toBool Convert native boolean value to a QString.
     *
     * Just a convenience method for converting from a native boolean to string
     * to ease marshaling to XML.
     *
     * @param value The value to be converted to string.
     *
     * @return This method returns "true" if the paramater was true. Otherwise
     * it returns "false".
     */
    static QString fromBool(const bool value);

private:
    /**
     * @brief name The name for the XML element. For complex types, this value
     * is also interpreted as the name of the C++ class used to store this
     * data in memory. This value is set when this object is instantiated and
     * is never changed during the life time of this class.
     */
    QString name;

    /**
     * @brief type The data type associated with this element. This value
     * is obtained via call to the  XMLElementInfo::getType() method. This
     * value corresponds to the QMetaType::Type list of values.
     */
    int type;

    /**
     * @brief isList Flag to indicate if this element represents a list of
     * values rather than a single element. If this flag is true, then
     * the pointer (instance variable) is interpreted as a QList<T> (where
     * the datatype of T is dependent on type).
     */
    bool isList;

    /**
     * @brief pointer An uninterpreted (generalized) reference to the the
     * storage area for reading/writing the data associated with this
     * XML element.
     */
    void* pointer;

    /**
     * @brief TypeNameMap Convenience dictionary to map type names generated by
     * various compilers to suitable types in QMetaType::Type enumeration.
     *
     * This hash map is populated with mappings of compiler generated type names
     * to corresponding type names in QMetaType object.  For example GCC names
     * integers as just "i" while Qt requires "int" as the type name. This map
     * provides a convenient mechanism to rapidly map various type names to a
     * consistent set of values.  The map is currently used only when
     * complex sub-elements are involved.
     */
    static QHash<QString, QString> TypeNameMap;

    /**
     * @brief TypeNameList A list of values for initialzing TypeNameMap.begin()
     *
     * This list is used as a backwards compatibility approach (in case the
     * C++ complier still does not support C++11 standard).  This list is
     * interpreted as {name, value} pairs (example: {"i", "int"}) with the
     * last entry of {NULL, NULL} used as a sentinel value indicating end of
     * list.  The XMLElementInfo default constructor (always called to
     * instantiate the Invalid constant) is used to sutiably populate the
     * TypeNameMap with pairs of values from this list (only when TypeNameMap
     * is empty and consequently is done only once upon startup).
     */
    static const char* TypeNameList[];

    /**
     * @brief XmlDateFormat The format string used to read and write date
     * objects (via QDate class) to corresponding XML format. This string
     * is set to "yyyy-MM-dd" consistent with the W3C XML standard of simple
     * date definition. If time zones and offets are needed then prefer to
     * use DateTime format instead.
     */
    static const QString XmlDateFormat;

    /**
     * @brief XmlTimeFormat The format string used to read and write time
     * objects (via QTime class) to corresponding XML format. This string
     * is set to "hh:mm:ss.z" consistent with the W3C XML standard.
     */
    static const QString XmlTimeFormat;

    /**
     * @brief XmlDateTimeFormat The format string used to read and write date-time
     * objects (via QDateTime class) to corresponding XML format. This string
     * is set to "yyyy-MM-ddThh:mm:ss" consistent with the W3C XML standard.
     */
    static const QString XmlDateTimeFormat;
};

#endif // XML_ELEMENT_INFO_H
