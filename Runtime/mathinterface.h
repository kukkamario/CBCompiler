#ifndef MATH_H
#define MATH_H
#include <cmath>
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

namespace math {
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

	int rand(int max);
	int rand(int min, int max);
	double rnd();
	double rnd(double max);
	double rnd(double min, double max);
	void randomize(int seed);
	double wrapAngle(double a);
}
#endif // MATH_H
