#include "transforminterface.h"
#include <allegro5/allegro5.h>
#include <vector>
#include <mathinterface.h>

static std::vector<ALLEGRO_TRANSFORM> sTransforms;

void transforms::setIdentity() {
	ALLEGRO_TRANSFORM transf;
	al_identity_transform(&transf);
	al_copy_transform(&sTransforms.back(), &transf);
	al_use_transform(&sTransforms.back());
}


void transforms::initTransforms() {
	ALLEGRO_TRANSFORM transf;
	al_identity_transform(&transf);
	sTransforms.push_back(transf);
	al_use_transform(&sTransforms.back());
}



void transforms::buildTransform(float x, float y, float sx, float sy, float deg) {
	ALLEGRO_TRANSFORM transf;
	math::wrapAngle(deg);
	al_build_transform(&transf, x, y, sx, sy, math::toRad(deg));
	al_copy_transform(&sTransforms.back(), &transf);
	al_use_transform(&sTransforms.back());
}


void transforms::translate(float x, float y) {
	al_translate_transform(&sTransforms.back(), x, y);
	al_use_transform(&sTransforms.back());
}


void transforms::scale(float x, float y) {
	al_scale_transform(&sTransforms.back(), x, y);
	al_use_transform(&sTransforms.back());
}

void transforms::rotate(float deg) {
	math::wrapAngle(deg);
	al_rotate_transform(&sTransforms.back(), math::toRad(deg));
	al_use_transform(&sTransforms.back());
}


void transforms::push() {
	sTransforms.push_back(sTransforms.back());
	al_use_transform(&sTransforms.back());
}


void transforms::pop() {
	sTransforms.pop_back();
	al_use_transform(&sTransforms.back());
}



void transforms::convertCoordinate(float &x, float &y) {
	al_transform_coordinates(al_get_current_transform(), &x, &y);

}


void transforms::invert() {
	al_invert_transform(&sTransforms.back());
	al_use_transform(&sTransforms.back());
}
