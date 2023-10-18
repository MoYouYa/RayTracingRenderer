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
	BIDIRECTIONAL_PATH_TRACING,
	PHOTON_MAPPING
};

class Render {
private:
	std::vector<Vector3f>* buffer;

	//Vector3f WhittedStylePathTracing(Scene& scene, Ray& ray, unsigned int bounce);

	void saveFile(std::string fileName, const unsigned int height, const unsigned int width);

	Vector3f pathTracing(Scene& scene, Ray& ray);

	std::vector<LightPoint*> getLightPoints(Scene& scene);

	void getLightPoints(Scene& scene, std::vector<LightPoint*>& lightPoints);

	std::vector<CameraPoint*> getCameraPoints(Scene& scene, Ray& ray);

	void getCameraPoints(Scene& scene, std::vector<CameraPoint*>& cameraPoints);

	Vector3f bidirectionalPathTracing(Scene& scene, Ray& ray,bool useMIS=true);

	void createPhotonMap(Scene& scene, std::vector<Photon*>* photonMap) {

	}

	Vector3f photonMapping(Scene& scene, Ray& ray, const std::vector<Photon*>* photonMap) {

	}
public:
	Render() { buffer = nullptr; }

	void renderPathTracing(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, const unsigned int& spp, std::string fileName);
	
	void renderBidirectionalPathTracing(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, const unsigned int& spp, std::string fileName,bool useMIS=true);
};

