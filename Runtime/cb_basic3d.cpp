#include "basic3d.h"
using namespace basic3d;


void CBF_begin3d() {
	begin3d();
}

void CBF_end3d() {
	end3d();
}

void CBF_setupViewport(int x, int y, int w, int h) {
	setupViewport(x, y, w, h);
}

void CBF_setupPerspective(float fov, float nearV, float farV) {
	setupPerspective(fov, nearV, farV);
}

void CBF_camera3d(float camX, float camY, float camZ, float targetX, float targetY, float targetZ, float upX, float upY, float upZ) {
	camera3d(camX, camY, camZ, targetX, targetY, targetZ, upX, upY, upZ);
}

void CBF_drawBasic3d(int type, CBArrayData<float, 1> vertices) {
	drawBasic3d(type, vertices);
}

void CBF_drawBasic3d(int type, CBArrayData<float, 1> vertices, CBArrayData<float, 1> colors) {
	drawBasic3d(type, vertices, colors);
}
