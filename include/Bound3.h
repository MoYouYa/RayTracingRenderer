#pragma once
#include "Vector.h"
#include "Ray.h"

class Bound3 {
public:
	Vector3f min, max;

	//get a error bound which's min bigger than max
	Bound3() { min = Vector3f(std::numeric_limits<float>::max()); max = Vector3f(std::numeric_limits<float>::lowest()); }
	
	Bound3(Vector3f _min, Vector3f _max) { min = _min; max = _max; }

	Bound3& merge(const Bound3& b) {
		Vector3f newMin = Vector3f(min.x < b.min.x ? min.x : b.min.x, min.y < b.min.y ? min.y : b.min.y, min.z < b.min.z ? min.z : b.min.z);
		Vector3f newMax = Vector3f(max.x > b.max.x ? max.x : b.max.x, max.y > b.max.y ? max.y : b.max.y, max.z > b.max.z ? max.z : b.max.z);
		min = newMin;
		max = newMax;
		return *this;
	}

	Vector3f getCenterPos() const{ return min * 0.5 + max * 0.5; }

	bool intersect(const Ray& ray)const;
};

