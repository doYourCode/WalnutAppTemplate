#pragma once

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include  "../../Walnut/vendor/glm/glm/glm.hpp"

#include <iostream>

#include <memory>

class Renderer
{
public:

	Renderer() = default;

	void Render();

	void OnResize(uint32_t width, uint32_t height);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

private:

	uint32_t PerPixel(glm::vec2 coord);

private:

	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

};

inline uint32_t ShowScreenUvCoords(glm::vec2 coord)
{
	uint8_t r = (uint8_t)(coord.x * 255.0f);
	uint8_t g = (uint8_t)(coord.y * 255.0f);

	uint32_t results = 0xff000000 | (g << 8) | r;

	return results;
}

inline uint32_t ShowCircle(glm::vec2 coord)
{
	// Lets first draw a circle since it's simpler (one less dimension)
	// 
	// Explaining the Math
	// 
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// 
	// Where:
	// a = ray origin
	// b = ray direction
	// r = radius of the sphete / circle
	// t = hit distance

	float radius = 0.1f;

	//glm::vec3 rayOrigin(0.0f); if camera is in 0 then all you'll se is sphere's color (camera is inside it)
	glm::vec3 rayOrigin(0.0f, 0.0f, 0.5f);

	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	rayDirection = glm::normalize(rayDirection);

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	// Quadratic formula discrimination
	// b^2 - 4ac

	float discriminant = b * b - 4.0f * a * c;

	if (discriminant >= 0.0f)
		return 0xffff00ff;
	else if (discriminant < 0.0f)
		return 0xff000000;

	return 0;
}

#include "Renderer.h"
#include <Walnut/Random.h>


void Renderer::Render()
{

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };

			// Map coordinates to -1 to 1
			coord = coord * 2.0f - 1.0f;

			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
		}
	}

	// Sends pixels data to VRAM
	m_FinalImage->SetData(m_ImageData);
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

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	//return ShowScreenUvCoords(coord);

	return ShowCircle(coord);
}

#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Renderer.h"

class ExampleLayer : public Walnut::Layer
{
public:

	virtual void OnUIRender() override
	{
		// Renders lateral menu with settings and info
		ImGui::Begin("Settings");

		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::Text("%.3fms", m_LastRenderTime);

		ImGui::End();

		// Renders the viewport with imagem buffer results
		ImGui::Begin("Viewport");


		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if (image)
			ImGui::Image(
				image->GetDescriptorSet(),
				{ (float)image->GetWidth(),
				(float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();

		Render();
	}

	void Render()
	{
		Walnut::Timer timer;

		// Renderer checks for resize
		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);

		// Renderer calls it's render function
		m_Renderer.Render();

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Cpu Raytracer";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
				{
					app->Close();
				}
				ImGui::EndMenu();
			}
		});
	return app;
}