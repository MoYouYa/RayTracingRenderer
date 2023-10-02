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

	Intersection intersect(const Ray& ray) {
		Intersection inter;
		if ((ray.startPos - center).getLen() < radius) {
			return inter;
		}
		Vector3f o = ray.startPos-center;//Å²µ½Ô­µã
		Vector3f dir = ray.dir.normalize();
		float a = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
		float b = 2 * (o.x * dir.x + o.y * dir.y + o.z * dir.z);
		float c = o.x * o.x + o.y * o.y + o.z * o.z - radius * radius;
		float delta = b * b - 4 * a * c;
		if ( delta< 0) {
			return inter;
		}
		float t1 = (-b + sqrtf(delta)) / 2 / a;
		float t2 = (-b - sqrtf(delta)) / 2 / a;
		if (t1 < 0) {
			return inter;
		}
		if (t2 < 0) {
			inter.hit = true;
			inter.distance = t1;
			inter.postion = ray.startPos + dir * t1;
			inter.normal = (inter.postion - center).normalize();
			//inter.textureCrood=...;
			inter.object = this;
		}
		else {
			inter.hit = true;
			inter.distance = t2;
			inter.postion = ray.startPos + dir * t2;
			inter.normal = (inter.postion - center).normalize();
			//inter.textureCrood=...;
			inter.object = this;
		}
		return inter;
	}

	//get the point and the pdf;
	void sample(Intersection& inter, float& pdf) {
		//float alph = getRandomFloat() * 2.0 * MY_PI;
		//float theta = getRandomFloat() * MY_PI;
		//Vector3f dir(std::cosf(alph) * std::cosf(theta), std::sinf(theta), std::sinf(alph) * std::cosf(theta));
		//inter.postion = center + dir * radius;
		//inter.normal = dir;
		//inter.object = this;
		//pdf = 1.0 / area;

		float theta = 2.0 * MY_PI * getRandomFloat(), phi = MY_PI * getRandomFloat();
		Vector3f dir(std::cos(phi), std::sin(phi) * std::cos(theta), std::sin(phi) * std::sin(theta));
		inter.postion = center + radius * dir;
		inter.normal = dir;
		inter.object = this;
		pdf = 1.0f / area;
	}

};