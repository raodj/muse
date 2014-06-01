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

XML marshalling & unmarshalling infrastructure:
-----------------------------------------------

This directory contains a custom XML marshalling and unmarshalling
infrastructure.  Unarshalling is the process of validating and reading
an XML file into an in-memory object hierarchy that eases processing
the XML data.  Conversely, marshalling is the process of generating a
valid XML from an in-memory object hierarchy.  This framework utilizes
the features of Qt's `xmlpatterns` library.  Consequently, ensure that
the project (.pro) file include the library via the directive:

    QT += core xmlpatterns

In MUSE-GUI, marhsalling and unmarshalling is used to load and save
workspace information.  A workspace is essentially a collection of
related objects that maintain information about various artifacts in
the workspace such as: servers, jobs, and projects.  The XML
processing framework consists of the following 4 core C++ classes:

* XMLElementInfo: A convenience class to encapsulate metadata (such
    as: XML element name, data type, etc.) about various sub-elements
    in an XML element to ease marshaling and unmarshaling. Refer to
    the documentation in XMLElementInfo.h for details on the types of
    XML entities (and their mapping to C++ types) handled by this
    class.

* XMLElement: This class represents a complex type element in an XML
    file and form the core API class for marshalling and unmarshalling
    the element.  This class must be the base class of all elements in
    an XML file (except for the root element for which use
    XMLRootElement class as the parent instead of this class).  Refer
    to the example code in XMLParser class on how to use this class.

* XMLRootElement: The XMLRootElement class that represents the root
    element in an XML document.  Currently, the framework assumes that
    each XML document has only a single root element.  This class
    extends the XMLElement (parent) class and adds namespacing and
    attributes features just for the root element.

* XMLParser: The top-level class that facilitates validation, reading,
  and writing/reading of objects to/from XML files.  This class serves
  as the top-level API for validation, marshaling and unmarshalling
  XML content to-and-from in memory class hierarchies (that ease
  processing). The class is essentially a thin wrapper around the
  QXmlStreamWriter, QXmlStreamReader, and associated classes from Qt's
  xmlpatterns library.

EXAMPLE USAGE
-------------

Source code for reading and writing hirearchy with 2 objects:
--------------------------------------------------------------

A simple operational example for using the XML framework is shown
below (and the whole source can be in a single file):

#include "XMLElement.h"
#include "XMLParser.h"

class Address : public XMLElement {
public:
    Address() {
        // Register elements in the order in which they shlould occurr.
        addElement(XMLElementInfo("number", &number));
        addElement(XMLElementInfo("street", &street));
        addElement(XMLElementInfo("city",   &city));
        addElement(XMLElementInfo("state",  &state));
        addElement(XMLElementInfo("zip",    &zip));
    }
    
    ~Address() {}
    
protected:
    int number;
    QString street;
    QString city;
    QString state;
    int zip;
};

class Person : public XMLElement {
public:
    Person() : XMLElement("Person") {
        qRegisterMetaType<Person>("Person");
        addElement(XMLElementInfo("name",    &name));
        addElement(XMLElementInfo("email",   &emails));
        addElement(XMLElementInfo("Address", &addresses));
    }
    
    ~Person() {}
    
protected:
    QString name;
    QStringList emails;
    QList<XMLElement*> addresses;
};

Q_DECLARE_METATYPE(Person)
Q_DECLARE_METATYPE(Address)

void readXML(const QString& xmlFileName, const QString& schemaFileName) {
    QScopedPointer<Person> person(new Person()); // root element
    XMLParser xmlParser;  // The parser
    QString errMsg = "";  // string to track error messages

    if ((errMsg = loadXML("info.xml", "Person.xsd", *person)) == "") {
        // XML data loaded successfully. Write the data to a copy file.
        errMsg = saveXML("copy.xml", person);
    }
    if (errMsg != "") {
        std::cerr << "Error reading or writing: " << errMsg;
    }
}

Person.xsd: The XML schema file being used in the above code
---------------------------------------------------------------

<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
        targetNamespace="http://pc2lab.cec.miamiOH.edu/"
        xmlns="http://pc2lab.cec.miamiOH.edu/" elementFormDefault="qualified">

  <!-- Datatype for the version ID used further below in this schema  -->
  <xsd:simpleType name="VersionID">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="1.0" />
    </xsd:restriction>
  </xsd:simpleType>

  <xsd:element name="Address">
    <xsd:complexType>
      <xsd:sequence>
        <xsd:element name="number" type="xsd:integer"/>
        <xsd:element name="street" type="xsd:string" />
        <xsd:element name="city"   type="xsd:string" />
        <xsd:element name="state"  type="xsd:string" />
        <xsd:element name="zip"    type="xsd:integer"/>
      </xsd:sequence>
    </xsd:complexType>  
  </xsd:element>
  
  <xsd:element name="Person">
    <xsd:complexType>
      <xsd:sequence>
        <xsd:element name="firstName" type="xsd:string" />
        <xsd:element name="lastName"  type="xsd:string" />
        <xsd:element name="ssn"       type="xsd:string" />
        <xsd:element name="date"      type="xsd:dateTime" />
        <xsd:element name="email"     type="xsd:string" minOccurs="0" maxOccurs="unbounded"/>
        <xsd:element ref="Address"  minOccurs="0" maxOccurs="unbounded"/>
      </xsd:sequence>
      <xsd:attribute name="Version" type="VersionID" use="required" />
    </xsd:complexType>
  </xsd:element>
  
</xsd:schema>


info.xml: The XML data file being processed in the above code
---------------------------------------------------------------

<?xml version="1.0"?>
<Person xmlns="http://pc2lab.cec.miamiOH.edu/"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xsi:schemaLocation="http://pc2lab.cec.miamiOH.edu/ Person.xsd" 
	Version="1.0">
    <firstName>Test</firstName>
    <lastName>Ing</lastName>
    <ssn>12345</ssn>
    <date>2002-05-30T09:00:00.0</date> 
    <email>test@gmail.com</email>
    <email>test@yahoo.com</email>
    <email>test@hotmail.com</email>
    <Address>
        <number>1600</number>
        <street>Pennsylvania Ave</street>
        <city>Washington</city>
        <state>DC</state>
        <zip>20500</zip>
    </Address>
    <Address>
        <number>205</number>
        <street>510 E. High Street</street>
        <city>Oxford</city>
        <state>Ohio</state>
        <zip>45056</zip>
    </Address>
</Person>
