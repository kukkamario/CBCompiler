#include "basic3d.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "gfxinterface.h"
#include "error.h"
#include <cstdio>

namespace basic3d {

static bool s3dEnabled = false;
static float sFOV = 90, sNear = 0.01, sFar = 1000.0;
static int sViewportX = 0, sViewportY = 0, sViewportW = 400, sViewportH = 300;
static float sCamX = 0, sCamY = 0, sCamZ = -10, sTargetX = 0, sTargetY = 0, sTargetZ = 0, sUpX = 0, sUpY = 1, sUpZ = 0;


void begin3d() {
	if (s3dEnabled) {
		error("3d drawing mode already enabled");
	}
	glPushMatrix();
	glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_POLYGON_BIT | GL_POINT_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT | GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_VERTEX_ARRAY);
	updateProjectionMatrix();
	updateViewMatrix();
	glViewport(sViewportX, sViewportY, sViewportW, sViewportH);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	s3dEnabled  = true;
}

void end3d() {
	if (!s3dEnabled) {
		error("No matching begin3d call");
		return;
	}
	glPopAttrib();
	glPopMatrix();
	s3dEnabled = false;
}

void setupViewport(int x, int y, int w, int h) {
	sViewportX = x;
	sViewportY = y;
	sViewportW = w;
	sViewportH = h;
	if (s3dEnabled) {
		glViewport(x, y, w, h);
		updateProjectionMatrix();
	}
}

void setupPerspective(float fov, float nearV, float farV) {
	sFOV = fov;
	sNear = nearV;
	sFar = farV;
	if (s3dEnabled) {
		updateProjectionMatrix();
	}
}

void camera3d(float camX, float camY, float camZ, float targetX, float targetY, float targetZ, float upX, float upY, float upZ) {
	sCamX = camX;
	sCamY = camY;
	sCamZ = camZ;
	sTargetX = targetX;
	sTargetY = targetY;
	sTargetZ = targetZ;
	sUpX = upX;
	sUpY = upY;
	sUpZ = upZ;
	if (s3dEnabled) {
		updateViewMatrix();
	}
}


void updateViewMatrix() {
	glLoadIdentity();
	gluLookAt(sCamX, sCamY, sCamZ, sTargetX, sTargetY, sTargetZ, sUpX, sUpY, sUpZ);
}

void updateProjectionMatrix() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(sFOV, (double)sViewportH / sViewportW, sNear, sFar);
	glMatrixMode(GL_MODELVIEW);
}

void drawBasic3d(int type, Array<float, 1> vertices) {
	const ALLEGRO_COLOR &drawColor = gfx::drawColor();
	glColor4f(drawColor.r, drawColor.g, drawColor.b, drawColor.a);
	glVertexPointer(3, GL_FLOAT, 0, vertices.cbegin());
	switch (type) {
		case 0: //Point
			glDrawArrays(GL_POINTS, 0, vertices.elements()); break;
		case 1: //Line
			glDrawArrays(GL_LINES, 0, vertices.elements()); break;
		case 2: //Triangle
			glDrawArrays(GL_TRIANGLES, 0, vertices.elements()); break;
		case 3: //Quad
			glDrawArrays(GL_QUADS, 0, vertices.elements()); break;
		case 4: //Line strip
			glDrawArrays(GL_LINE_STRIP, 0, vertices.elements()); break;
	}
}

void drawBasic3d(int type, Array<float, 1> vertices, Array<float, 1> colors) {
	/*printf("Vertices: %i\n", vertices.elements());
	for (int i = 0; i < vertices.elements(); i += 3) {
		printf("\tVertex %f, %f, %f\n", vertices[i], vertices[i + 1], vertices[i + 2]);
	}*/
	glVertexPointer(3, GL_FLOAT, 0, vertices.cbegin());
	glEnable(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, colors.cbegin());
	switch (type) {
		case 0: //Point
			glDrawArrays(GL_POINTS, 0, vertices.elements()); break;
		case 1: //Line
			glDrawArrays(GL_LINES, 0, vertices.elements()); break;
		case 2: //Triangle
			glDrawArrays(GL_TRIANGLES, 0, vertices.elements()); break;
		case 3: //Quad
			glDrawArrays(GL_QUADS, 0, vertices.elements()); break;
		case 4: //Line strip
			glDrawArrays(GL_LINE_STRIP, 0, vertices.elements()); break;
	}
	glDisable(GL_COLOR_ARRAY);
}



}
