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
	Object* obj;
	Vector3f pos;
	Vector3f normal;
	Vector3f emission;
	float pdf;
	
	LightPoint() { obj = nullptr; pdf = 0; }
	
	LightPoint(Object* _obj,const Vector3f& _pos,const Vector3f& _normal,const Vector3f& _emission,const float& _pdf) {
		obj = _obj;
		pos = _pos;
		normal = _normal;
		emission = _emission;
		pdf = _pdf;
	}
};

struct CameraPoint {
	Object* obj;
	Vector3f pos;
	Vector3f normal;
	Vector3f kd;
	float pdf;
	
	CameraPoint() { obj = nullptr; pdf = 0; }
	
	CameraPoint(Object* _obj, const Vector3f& _pos, const Vector3f& _normal, const Vector3f& _kd, const float& _pdf) { 
		obj = _obj;
		pos = _pos;
		normal = _normal;
		kd = _kd;
		pdf = _pdf;
	}
};
