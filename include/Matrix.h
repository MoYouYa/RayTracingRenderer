#pragma once
#include "Vector.h"
#include <cassert>

class Mat4f {
private:
	Vector4f v0;
	Vector4f v1;
	Vector4f v2;
	Vector4f v3;

public:
	Mat4f() { 
		v0 = Vector4f(1, 0, 0, 0); 
		v1 = Vector4f(0, 1, 0, 0);
		v2 = Vector4f(0, 0, 1, 0);
		v3 = Vector4f(0, 0, 0, 1);
	}
	Mat4f(float _x0,float _y0,float _z0,float _w0,
		float _x1, float _y1, float _z1, float _w1,
		float _x2, float _y2, float _z2, float _w2,
		float _x3, float _y3, float _z3, float _w3) {
		v0 = Vector4f(_x0, _y0, _z0, _w0);
		v1 = Vector4f(_x1, _y1, _z1, _w1);
		v2 = Vector4f(_x2, _y2, _z2, _w2);
		v3 = Vector4f(_x3, _y3, _z3, _w3);
	}
	Mat4f(Vector4f _v0, Vector4f _v1, Vector4f _v2, Vector4f _v3) :v0(_v0),v1(_v1),v2(_v2),v3(_v3){}

	Vector4f operator[](int index) {
		assert(index >= 0);
		assert(index <= 3);
		switch (index) {
			case 0:return v0;
			case 1:return v1;
			case 2:return v2;
			case 3:return v3;
		}
	}

	Mat4f operator*(Mat4f mat) {
		return Mat4f(
			v0.x * mat[0].x + v0.y * mat[1].x + v0.z * mat[2].x + v0.w * mat[3].x,
			v0.x * mat[0].y + v0.y * mat[1].y + v0.z * mat[2].y + v0.w * mat[3].y,
			v0.x * mat[0].z + v0.y * mat[1].z + v0.z * mat[2].z + v0.w * mat[3].z,
			v0.x * mat[0].w + v0.y * mat[1].w + v0.z * mat[2].w + v0.w * mat[3].w,
			v1.x * mat[0].x + v1.y * mat[1].x + v1.z * mat[2].x + v1.w * mat[3].x,
			v1.x * mat[0].y + v1.y * mat[1].y + v1.z * mat[2].y + v1.w * mat[3].y,
			v1.x * mat[0].z + v1.y * mat[1].z + v1.z * mat[2].z + v1.w * mat[3].z,
			v1.x * mat[0].w + v1.y * mat[1].w + v1.z * mat[2].w + v1.w * mat[3].w,
			v2.x * mat[0].x + v2.y * mat[1].x + v2.z * mat[2].x + v2.w * mat[3].x,
			v2.x * mat[0].y + v2.y * mat[1].y + v2.z * mat[2].y + v2.w * mat[3].y,
			v2.x * mat[0].z + v2.y * mat[1].z + v2.z * mat[2].z + v2.w * mat[3].z,
			v2.x * mat[0].w + v2.y * mat[1].w + v2.z * mat[2].w + v2.w * mat[3].w,
			v3.x * mat[0].x + v3.y * mat[1].x + v3.z * mat[2].x + v3.w * mat[3].x,
			v3.x * mat[0].y + v3.y * mat[1].y + v3.z * mat[2].y + v3.w * mat[3].y,
			v3.x * mat[0].z + v3.y * mat[1].z + v3.z * mat[2].z + v3.w * mat[3].z,
			v3.x * mat[0].w + v3.y * mat[1].w + v3.z * mat[2].w + v3.w * mat[3].w
		);
	}

	Vector4f operator*(Vector4f v4) {
		return Vector4f(
			vec::dotProduct(v0, v4),
			vec::dotProduct(v1, v4),
			vec::dotProduct(v2, v4),
			vec::dotProduct(v3, v4));
	}

	Mat4f transpose() {
		return Mat4f(
			v0.x, v1.x, v2.x, v3.x,
			v0.y, v1.y, v2.y, v3.y,
			v0.z, v1.z, v2.z, v3.z,
			v0.w, v1.w, v2.w, v3.w
		);
	}
};