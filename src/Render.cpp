#include "Render.h"

void Render::saveFile(std::string fileName, const unsigned int height, const unsigned int width) {
	//save this image this file x is downward ,y is towards the right
	FILE* fp = fopen((fileName + ".ppm").c_str(), "wb");
	try {
		(void)fprintf(fp, "P6\n%d %d\n255\n", height, width);
		for (auto i = 0; i < height * width; ++i) {
			static unsigned char color[3];
			color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, (*buffer)[i].x), 0.6f));
			color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, (*buffer)[i].y), 0.6f));
			color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, (*buffer)[i].z), 0.6f));
			fwrite(color, 1, 3, fp);
		}
		fclose(fp);
	}
	catch (std::exception e) {
		std::cout << e.what();
		fclose(fp);
	}
}

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

	if (getRandomFloat() < 0.6f || cameraPoints.size() == 1) {
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

	cameraPoints.emplace_back(new CameraPoint(inter.object, inter.postion, inter.normal.normalize(), point->kd * kd, point->pdf * pdf * (cameraPoints.size() == 1?1.0f:0.6f)));

	if (inter.object->getMaterial()->hasEmission()) {
		return;
	}

	if (getRandomFloat() < 0.6f/*cameraPoints.size()<5*/) {
		getCameraPoints(scene, cameraPoints);
	}
}

Vector3f Render::bidirectionalPathTracing(Scene& scene, Ray& ray, bool useMIS) {
	Vector3f res(0);

	std::vector<LightPoint*> lightPoints = getLightPoints(scene);
	std::vector<CameraPoint*> cameraPoints = getCameraPoints(scene, ray);
	int S = lightPoints.size();
	int T = cameraPoints.size();

	for (int s = -1; s < S; s++) {
		for (int t = 0; t < T; t++) {
			//a test if
			//if (s !=0)continue;
			// fs = f(xs-2, xs-1, xs),  ft = f(xs-1, xs, xs+1)
			if (s + t == -1)continue;
			Vector3f fs, ft, connect;
			float g = 1.0f;

			Vector3f wi, wo, N;
			if (s >= 0) {
				//int a = 0;
				//if ((lightPoints[s]->pos - cameraPoints[t]->pos).getLen() <= 35.0f) {
				//	a++;
				//}
				//a = 2;
				if (t > 0) {
					float distance2;
					wi = (lightPoints[s]->pos - cameraPoints[t]->pos);
					distance2 = wi.norm();
					wi = wi.normalize();
					wo = (cameraPoints[t - 1]->pos - cameraPoints[t]->pos).normalize();
					N = cameraPoints[t]->normal.normalize();

					ft = cameraPoints[t]->obj->getMaterial()->eval(wi, wo, N);
					if (scene.checkConnectable(lightPoints[s]->pos, cameraPoints[t]->pos)) {
						g /= distance2;
						g *= std::max(vec::dotProduct(lightPoints[s]->normal.normalize(), -wi), 0.0001f);
						g *= std::max(vec::dotProduct(cameraPoints[t]->normal.normalize(), wi), 0.0001f);
					}
					else {
						g = 0.0f;
					}
				}
				else {
					float distance2;
					wi = (lightPoints[s]->pos - cameraPoints[t]->pos);
					distance2 = wi.norm();

					ft = std::powf(vec::dotProduct(wi, cameraPoints[t]->normal.normalize()), 2.0f);
					if (scene.checkConnectable(lightPoints[s]->pos, cameraPoints[t]->pos)) {
						g /= distance2;
						g *= std::max(vec::dotProduct(lightPoints[s]->normal.normalize(), -wi), 0.0001f);
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
						float distance2 = (cameraPoints[t]->pos - cameraPoints[t - 1]->pos).norm();
						connect = connect / distance2;
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
			if (useMIS && g > 1e-5) {
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
				if (s > -1) {
					pfs[0] = lightPoints[0]->pdf;
				}
				else {
					pfs[0] = 1.0f;
				}
				{
					Vector3f dir = (poss[1] - poss[0]);
					//float distance = dir.getLen();
					dir = dir.normalize();
					float temp = vec::dotProduct(normals[1], -dir);// / distance / distance;
					pfs[1] = 1.0f / MY_PI *temp;
				}
				for (int i = 2; i < s + t + 2; i++) {
					Vector3f wi = (poss[i - 2] - poss[i - 1]).normalize();
					Vector3f wo = poss[i] - poss[i - 1];
					//float distance = wo.getLen();
					wo = wo.normalize();
					Vector3f N = normals[i - 1];
					Vector3f N2 = normals[i];
					float temp = vec::dotProduct(-wo, N2);// / distance / distance;
					pfs[i] = objs[i - 1]->getMaterial()->pdf(wi, wo, N) * temp * 0.6f;
				}

				prs[s + t + 2 - 1] = 1.0f;
				{
					Vector3f dir = (poss[s + t + 2 - 2] - poss[s + t + 2 - 1]);
					float distance2 = dir.norm();
					dir = dir.normalize();
					float temp = vec::dotProduct(normals[s + t + 2 - 2], -dir) / distance2;
					prs[s + t + 2 - 2] = 1.0f *temp;
				}
				for (int i = 0; i < s + t + 2 - 2; i++) {
					Vector3f wi = (poss[i + 2] - poss[i + 1]).normalize();
					Vector3f wo = (poss[i] - poss[i + 1]);
					float distance2 = wo.norm();
					wo = wo.normalize();
					Vector3f N = normals[i + 1];
					Vector3f N2 = normals[i];
					float temp = vec::dotProduct(-wo, N2) / distance2;
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
				if (std::isnan(weight) || std::isinf(weight)) {
					weight = 1.0f;
				}
			}
			Vector3f contribution = (s == -1 ? 1.0f : lightPoints[s]->emission / lightPoints[s]->pdf) * connect * cameraPoints[t]->kd / cameraPoints[t]->pdf;

			float w = useMIS ? (s >= 0 ? weight : 1.0f) : 1.0f / float(s + t + 1 > 1 ? s + t + 1 : 1);

			//if ((contribution * w).x > 1.0f && (contribution * w).y > 1.0f && (contribution * w).z > 1.0f) {
			//	res = res + Vector3f(0);
			//}

			res = res + contribution * w;// / (s + t + 1);

			//if (!(res.x < 1.0f || res.y < 1.0f || res.z < 1.0f)) {
			//	res =res+ Vector3f(0);
			//}
		}
	}


	return res;
}

void Render::createPhotonMap(Scene& scene, std::vector<Photon*>* photonMap, unsigned int photonNum) {

	std::function<void(unsigned int, unsigned int)> createPhotonMapThread = [&](unsigned int startIndex, unsigned int endIndex) {
		int k = startIndex;
		while (k < endIndex) {
			Intersection inter;
			float pdf = -1.0f;
			scene.samplingLight(inter, pdf);
			if (inter.object == nullptr) {
				break;
			}

			Photon* prePhoton = new Photon();
			prePhoton->pos = inter.postion;
			prePhoton->wi = inter.normal.normalize();
			prePhoton->power = inter.object->getMaterial()->getEmission();

			int maxDeep = 64;
			do {
				maxDeep--;
				Vector3f preWo = inter.object->getMaterial()->sample(prePhoton->wi, inter.normal.normalize());

				if (preWo.norm() < 1e-5)break;

				preWo = preWo.normalize();
				Ray ray(prePhoton->pos, preWo);
				Intersection nextInter = scene.intersect(ray);

				if (nextInter.hit && !nextInter.object->getMaterial()->hasEmission()) {
					Photon* photon = new Photon();
					photon->pos = nextInter.postion;
					photon->wi = -preWo;
					if (!inter.object->getMaterial()->hasEmission()) {
						photon->power =
							inter.object->getMaterial()->eval(prePhoton->wi, preWo, inter.normal.normalize())
							* prePhoton->power
							* vec::dotProduct(inter.normal.normalize(), preWo)
							* vec::dotProduct(nextInter.normal.normalize(), photon->wi);
					}
					else {
						photon->power =
							prePhoton->power
							* vec::dotProduct(inter.normal.normalize(), preWo)
							* vec::dotProduct(nextInter.normal.normalize(), photon->wi);
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
			} while (k < endIndex && maxDeep >= 0 && (getRandomFloat() < 0.6f || inter.object->getMaterial()->getMaterialType() != MaterialType::DIFFUSE));
		}
	};

	std::vector<std::thread> threads;
	unsigned int maxThreadNum = 16;
	if (photonNum < maxThreadNum) {
		threads.emplace_back(std::thread(createPhotonMapThread, 0, photonNum));
		threads[0].join();
	}
	else {
		int step = photonNum / maxThreadNum;
		for (int i = 0; i < maxThreadNum - 1; i++) {
			threads.emplace_back(std::thread(createPhotonMapThread, i * step, i * step + step));
		}
		threads.emplace_back(std::thread(createPhotonMapThread, step * (maxThreadNum - 1), photonNum));
		for (int i = 0; i < maxThreadNum; i++) {
			threads[i].join();
		}
	}
}

KDTree* Render::createPhotonMapKDTree(std::vector<Photon*>* photonMap) {
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
	}
	break;
	case 2:
	{
		std::sort(photonMap->begin(), photonMap->end(), [=](Photon* a, Photon* b) {
			return a->pos.y < b->pos.y;
			});
	}
	break;
	case 3:
	{
		std::sort(photonMap->begin(), photonMap->end(), [=](Photon* a, Photon* b) {
			return a->pos.z < b->pos.z;
			});
	}
	break;
	}
	int n = photonMap->size();
	std::vector<Photon*>* leftList = new std::vector<Photon*>(photonMap->begin(), photonMap->begin() + n / 2);
	std::vector<Photon*>* rightList = new std::vector<Photon*>(photonMap->begin() + n / 2, photonMap->end());
	root->left = createPhotonMapKDTree(leftList);
	root->right = createPhotonMapKDTree(rightList);
	root->bound.merge(root->left->bound);
	root->bound.merge(root->right->bound);
	return root;
}

float Render::getNearDis2(const Vector3f& pos, const Bound3& bound) {
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

void Render::getTopKPhotons(std::priority_queue<CmpPhotonDis, std::vector<CmpPhotonDis>, std::less<CmpPhotonDis>>& photons,KDTree* kdTree,Vector3f& pos,const unsigned int& checkNum) {

	if (kdTree == nullptr) {
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
		if (photons.size() < checkNum || dis2 < photons.top().dis2) {
			KDTree* lnode = kdTree->left;
			KDTree* rnode = kdTree->right;
			float ldis2 = getNearDis2(pos, lnode->bound);
			float rdis2 = getNearDis2(pos, rnode->bound);
			if (ldis2 < rdis2) {
				getTopKPhotons(photons, lnode, pos, checkNum);
				getTopKPhotons(photons, rnode, pos, checkNum);
			}
			else {
				getTopKPhotons(photons, rnode, pos, checkNum);
				getTopKPhotons(photons, lnode, pos, checkNum);
			}
		}
	}
}

Vector3f Render::photonMapping(Scene& scene, Ray& ray, KDTree* kdTree, std::vector<Photon*>* photonMap, const unsigned int& checkNum, int maxDeep) {
	Vector3f res(0);
	maxDeep--;
	if (kdTree == nullptr || checkNum <= 0)return res;
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
				std::priority_queue<CmpPhotonDis, std::vector<CmpPhotonDis>, std::less<CmpPhotonDis>> photons;
				int searchNum = 0;
				getTopKPhotons(photons, kdTree, inter.postion, checkNum);

				while (!photons.empty()) {
					res = res + photons.top().photon->power;
					photons.pop();
				}

				res = res * inter.object->getMaterial()->eval(inter.normal.normalize(), wi, N) / MY_PI / checkNum;
			}
			else {
				Vector3f wo = inter.object->getMaterial()->sample(wi, N).normalize();
				float pdf = inter.object->getMaterial()->pdf(wi, wo, N);
				Vector3f e = inter.object->getMaterial()->eval(wi, wo, N);
				Ray nextRay(inter.postion, wo);
				res = photonMapping(scene, nextRay, kdTree, photonMap, checkNum, maxDeep)
					* e
					* vec::dotProduct(N, wo)
					/ pdf;
			}
		}
	}
	return res;
}

void Render::renderPathTracing(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight,  const unsigned int& spp, std::string fileName) {
	std::cout << "render start, spp = " << spp << "\n";
	auto start = std::chrono::system_clock::now();

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
				Vector3f dir = (frontDir + (-width / 2 + i) * horizontalStep + (-height / 2 + j) * verticalStep).normalize();
				Ray ray(eyePos, dir);

				Vector3f color(0);
				for (int k = 0; k < spp; k++) {
					color = color + pathTracing(scene, ray);
				}
				color = color / (float)spp;
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
			threads.emplace_back(std::thread(renderThread, i*step, i*step+step));
		}
		threads.emplace_back(std::thread(renderThread, (maxThreadNum-1)*step, height));

		for (int i = 0; i < maxThreadNum; i++) {
			threads[i].join();
		}
	}
	else {
		threads.emplace_back(std::thread(renderThread, startRow, 1));
		threads[0].join();
	}

	saveFile(fileName, height, width);

	std::cout << "render end, spp = " << spp << "\n";
	auto stop = std::chrono::system_clock::now();
	std::cout << "Render complete: \n";
	std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
	std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
	std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
}

void Render::renderBidirectionalPathTracing(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, const unsigned int& spp, std::string fileName,bool useMIS) {
	std::cout << "render start, spp = " << spp << "\n";
	auto start = std::chrono::system_clock::now();

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
				Vector3f dir = (frontDir + (-width / 2 + i) * horizontalStep + (-height / 2 + j) * verticalStep).normalize();
				Ray ray(eyePos, dir);

				Vector3f color(0);
				for (int k = 0; k < spp; k++) {
					color = color + bidirectionalPathTracing(scene, ray,useMIS);
				}
				color = color / (float)spp;
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
	else {
		threads.emplace_back(std::thread(renderThread, startRow, 1));
		threads[0].join();
	}

	saveFile(fileName, height, width);

	std::cout << "render end, spp = " << spp << "\n";
	auto stop = std::chrono::system_clock::now();
	std::cout << "Render complete: \n";
	std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
	std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
	std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
}

void Render::renderPhotonMapping(Scene& scene, const unsigned int& imageWidth, const unsigned int& imageHeight, const unsigned int& photonNum, const unsigned int& checkNum, std::string fileName) {
	std::cout << "render start, photonNum = " << photonNum << " ; checkNum = " << checkNum << "\n";
	auto start = std::chrono::system_clock::now();

	std::vector<Photon*>* photonMap = new std::vector<Photon*>(photonNum);
	createPhotonMap(scene, photonMap, photonNum);
	KDTree* kdTree = createPhotonMapKDTree(photonMap);

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
				Vector3f dir = (frontDir + (-width / 2 + i) * horizontalStep + (-height / 2 + j) * verticalStep).normalize();
				Ray ray(eyePos, dir);

				Vector3f color(0);
				color = color + photonMapping(scene, ray, kdTree, photonMap, checkNum, 64);
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

	clearKDTree(kdTree);
	for (Photon* p : *photonMap) {
		delete p;
	}
	delete photonMap;
}
