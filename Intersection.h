#pragma once
#include "Vector.h"

class Object;

struct Intersection {
	bool hit;
	float distance;
	Vector3f postion;
	Vector3f normal;
	Vector2f textureCoord;
	Object* object;
	Intersection() { 
		hit = false;
		distance = 0;
		object = nullptr; 
	}
};

struct LightPoint {
	Intersection inter;
	Vector3f emission;
	LightPoint() {}
};