#pragma once
#include<opencv2/opencv.hpp>
#include <fstream>
#include "Scene.h"
#include "Global.h"

#pragma warning(disable : 4996)

enum RenderType {
	/*WHITTED_STYLE_PATH_TRACING,*/
	PATH_TRACING,
	BIDIRECTIONAL_PATH_TRACING
};

class Render {
private:

	/*Vector3f WhittedStylePathTracing(Scene& scene, Ray& ray,unsigned int bounce) {
		Vector3f emission(0);

		if (bounce <= 0) {
			return emission;
		}
		Intersection inter = scene.intersect(ray);

		Vector3f wi = -ray.dir.normalize();
		Vector3f N = inter.normal.normalize();
		if (!inter.hit) {
			return emission;
		}
		if (inter.object->getMaterial()->hasEmission()) {
			return inter.object->getMaterial()->getEmission();
		}
		else {
			std::vector<Vector3f> lightEmissions;
			std::vector<Vector3f> lightDirs;
			scene.lightPoint(inter, lightEmissions,lightDirs);
			for (int i = 0; i < lightEmissions.size(); i++) {
				switch (inter.object->getMaterial()->getMaterialType()) {
					case MaterialType::DIFFUSE: {
						float cosKd = vec::dotProduct(wi, N);
						cosKd = cosKd > 0 ? cosKd : 0;
						Vector3f kd = inter.object->getMaterial()->getKd()*cosKd;
						emission = emission + lightEmissions[i] * kd;
					}break;
					case MaterialType::GLOSSY: {
						float cosKd = vec::dotProduct(wi, N);
						cosKd = cosKd > 0 ? cosKd : 0;
						Vector3f half = (-lightDirs[i].normalize() - ray.dir.normalize()).normalize();
						float cosKs = vec::dotProduct(half,N);
						cosKs = cosKs > 0 ? cosKs : 0;
						Vector3f kd = inter.object->getMaterial()->getKd()*cosKd;
						Vector3f ks = inter.object->getMaterial()->getKs() * pow(cosKs, inter.object->getMaterial()->getSpecularExponent());
						emission = emission + lightEmissions[i] * (kd + ks);
					}break;
					case MaterialType::SPECULAR: {

					}break;
				}

			}
			if (bounce > 1 && lightEmissions.empty()) {
				Vector3f reflectEmission(0);
				Ray reflectRay(inter.postion, inter.object->getMaterial()->getReflectVector(ray, N));
				reflectEmission = WhittedStylePathTracing(scene, reflectRay,bounce-1);
				emission = emission * 0.8 + reflectEmission * 0.2;
			}
		}
		return emission;
	}*/

	Vector3f pathTracing(Scene& scene, Ray& ray) {
		Vector3f res(0);

		Intersection inter = scene.intersect(ray);

		Vector3f wi = -ray.dir.normalize();
		Vector3f N = inter.normal.normalize();
		if (inter.hit) {
			if (inter.object->getMaterial()->hasEmission()) {
				res = inter.object->getMaterial()->getEmission();
			}
			else {
				//对光源采样
				Intersection lightInter;
				float lightPdf=-1.0f;
				scene.samplingLight(lightInter,lightPdf);
				if (lightPdf > 0) {
					Ray lightRay(inter.postion, (lightInter.postion - inter.postion).normalize());
					Intersection testInter = scene.intersect(lightRay);
					if (testInter.hit) {
						float disLen = (testInter.postion - lightInter.postion).getLen();
						if (disLen >= -1e-3 && disLen <= 1e-3) {
							res = res
								+ lightInter.object->getMaterial()->getEmission()
								* inter.object->getMaterial()->eval(wi, lightRay.dir.normalize(), N)
								* vec::dotProduct(N, lightRay.dir.normalize())
								* vec::dotProduct(lightInter.normal.normalize(), -lightRay.dir.normalize())
								/ std::powf(testInter.distance, 2)
								/ lightPdf;
							//std::cout << "res=" << res.x << " , " << res.y << " , " << res.z << "\n";
						}
					}
				}

				//反射光
				if (getRandomFloat() < 0.6f) {
					Vector3f wo;
					float pdfReflect=-1.0;
					wo = inter.object->getMaterial()->sample(wi, N);
					pdfReflect=inter.object->getMaterial()->pdf(wi, wo.normalize(), N);
					Ray reflectRay(inter.postion, wo.normalize());
					Intersection reflectInter = scene.intersect(reflectRay);
					if (reflectInter.hit && !reflectInter.object->getMaterial()->hasEmission()) {
						res = res
							+ pathTracing(scene, reflectRay)
							* inter.object->getMaterial()->eval(wi, reflectRay.dir.normalize(), N)
							* vec::dotProduct(N, wo.normalize())
							/ pdfReflect
							/ 0.6f;
						//std::cout << "res=" << res.x << " , " << res.y << " , " << res.z << "\n";
					}
				}
			}
		}
		return res;
	}

	Vector3f bidirectionalPathTracing(Scene& scene, Ray& ray) {
		Vector3f res(0);
		
		Intersection inter = scene.intersect(ray);

		Vector3f wi = -ray.dir.normalize();
		Vector3f N = inter.normal.normalize();
		if (inter.hit) {
			if (inter.object->getMaterial()->hasEmission()) {
				res = inter.object->getMaterial()->getEmission();
			}
			else {
				//对光源采样
				//Intersection lightInter;
				//float lightPdf = -1.0f;
				//scene.samplingLight(lightInter, lightPdf);
				//if (lightPdf > 1e-5) {
				//	Ray lightRay(inter.postion, (lightInter.postion - inter.postion).normalize());
				//	Intersection testInter = scene.intersect(lightRay);
				//	if (testInter.hit) {
				//		float disLen = (testInter.postion - lightInter.postion).getLen();
				//		if (disLen >= -1e-3 && disLen <= 1e-3) {
				//			res = res
				//				+ lightInter.object->getMaterial()->getEmission()
				//				* inter.object->getMaterial()->eval(wi, lightRay.dir.normalize(), N)
				//				* vec::dotProduct(N, lightRay.dir.normalize())
				//				* vec::dotProduct(lightInter.normal.normalize(), -lightRay.dir.normalize())
				//				/ std::pow(testInter.distance, 2)
				//				/ lightPdf;
				//		}
				//	}
				//}

				//对光路上的点采样
				std::vector<LightPoint> lightPoints;
				float lightPdf = -1;
				scene.getLightPath(lightPoints,lightPdf);
				if (lightPdf > 0 && !lightPoints.empty()) {
					Vector3f e(0);
					for (int i = 0; i < lightPoints.size(); i++) {
						Ray lightRay(inter.postion, (lightPoints[i].inter.postion - inter.postion).normalize());
						Intersection testInter = scene.intersect(lightRay);
						if (testInter.hit) {
							float disLen = (testInter.postion - lightPoints[i].inter.postion).getLen();
							if (disLen >= -1e-3 && disLen <= 1e-3) {
								e = e
									+ lightPoints[i].emission
									* inter.object->getMaterial()->eval(wi, lightRay.dir.normalize(), N)
									* vec::dotProduct(N, lightRay.dir.normalize())
									* vec::dotProduct(lightPoints[i].inter.normal.normalize(), -lightRay.dir.normalize())
									/ std::pow(testInter.distance, 2)
									/ lightPdf;
							}
						}
					}
					//e = e / lightPoints.size();
					res = res + e;
				}

				//反射光
				if (getRandomFloat() < 0.8f) {
					Vector3f wo;
					float pdfReflect = -1.0;
					wo=inter.object->getMaterial()->sample(wi, N);
					pdfReflect=inter.object->getMaterial()->pdf(wi, wo.normalize(), N);
					Ray reflectRay(inter.postion, wo.normalize());
					if (pdfReflect > 1e-5 && scene.intersect(reflectRay).hit) {
						res = res
							+ bidirectionalPathTracing(scene, reflectRay)
							* inter.object->getMaterial()->eval(wi, reflectRay.dir.normalize(), N)
							* vec::dotProduct(N, wo.normalize())
							/ pdfReflect
							/ 0.8f;
					}
				}
			}
		}
		return res;
	}
public:
	Render() {}

	void render(Scene& scene,const unsigned int& imageWidth, const unsigned int& imageHeight,RenderType renderType,const unsigned int& spp,std::string fileName) {
		std::cout << "render start, spp = " << spp << "\n";
		auto start = std::chrono::system_clock::now();

		std::vector<Vector3f>* buffer = new std::vector<Vector3f>(imageWidth*imageHeight);

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

		std::function<void(unsigned int,unsigned int)> renderThread = [&](unsigned int startRow,unsigned int step) {
			for (int i = 0; i < width; i++) {
				for (int j = startRow; j < startRow+step; j++) {
					Vector3f dir = (frontDir + (-width / 2 + i) * horizontalStep + (-height / 2 + j) * verticalStep).normalize();
					Ray ray(eyePos, dir);

					switch (renderType) {
						//case RenderType::WHITTED_STYLE_PATH_TRACING:
						//{
						//	Vector3f color = WhittedStylePathTracing(scene, ray, 4);
						//	(*buffer)[i * height + j] = color;
						//}break;
						case RenderType::PATH_TRACING:
						{
							Vector3f color(0);
							for (int k = 0; k < spp; k++) {
								color = color + pathTracing(scene, ray);
							}
							color = color / (float)spp;
							(*buffer)[i * height + j] = color;
						}break;
						case RenderType::BIDIRECTIONAL_PATH_TRACING: 
						{
							Vector3f color(0);
							for (int k = 0; k < spp; k++) {
								color = color + bidirectionalPathTracing(scene, ray);
							}
							color = color / (float)spp;
							(*buffer)[i * height + j] = color;
						}break;
					}
				}
			}
		};

		std::vector<std::thread> threads;
		int maxThreadNum = 16;
		int step = height / maxThreadNum;
		int startRow = 0;
		if (step > 0) {
			while (startRow < height) {
				threads.emplace_back(std::thread(renderThread,startRow,step));
				startRow += step;
			}
			for (int i = 0; i < threads.size(); i++) {
				threads[i].join();
			}
		}
		else {
			threads.emplace_back(std::thread(renderThread, startRow, 1));
		}

		std::function<std::string(int)> intToStr = [=](int x) {
			std::string s;
			while (x) {
				s.push_back('0' + x % 10);
				x /= 10;
			}
			reverse(s.begin(), s.end());
			return s;
		};


		//save this image this file x is downward ,y is towards the right
		FILE* fp = fopen((fileName+"_"+ intToStr(spp) + "spp.ppm").c_str(), "wb");
		(void)fprintf(fp, "P6\n%d %d\n255\n", width, height);
		for (auto i = 0; i < height * width; ++i) {
			static unsigned char color[3];
			color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, (*buffer)[i].x), 0.6f));
			color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, (*buffer)[i].y), 0.6f));
			color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, (*buffer)[i].z), 0.6f));
			fwrite(color, 1, 3, fp);
		}
		fclose(fp);

		std::cout << "render end, spp = " << spp << "\n";
		auto stop = std::chrono::system_clock::now();
		std::cout << "Render complete: \n";
		std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
		std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
		std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
	}
};