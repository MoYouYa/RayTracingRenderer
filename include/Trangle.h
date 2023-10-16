#pragma once
#include<iostream>
#include <vector>
#include <cassert>
#include "Object.h"
#include "BVH.h"

class Trangle :Object {
private:
	Vector3f p0, p1, p2;
	Vector3f normal;
	Vector2f textureCoords0, textureCoords1, textureCoords2;

	//all object should compute its bound when construct!
	void computeBound();

public:
	Trangle(
		Material* _material,
		Vector3f _p0,
		Vector3f _p1,
		Vector3f _p2,
		Vector3f _normal,
		Vector2f _textureCoords0,
		Vector2f _textureCoords1,
		Vector2f _textureCoords2) :Object(_material)
	{
		p0 = _p0; p1 = _p1; p2 = _p2;
		if (_normal.x * _normal.x + _normal.y * _normal.y + _normal.z * _normal.z >= 1.0 - 1e-4) {
			normal = _normal.normalize();
		}
		else {
			normal = vec::crossProduct(Vector3f(p1 - p0), Vector3f(p2 - p0)).normalize();
		}
		textureCoords0 = _textureCoords0; textureCoords1 = _textureCoords1; textureCoords2 = _textureCoords2;
		computeBound();
		float len = vec::crossProduct(Vector3f(p1 - p0), Vector3f(p2 - p0)).getLen(); 
		area = len;
	}

	float getArea() { return area; }

	Intersection intersect(const Ray& ray);

	void sample(Intersection& inter, float& pdf);

	~Trangle(){}
};

class TrangleMesh :Object {
private:
	std::vector<Object*> trangles;

	BVH* bvh;

	//all object should compute its bound when construct!
	void computeBound() { bound = bvh->getBound(); };

public:
	TrangleMesh() = delete;
	TrangleMesh(
		Material* _material,
		std::vector<Vector3f>* _positions,
		std::vector<unsigned int>* _posIndices,
		std::vector<Vector3f>* _normals = nullptr,
		std::vector<Vector2f>* _textureCoords = nullptr) :Object(_material)
	{
		assert(_positions != nullptr, "Don't let position null!");
		assert(_posIndices != nullptr, "Don't let posIndices null!");
		assert(_posIndices->size() % 3 == 0, "The posIndices is not trangles indices!");

		trangles=std::vector<Object*>(_posIndices->size() / 3);
		for (int i = 0; i <_posIndices->size(); i += 3) {
			unsigned int index0 = (*_posIndices)[i];
			unsigned int index1 = (*_posIndices)[i + 1];
			unsigned int index2 = (*_posIndices)[i + 2];
			Vector3f aveNormal(0);
			Vector2f uvs[3];
			if (_normals != nullptr) {
				aveNormal = (*_normals)[index0] + (*_normals)[index1] + (*_normals)[index2];
			}
			if (_textureCoords != nullptr) {
				uvs[0] = (*_textureCoords)[index0];
				uvs[1] = (*_textureCoords)[index1];
				uvs[2] = (*_textureCoords)[index2];
			}
			Trangle* t=new Trangle(_material, (*_positions)[index0], (*_positions)[index1], (*_positions)[index2], aveNormal.normalize(), uvs[0], uvs[1], uvs[2]);
			area += t->getArea();
			trangles[i / 3] = (Object*)t;
		}

		bvh = new BVH();
		bvh->buildBVH(trangles);
		computeBound();
	}

	float getArea() { return area; }

	Intersection intersect(const Ray& ray){ 
		return bvh->intersect(ray); 
	}

	void sample(Intersection& inter, float& pdf) {
		pdf = -1.0f;
		float resArea = getRandomFloat() * area;
		bvh->sample(inter, pdf, resArea);
		pdf/=area;
	};

	~TrangleMesh() {
		while (!trangles.empty()) {
			Trangle* t =(Trangle*) trangles.back();
			trangles.pop_back();
			delete t;
		}
		bvh->~BVH();
		delete bvh;
	}
};

#pragma region Function implementation

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
	min.x = p0.x < p1.x ? (p0.x < p2.x ?  p0.x : p2.x) : (p1.x < p2.x ? p1.x : p2.x);
	min.y = p0.y < p1.y ? (p0.y < p2.y ? p0.y : p2.y) : (p1.y < p2.y ? p1.y : p2.y);
	min.z = p0.z < p1.z ? (p0.z < p2.z ? p0.z : p2.z) : (p1.z < p2.z ? p1.z : p2.z);

	max.x = p0.x > p1.x ? (p0.x > p2.x ? p0.x : p2.x) : (p1.x > p2.x ? p1.x : p2.x);
	max.y = p0.y > p1.y ? (p0.y > p2.y ? p0.y : p2.y) : (p1.y > p2.y ? p1.y : p2.y);
	max.z = p0.z > p1.z ? (p0.z > p2.z ? p0.z : p2.z) : (p1.z > p2.z ? p1.z : p2.z);
	bound = Bound3(min,max);
}

void Trangle::sample(Intersection& inter, float& pdf) {
	float b1 = getRandomFloat();
	float b2 = getRandomFloat() * (1.0-b1);
	inter.postion = (1 - b1 - b2) * p0 + b1 * p1 + b2 * p2;
	inter.normal = this->normal;
	inter.object = this;
	pdf = 1.0f / area;
}

#pragma endregion