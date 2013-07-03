#include "common.h"
#include "mathinterface.h"

using namespace math;

CBEXPORT float CBF_sinF(float f) {
	return sin(toRad(f));
}

CBEXPORT float CBF_cosF(float f) {
	return sin(toRad(f));
}

CBEXPORT float CBF_tanF(float f) {
	return sin(toRad(f));
}

CBEXPORT float CBF_asinF(float f) {
	return toDeg(asin(f));
}

CBEXPORT float CBF_acosF(float f) {
	return toDeg(acos(f));
}

CBEXPORT float CBF_atanF(float f) {
	return toDeg(atan(f));
}

CBEXPORT float CBF_absF(float f) {
	return fabs(f);
}

CBEXPORT int CBF_absI(int i) {
	return abs(i);
}

CBEXPORT float CBF_wrapAngleF(float a) {
	return wrapAngle(a);
}

CBEXPORT float CBF_rndF(float max) {
	return rnd(max);
}

CBEXPORT float CBF_rndFF(float min, float max) {
	return rnd(min, max);
}

CBEXPORT int CBF_randI(int max) {
	return rand(max);
}

CBEXPORT int CBF_randII(int min, int max) {
	return rand(min, max);
}

CBEXPORT void randomizeI(int seed) {
	randomize(seed);
}
