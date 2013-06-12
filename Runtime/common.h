#ifndef COMMON_H
#define COMMON_H

#include <cmath>
#include <stdlib.h>

#define CBEXPORT extern "C"
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif
inline float toRad(float f) {
	return f * M_PI / 180.0;
}

inline double toRad(double f) {
	return f * M_PI / 180.0;
}

inline float toDeg(float f) {
	return f * 180.0 / M_PI;
}

inline double toDeg(double f) {
	return f * 180.0 / M_PI;
}

#endif // COMMON_H
