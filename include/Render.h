#pragma once
#include <fstream>
#include <functional>
#include <thread>
#include "Scene.h"
#include "Global.h"
#include "Photon.h"

#pragma warning(disable : 4996)

enum RenderType {
	/*WHITTED_STYLE_PATH_TRACING,*/
	PATH_TRACING,
	BIDIRECTIONAL_PATH_TRACING
};

class Render {
private:

	//Vector3f WhittedStylePathTracing(Scene& scene, Ray& ray, unsigned int bounce);

	//all the wi ,wo and normal are facing the same hemisphere
	Vector3f pathTracing(Scene& scene, Ray& ray);

	std::vector<LightPoint*> getLightPoints(Scene& scene);

	void getLightPoints(Scene& scene, std::vector<LightPoint*>& lightPoints);

	std::vector<CameraPoint*> getCameraPoints(Scene& scene, Ray& ray);

	void getCameraPoints(Scene& scene, std::vector<CameraPoint*>& cameraPoints);

	Vector3f bidirectionalPathTracing(Scene& scene, Ray& ray,bool useMIS=true);

	std::vector<Photon*> getPhotonMap();

	Vector3f photonMapping(Scene& scene, Ray& ray,unsigned int minPhotonNum);
public:
	Render() {}

	void render(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, RenderType renderType, const unsigned int& spp, std::string fileName);
};

