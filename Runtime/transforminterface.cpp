#include "transforminterface.h"
#include <allegro5/allegro5.h>
#include <vector>
#include <mathinterface.h>

static std::vector<ALLEGRO_TRANSFORM> sModelTransforms;
static std::vector<ALLEGRO_TRANSFORM> sViewTransforms;
static int sMode = transforms::View;

void transforms::setIdentity() {
	ALLEGRO_TRANSFORM transf;
	al_identity_transform(&transf);
	switch(sMode) {
		case Model:
			al_copy_transform(&sModelTransforms.back(), &transf);
		break;
		case View:
			al_copy_transform(&sViewTransforms.back(), &transf);
		break;
	}
	updateInternal();
}


void transforms::initTransforms() {
	ALLEGRO_TRANSFORM transf;
	al_identity_transform(&transf);
	sModelTransforms.push_back(transf);
	sViewTransforms.push_back(transf);
	updateInternal();
}



void transforms::buildTransform(float x, float y, float sx, float sy, float deg) {
	ALLEGRO_TRANSFORM transf;
	math::wrapAngle(deg);
	switch(sMode) {
		case Model:
			al_build_transform(&transf, x, y, sx, sy, math::toRad(deg));
			al_copy_transform(&sModelTransforms.back(), &transf);
		break;
		case View:
			al_build_transform(&transf, -x, y, sx, sy, -math::toRad(deg));
			al_copy_transform(&sViewTransforms.back(), &transf);
		break;
	}
	updateInternal();
}


void transforms::translate(float x, float y) {
	switch(sMode) {
		case View:
			al_translate_transform(&sViewTransforms.back(), -x, -y);
		break;
		case Model:
			al_translate_transform(&sModelTransforms.back(), x, y);
		break;
	}
	updateInternal();
}


void transforms::scale(float x, float y) {
	switch(sMode) {
		case View:
			al_scale_transform(&sModelTransforms.back(), x, y);
		break;
		case Model:
			al_scale_transform(&sViewTransforms.back(), x, y);
		break;
	}
	updateInternal();
}

void transforms::rotate(float deg) {
	math::wrapAngle(deg);
	switch(sMode) {
		case View:
			al_rotate_transform(&sViewTransforms.back(), -math::toRad(deg));
		break;
		case Model:
			al_rotate_transform(&sModelTransforms.back(), math::toRad(deg));
		break;
	}
	updateInternal();
}


void transforms::push() {
	switch(sMode) {
		case Model: {
			sModelTransforms.push_back(sModelTransforms.back());
		} break;
		case View: {
			sViewTransforms.push_back(sViewTransforms.back());
		} break;
	}
	updateInternal();
}


void transforms::pop() {
	switch(sMode) {
		case View:
			if(sViewTransforms.size() > 1) {
				sViewTransforms.pop_back();
			}
		break;
		case Model:
			if(sModelTransforms.size() > 1) {
				sModelTransforms.pop_back();
			}
		break;
	}
	updateInternal();
}


void transforms::updateInternal() {
	ALLEGRO_TRANSFORM comb;
	al_identity_transform(&comb);
	al_compose_transform(&comb, &sModelTransforms.back());
	al_compose_transform(&comb, &sViewTransforms.back());
	al_use_transform(&comb);
}




void transforms::setTransformMode(int mode) {
	sMode = mode;
}


void transforms::convertCoordinate(float &x, float &y) {
	al_transform_coordinates(al_get_current_transform(), &x, &y);

}
