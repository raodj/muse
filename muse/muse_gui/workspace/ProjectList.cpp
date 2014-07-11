#ifndef PROJECT_LIST_CPP
#define PROJECT_LIST_CPP

#include "ProjectList.h"

ProjectList::ProjectList() : XMLElement("ProjectList") {
    addElement(XMLElementInfo("Project", &projectList));
}

void
ProjectList::addProject(Project &entry) {
    projectList.append(reinterpret_cast<XMLElement*> (&entry));
}

#endif
