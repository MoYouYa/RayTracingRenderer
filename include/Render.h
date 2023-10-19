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

//enum RenderType {
//	/*WHITTED_STYLE_PATH_TRACING,*/
//	PATH_TRACING,
//	BIDIRECTIONAL_PATH_TRACING,
//	PHOTON_MAPPING
//};

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

	void createPhotonMap(Scene& scene, std::vector<Photon*>* photonMap,unsigned int photonNum) {

		std::function<void(unsigned int, unsigned int)> createPhotonMapThread = [&](unsigned int startIndex, unsigned int endIndex) {
			int k = startIndex;
			while (k < endIndex) {
				Intersection inter;
				float pdf=-1.0f;
				scene.samplingLight(inter, pdf);
				if (inter.object == nullptr) {
					break;
				}

				Photon* prePhoton = new Photon();
				prePhoton->pos = inter.postion;
				prePhoton->wi = inter.normal.normalize();
				prePhoton->power = inter.object->getMaterial()->getEmission();// / pdf;

				int maxDeep = 64;
				do {
					maxDeep--;
					Vector3f preWo = inter.object->getMaterial()->sample(prePhoton->wi, inter.normal.normalize());
					
					if (preWo.norm() < 1e-5)break;

					preWo = preWo.normalize();
					Ray ray(prePhoton->pos, preWo);
					Intersection nextInter = scene.intersect(ray);
					//if (maxDeep == 63)prePhoton->power =prePhoton->power/ (inter.postion-nextInter.postion).norm();

					if (nextInter.hit && !nextInter.object->getMaterial()->hasEmission()) {
						Photon* photon = new Photon();
						photon->pos = nextInter.postion;
						photon->wi = -preWo;
						if (!inter.object->getMaterial()->hasEmission()) {
							photon->power =
								inter.object->getMaterial()->eval(prePhoton->wi, preWo, inter.normal.normalize())
								* prePhoton->power
								* vec::dotProduct(inter.normal.normalize(), preWo)
								* vec::dotProduct(nextInter.normal.normalize(), photon->wi)
								;// / distance2;
						}
						else {
							photon->power =
								prePhoton->power
								* vec::dotProduct(inter.normal.normalize(), preWo)
								* vec::dotProduct(nextInter.normal.normalize(), photon->wi)
								;// / distance2;
						}

						if (inter.object->getMaterial()->getMaterialType() == MaterialType::DIFFUSE) {
							(*photonMap)[k] = photon;
							k++;
						}
						prePhoton = photon;
						inter = nextInter;
					}
					else {
						break;
					}
				} while (k < endIndex && maxDeep>=0 && (getRandomFloat() < 0.6f || inter.object->getMaterial()->getMaterialType()!=MaterialType::DIFFUSE));
			}
		};

		std::vector<std::thread> threads;
		unsigned int maxThreadNum = 16;
		if (photonNum < maxThreadNum) {
			threads.emplace_back(std::thread(createPhotonMapThread,0, photonNum));
			threads[0].join();
		}
		else {
			int step = photonNum / maxThreadNum;
			for (int i = 0; i < maxThreadNum-1; i++) {
				threads.emplace_back(std::thread(createPhotonMapThread, i * step, i * step + step));
			}
			threads.emplace_back(std::thread(createPhotonMapThread, step * (maxThreadNum - 1), photonNum));
			for (int i = 0; i < maxThreadNum; i++) {
				threads[i].join();
			}
		}
	}

	KDTree* createPhotonMapKDTree(std::vector<Photon*>* photonMap,int& nodesNum) {
		nodesNum++;
		KDTree* root = new KDTree();
		if (photonMap->size() == 1) {
			root->photon = photonMap->back();
			root->bound.min = root->photon->pos;
			root->bound.max = root->photon->pos;
			return root;
		}
		float xmin = (*photonMap)[0]->pos.x;
		float ymin = (*photonMap)[0]->pos.y;
		float zmin = (*photonMap)[0]->pos.z;
		float xmax = (*photonMap)[0]->pos.x;
		float ymax = (*photonMap)[0]->pos.y;
		float zmax = (*photonMap)[0]->pos.z;
		for (Photon* p : *photonMap) {
			xmin = xmin < p->pos.x ? xmin : p->pos.x;
			ymin = ymin < p->pos.y ? ymin : p->pos.y;
			zmin = zmin < p->pos.z ? zmin : p->pos.z;
			xmax = xmax > p->pos.x ? xmax : p->pos.x;
			ymax = ymax > p->pos.y ? ymax : p->pos.y;
			zmax = zmax > p->pos.z ? zmax : p->pos.z;
		}
		float xlen = xmax - xmin;
		float ylen = ymax - ymin;
		float zlen = zmax - zmin;
		int maxLen = xlen > ylen ? (xlen > zlen ? 1 : 3) : (ylen > zlen ? 2 : 3);
		switch (maxLen) {
			case 1: 
			{
				std::sort(photonMap->begin(), photonMap->end(), [=](Photon* a, Photon* b) {
					return a->pos.x < b->pos.x;
				});
				//std::cout << "cut by x\n";
			}
			break;
			case 2:
			{
				std::sort(photonMap->begin(), photonMap->end(), [=](Photon* a, Photon* b) {
					return a->pos.y < b->pos.y;
				});
				//std::cout << "cut by y\n";
			}
			break;
			case 3:
			{
				std::sort(photonMap->begin(), photonMap->end(), [=](Photon* a, Photon* b) {
					return a->pos.z < b->pos.z;
				});
				//std::cout << "cut by z\n";
			}
			break;
		}
		int n = photonMap->size();
		std::vector<Photon*>* leftList = new std::vector<Photon*>(photonMap->begin(), photonMap->begin() + n / 2);
		std::vector<Photon*>* rightList = new std::vector<Photon*>(photonMap->begin() + n / 2, photonMap->end());
		root->left = createPhotonMapKDTree(leftList,nodesNum);
		root->right = createPhotonMapKDTree(rightList,nodesNum);
		root->bound.merge(root->left->bound);
		root->bound.merge(root->right->bound);
		return root;
	}

	float getNearDis2(const Vector3f& pos, const Bound3& bound) {
		if (pos.x > bound.min.x && pos.x<bound.max.x
			&& pos.y>bound.min.y && pos.y<bound.max.y
			&& pos.z>bound.min.z && pos.z < bound.max.z) {
			return 0.0f;
		}
		Bound3 boundp(pos, pos);
		boundp.merge(bound);
		Vector3f dis = boundp.getCenterPos() - bound.getCenterPos();
		return dis.norm();
	}

	void getTopKPhotons(
		std::priority_queue<CmpPhotonDis,std::vector<CmpPhotonDis>,std::less<CmpPhotonDis>>& photons,
		KDTree* kdTree,
		Vector3f& pos,
		const unsigned int& checkNum,
		int& searchNum) {

		searchNum++;
		if (kdTree==nullptr) {
			return;
		}
		if (kdTree->photon != nullptr) {
			if (photons.size() < checkNum) {
				photons.push(CmpPhotonDis(kdTree->photon, (kdTree->photon->pos - pos).norm()));
			}
			else {
				if ((kdTree->photon->pos - pos).norm() < photons.top().dis2) {
					photons.pop();
					photons.push(CmpPhotonDis(kdTree->photon, (kdTree->photon->pos - pos).norm()));
				}
			}
		}
		else {
			float dis2 = getNearDis2(pos, kdTree->bound);
			//dis2 *= dis2;
			if (photons.size() < checkNum || dis2 < photons.top().dis2) {
				KDTree* lnode = kdTree->left;
				KDTree* rnode = kdTree->right;
				float ldis2 = getNearDis2(pos, lnode->bound);
				float rdis2 = getNearDis2(pos, rnode->bound);
				if (ldis2 < rdis2) {
					getTopKPhotons(photons, lnode, pos,checkNum, searchNum);
					getTopKPhotons(photons, rnode, pos,checkNum, searchNum);
				}
				else {
					getTopKPhotons(photons, rnode, pos, checkNum, searchNum);
					getTopKPhotons(photons, lnode, pos, checkNum, searchNum);
				}
			}
		}
	}

	Vector3f photonMapping(Scene& scene, Ray& ray,KDTree* kdTree,std::vector<Photon*>* photonMap,const unsigned int& checkNum,int maxDeep) {
		Vector3f res(0);
		maxDeep--;
		if (kdTree == nullptr || checkNum<=0)return res;
		if (maxDeep < 0) {
			return res;
		}
		Intersection inter = scene.intersect(ray);
		if (inter.hit) {
			Vector3f wi = -ray.dir.normalize();
			Vector3f N = inter.normal.normalize();
			if (inter.object->getMaterial()->hasEmission()) {
				res = inter.object->getMaterial()->getEmission();
			}
			else {
				if (inter.object->getMaterial()->getMaterialType() == MaterialType::DIFFUSE) {
					//todo 
					std::priority_queue<CmpPhotonDis,std::vector<CmpPhotonDis>,std::less<CmpPhotonDis>> photons;
					int searchNum = 0;
					getTopKPhotons(photons, kdTree,inter.postion,checkNum,searchNum);
					//std::cout << searchNum << "\n";
					//{
					//	for (Photon* p : *photonMap) {
					//		if (photons.size() < checkNum) {
					//			photons.push(CmpPhotonDis(p, (p->pos - inter.postion).norm()));
					//		}
					//		else {
					//			if ((p->pos - inter.postion).norm() < photons.top().dis2) {
					//				photons.pop();
					//				photons.push(CmpPhotonDis(p, (p->pos - inter.postion).norm()));
					//			}
					//		}
					//	}
					//}
					float radius2 = 1.0f;// photons.top().dis2;
				
					while (!photons.empty()) {
						res = res + photons.top().photon->power;
						photons.pop();
					}

					res = res*inter.object->getMaterial()->eval(inter.normal.normalize(),wi,N) / radius2 / MY_PI / checkNum;
				}
				else {
					Vector3f wo = inter.object->getMaterial()->sample(wi,N).normalize();
					float pdf = inter.object->getMaterial()->pdf(wi, wo, N);
					Vector3f e = inter.object->getMaterial()->eval(wi, wo, N);
					Ray nextRay(inter.postion, wo);
					res = photonMapping(scene, nextRay, kdTree,photonMap,checkNum, maxDeep)
						*e
						*vec::dotProduct(N,wo)
						/pdf;
				}
			}
		}
		return res;
	}
public:
	Render() { buffer = nullptr; }

	void renderPathTracing(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, const unsigned int& spp, std::string fileName);
	
	void renderBidirectionalPathTracing(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, const unsigned int& spp, std::string fileName,bool useMIS=true);

	/*
	* @param photonNum : the number of photons which is on photonmap
	* @param checkNum : photons number used to estimate results
	*/
	void renderPhotonMapping(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, const unsigned int& photonNum,const unsigned int& checkNum, std::string fileName) {
		std::cout << "render start, photonNum = " << photonNum <<" ; checkNum = "<<checkNum<< "\n";
		auto start = std::chrono::system_clock::now();

		std::vector<Photon*>* photonMap = new std::vector<Photon*>(photonNum);
		createPhotonMap(scene, photonMap, photonNum);
		int nodesNum = 0;
		KDTree* kdTree = createPhotonMapKDTree(photonMap,nodesNum);
		std::cout << "nodesNum = " << nodesNum << "\n";

		std::cout << "created photon map and build kd-tree, photonNum = " << photonNum << " ; checkNum = " << checkNum << "\n";
		auto kdtreeTime = std::chrono::system_clock::now();
		std::cout << "Render complete: \n";
		std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(kdtreeTime - start).count() << " hours\n";
		std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(kdtreeTime - start).count() << " minutes\n";
		std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(kdtreeTime - start).count() << " seconds\n";

		if (buffer != nullptr)delete buffer;
		buffer = new std::vector<Vector3f>(imageWidth * imageHeight);

		const Camera* camera = scene.getCamera();
		Vector3f eyePos = camera->position;
		Vector3f frontDir = camera->frontDir;
		Vector3f upDir = camera->upDir;
		Vector3f rightDir = vec::crossProduct(frontDir, upDir).normalize();
		int width = imageWidth;
		int height = imageHeight;
		float fov = camera->fov;

		Vector3f verticalStep = tanf(fov / 2.0 / MY_PI / 2.0) * 2.0 / (float)height * upDir;//垂直方向的每一步向量
		Vector3f horizontalStep = tanf(fov / 2.0 / MY_PI / 2.0) * 2.0 / (float)height * rightDir;//水平方向的每一步向量 

		
		std::function<void(unsigned int, unsigned int)> renderThread = [&](unsigned int startRow, unsigned int endRow) {
			for (int i = 0; i < width; i++) {
				for (int j = startRow; j < endRow; j++) {
					//if (abs(i-j)>0)continue;
					Vector3f dir = (frontDir + (-width / 2 + i) * horizontalStep + (-height / 2 + j) * verticalStep).normalize();
					Ray ray(eyePos, dir);

					Vector3f color(0);
					color = color + photonMapping(scene, ray, kdTree,photonMap,checkNum, 64);
					(*buffer)[i * height + j] = color;
				}
			}
		};

		std::vector<std::thread> threads;
		int maxThreadNum = 16;
		int step = height / maxThreadNum;
		int startRow = 0;
		if (step > 0) {
			for (int i = 0; i < maxThreadNum - 1; i++) {
				threads.emplace_back(std::thread(renderThread, i * step, i * step + step));
			}
			threads.emplace_back(std::thread(renderThread, (maxThreadNum - 1) * step, height));

			for (int i = 0; i < maxThreadNum; i++) {
				threads[i].join();
			}
		}

		saveFile(fileName, height, width);

		std::cout << "render end, photonNum = " << photonNum << " ; checkNum = " << checkNum << "\n";
		auto stop = std::chrono::system_clock::now();
		std::cout << "Render complete: \n";
		std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
		std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
		std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
	}
};

