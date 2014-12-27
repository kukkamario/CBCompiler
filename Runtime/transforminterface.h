#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

namespace transforms {

	enum {
		Model = 0,
		View
	};

	void initTransforms();


	void setTransformMode();
	void updateInternal();


	void setIdentity();
	void buildTransform(float x, float y, float sx, float sy, float deg);

	void translate(float x, float y);
	void scale(float x, float y);
	void rotate(float deg);

	void convertCoordinate(float& x, float& y);

	void setTransformMode(int mode);

	void push();
	void pop();
}

#endif // TRANSFORM_HPP
