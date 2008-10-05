#ifndef MUSE_COMMUNICATOR_CPP
#define MUSE_COMMUNICATOR_CPP

#include "Communicator.h"

using namespace muse;

Communicator::Communicator(){}

void Communicator::initialize(){}

void Communicator::sendEvent(Event *, SimulatorID *){}

void Communicator::sendMessage(char *, SimulatorID *){}

void Communicator::sendMessageToAll(char *){}

Communicator::~Communicator(){}

#endif
