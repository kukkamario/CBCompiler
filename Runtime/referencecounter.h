#ifndef REFERENCECOUNTER_H
#define REFERENCECOUNTER_H
extern "C" bool atomic_increase(volatile int *val);
extern "C" bool atomic_decrease(volatile int *val);
class ReferenceCounter {
	public:
		ReferenceCounter(): mCounter(0) {}
		ReferenceCounter(int v): mCounter(1) {}

		/** Returns true if value is non-zero */
		bool increase();

		/** Returns true if value is non-zero */
		bool decrease();

		operator int();
	private:
		volatile int mCounter;
};

#endif // REFERENCECOUNTER_H
