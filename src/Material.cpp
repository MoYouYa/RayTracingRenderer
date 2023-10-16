#include "../include/Material.h"

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
		wo = getReflectVector(-wi, normal);
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
		pdf = 0.5f / MY_PI;

	}break;
	}
	return pdf;
}
