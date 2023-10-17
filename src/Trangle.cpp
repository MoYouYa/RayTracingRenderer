#include "Trangle.h"

Intersection Trangle::intersect(const Ray& ray) {
	Intersection inter;
	if (vec::dotProduct(-ray.dir, normal) < 0) {
		return inter;
	}
	Vector3f e1 = p1 - p0;
	Vector3f e2 = p2 - p0;
	Vector3f s = ray.startPos - p0;
	Vector3f s1 = vec::crossProduct(ray.dir.normalize(), e2);
	Vector3f s2 = vec::crossProduct(s, e1);
	float s_e = vec::dotProduct(s1, e1);
	float t = 1 / s_e * vec::dotProduct(s2, e2);
	float b1 = 1 / s_e * vec::dotProduct(s1, s);
	float b2 = 1 / s_e * vec::dotProduct(s2, ray.dir.normalize());
	if (t < -1e-5 || b1 < -1e-5 || b2 < -1e-5 || (1 - b1 - b2) < -1e-5) {
		return inter;
	}

	inter.hit = true;
	inter.distance = t;
	inter.postion = p0 * (1 - b1 - b2) + p1 * b1 + p2 * b2;
	inter.normal = normal;
	inter.textureCoord = textureCoords0 * (1 - b1 - b2) + textureCoords1 * b1 + textureCoords2 * b2;
	inter.object = this;
	return inter;
}

void Trangle::computeBound() {
	Vector3f min, max;
	min.x = p0.x < p1.x ? (p0.x < p2.x ? p0.x : p2.x) : (p1.x < p2.x ? p1.x : p2.x);
	min.y = p0.y < p1.y ? (p0.y < p2.y ? p0.y : p2.y) : (p1.y < p2.y ? p1.y : p2.y);
	min.z = p0.z < p1.z ? (p0.z < p2.z ? p0.z : p2.z) : (p1.z < p2.z ? p1.z : p2.z);

	max.x = p0.x > p1.x ? (p0.x > p2.x ? p0.x : p2.x) : (p1.x > p2.x ? p1.x : p2.x);
	max.y = p0.y > p1.y ? (p0.y > p2.y ? p0.y : p2.y) : (p1.y > p2.y ? p1.y : p2.y);
	max.z = p0.z > p1.z ? (p0.z > p2.z ? p0.z : p2.z) : (p1.z > p2.z ? p1.z : p2.z);
	bound = Bound3(min, max);
}

void Trangle::sample(Intersection& inter, float& pdf) {
	float b1 = getRandomFloat();
	float b2 = getRandomFloat() * (1.0 - b1);
	inter.postion = (1 - b1 - b2) * p0 + b1 * p1 + b2 * p2;
	inter.normal = this->normal;
	inter.object = this;
	pdf = 1.0f / area;
}
