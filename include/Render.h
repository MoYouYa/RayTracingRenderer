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

	//Vector3f WhittedStylePathTracing(Scene& scene, Ray& ray, unsigned int bounce);

	//all the wi ,wo and normal are facing the same hemisphere
	Vector3f pathTracing(Scene& scene, Ray& ray);

	std::vector<LightPoint*> getLightPoints(Scene& scene);

	void getLightPoints(Scene& scene, std::vector<LightPoint*>& lightPoints);

	std::vector<CameraPoint*> getCameraPoints(Scene& scene, Ray& ray);

	void getCameraPoints(Scene& scene, std::vector<CameraPoint*>& cameraPoints);

	Vector3f bidirectionalPathTracing(Scene& scene, Ray& ray,bool useMIS=true);
public:
	Render() {}

	void render(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, RenderType renderType, const unsigned int& spp, std::string fileName);
};

#pragma region Function implementation

//Vector3f Render::WhittedStylePathTracing(Scene& scene, Ray& ray, unsigned int bounce) {
//	Vector3f emission(0);
//
//	if (bounce <= 0) {
//		return emission;
//	}
//	Intersection inter = scene.intersect(ray);
//
//	Vector3f wi = -ray.dir.normalize();
//	Vector3f N = inter.normal.normalize();
//	if (!inter.hit) {
//		return emission;
//	}
//	if (inter.object->getMaterial()->hasEmission()) {
//		return inter.object->getMaterial()->getEmission();
//	}
//	else {
//		std::vector<Vector3f> lightEmissions;
//		std::vector<Vector3f> lightDirs;
//		scene.lightPoint(inter, lightEmissions, lightDirs);
//		for (int i = 0; i < lightEmissions.size(); i++) {
//			switch (inter.object->getMaterial()->getMaterialType()) {
//			case MaterialType::DIFFUSE: {
//				float cosKd = vec::dotProduct(wi, N);
//				cosKd = cosKd > 0 ? cosKd : 0;
//				Vector3f kd = inter.object->getMaterial()->getKd() * cosKd;
//				emission = emission + lightEmissions[i] * kd;
//			}break;
//			case MaterialType::GLOSSY: {
//				float cosKd = vec::dotProduct(wi, N);
//				cosKd = cosKd > 0 ? cosKd : 0;
//				Vector3f half = (-lightDirs[i].normalize() - ray.dir.normalize()).normalize();
//				float cosKs = vec::dotProduct(half, N);
//				cosKs = cosKs > 0 ? cosKs : 0;
//				Vector3f kd = inter.object->getMaterial()->getKd() * cosKd;
//				Vector3f ks = inter.object->getMaterial()->getKs() * pow(cosKs, inter.object->getMaterial()->getSpecularExponent());
//				emission = emission + lightEmissions[i] * (kd + ks);
//			}break;
//			case MaterialType::SPECULAR: {
//
//			}break;
//			}
//
//		}
//		if (bounce > 1 && lightEmissions.empty()) {
//			Vector3f reflectEmission(0);
//			Ray reflectRay(inter.postion, inter.object->getMaterial()->getReflectVector(ray, N));
//			reflectEmission = WhittedStylePathTracing(scene, reflectRay, bounce - 1);
//			emission = emission * 0.8 + reflectEmission * 0.2;
//		}
//	}
//	return emission;
//}

Vector3f Render::pathTracing(Scene& scene, Ray& ray) {
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
			float lightPdf = -1.0f;
			scene.samplingLight(lightInter, lightPdf);
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
				float pdfReflect = -1.0;
				wo = inter.object->getMaterial()->sample(wi, N);
				pdfReflect = inter.object->getMaterial()->pdf(wi, wo.normalize(), N);
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

std::vector<LightPoint*> Render::getLightPoints(Scene& scene) {
	std::vector<LightPoint*> lightPoints;

	//sample a light
	Intersection startInter;
	float startPdf;
	scene.samplingLight(startInter, startPdf);
	if (startInter.object == nullptr || !startInter.object->getMaterial()->hasEmission()) {
		return lightPoints;
	}
	lightPoints.emplace_back(new LightPoint(startInter.object, startInter.postion, startInter.normal.normalize(), startInter.object->getMaterial()->getEmission(), startPdf));

	if (getRandomFloat() < 0.6f /*lightPoints.size()<5*/) {
		getLightPoints(scene, lightPoints);
	}
	return lightPoints;
}

void Render::getLightPoints(Scene& scene, std::vector<LightPoint*>& lightPoints) {
	const LightPoint* point = lightPoints.back();
	Vector3f wi, wo, e;
	float pdf;
	if (lightPoints.size() == 1) {
		wo = point->normal;
		pdf = 1.0f / MY_PI;
		e = Vector3f(1 / MY_PI);
	}
	else {
		wi = (lightPoints[lightPoints.size() - 2]->pos - point->pos).normalize();
		wo = point->obj->getMaterial()->sample(wi, point->normal).normalize();
		pdf = point->obj->getMaterial()->pdf(wi, wo, point->normal);
		e = point->obj->getMaterial()->eval(wi, wo, point->normal) * std::max(vec::dotProduct(wo, point->normal), 0.0001f);
	}

	Ray ray(point->pos, wo);

	Intersection inter = scene.intersect(ray);
	if (!inter.hit || inter.object->getMaterial()->hasEmission()) {
		return;
	}

	lightPoints.emplace_back(new LightPoint(inter.object, inter.postion, inter.normal.normalize(), point->emission * e, point->pdf * pdf * 0.6f));

	if (getRandomFloat() < 0.6f /*lightPoints.size()<5*/) {
		getLightPoints(scene, lightPoints);
	}
}

std::vector<CameraPoint*> Render::getCameraPoints(Scene& scene, Ray& ray) {
	std::vector<CameraPoint*> cameraPoints;

	//get the look point
	cameraPoints.emplace_back(new CameraPoint(nullptr, ray.startPos, ray.dir.normalize(), Vector3f(1.0f), 1.0f));

	if (getRandomFloat() < 0.6f || cameraPoints.size()==1) {
		getCameraPoints(scene, cameraPoints);
	}

	return cameraPoints;
}

void Render::getCameraPoints(Scene& scene, std::vector<CameraPoint*>& cameraPoints) {
	const CameraPoint* point = cameraPoints.back();
	Vector3f wi, wo, kd;
	float pdf;
	if (cameraPoints.size() == 1) {
		wo = point->normal;
		pdf = 1.0f;
		kd = Vector3f(1.0f);
	}
	else {
		wi = (cameraPoints[cameraPoints.size() - 2]->pos - point->pos).normalize();
		wo = point->obj->getMaterial()->sample(wi, point->normal).normalize();
		pdf = point->obj->getMaterial()->pdf(wi, wo, point->normal);
		kd = point->obj->getMaterial()->eval(wi, wo, point->normal) * std::max(vec::dotProduct(wo, point->normal), 0.0001f);
	}

	Ray ray(point->pos, wo);

	Intersection  inter = scene.intersect(ray);
	if (!inter.hit) {
		return;
	}

	cameraPoints.emplace_back(new CameraPoint(inter.object, inter.postion, inter.normal.normalize(), point->kd * kd, point->pdf * pdf * 0.6f));

	if (inter.object->getMaterial()->hasEmission()) {
		return;
	}

	if (getRandomFloat() < 0.6f/*cameraPoints.size()<5*/) {
		getCameraPoints(scene, cameraPoints);
	}
}

Vector3f Render::bidirectionalPathTracing(Scene& scene, Ray& ray,bool useMIS) {
	Vector3f res(0);

	std::vector<LightPoint*> lightPoints = getLightPoints(scene);
	std::vector<CameraPoint*> cameraPoints = getCameraPoints(scene, ray);
	int S = lightPoints.size();
	int T = cameraPoints.size();

	for (int s = -1; s < S; s++) {
		for (int t = 0; t < T; t++) {
			// fs = f(xs-2, xs-1, xs),  ft = f(xs-1, xs, xs+1)
			if (s + t == -1)continue;
			Vector3f fs, ft, connect;
			float g = 1.0f;

			Vector3f wi, wo, N;
			if (s >= 0) {
				if (t > 0) {
					float distance;
					wi = (lightPoints[s]->pos - cameraPoints[t]->pos);
					distance = wi.getLen();
					wi = wi.normalize();
					wo = (cameraPoints[t - 1]->pos - cameraPoints[t]->pos).normalize();
					N = cameraPoints[t]->normal.normalize();

					ft = cameraPoints[t]->obj->getMaterial()->eval(wi, wo, N);
					if (scene.checkConnectable(lightPoints[s]->pos, cameraPoints[t]->pos)) {
						g /= distance * distance;
						g *= std::max(vec::dotProduct(lightPoints[s]->normal.normalize(), -wi), 0.0001f);
						g *= std::max(vec::dotProduct(cameraPoints[t]->normal.normalize(), wi), 0.0001f);
					}
					else {
						g = 0.0f;
					}
				}
				else {
					float distance;
					wi = (lightPoints[s]->pos - cameraPoints[t]->pos);
					distance = wi.getLen();

					ft = std::powf(vec::dotProduct(wi, cameraPoints[t]->normal.normalize()), 2.0f);
					if (scene.checkConnectable(lightPoints[s]->pos, cameraPoints[t]->pos)) {
						g /= distance * distance;
						g *= std::max(vec::dotProduct(lightPoints[s]->normal.normalize(), -wi),0.0001f);
					}
					else {
						g = 0.0f;
					}
				}
			}
			if (s == -1) {
				if (t > 0 && cameraPoints[t]->obj->getMaterial()->hasEmission()) {
					connect = cameraPoints[t]->obj->getMaterial()->getEmission();
					//别忘了距离衰减项
					if (t > 1) {
						float distance = (cameraPoints[t]->pos - cameraPoints[t - 1]->pos).getLen();
						connect = connect / distance / distance;
					}
				}
				else {
					connect = Vector3f(0);
				}
			}
			else if (s == 0) {
				fs = 1.0f;
				connect = fs * g * ft;
			}
			else {
				wi = (lightPoints[s - 1]->pos - lightPoints[s]->pos).normalize();
				wo = (cameraPoints[t]->pos - lightPoints[s]->pos).normalize();
				N = lightPoints[s]->normal.normalize();
				fs = lightPoints[s]->obj->getMaterial()->eval(wi, wo, N);
				connect = fs * g * ft;
			}

			float weight = 1.0f;
			if (useMIS && g>1e-5) {
				std::vector<float> pfs(s + t + 2);
				std::vector<float> prs(s + t + 2);
				std::vector<Vector3f> poss;
				std::vector<Vector3f> normals;
				std::vector<Object*> objs;

				for (int i = 0; i <= s; i++) {
					poss.emplace_back(lightPoints[i]->pos);
					normals.emplace_back(lightPoints[i]->normal.normalize());
					objs.emplace_back(lightPoints[i]->obj);
				}
				for (int i = t; i >= 0; i--) {
					poss.emplace_back(cameraPoints[i]->pos);
					normals.emplace_back(cameraPoints[i]->normal.normalize());
					objs.emplace_back(cameraPoints[i]->obj);
				}
				pfs[0] = lightPoints[0]->pdf;
				{
					Vector3f dir = (poss[1] - poss[0]);
					float distance = dir.getLen();
					dir = dir.normalize();
					float temp = vec::dotProduct(normals[0], dir) * vec::dotProduct(normals[1], -dir) / distance / distance;
					pfs[1] = 1.0f / MY_PI * g;
				}
				for (int i = 2; i < s + t + 2 ; i++) {
					Vector3f wi = (poss[i - 2] - poss[i - 1]).normalize();
					Vector3f wo = poss[i] - poss[i - 1];
					float distance = wo.getLen();
					wo = wo.normalize();
					Vector3f N = normals[i - 1];
					Vector3f N2 = normals[i];
					float temp = vec::dotProduct(-wo, N2) / distance / distance;
					pfs[i] = objs[i - 1]->getMaterial()->pdf(wi, wo, N) * g * 0.6f;
				}

				prs[s + t + 2 - 1] = 1.0f;
				{
					float distance = (poss[s + t + 2 - 1] - poss[s + t + 2 - 2]).getLen();
					prs[s + t + 2 - 2] = 1.0f / distance / distance;
				}
				for (int i = 0; i < s + t + 2 - 2; i++) {
					Vector3f wi = (poss[i + 2] - poss[i + 1]).normalize();
					Vector3f wo = (poss[i] - poss[i + 1]);
					float distance = wo.getLen();
					wo = wo.normalize();
					Vector3f N = normals[i + 1];
					Vector3f N2 = normals[i];
					float temp = vec::dotProduct(-wo, N2) / distance / distance;
					prs[i] = objs[i + 1]->getMaterial()->pdf(wi, wo, N) * temp * 0.6f;
				}

				float ri = 1.0f;
				for (int i = s; i >= 0; i--) {
					ri *= prs[i] / pfs[i];
					weight += ri;
				}
				ri = 1.0f;
				for (int i = s + 2; i < s + t + 2; i++) {
					ri *= pfs[i] / prs[i];
					weight += ri;
				}
				weight = 1.0f / weight;
			}
			Vector3f contribution = (s == -1 ? 1.0f : lightPoints[s]->emission / lightPoints[s]->pdf) * connect * cameraPoints[t]->kd / cameraPoints[t]->pdf;

			float w = useMIS ? (s >= 0 ? weight : 1.0f ) : 1.0f / float(s + t + 1 > 1 ? s + t + 1 : 1);

			if ((contribution * w).x > 1.0f) {
				res = res + Vector3f(0);
			}

			res = res + contribution * w;// / (s + t + 1);

		}
	}



	return res;
}

void Render::render(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, RenderType renderType, const unsigned int& spp, std::string fileName) {
	std::cout << "render start, spp = " << spp << "\n";
	auto start = std::chrono::system_clock::now();

	std::vector<Vector3f>* buffer = new std::vector<Vector3f>(imageWidth * imageHeight);

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

	std::function<void(unsigned int, unsigned int)> renderThread = [&](unsigned int startRow, unsigned int step) {
		for (int i = 0; i < width; i++) {
			for (int j = startRow; j < startRow + step; j++) {
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
			threads.emplace_back(std::thread(renderThread, startRow, step));
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
	FILE* fp = fopen((fileName + "_" + intToStr(spp) + "spp.ppm").c_str(), "wb");
	(void)fprintf(fp, "P6\n%d %d\n255\n", height, width);
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
#pragma endregion
