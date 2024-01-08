#include "Water.h"

#include "Include/imgui/imgui.h"
#include "Include/imgui/imgui_impl_glfw.h"
#include "Include/imgui/imgui_impl_opengl3.h"

namespace Rendering
{
	// ---------------------------------------------

	WaterSimulation::WaterSimulation()
		: mWaterVAO(nullptr)
		, mWaterVBO(nullptr)
		, mWaterMovementComputeShader(nullptr)
		, mSurfaceRenderShaders(nullptr)
		, mFFTConfigurations()
		, mSimulationPaused(false)
		, mWireframe(false)
	{
		// -------------------------------------------------------------

		mSurfaceRenderShaders = new ShaderPrograms::ShaderProgram();

		Shaders::VertexShader*   vertexShader   = new Shaders::VertexShader("WaterArtefact/Code/Shaders/Vertex/WaterSurface.vert");
		Shaders::FragmentShader* fragmentShader = new Shaders::FragmentShader("WaterArtefact/Code/Shaders/Fragment/WaterSurface.frag");

		mSurfaceRenderShaders->AttachShader(vertexShader);
		mSurfaceRenderShaders->AttachShader(fragmentShader);

		mSurfaceRenderShaders->LinkShadersToProgram();

		mSurfaceRenderShaders->DetachShader(vertexShader);
		mSurfaceRenderShaders->DetachShader(fragmentShader);

		delete vertexShader;
		delete fragmentShader;

		// -------------------------------------------------------------



		// -------------------------------------------------------------
	}

	// ---------------------------------------------

	WaterSimulation::~WaterSimulation()
	{
		delete mSurfaceRenderShaders;
		mSurfaceRenderShaders = nullptr;

		delete mWaterVAO;
		mWaterVAO = nullptr;

		delete mWaterVBO;
		mWaterVBO = nullptr;

		mFFTConfigurations.clear();

		delete mWaterMovementComputeShader;
		mWaterMovementComputeShader = nullptr;
	}

	// ---------------------------------------------

	void WaterSimulation::RenderDebugMenu()
	{
		ImGui::Begin("Water Simulation Debug");

			if (ImGui::Button("Toggle Simulation Pause"))
			{
				mSimulationPaused = !mSimulationPaused;
			}

			if (ImGui::Button("Toggle Wireframe"))
			{
				mWireframe = !mWireframe;
			}

			if (ImGui::Button("Run performance tests"))
			{
				PerformanceTesting();
			}

		ImGui::End();
	}

	// ---------------------------------------------

	void WaterSimulation::Update(const float deltaTime)
	{
		if (mSimulationPaused)
			return;

		// Update the movement of the water's surface

	}

	// ---------------------------------------------

	void WaterSimulation::Render()
	{
		
	}

	// ---------------------------------------------

	void WaterSimulation::PerformanceTesting()
	{

	}

	// ---------------------------------------------

	bool WaterSimulation::IsBelowSurface(Maths::Vector::Vector3D<float> position)
	{
		return false;
	}

	// ---------------------------------------------
}