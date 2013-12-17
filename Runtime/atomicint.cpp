#include "atomicint.h"

#ifdef USE_BOOST_ATOMIC
void atomicIncrease(AtomicInt &i) {
	i.fetch_add(1u, boost::memory_order_relaxed);
}


bool atomicDecrease(AtomicInt &i) {
	return i.fetch_sub(1, boost::memory_order_release) == 1;
}


void atomicThreadFenceAcquire() {
	boost::atomic_thread_fence(boost::memory_order_acquire);
}

int atomicLoad(AtomicInt &i) {
	return i.load(boost::memory_order_release);
}
#else
void atomicIncrease(AtomicInt &i) {
	++i;
}


bool atomicDecrease(AtomicInt &i) {
	return --i == 0;
}


void atomicThreadFenceAcquire() {
}


int atomicLoad(AtomicInt &i) {
	return i;
}

#endif
