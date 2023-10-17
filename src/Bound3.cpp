#include "Bound3.h"

bool Bound3::intersect(const Ray& ray) const {
	Vector3f pos = ray.startPos;
	Vector3f dir = ray.dir.normalize();

	float tminx = (min.x - pos.x) / dir.x;
	float tmaxx = (max.x - pos.x) / dir.x;
	if (tminx > tmaxx)std::swap(tminx, tmaxx);

	float tminy = (min.y - pos.y) / dir.y;
	float tmaxy = (max.y - pos.y) / dir.y;
	if (tminy > tmaxy)std::swap(tminy, tmaxy);

	float tminz = (min.z - pos.z) / dir.z;
	float tmaxz = (max.z - pos.z) / dir.z;
	if (tminz > tmaxz)std::swap(tminz, tmaxz);

	float tmin = tminx > tminy ? (tminx > tminz ? tminx : tminz) : (tminy > tminz ? tminy : tminz);
	float tmax = tmaxx < tmaxy ? (tmaxx < tmaxz ? tmaxx : tmaxz) : (tmaxy < tmaxz ? tmaxy : tmaxz);
	return tmin <= tmax;
}
