#pragma once
#include<limits.h>
#include <algorithm>
#include <vector>
#include "Object.h"
#include "Bound3.h"


struct BVHNode {
	Bound3 bound;
	Object* object;
	BVHNode* left;
	BVHNode* right;
	BVHNode(Bound3 _bound=Bound3(), Object* obj = nullptr, BVHNode* l = nullptr, BVHNode* r = nullptr) {
		bound = _bound;
		object = obj;
		left = l;
		right = r;
	}

};

class BVH {
private:
	BVHNode* root;

	BVHNode* selfBuildBVH(std::vector<Object*>& objs);

	//before rebuild bvh, we should delete the memory of old data
	void clear() { deleteNode(root); }

	void deleteNode(BVHNode* node);

	Intersection intersect(const Ray& ray, BVHNode* root) const;

	void getSample(BVHNode* root, Intersection& inter, float& pdf, float& resArea)const;
public:
	BVH() { root = nullptr; }

	void buildBVH(std::vector<Object*>& objs) {
		if (root != nullptr) {
			clear();
		}
		root = selfBuildBVH(objs);
	}

	Intersection intersect(const Ray& ray) const{ return intersect(ray, this->root); }

	Bound3 getBound() { return root->bound; }

	void sample(Intersection& inter, float& pdf, float& resArea)const { getSample(root, inter, pdf, resArea); }

	~BVH() { clear(); }
};
