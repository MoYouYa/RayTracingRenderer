#pragma once
#include "Vector.h"

struct Ray{
	Vector3f startPos;
	Vector3f dir;
	
	Ray() = delete;
	
	Ray(Vector3f _startPos, Vector3f _dir) {
		startPos = _startPos;
		dir = _dir;
	}
};