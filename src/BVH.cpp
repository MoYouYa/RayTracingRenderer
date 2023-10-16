#include "../include/BVH.h"

void BVH::deleteNode(BVHNode* node) {
	if (node == nullptr)return;
	if (node->left == nullptr && node->right == nullptr) {
		delete node;
		node = nullptr;
		return;
	}
	else {
		deleteNode(node->left);
		deleteNode(node->right);
		delete node;
		node = nullptr;
		return;
	}
}

BVHNode* BVH::selfBuildBVH(std::vector<Object*>& objs) {
	if (objs.size() == 0) {
		return nullptr;
	}
	BVHNode* root = new BVHNode();

	Bound3 bound;
	for (int i = 0; i < objs.size(); i++) {
		Bound3 objBound = objs[i]->getBound();
		bound.merge(objBound);
	}
	root->bound = bound;

	if (objs.size() == 1) {
		root->object = objs[0];
	}
	else {
		float xlen = bound.max.x - bound.min.x;
		float ylen = bound.max.y - bound.min.y;
		float zlen = bound.max.z - bound.min.z;
		int maxLen = xlen > ylen ? (xlen > zlen ? 0 : 2) : (ylen > zlen ? 1 : 2);
		switch (maxLen) {
		case 0: {
			std::sort(objs.begin(), objs.end(), [](Object* a, Object* b) {
				return a->getBound().getCenterPos().x < b->getBound().getCenterPos().x;
				});
		}break;
		case 1: {
			std::sort(objs.begin(), objs.end(), [](Object* a, Object* b) {
				return a->getBound().getCenterPos().y < b->getBound().getCenterPos().y;
				});
		}break;
		case 2: {
			std::sort(objs.begin(), objs.end(), [](Object* a, Object* b) {
				return a->getBound().getCenterPos().z < b->getBound().getCenterPos().z;
				});
		}break;

		}
		std::vector<Object*> left = std::vector<Object*>(objs.begin(), objs.begin() + objs.size() / 2);
		std::vector<Object*> right = std::vector<Object*>(objs.begin() + objs.size() / 2, objs.end());
		root->left = selfBuildBVH(left);
		root->right = selfBuildBVH(right);
	}
	return root;
}

Intersection BVH::intersect(const Ray& ray, BVHNode* root)const {
	Intersection inter;
	if (root->bound.intersect(ray)) {
		if (root->object != nullptr) {
			inter = root->object->intersect(ray);
		}
		else {
			Intersection l = intersect(ray, root->left);
			Intersection r = intersect(ray, root->right);
			inter = l.hit ? l : inter;
			inter = r.hit ? (inter.hit ? (inter.distance < r.distance ? inter : r) : r) : inter;
		}
	}
	return inter;
}

void BVH::getSample(BVHNode* root, Intersection& inter, float& pdf, float& resArea) const {
	if (root->left == nullptr && root->right == nullptr) {
		resArea -= root->object->getArea();
		if (resArea < 1e-3) {
			root->object->sample(inter, pdf);
			pdf *= root->object->getArea();
			return;
		}
	}
	else {
		getSample(root->left, inter, pdf, resArea);
		getSample(root->right, inter, pdf, resArea);
	}
}
