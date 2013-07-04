#include "mathinterface.h"
#include "random"
int math::rand(int max) {
	return rand(0, max);
}


int math::rand(int min, int max) {
	return min + ::rand() % (max - min);
}


void math::randomize(int seed){
	::srand(seed);
}


double math::wrapAngle(double a) {
	a = fmod(a, 360.0);
	if (a < 0)
		a += 360;
	return a;
}


double math::rnd() {
	return ::rand() / (double)RAND_MAX;
}

double math::rnd(double max) {
	return rnd(0.0, max);
}

double math::rnd(double min, double max) {
	return min + rnd() * (max - min);
}
