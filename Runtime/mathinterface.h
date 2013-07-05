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
	double distance(double x1, double y1, double x2, double y2);

	template <typename T>
	T square(T a) {
		return a * a;
	}

	template <typename T>
	T max(T a, T b) {
		return a > b ? a : b;
	}

	template <typename T>
	T min(T a, T b) {
		return a < b ? a : b;
	}
}
#endif // MATH_H
