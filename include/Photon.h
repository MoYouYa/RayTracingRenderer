#pragma once
#include "Bound3.h"

struct Photon {
	Vector3f pos;
	Vector3f wi;
	Vector3f power;
};

struct CmpPhotonDis {
	Photon* photon;
	float dis2;
	
	CmpPhotonDis(Photon* _photon = nullptr, float _dis2 = 0.0f) {
		photon = _photon;
		dis2 = _dis2;
	}

	friend bool operator>(CmpPhotonDis a, CmpPhotonDis b) {
		return a.dis2 > b.dis2;
	}

	friend bool operator<(CmpPhotonDis a, CmpPhotonDis b) {
		return a.dis2 < b.dis2;
	}
};

struct KDTree {
	Photon* photon;
	Bound3 bound;
	KDTree* left;
	KDTree* right;
	
	KDTree() { photon = nullptr; left = nullptr; right = nullptr;}
};

static void clearKDTree(KDTree* root) {
	if (root == nullptr)return;
	if (root->left == nullptr && root->right == nullptr) {
		delete root;
		return;
	}
	clearKDTree(root->left);
	clearKDTree(root->right);
};

