#include "common.h"


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
	a = fmod(a,360.0);
	if (a < 0)
		a += 360;
	return a;
}
