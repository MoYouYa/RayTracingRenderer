#pragma once
#include "Object.h"

struct Photon {
	Object* obj;
	Vector3f pos;
	Vector3f power;
	Vector3f wi;
};