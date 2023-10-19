#pragma once
#include <fstream>
#include <functional>
#include <thread>
#include "Scene.h"
#include "Global.h"
#include "Photon.h"
#include <queue>
#include <stack>

#pragma warning(disable : 4996)

//记得将指针改为只能指针或者记得手动回收！！！


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

	Vector3f bidirectionalPathTracing(Scene& scene, Ray& ray, bool useMIS = true);

	void createPhotonMap(Scene& scene, std::vector<Photon*>* photonMap, unsigned int photonNum);

	KDTree* createPhotonMapKDTree(std::vector<Photon*>* photonMap);

	float getNearDis2(const Vector3f& pos, const Bound3& bound);

	void getTopKPhotons(std::priority_queue<CmpPhotonDis, std::vector<CmpPhotonDis>, std::less<CmpPhotonDis>>& photons,KDTree* kdTree,Vector3f& pos,const unsigned int& checkNum);

	Vector3f photonMapping(Scene& scene, Ray& ray, KDTree* kdTree, std::vector<Photon*>* photonMap, const unsigned int& checkNum, int maxDeep);
public:
	Render() { buffer = nullptr; }

	void renderPathTracing(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, const unsigned int& spp, std::string fileName);

	void renderBidirectionalPathTracing(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, const unsigned int& spp, std::string fileName, bool useMIS = true);

	/*
	* @param photonNum : the number of photons which is on photonmap
	* @param checkNum : photons number used to estimate results
	*/
	void renderPhotonMapping(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, const unsigned int& photonNum, const unsigned int& checkNum, std::string fileName);

};