#include "mathinterface.h"
#include <cstdlib>

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


double math::distance(double x1, double y1, double x2, double y2) {
	return sqrt(square(x1 - x2) + square(y1 - y2));
}


double math::getAngle(double x1, double y1, double x2, double y2) {
	return toDeg(M_PI - atan2(y1 - y2, x1 - x2));
}
