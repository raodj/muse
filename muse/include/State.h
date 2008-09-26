#ifndef _MUSE_STATE_H_
#define _MUSE_STATE_H_

//---------------------------------------------------------------------------
// USE AS IS, muse will not be responsible if anything breaks, while using muse.
// Authors: Meseret Gebre       gebremr@muohio.edu
//
//---------------------------------------------------------------------------

/** The muse namespace.
 * Everything in the api is in the muse namespace.
 *
 */
namespace muse { //begin namespace declaration 

/** The State class.
 * This class is a base class and SHOULD be used as a superclass. All agents must have a State. 
 * Think of the State of all the information that can be changed within an agent. When an agent
 * has to process a collection of events, what actually happens is information in the state is changed
 * based on the event being processed. 
 */
class State {
public:
        /** the getClone method.
         * This method must be implamented by the client. This is because there is no way 
         * muse can know what information the client has in the State class. 
         *
         * @return State* a pointer to the clone of this State.
         *@todo figure out how we will dispose of the cloned State.
         *
         */
	virtual State * getClone(); //must override this.
        
        /**The ctor method.
         *
         */
	State();
        /**The dtor method.
         * Cleans up all mess created by muse.
         *
         */
	virtual ~State();
};

}//end namespace declaration
#endif