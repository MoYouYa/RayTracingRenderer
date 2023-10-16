#pragma once
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

#pragma region Function implementation

float Material::DistributionGGX(const Vector3f& N, const Vector3f& H)const {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = std::max(vec::dotProduct(N.normalize(), H.normalize()), 0.0f);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = MY_PI * denom * denom;

	return nom / std::max(denom, 0.0001f); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}

//float Material::NormalDistributionFunction(const Vector3f& wh, const Vector3f& N) const {
//	float a2 = roughness * roughness;
//	float cosh = vec::dotProduct(wh.normalize(), N.normalize());
//	float cosh2 = cosh * cosh;
//	float sinh2 = 1.0f - cosh2;
//	return a2 / std::powf(a2 * cosh2 + sinh2, 2) / MY_PI;
//}

float Material::GeometrySmith(const Vector3f& N, const Vector3f& V, const Vector3f& L)const
{
	auto GeometrySchlickGGX = [=](float NdotV, float k)
	{
		float nom = NdotV;
		float denom = NdotV * (1.0 - k) + k;

		return nom / denom;
	};

	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;
	float NdotV = std::max(vec::dotProduct(N, V), 0.0f);
	float NdotL = std::max(vec::dotProduct(N, L), 0.0f);
	float ggx2 = GeometrySchlickGGX(NdotV, k);
	float ggx1 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

void Material::fresnel(const Vector3f& I, const Vector3f& N, const float& ior, float& kr) const
{
	float cosi = clamp(-1.0f, 1.0f, vec::dotProduct(I, N));
	float etai = 1, etat = ior;
	if (cosi > 0) { std::swap(etai, etat); }
	// Compute sini using Snell's law
	float sint = etai / etat * std::sqrtf(std::max(0.f, 1 - cosi * cosi));
	// Total internal reflection
	if (sint >= 1) {
		kr = 1;
	}
	else {
		float cost = std::sqrtf(std::max(0.f, 1 - sint * sint));
		cosi = std::fabsf(cosi);
		float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
		float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
		kr = (Rs * Rs + Rp * Rp) / 2;
	}
	// As a consequence of the conservation of energy, transmittance is given by:
	// kt = 1 - kr;
}

Vector3f Material::eval(const Vector3f& wi, const Vector3f& wo, const Vector3f& normal) const {
	Vector3f res;
	switch (type) {
		case MaterialType::DIFFUSE:
		{
			float cosValue = vec::dotProduct(wo.normalize(), normal.normalize());
			res = kd * (cosValue > 0.0f ? 1.0f / MY_PI : 0.0f);
		}break;
		case MaterialType::SPECULAR:
		{
			Vector3f half = wi.normalize() + wo.normalize();
			half = half.normalize();
			Vector3f N = normal.normalize();
			res = vec::dotProduct(half, N) > 1.0f - 1e-3 ? kd : Vector3f(0);
		}break;
		case MaterialType::MICROFACET:
		{
			//todo
			float kr;
			fresnel(wi.normalize(), normal.normalize(), ior, kr);
			float d = DistributionGGX(normal.normalize(), (wi.normalize() + wo.normalize()).normalize());
			float g = GeometrySmith(normal.normalize(), wi.normalize(), wo.normalize());
			float coso = vec::dotProduct(wo.normalize(), normal.normalize());
			float cosi = vec::dotProduct(wi.normalize(), normal.normalize());
			res = ks * (kr * d * g / 4.0f / std::max(coso * cosi, 0.0001f)) + kd * (1 - kr) / MY_PI;
			
			//if(std::isnan(res.x)){
			//	TEST::printVector3f(res);
			//	float cosValue = vec::dotProduct(wo.normalize(), normal.normalize());
			//	res = kd * (cosValue > 0.0f ? 1.0f / MY_PI : 0.0f);
			//	TEST::printVector3f(res);
			//}

		}break;
	}
	return res;
}

Vector3f Material::sample(const Vector3f& wi, const Vector3f& normal) const {
	//assert(type != MaterialType::GLOSSY, "this material is not supported!");
	Vector3f wo;
	std::function<Vector3f(const Vector3f&, const Vector3f)> toWorld = [=](const Vector3f& local, const Vector3f& normal) {
		Vector3f B, C;
		Vector3f N = normal.normalize();
		if (std::fabs(N.x) > std::fabs(N.y)) {
			float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
			C = Vector3f(N.z * invLen, 0.0f, -N.x * invLen);
		}
		else {
			float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
			C = Vector3f(0.0f, N.z * invLen, -N.y * invLen);
		}
		B = vec::crossProduct(C, N);
		return local.x * B + local.y * C + local.z * N;
	};

	switch (type) {
		case MaterialType::DIFFUSE:
		{
			//float alph = getRandomFloat() * 2.0f * MY_PI;
			//float theta = getRandomFloat() * MY_PI;
			//float z = std::sinf(theta);
			//float r = std::sqrtf(1.0f - z * z);
			//Vector3f local(r * std::cosf(alph), r * std::sinf(alph), z);
			//wo = toWorld(local, normal);

			float alph = getRandomFloat() * 2.0f * MY_PI;
			float theta = getRandomFloat() * 2.0f * MY_PI;
			wo = Vector3f(std::cosf(alph) * std::cosf(theta), std::sinf(theta), std::sinf(alph) * std::cosf(theta));
			if (vec::dotProduct(wo, normal) < 0) {
				wo = -wo;
			}
		}break;
		case MaterialType::SPECULAR:
		{
			wo = getReflectVector(-wi,normal);
		}break;
		case MaterialType::MICROFACET:
		{
			//todo
			float alph = getRandomFloat() * 2.0f * MY_PI;
			float theta = getRandomFloat() * 2.0f * MY_PI;
			wo = Vector3f(std::cosf(alph) * std::cosf(theta), std::sinf(theta), std::sinf(alph) * std::cosf(theta));
			if (vec::dotProduct(wo, normal) < 0) {
				wo = -wo;
			}
		}break;
	}
	return wo;
}

float Material::pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& normal) const {
	float pdf = 0;
	switch (type) {
	case MaterialType::DIFFUSE:
	{
		pdf = 0.5f / MY_PI;
	}break;
	case MaterialType::SPECULAR:
	{
		pdf = 1.0f;
	}break;
	case MaterialType::MICROFACET:
	{
		//todo
		pdf = 0.5f/MY_PI;
		
	}break;
	}
	return pdf;
}

#pragma endregion
