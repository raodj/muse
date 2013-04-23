#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <math.h>

const float SIGNAL_RADIUS = 10.0;

enum agentType {
	volunteer,
	medic,
	victim,
	agentType_END
};

struct agent {
	int ID;
	agentType type;
	double xpos;
	double ypos;
	std::vector<agent> nearby;
};

std::ostream& operator<<(std::ostream& os, const agent& ag) {
	return (os << ag.ID << " " << ag.type << " " << ag.xpos << " " << ag.ypos);
}

std::istream& operator>>(std::istream& is, agent& ag) {
	is >> ag.ID;
	int ty = 0;
	is >> ty;
	ag.ID = ty;
	is >> ag.xpos >> ag.ypos;
	return is;
}

class RescueSimOutputHelper {
private:
	int current;
	bool specifiedSizes;
	std::vector<agent> vols;
	std::vector<agent> meds;
	std::vector<agent> vics;
	int findAgent(agent A);
	int findAgent(int id, agentType type);
	bool withinRange(agent a1, agent a2);
	bool copy_ifAgent(agent a, agent b);
	std::vector<agent>::iterator copy_if (std::vector<agent>::iterator first, 
		std::vector<agent>::iterator last, std::vector<agent>::iterator result, agent a1);
public:
	RescueSimOutputHelper();
	RescueSimOutputHelper(int numVol, int numMed, int numVic);
	std::vector<agent> getNearby(int a_id, agentType a_type);
	bool updateAgent(agent agent_p);
	bool createTextOutput(int time);
	bool createGraphicalOutput();
	bool declareVictimSaved(int i);
};