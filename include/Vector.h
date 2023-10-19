#pragma once
#include<iostream>

class Vector2f {
public:
	float x;
	float y;
	Vector2f() :x(0), y(0) {}
	Vector2f(float value) :x(value), y(value) {}
	Vector2f(float _x , float _y) :x(_x), y(_y) {}

	Vector2f operator+(const Vector2f& v2)const {	return Vector2f(x + v2.x, y + v2.y);}

	Vector2f operator-(const Vector2f& v2)const {return Vector2f(x - v2.x, y - v2.y);}

	Vector2f operator-()const {return Vector2f(-x, -y);}

	Vector2f operator*(const float& n)const {return Vector2f(x * n, y * n);}

	Vector2f operator*(const Vector2f& v2)const {return Vector2f(x * v2.x, y * v2.y);}

	Vector2f operator/(const float& n)const {return Vector2f(x / n, y / n);}

	friend Vector2f operator*(const float& n, const Vector2f& v) { return Vector2f(n * v.x, n * v.y); }

	float getLen()const { return std::sqrtf(x * x + y * y); }

	float norm()const { return x * x + y * y; }

	Vector2f normalize() const{
		float len = getLen();
		return Vector2f(x / len, y / len);
	}
};

class Vector3f {
public:
	float x;
	float y;
	float z;
	Vector3f() :x(0), y(0), z(0) {}
	Vector3f(float value ) :x(value), y(value), z(value) {}
	Vector3f(float _x , float _y , float _z ) :x(_x), y(_y), z(_z) {}

	Vector3f operator+(const Vector3f v3) const {return Vector3f(x + v3.x, y + v3.y, z + v3.z);}

	Vector3f operator-(const Vector3f v3)const {return Vector3f(x - v3.x, y - v3.y, z - v3.z);}

	Vector3f operator-()const {return Vector3f(-x, -y, -z);}

	Vector3f operator*(const float n)const {return Vector3f(x * n, y * n, z * n);}

	Vector3f operator*(const Vector3f v3)const {return Vector3f(x * v3.x, y * v3.y, z * v3.z);}

	Vector3f operator/(const float n)const {return Vector3f(x / n, y / n, z / n);}

	friend Vector3f operator*(const float& n, const Vector3f& v) { return Vector3f(n * v.x, n * v.y, n * v.z); }

	float getLen() const { return std::sqrtf(x * x + y * y + z * z); }

	//get vector lenght square
	float norm()const { return x * x + y * y + z * z; }

	Vector3f normalize() const {
		float len = getLen();
		return Vector3f(x / len, y / len, z / len);
	}
};

class Vector4f {
public:
	float x;
	float y;
	float z;
	float w;
	Vector4f() :x(0), y(0), z(0), w(0) {}
	Vector4f(float value) :x(value), y(value), z(value), w(value) {}
	Vector4f(float _x , float _y , float _z , float _w ) :x(_x), y(_y), z(_z), w(_w) {}

	Vector4f operator+(const Vector4f v4)const {return Vector4f(x + v4.x, y + v4.y, z + v4.z, w + v4.w);}

	Vector4f operator-(const Vector4f v4)const {return Vector4f(x - v4.x, y - v4.y, z - v4.z, w - v4.w);}

	Vector4f operator-() {return Vector4f(-x, -y, -z,-w);}

	Vector4f operator*(const float n)const {return Vector4f(x * n, y * n, z * n, w * n);}

	Vector4f operator*(const Vector4f v4) const {return Vector4f(x * v4.x, y * v4.y, z * v4.z, w * v4.w);}

	Vector4f operator/(const float n) const {return Vector4f(x / n, y / n, z / n, w / n);}

	friend Vector4f operator*(const float& n, const Vector4f& v) { return Vector4f(n * v.x, n * v.y, n * v.z, n * v.w); }

	float getLen()const { return std::sqrtf(x * x + y * y + z * z + w * w); }

	float norm()const { return x * x + y * y + z * z + w * w; }

	Vector4f normalize() const {
		float len = getLen();
		return Vector4f(x / len, y / len, z / len, w / len);
	}
};

namespace vec {
	static inline float dotProduct( Vector2f v0,  Vector2f v1) {
		return v0.x * v1.x + v0.y * v1.y;
	}

	static inline float dotProduct( Vector3f v0,  Vector3f v1) {
		return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
	}

	static inline float dotProduct(Vector4f v0, Vector4f v1) {
		return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
	}

	static inline Vector3f crossProduct( Vector3f v0,  Vector3f v1) {
		return Vector3f(v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v1.z * v0.x, v0.x * v1.y - v0.y * v1.x);
	}

}

namespace TEST {
	static inline void printVector3f(Vector3f v) {
		std::cout << v.x << " , " << v.y << " , " << v.z << "\n";
	}
}