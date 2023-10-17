#pragma once
#include <vector>
#include"Matrix.h"
#include "BVH.h"

struct Camera {
	Vector3f position;
	Vector3f frontDir;
	Vector3f upDir;
	float fov;
	Camera() = delete;
	Camera(Vector3f _position, Vector3f _frontDir, Vector3f _upDir, float _fov) {
		position = _position;
		frontDir = _frontDir;
		upDir = _upDir;
		fov = _fov;
	}
};


class Scene {
private:
	Camera* camera;
	std::vector<Object*> objects;
	std::vector<Object*> lights;

	BVH* bvh;

public:
	Scene() = delete;
	Scene(Camera* _camera) { camera = _camera; bvh = new BVH(); }

	const Camera* getCamera() { return camera; }

	void addObject(Object* obj) { 
		objects.emplace_back(obj); 
		if (obj->getMaterial()->hasEmission()) {
			lights.emplace_back(obj);
		}
	}

	void rebuildBVH() { bvh->buildBVH(objects); }

	Intersection intersect(const Ray& ray) { return bvh->intersect(ray); }

	//check point a is connected point b
	bool checkConnectable(Vector3f a, Vector3f b,float ESP=1e-3) {
		Ray ray(a, (b - a).normalize());
		Intersection inter = intersect(ray);
		if (inter.hit && std::abs(inter.distance - (b - a).getLen()) < 1e-3) {
			return true;
		}
		return false;
	}

	//get lightEmissions and lightDirs ,this function is only used by WhittedStyleRayTracing
	void lightPoint(const Intersection& inter, std::vector<Vector3f>& outLightEmissions, std::vector<Vector3f>& outLightDirs);

	//sampling at light direction
	void samplingLight(Intersection& inter, float& pdf);
};


