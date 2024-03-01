#pragma once

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include  "../../Walnut/vendor/glm/glm/glm.hpp"

#include <iostream>

#include <memory>
#include "FastRandom.h"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

struct HitPayload
{
	float HitDistance;
	glm::vec3 WorldPosition;
	glm::vec3 WorldNormal;

	int ObjectIndex;
};

class Renderer
{
public:

	Renderer() = default;

	void Render(const Scene& scene, const Camera& camera);

	void OnResize(uint32_t width, uint32_t height);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

	void SetBounces(uint32_t value) { m_Bounces = glm::clamp((int)value, 0, 10); }

private:

	glm::vec4 RayGen(uint32_t x, uint32_t y); // Per pixel

	HitPayload TraceRay(const Ray& ray);

	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);

	HitPayload Miss(const Ray& ray);

	// HitPayload AnyHit(const Ray& ray); might be good for transluscent objects

private:

	std::shared_ptr<Walnut::Image> m_FinalImage;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	uint32_t* m_ImageData = nullptr;

	uint32_t m_Bounces = 3;

	uint32_t m_FrameIndex = 0;

};

inline uint32_t ShowScreenUvCoords(glm::vec2 coord)
{
	uint8_t r = (uint8_t)(coord.x * 255.0f);
	uint8_t g = (uint8_t)(coord.y * 255.0f);

	uint32_t results = 0xff000000 | (g << 8) | r;

	return results;
}

inline uint32_t ShowRandom(uint32_t seed)
{
	//uint32_t result = WangHash(seed);
	//uint32_t result = PcgHash(seed);
	uint32_t result = Walnut::Random::UInt();

	result |= 0xff000000;
	return result;
}

namespace Utils
{
	inline uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		//std::cout << "(" << r << "," << g << "," << b << "," << a << ")" << std::endl;

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;

		return result;
	}
}
