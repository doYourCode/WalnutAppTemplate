#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Renderer.h"
#include "glm/gtc/type_ptr.hpp"
//
class ExampleLayer : public Walnut::Layer
{
public:

	ExampleLayer()
		: m_Camera(45.f, 0.1f, 100.0f), m_Scene()
	{
		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, -4.0f };
			sphere.Radius = 1.0f;
			sphere.Albedo = { 1.0f, 0.0f, 1.0f };

			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 0.0f, -101.0f, -5.0f };
			sphere.Radius = 100.0f;
			sphere.Albedo = { 0.2f, 0.3f, 1.0f };

			m_Scene.Spheres.push_back(sphere);
		}


	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
	}

	virtual void OnUIRender() override
	{
		// Renders lateral menu with settings and info
		ImGui::Begin("Settings");

		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::Text("%.3fms", m_LastRenderTime);

		ImGui::SliderInt("Bounces", &m_GuiBounces, 0, 10);
		m_Renderer.SetBounces(m_GuiBounces);

		ImGui::End();

		ImGui::Begin("Scene");

		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			ImGui::Text("Object ID: %i", i);

			Sphere& sphere = m_Scene.Spheres[i];

			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Albedo), 0.1f);

			ImGui::PopID();

			ImGui::Separator();
		}

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
				ImVec2(0,1), ImVec2(1,0));

		ImGui::End();

		Render();
	}

	void Render()
	{
		Walnut::Timer timer;

		// Renderer checks for resize
		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);

		// Corrects camera
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);

		// Renderer calls it's render function
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Scene m_Scene;
	Camera m_Camera;
	Renderer m_Renderer;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;

	int m_GuiBounces = 2;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Cpu Raytracer";
	spec.Width = 1280;
	spec.Height = 720;

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