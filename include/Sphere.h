#pragma once
#include "Object.h"

class Sphere :Object{
private:
	Vector3f center;
	float radius;
	float area;

	//all object should compute its bound when construct!
	void computeBound() { bound = Bound3(center - Vector3f(radius), center + Vector3f(radius)); }

public:
	Sphere() = delete;
	
	Sphere(Material* _material,Vector3f _center, float _radius):Object(_material) {
		center = _center;
		radius = _radius > 0 ? _radius : 0;
		computeBound();
		area = 4.0 * MY_PI * radius * radius;
	}

	float getArea() { return area; }

	Intersection intersect(const Ray& ray);

	//get the point and the pdf;
	void sample(Intersection& inter, float& pdf);

};

