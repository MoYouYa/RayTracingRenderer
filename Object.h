#pragma once
#include "Ray.h"
#include "Intersection.h"
#include "Material.h"
#include "Bound3.h"


class Object {
protected:
	Material* material;
	Bound3 bound;
	float area;

	//all object should compute its bound when construct!
	virtual void computeBound() = 0;

public:
	Object() = delete;
	Object( Material* _material) {
		material = _material;
	}

	const Material* getMaterial() { return material; }
	Bound3 getBound() { return bound; }
	virtual float getArea() = 0;
	virtual Intersection intersect(const Ray& ray) = 0;
	//get a point,a light dir and the pdf of create this intersection.
	virtual void sample(Intersection& inter, float& pdf) = 0;

	virtual ~Object() {}
};

