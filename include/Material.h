#pragma once
#include<functional>
#include "Global.h"
#include "Vector.h"

enum MaterialType
{
	DIFFUSE,
	SPECULAR,
	MICROFACET
};

class Material {
private:
	MaterialType type;
	Vector3f emission;
	Vector3f kd;
	Vector3f ks;
	float ior;//折射率 between 0-1
	float specularExponent;//高光反射系数
	float roughness;//粗糙度 between 0-1

	//Trowbridge-Reitz(GGX)模型 D(wh) 
	float DistributionGGX(const Vector3f& N, const Vector3f& H)const;

	//Trowbridge-Reitz(GGX)模型 D(wh) 
	/*float NormalDistributionFunction(const Vector3f& wh, const Vector3f& N) const;*/


	//GGX模型 G(wi,wo)
	float GeometrySmith(const Vector3f& N, const Vector3f& V, const Vector3f& L)const;

	//GGX模型 G(wi,wo)
	/*float ShadowingMasking(const Vector3f& wi,const Vector3f& wo,const Vector3f& N) const {
		auto f = [&](const Vector3f& w) {
			float cosValue = vec::dotProduct(w.normalize(), N.normalize());
			float cos2 = cosValue * cosValue;
			float sin2 = 1.0f - cos2;
			return (-1.0f + std::sqrtf(1 + roughness * roughness * sin2 / cos2)) / 2.0f;
		};
		return 1.0f / (1.0f + f(wi) + f(wo));
	}*/

	// Compute Fresnel equatio
	// \param I is the incident view direction
	// \param N is the normal at the intersection point
	// \param ior is the material refractive index
	// \param[out] kr is the amount of light reflected
	void fresnel(const Vector3f& I, const Vector3f& N, const float& ior, float& kr) const;
public:
	Material() = delete;
	
	Material(MaterialType _type, Vector3f _emission, Vector3f _kd = Vector3f(0), Vector3f _ks = Vector3f(0), float _ior = 1e-5,float _roughness=1e-5, float _specularExponent = 32) {
		type = _type;
		emission = _emission;
		kd = _kd;
		ks = _ks;
		ior = _ior > 0 ? _ior : 1e-5;
		roughness = _roughness > 0 ? (_roughness < 1.0f ? _roughness : 1.0f) : 1e-5;
		specularExponent = _specularExponent > 1 ?  _specularExponent : 1.0f;
	}

	MaterialType getMaterialType()const { return type; }
	
	bool hasEmission()const { return emission.x > 1e-4 || emission.y > 1e-4 || emission.z > 1e-4; }
	
	Vector3f getEmission()const { return emission; }
	
	Vector3f getKd()const { return kd; }
	
	Vector3f getKs()const { return ks; }
	
	float getIor()const { return ior; }
	
	float getRoughness()const { return roughness; }
	
	float getSpecularExponent()const { return specularExponent; }

	//this wi is pointing to the target point
	Vector3f getReflectVector(const Vector3f& wi,const Vector3f& normal) const{
		Vector3f i = wi.normalize();
		Vector3f n = normal.normalize();
		return i - 2.0f * vec::dotProduct(n, i) * n;
	}

	Vector3f eval(const Vector3f& wi, const Vector3f& wo, const Vector3f& normal) const;

	Vector3f sample(const Vector3f& wi, const Vector3f& normal) const;

	float pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& normal) const;
};


