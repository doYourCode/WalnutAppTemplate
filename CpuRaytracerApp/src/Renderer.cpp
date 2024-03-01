#include "Renderer.h"
#include <Walnut/Random.h>


void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	const glm::vec3& rayOrigin = camera.GetPosition();

	uint32_t width = m_FinalImage->GetWidth(), height = m_FinalImage->GetHeight();
	float aspectRatio = width / (float)height;

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			// Generate the rays on a Per Pixel base
			glm::vec4 color = RayGen(x, y);

			// Limit color channels ranges to 0.0f - 1.0f (change it so you can have HDR color data if you need)
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));

			// Put the resulting color into our Frame Buffer at a defined index
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color); // PS. In an RT pipeline you should be writing to your fb from your raygen shader
		}
	}

	// Sends pixels data to VRAM
	m_FinalImage->SetData(m_ImageData);

	m_FrameIndex++;
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize is necessarry
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

glm::vec4 Renderer::RayGen(uint32_t x, uint32_t y)
{
	// Create and trace rays from our perspective
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 color(0.0f);
	float multiplier = 1.0f;

	for (uint32_t i = 0; i < m_Bounces; i++)
	{
		HitPayload payload = TraceRay(ray);

		// If we didn't hit anything then return the "clear color"
		if (payload.HitDistance < 0)
		{
			glm::vec3 clearColor = glm::vec3();
			color += clearColor * multiplier;
			break;
		}	

		// Calculate lighting
		glm::vec3 lightDirection(-1, -1, -0.75);
		lightDirection = glm::normalize(lightDirection);
		float diffuseTerm = glm::max(glm::dot(payload.WorldNormal, -lightDirection), 0.0f);

		// Determine objects colors
		const Sphere& sphete = m_ActiveScene->Spheres[payload.ObjectIndex];
		glm::vec3 sphereColor = sphete.Albedo;
		sphereColor *= diffuseTerm;

		color += sphereColor * multiplier;

		multiplier *= 0.7f;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal);
	}

	return glm::vec4(color, 1.0f);
}

HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	HitPayload payload;

	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - closestSphere.Position;
	payload.WorldPosition = origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	payload.WorldPosition += closestSphere.Position;

	return payload ;
}

HitPayload Renderer::Miss(const Ray& ray)
{
	// If we return negative one then we can latter check this for miss (since it doesn't makes sense)
	HitPayload payload;
	payload.HitDistance = -1.0;

	return payload;
}

HitPayload Renderer::TraceRay(const Ray& ray)
{
	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();;

	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			continue;

		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		if (closestT > 0.0f && closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphere = (int)i;
		}
	}

	if (closestSphere < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestSphere);
}