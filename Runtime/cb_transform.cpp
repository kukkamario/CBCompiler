
#include "transforminterface.h"

void CBF_ResetTransform() {
	transforms::setIdentity();
}

void CBF_TranslateTransform(float x, float y) {
	transforms::translate(x, y);
}

void CBF_ScaleTransform(float x, float y) {
	transforms::scale(x, y);
}

void CBF_RotateTransform(float deg) {
	transforms::rotate(deg);
}

void CBF_BuildTransform(float x, float y, float sx, float sy, float deg) {
	transforms::buildTransform(x, y, sx, sy, deg);
}

void CBF_PushTransform() {
	transforms::push();
}

void CBF_PopTransform() {
	transforms::pop();
}

void InvertTransform() {
	transforms::invert();
}


float CBF_TransformedX(float x, float y) {
	transforms::convertCoordinate(x, y);
	return x;
}

float CBF_TransformedY(float x, float y) {
	transforms::convertCoordinate(x, y);
	return y;
}

