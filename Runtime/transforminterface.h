#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

namespace transforms {

	void initTransforms();

	void setIdentity();
	void buildTransform(float x, float y, float sx, float sy, float deg);

	void translate(float x, float y);
	void scale(float x, float y);
	void rotate(float deg);
	void invert();

	void convertCoordinate(float& x, float& y);


	void push();
	void pop();
}

#endif // TRANSFORM_HPP
