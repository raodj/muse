#ifndef BUG_H
#define	BUG_H

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

#include "Agent.h"
#include "State.h"
#include "BugDataTypes.h"
#include "BugEvent.h"
#include "BugState.h"

/** The Bug agent.

    This agent class models the following life cycle activites of a
    Bug in the simulation:

    <ul>

    <li>\b inception: At the time of initialization, the bug initially
    randomly chooses and requests to move into a specific Eco area in
    the simulation by scheduling a MoveIn event.</li>

    <li>\b MoveIn: The bug schedules MoveIn events to an Eco area to
    request to move into that Eco area.  The MoveIn event is scheduled
    either at inception or after a "MoveOut" (a scenario when
    food/resources in the Eco area have been exhausted).  The bug
    cannot move into the Eco area until the Eco area acknowledges that
    the Bug can indeeded move in.  If an Eco area rejects the request
    for the bug to move in, then the Bug tries another random Eco
    area.  This operation is performed by initialize and executeMoveIn
    methods in this class.</li>
    
    <li>\b Eat: Once a bug moves into an Eco area it starts consuming
    resources in the area by scheduling Eat events to the Eco Area.
    The Eco area responds with amount of food consumable by the Bug.
    If the food consumed drops to zero, then the Bug scouts adjacent
    eco areas by scheduling Scout events to them.</li>

    <li>\b Move out: Once the food resources in the current Eco area
    have been exhausted (see \c executeEat method), the Bug schedules
    Scout events that are sent to exactly 4 adjacent Eco areas.  When
    responses from all 4 Eco areas is received (see executeScout
    method), the Bug schedules a MoveOut event to the Eco area that
    has maximum amount of food/resources.  The MoveOut event is
    processed by executeMoveOut method that schedules a Move event  </li>
    
    </ul>
*/
class Bug : public muse::Agent {
    friend std::ostream& ::operator<<(ostream&, const Bug&);
public:
    /** The constructor for the Bug agent.

	The constructor merely initializes various instance variables
	to their default initial value.  In addition, the agent also
	creates a default Bug agent state to reflect initial state of
	the agent.
	
	\param[in] agentID The unique ID value to be associated with
	this bug.

	\param[in,out] coordMap A map of Eco area agents corresponding
	to each logical grid space.  This data structure is
	essentially a read-only data structure and is shared (to
	reduce memory footprint) between all the Bug agents.  This
	class holds a reference to it for convenient corss reference.

	\param[in] col The maximum number of columns in the local Eco
	area being simulated.

	\param[in] row The maximum number of rows in the logical Eco
	area being simulated.
    */
    Bug(muse::AgentID agentID, CoordAgentIDMap& coordMap, int col, int row);

    /** The initialization MUSE API method that initiates this agent's
	life cycle.

	This method schedules an initial MoveIn event for the Bug to
	randomly move into a specific space.  This method uses the
	scheduleMoveIn() method to perform the actual task.
    */
    void initialize() throw (std::exception);

    /** Muse API method to process events scheduled for this agent.

	This method is periodically invoked by the underlying MUSE
	kernel whenever this agent has events to be processed.  This
	method essentially acts as a delegator and invokes various
	helper methods in this class (such as: executeMoveIn(),
	executeMoveOut(), etc.) to actually process the events
	scheduled for this agent.

	\param[in] events The list of events to be processed by this
	agent.
    */
    void executeTask(const EventContainer& events);

    /** The MUSE API method invoked at the end of the simulation.

	This method is the symmetric dual of the intialize method that
	is invoked once at the end of the simulation. Currently, this
	method does not have any specific operations to perform and is
	merely present as a place holder for future extensions.
    */
    void finalize();

protected:
    /** A convenience method to generate a random value such that it
	does not exceed a given maximum value.

	\param[in] max The maximum value for limiting the range of the
	random number generated by this method.

	\return A random number \i rnd (0 < \i rnd < max).
    */
    int getRandom(const int max) const;

    /** A refactored utility method to schedule MoveIn events.

	This is a utility method that randomly selects a row and
	column (using rows and cols instance variable) and schedules a
	MoveIn event to the Space agent associated with that logical
	location.
	
	\param[in] timeFactor A scale or time factor that is
	multiplied with a random number to determine future time when
	the MoveIn event is to be scheduled.
    */
    void scheduleMoveInEvent(const int timeFactor);

    /** Helper method to process MoveIn events.

	This method is invoked from the executeTaks() method to
	process MoveIn events.  The MoveIn event is received from a
	Space agent (to which the bug scheduled a MoveIn event
	earlier).  This method performs the following two broad sets
	operations:

	<ol>

	<li>If the MoveIn event indicates that the Bug can move into
	an area, then it updates its state with the location and moves
	schedules an Eat event back to the Space agent.</li>

	<li>If the Space agent rejects the Bug then this method
	schedules a MoveIn event to a randomly chosen Space agent by
	calling the scheduleMoveInEvent() method. </li>

	</ol>

	\param[in] event The MoveIn event to be processed by this
	method.

	\param[in] state The current state of the Bug to be updated
	and used for various operations.
    */
    void executeMoveIn(BugEvent* event, BugState& state);

    /** Helper method to process MoveOut events.

	This method is relatively straightforward and simply schedules
	a MoveIn event to the adjacent Space scouted by the Bug.  The
	adjacent space to move to is obtained from
	BugState::getScoutSpace() method.

	\param[in] event The MoveOut event.  This event is not really
	used by the method.

	\param[in] state The current state of the Bug to be updated
	and used for various operations.
    */
    void executeMoveOut(BugEvent* event, BugState& state);

    /** Helper method to process Grow events (scheduled after the Bug
	eat's some food).

	This method is relatively straightforward and simply updates
	teh size of the bug in the state.

	\param[in] event The Grow event to processed.  The event
	contains the size by which the bug must logically grow.

	\param[in] state The current state of the Bug to be updated
	and used for various operations.
    */    
    void executeGrow(BugEvent* event, BugState& state);

    /** Helper method to process Eat events.

	This method is invoked from the executeTaks() method to
	process Eat events.  The Eat event is received from a Space
	agent (to which the bug scheduled an Eat event earlier).  This
	method performs the following two broad sets operations:

	<ol>

	<li>If the Eat event indicates that the Bug can move into an
	area, then it schedules a Grow event (to itself) and another
	Eat event back to the Space agent (to consume more
	food).</li>

	<li>If the Space agent indicated that no food is left, then
	the Bug schedules \i four (4) Scout events to adjacent Space
	locations to identify the most resource-rich space to move to
	next. </li>

	</ol>

	\param[in] event The Eat event to be processed by this method.
	
	\param[in] state The current state of the Bug to be updated
	and used for various operations.
    */    
    void executeEat(BugEvent* event, BugState& state);

    /** Handle scouting responses from Space agent.

	This method is used to process Scout events that are generated
	by Space agents in response to Scout requests dispatched by
	this Bug earlier in the simulation.  This method tracks the
	Space with maximum food reported thusfar.  Once scout
	responses from the \i four adjacent spaces have been received,
	this method schedules a MoveOut event (to move out of the
	current space and move into the most food-rich space).

	\param[in] event The Scout even to be handled by this method.

	\param[in] state The current state of the Bug to be updated
	and used for various operations.
    */
    void executeScout(BugEvent* event, BugState& state);

private:
    /** A map of Space agent ID's correspnding to a given row and
	column of the Eco area used in the simulation.

	This hash map provides a convenient look up to determine the
	agent ID of the Space agent coresponding to a given row and
	column in the gridded Eco area in the simulation.
    */
    const CoordAgentIDMap& coord_map;

    /** The maximum number of columns in the gridded space area for
	used for simulation.
    */
    const int cols;

    /** The maximum number of rows in the gridded rectangular space
	area in the simulation.
    */
    const int rows;
    
    /** A temporary variable to hold the requested location for this
	Bug. Ideally this should be part of the MoveIn events so that
	this value is absolutely safe even across rollbacks.
    */
    coord my_location;
};

#endif
