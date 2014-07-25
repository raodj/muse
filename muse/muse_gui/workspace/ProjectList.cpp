#ifndef PROJECT_LIST_CPP
#define PROJECT_LIST_CPP

#include "ProjectList.h"

ProjectList::ProjectList() : XMLElement("ProjectList") {
    addElement(XMLElementInfo("Project", &projectList));
}

void
ProjectList::addProject(Project &entry) {
    Project* project = new Project(entry.getName(), entry.getMakeFilePath(),
                                   entry.getExecutablePath(),
                                   entry.getOutputDirPath(), entry.getJobCount());
    projectList.append(project);
}

#endif
