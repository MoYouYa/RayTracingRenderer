#pragma once
#include <random>

#define MY_PI 3.1415926

static std::random_device dev;
static std::mt19937 rng(dev());
static std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [1, 6]

//return 0.0001f - 1.0f
static inline float getRandomFloat() {
	return dist(rng);
}

 static inline float clamp(const float& lo, const float& hi, const float& v)
{
	return std::max(lo, std::min(hi, v));
}

 static std::string intToStr(int x) {
	 std::string s;
	 while (x) {
		 s.push_back('0' + x % 10);
		 x /= 10;
	 }
	 reverse(s.begin(), s.end());
	 return s;
 };