#pragma once

#include "Buffers.h"

#include "Shaders/Shader.h"
#include "Shaders/ShaderProgram.h"

#include "FFT.h"
#include "Maths/Code/Vector.h"

#include <vector>
#include <string>

namespace Rendering
{
	// ---------------------------------------

	class WaterSimulation final
	{
	public:
		WaterSimulation();
		~WaterSimulation();

		void RenderDebugMenu();

		void Update(const float deltaTime);
		void Render();

		bool IsBelowSurface(Maths::Vector::Vector3D<float> position);

	private:
		void PerformanceTesting();

		Buffers::VertexArrayObject*    mWaterVAO;
		Buffers::VertexBufferObject*   mWaterVBO;

		// Shader for modeling the movement of waves
		Shaders::ComputeShader*        mWaterMovementComputeShader;

		// Shader program used for rendering the surface of the water volume
		ShaderPrograms::ShaderProgram* mSurfaceRenderShaders;

		std::vector<std::pair<std::string, FFTConfiguration>> mFFTConfigurations;

		bool                           mSimulationPaused;
		bool                           mWireframe;
	};

	// ---------------------------------------
}