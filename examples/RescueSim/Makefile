CXXC=mpicxx

CXXFLAGS=-I../../include -I./agents/include -I./events/include -I./states/include -L../../kernel -lmuse -lstdc++

EXEC=exec_RescueSim

SOURCES=RescueSim_main.cpp agents/src/Volunteer.cpp agents/src/RescueArea.cpp agents/src/Victim.cpp agents/src/Medic.cpp tools/RescueSimOutputHelper.cpp tools/arg_parser.cpp states/src/RescueAreaState.cpp states/src/VictimState.cpp states/src/MedicState.cpp states/src/VolunteerState.cpp events/src/UpdatePosition.cpp events/src/CallMedic.cpp events/src/PlayerEvent.cpp events/src/UpdateNearby.cpp events/src/VictimFound.cpp 

all:
	$(CXXC) $(CXXFLAGS) $(SOURCES) -o $(EXEC)

clean:
	rm -rf *o $(EXEC)
