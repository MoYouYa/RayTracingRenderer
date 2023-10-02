#pragma once
#include <vector>
#include"Matrix.h"
#include "Object.h"
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

	void getLightPoints(const Ray& ray,Vector3f emission, std::vector<LightPoint>& lightPoints) {
		Intersection inter = intersect(ray);
		if (!inter.hit || inter.object->getMaterial()->hasEmission()) {
			return;
		}
		Vector3f reflectDir;
		float pdf = -1;
		reflectDir=inter.object->getMaterial()->sample(-ray.dir.normalize(), inter.normal.normalize());
		pdf = inter.object->getMaterial()->pdf(-ray.dir.normalize(), reflectDir.normalize(), inter.normal.normalize());
		//pdf = 1.0f;
		if (pdf > 0) {
			emission = emission
				* inter.object->getMaterial()->eval(-ray.dir.normalize(), reflectDir.normalize(), inter.normal.normalize())
				* vec::dotProduct(-ray.dir.normalize(), inter.normal.normalize())
				/*/ inter.distance / inter.distance*/ 
				/ pdf;
			LightPoint p;
			p.inter = inter;
			p.emission = emission;
			lightPoints.emplace_back(p);
			Ray reflectRay(inter.postion, reflectDir.normalize());
			if (getRandomFloat() < 0.8f) {
				getLightPoints(reflectRay, emission, lightPoints);
			}
		}
		return;
	}
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

	//get lightEmissions and lightDirs ,this function is only used by WhittedStyleRayTracing
	void lightPoint(const Intersection& inter,std::vector<Vector3f>& outLightEmissions,std::vector<Vector3f>& outLightDirs) {
		for (int i = 0; i < lights.size(); i++) {
			Vector3f lightCenter = lights[i]->getBound().getCenterPos();
			Ray lightDir(lightCenter,(inter.postion - lightCenter).normalize());
			Intersection hitPoint = intersect(lightDir);
			if (hitPoint.hit && (hitPoint.postion-inter.postion).getLen()<1) {
				outLightEmissions.emplace_back(lights[i]->getMaterial()->getEmission());
				outLightDirs.emplace_back((inter.postion - lightCenter).normalize());
			}
		}
		return ;
	}

	//sampling at light direction
	void samplingLight(Intersection& inter,float& pdf) {
		float area = 0;
		for (int i = 0; i < lights.size(); i++) {
			area += lights[i]->getArea();
		}
		float pickArea = getRandomFloat() * area;
		for (int i = 0; i < lights.size(); i++) {
			if (pickArea < 1e-3) {
				lights[i]->sample(inter, pdf);
				break;
			}
			else {
				pickArea -= lights[i]->getArea();
			}
		}
		if (pdf < 0 && lights.size()>0) {
			lights.back()->sample(inter, pdf);
		}
		return;
	}

	//get a light path and save lightPoints which is on this path.
	void getLightPath(std::vector<LightPoint>& lightPoints,float& pdf) {
		float area = 0;
		Intersection inter;
		LightPoint p;
		for (int i = 0; i < lights.size(); i++) {
			area += lights[i]->getArea();
		}
		float pickArea = getRandomFloat() * area;
		for (int i = 0; i < lights.size(); i++) {
			if (pickArea < 1e-3) {
				lights[i]->sample(inter, pdf);
				p.emission = lights[i]->getMaterial()->getEmission();
				break;
			}
			else {
				pickArea -= lights[i]->getArea();
			}
		}
		if (lightPoints.empty()) {
			lights.back()->sample(inter, pdf);
			p.emission = lights.back()->getMaterial()->getEmission();
		}
		if (inter.hit) {
			p.inter = inter;
			
			lightPoints.emplace_back(p);
			Ray ray(inter.postion, inter.normal.normalize());
			if (getRandomFloat() < 0.8f) {
				getLightPoints(ray, inter.object->getMaterial()->getEmission(), lightPoints);
			}
		}
		return;
	}
};
