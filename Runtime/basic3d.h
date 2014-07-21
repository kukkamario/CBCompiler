#ifndef BASIC3D_H
#define BASIC3D_H
#include "cbarray.h"
namespace basic3d {
	void begin3d();
	void end3d();
	void setupViewport(int x, int y, int w, int h);
	void setupPerspective(float fov, float nearV, float farV);
	void camera3d(float camX, float camY, float camZ, float targetX, float targetY, float targetZ, float upX, float upY, float upZ);
	void updateProjectionMatrix();
	void updateViewMatrix();
	void drawBasic3d(int type, Array<float, 1> vertices);
	void drawBasic3d(int type, Array<float, 1> vertices, Array<float, 1> colors);
}

#endif // BASIC3D_H
