#include "../include/Scene.h"

void Scene::lightPoint(const Intersection& inter, std::vector<Vector3f>& outLightEmissions, std::vector<Vector3f>& outLightDirs) {
	for (int i = 0; i < lights.size(); i++) {
		Vector3f lightCenter = lights[i]->getBound().getCenterPos();
		Ray lightDir(lightCenter, (inter.postion - lightCenter).normalize());
		Intersection hitPoint = intersect(lightDir);
		if (hitPoint.hit && (hitPoint.postion - inter.postion).getLen() < 1) {
			outLightEmissions.emplace_back(lights[i]->getMaterial()->getEmission());
			outLightDirs.emplace_back((inter.postion - lightCenter).normalize());
		}
	}
	return;
}

void Scene::samplingLight(Intersection& inter, float& pdf) {
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
