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
		class ShaderStorageBufferObject;
	}

	class Camera;

	// ---------------------------------------

	struct SingleSineDataSet final
	{
		SingleSineDataSet()
			: mAmplitude(0.1f)
			, mSteepnessFactor(1.0f)
			, mWaveLength(9.0f)
			, mSpeedOfWave(10.0f)
			, mDirectionOfWave(1.0f, 0.0f)
			, padding1(0.0f)
			, padding2(0.0f)
		{

		}

		float                          mAmplitude;
		float                          mSteepnessFactor;
		float                          mWaveLength;
		float                          mSpeedOfWave;
		Maths::Vector::Vector2D<float> mDirectionOfWave;
		float padding1;
		float padding2;
	};

	struct SingleGerstnerWaveData final
	{
		SingleGerstnerWaveData()
			: mAmplitude(0.3f)
			, mSteepness(0.3f)
			, mSpeedOfWave(6.0f)
			, mWaveLength(20.0f)
			, mDirectionOfWave(0.5f, 0.1f)
			, mPadding()
		{

		}

		float                          mAmplitude;
		float                          mSteepness;
		float                          mSpeedOfWave;
		float                          mWaveLength;
		Maths::Vector::Vector2D<float> mDirectionOfWave;
		Maths::Vector::Vector2D<float> mPadding;
	};

	struct TessendorfWaveData final
	{
		TessendorfWaveData()
			: mWaveCount(5)
			, mWindVelocity(15.0f, 0.0f)
			, mGravity(9.81f)
			, mRepeatAfterTime(10.0f)
		{

		}

		int                            mWaveCount;
		Maths::Vector::Vector2D<float> mWindVelocity;
		float                          mGravity;
		float                          mRepeatAfterTime;
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

		Texture::Texture2D* GetPositionalBuffer() { return mPositionalBuffer; }
		Texture::Texture2D* GetNormalBuffer()     { return mNormalBuffer;     }
		Texture::Texture2D* GetTangentBuffer()    { return mTangentBuffer;    }
		Texture::Texture2D* GetBinormalBuffer()   { return mBiNormalBuffer;   }

	private:
		void PerformanceTesting();

		void SetupBuffers();
		void SetupShaders();
		void SetupTextures();

		void UpdateSineWaveDataSet();
		void UpdateGerstnerWaveDataSet();

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
		ShaderPrograms::ShaderProgram* mActiveWaterRenderingApproach;

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
		std::vector<SingleSineDataSet>      mSineWaveData;
		Buffers::ShaderStorageBufferObject* mSineWaveSSBO;

		std::vector<SingleGerstnerWaveData> mGersnterWaveData;
		Buffers::ShaderStorageBufferObject* mGerstnerWaveSSBO;

		TessendorfWaveData     mTessendorfData;

		// --------------------- Rendering surface --------------------- //
		Buffers::VertexArrayObject*    mWaterVAO;

		// Shader program used for rendering the surface of the water volume
		ShaderPrograms::ShaderProgram* mSurfaceRenderShaders_Sine;
		ShaderPrograms::ShaderProgram* mSurfaceRenderShaders_Gersnter;
		ShaderPrograms::ShaderProgram* mSurfaceRenderShaders_Tessendorf;

		unsigned int mVertexCount;

		// --------------------- Other --------------------- //

		// If the simuation is being updated
		bool                           mSimulationPaused;
		bool                           mWireframe;
	};

	// ---------------------------------------
}