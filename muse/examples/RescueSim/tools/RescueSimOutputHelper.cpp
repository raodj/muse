#include "RescueSimOutputHelper.h"

RescueSimOutputHelper::RescueSimOutputHelper() {
	current = 0;
	specifiedSizes = false;
}

RescueSimOutputHelper::RescueSimOutputHelper(int numVol, int numMed, int numVic) {
	current = 0;
	specifiedSizes = true;
	agent temp;
	temp.xpos = 0.0;
	temp.ypos = 0.0;
	int maxNum = std::max(std::max(numVol, numMed), numVic);
	for(int i = 1; i <= maxNum; i++) {
		temp.ID = i;
		if(i <= numVol) {
			vols.push_back(temp);
			temp.type = volunteer;
		}
		if(i <= numMed) {
			meds.push_back(temp);
			temp.type = medic;
		}
		if(i <= numVic) {
			vics.push_back(temp);
			temp.type = victim;
		}
	}
}

std::vector<agent> RescueSimOutputHelper::getNearby(int a_id, agentType a_type) {
	int index = findAgent(a_id, a_type);
	agent temp;
	std::vector<agent> output;
	if(index >= 0) {
		switch(a_type) {
		case volunteer:
			temp = vols[index];
			std::copy_if(vols.begin(), vols.end(), output.begin(), [temp](agent a){ return ((a.ID != temp.ID) && withinRange(temp, a)); });
			std::copy_if(meds.begin(), meds.end(), output.end(), [temp](agent a){ return withinRange(temp, a); });
			std::copy_if(vics.begin(), vics.end(), output.end(), [temp](agent a){ return withinRange(temp, a); });
			vols[index].nearby = output;
			break;
		case medic:
			temp = vols[index];
			std::copy(vols.begin(), vols.end(), output.begin());
			std::copy_if(meds.begin(), meds.end(), output.end(), [a_id](agent a){ return (a.ID != a_id); });
			std::copy(vics.begin(), vics.end(), output.end());
			meds[index].nearby = output;
			break;
		case victim:
			temp = vols[index];
			std::copy(vols.begin(), vols.end(), output.begin());
			std::copy(meds.begin(), meds.end(), output.end());
			std::copy_if(vics.begin(), vics.end(), output.end(), [a_id](agent a){ return (a.ID != a_id); });
			vics[index].nearby = output;
			break;
		default: break;
		}
	}
	return output;
}

bool RescueSimOutputHelper::updateAgent(agent agent_p) {
	int index = findAgent(agent_p);
	if(index >= 0) {
		switch(agent_p.type) {
		case volunteer:
			vols[index] = agent_p; break;
		case medic:
			meds[index] = agent_p; break;
		case victim:
			vics[index] = agent_p; break;
		default: return false;
		}
	}
	else if(!specifiedSizes) {
		switch(agent_p.type) {
		case volunteer:
			vols.push_back(agent_p); break;
		case medic:
			meds.push_back(agent_p); break;
		case victim:
			vics.push_back(agent_p); break;
		default: return false;
		}
	}
	else return false;
	return true;
}

bool RescueSimOutputHelper::createTextOutput(int time) {
	std::stringstream ss;
	ss << "/textOutput/Snapshot" << current << ".txt";
	std::ofstream os (ss.str());
	os << time << "\n";
	std::copy(vols.begin(), vols.end(), std::ostream_iterator<agent>(os, "\n"));
	std::copy(meds.begin(), meds.end(), std::ostream_iterator<agent>(os, "\n"));
	std::copy(vics.begin(), vics.end(), std::ostream_iterator<agent>(os, "\n"));
	current++;
	return true;
}

bool RescueSimOutputHelper::createGraphicalOutput() { //TO DO
	for(int i = 0; i < current; i++) {
		std::stringstream ss;
		ss << "/textOutput/Snapshot" << i << ".txt";
		std::ifstream fs (ss.str());

	}
	return true;
}

int RescueSimOutputHelper::findAgent(agent A) {
	return findAgent(A.ID, A.type);
}

int RescueSimOutputHelper::findAgent(int id, agentType type) {
	switch(type) {
	case volunteer:
		for(int i = 0; i < vols.size(); i++)
			if(vols[i].ID == id) return i;
	case medic:
		for(int i = 0; i < meds.size(); i++)
			if(meds[i].ID == id) return i;
	case victim:
		for(int i = 0; i < vics.size(); i++)
			if(vics[i].ID == id) return i;
	default: return -1;
	}
	return -1;
}

bool RescueSimOutputHelper::withinRange(agent a1, agent a2) {
	return (std::sqrt(std::pow((a2.xpos-a1.xpos), 2) + std::pow((a2.ypos-a1.ypos), 2)) <= SIGNAL_RADIUS);
}

bool RescueSimOutputHelper::declareVictimSaved(int i) {
	int index = findAgent(i, victim);
	vics[index].xpos = 0.0;
	vics[index].ypos = 0.0;
	return true;
}
