#ifndef _MUSE_TIME_H_
#define _MUSE_TIME_H_

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

		/** The Enum TimeType.
         * When retrieveing the simulation time you must pass in a TimeType, by default TimeType = NOW.
         * @see getSimulationTime()
         */
        enum TimeType {START_TIME, NOW, END_TIME };

		//EPOCH is the starting time 
		const unsigned int EPOCH = 0;
/** The Time class.
 * This class SHOULD NOT be derived from. This is used to describe time in muse.
 *@todo Note sure if I even need to represent with a class. Maybe struct will do just fine.
 *
 */
class Time {
public:
    
        /**The ctor method.
         *
         */
	Time(int t);
        
        /**The dtor method.
         * Cleans up all mess created by muse.
         *
         */
	~Time();

	//for now we'll use an int, but soon this will use templates!!!
	unsigned int time;
	
};

} //end namespace declaration 
#endif