#include "Sphere.h"

Intersection Sphere::intersect(const Ray& ray) {
	Intersection inter;
	if ((ray.startPos - center).getLen() < radius) {
		return inter;
	}
	Vector3f o = ray.startPos - center;//Å²µ½Ô­µã
	Vector3f dir = ray.dir.normalize();
	float a = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
	float b = 2 * (o.x * dir.x + o.y * dir.y + o.z * dir.z);
	float c = o.x * o.x + o.y * o.y + o.z * o.z - radius * radius;
	float delta = b * b - 4 * a * c;
	if (delta < 0) {
		return inter;
	}
	float t1 = (-b + sqrtf(delta)) / 2 / a;
	float t2 = (-b - sqrtf(delta)) / 2 / a;
	if (t1 < 1e-3) {
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

void Sphere::sample(Intersection& inter, float& pdf) {
	float theta = 2.0 * MY_PI * getRandomFloat(), phi = MY_PI * getRandomFloat();
	Vector3f dir(std::cos(phi), std::sin(phi) * std::cos(theta), std::sin(phi) * std::sin(theta));
	inter.postion = center + radius * dir;
	inter.normal = dir;
	inter.object = this;
	pdf = 1.0f / area;
}
