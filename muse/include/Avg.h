#ifndef MUSE_AVG_H
#define MUSE_AVG_H

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include <iostream>
#include "Utilities.h"

BEGIN_NAMESPACE(muse);

/** Helper class to streamline the process of tracking average statistics.

    This is a convenience class that that has been developed to
    streamline the process of collecting average values of runtime
    statistics.
*/
class Avg {
    /** Standard stream insertion operator to print Avg object.
        
        \param[out] os The output stream to where the value is to be
        written.
        
        \param[in] ag The Avg object to be printed.
    */
    friend std::ostream& operator<<(std::ostream& os, const Avg& ag) {
        return (os << ag.sum << " / " << ag.samples << " = " << ag.mean);
    }
public:
    /** Constructor to create a default Avg object.

        The constructor merrely initializes the instance variables
        with default initial values.
        
        \param[in] mean Initial mean value (if any).  Default is
        assumed to be zero.

        \param[in] samples The number of samples used to compute the
        initial mean.  The default is assumed to be zero.
     */
    Avg(double mean = 0.0, long samples = 0L) : mean(mean), samples(samples) {
        sum = mean * samples;
    }

    /** A default (dummy destructor.

        This class is simple/lightweight and consequently the
        destructor does not any work to do.
    */
    ~Avg() {}

    /** Add a value/sample to this running average object.

        \param[in] value The sample to be added to this
        running-average tracking object.
     */
    void add(const double value) {
        sum += value;
        mean = ((mean * samples) + value) / (samples + 1);
        samples++;
    }

    /** Add 1 as the sample to the value tracked by this object.  The
        operation performed by this class is the same as calling
        add(1.0).

        \return This method returns a reference to this object as per
        API requirements.
    */        
    Avg& operator++() {
        add(1.0);
        return *this;
    }

    /** Add a given value as the sample to the value tracked by this
        object.  The operation performed by this class is the same as
        calling add(value).

        \return This method returns a reference to this object as per
        API requirements.
    */    
    Avg& operator+=(const double value) {
        add(value);
        return *this;
    }

protected:
    // Currently this class does not have any protected members.

private:
    /** The running mean tracked by this object.  The running mean is
        used to provide a better mean to minimize
        rounding/floating-point errors.
    */
    double mean;

    /** The sum of all samples added to this object. The value is not
        very reliable for large numbers due to rounding issues with
        floating-point numbers.
    */
    double sum;

    /** The number of samples added to this object thus far. */
    long   samples;
};

END_NAMESPACE(muse);

#endif
