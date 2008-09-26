class ClockAgent : public muse::Agent {

public:
	
	//quick ctor define
	ClockAgent(){
		time_zero = new Time(0);
		c_state = new ClockState(time_zero);
	}

	//quick dtor define
	~ClockAgent(){}

	//Do all things to init the agent for the simulation here.
	void initialize(){
		//For our example here we'll simply print that we are
		//initializing the agent
		std::cout << "ClockAgent has initialized!!" << std::endl;
	}

	//When there is an event to be processed by an agent
	//muse will call this function and pass in the event(s) via the
	//EventContainer (Please refer to the documentation for more info on EventContainer)
	void executeTask(const muse::EventContainer & events){
		//For our example, when we get an event to process
		//we'll simply print out that an hour of simulation time has elasped 
		//and then schedule an event to ourself to be sent an hour later.
		
	}//end executeTask

	//Do all things to finalize the agent for the simulation here.
	void finalize(){
		//For our example we will clean up all memory allocations and 
		//print that we are finalizing our agent.
		//delete(c_state); //despose our clock state
		//delete(time_zero); //depose our time object
		std::cout << "ClockAgent has finalized!!" << std::endl;
	}

private:
	//every agent must have a State to be modifed and archieved
	Time * time_zero;
	ClockState * c_state;
};