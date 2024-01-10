#pragma once

#include "FFT.h"
#include "Maths/Code/Vector.h"

#include <vector>
#include <string>

namespace Rendering
{
	namespace Texture
	{
		class Texture2D;
	}

	namespace ShaderPrograms
	{
		class ShaderProgram;
	}

	namespace Buffers
	{
		class VertexBufferObject;
		class VertexArrayObject;
	}

	class Camera;

	struct SingleSineDataSet
	{
		SingleSineDataSet()
			: mAmplitude(0.1f)
			, mDirectionOfWave(1.0f, 0.0f)
			, mSpeedOfWave(10.0f)
			, mWaveLength(9.0f)
		{

		}

		float                          mAmplitude;
		Maths::Vector::Vector2D<float> mDirectionOfWave;
		float                          mSpeedOfWave;
		float                          mWaveLength;
	};

	// ---------------------------------------

	class WaterSimulation final
	{
	public:
		WaterSimulation();
		~WaterSimulation();

		void RenderDebugMenu();

		void Update(const float deltaTime);
		void Render(Rendering::Camera* camera);

		bool IsBelowSurface(Maths::Vector::Vector3D<float> position);

	private:
		void PerformanceTesting();

		void SetupBuffers();
		void SetupShaders();
		void SetupTextures();

		Maths::Vector::Vector2D<float>* GenerateVertexData(unsigned int dimensions, float distanceBetweenVertex);

		// --------------------- Modelling surface --------------------- //
		// Buffer holding the verticies of the water's surface
		Buffers::VertexBufferObject*   mWaterVBO;

		// Shader for modeling the movement of waves
		// Writes out the new X-Y-Z position of the verticies to an RGB buffer
		ShaderPrograms::ShaderProgram* mWaterMovementComputeShader_Sine;
		ShaderPrograms::ShaderProgram* mWaterMovementComputeShader_Gerstner;
		ShaderPrograms::ShaderProgram* mWaterMovementComputeShader_Tessendorf;

		ShaderPrograms::ShaderProgram* mActiveWaterModellingApproach;

		// Buffer that holds the X-Y-Z output from the compute shader above
		Texture::Texture2D*            mPositionalBuffer;

		// Buffer that holds the normal of the point given out by the computer shader
		Texture::Texture2D*            mNormalBuffer;

		// Buffer that holds the tangent of the point given out by the computer shader
		Texture::Texture2D*            mTangentBuffer;

		// Buffer that holds the BiNormal of the point given out by the computer shader
		Texture::Texture2D*            mBiNormalBuffer;

		// Different configurations as to how the surface will be being modelled
		std::vector<std::pair<std::string, FFTConfiguration>> mFFTConfigurations;

		float mRunningTime;

		// Single sine wave modelling variables
		SingleSineDataSet mSineWaveData;

		// --------------------- Rendering surface --------------------- //
		Buffers::VertexArrayObject*    mWaterVAO;

		// Shader program used for rendering the surface of the water volume
		ShaderPrograms::ShaderProgram* mSurfaceRenderShaders;

		unsigned int mVertexCount;

		// --------------------- Other --------------------- //

		// If the simuation is being updated
		bool                           mSimulationPaused;
		bool                           mWireframe;
	};

	// ---------------------------------------
}