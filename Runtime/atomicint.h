#ifndef ATOMICINT_H
#define ATOMICINT_H

#ifdef USE_BOOST_ATOMIC
	#include <boost/atomic.hpp>
	typedef boost::atomic_int AtomicInt;
#else
	typedef int AtomicInt;
#endif
int atomicLoad(AtomicInt &i);
void atomicIncrease(AtomicInt &i);
bool atomicDecrease(AtomicInt &i);
void atomicThreadFenceAcquire();


#endif // ATOMICINT_H
