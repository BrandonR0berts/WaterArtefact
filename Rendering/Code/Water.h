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
		class CubeMapTexture;
	}

	namespace ShaderPrograms
	{
		class ShaderProgram;
	}

	namespace Buffers
	{
		class VertexBufferObject;
		class VertexArrayObject;
		class ElementBufferObjects;
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

		SingleSineDataSet(float amplitude, Maths::Vector::Vector2D<float> direction, float speed, float wavelength)
			: mAmplitude(amplitude)
			, mSteepnessFactor(1.0f)
			, mWaveLength(wavelength)
			, mSpeedOfWave(speed)
			, mDirectionOfWave(direction)
			, padding1(0.0f)
			, padding2(0.0f)
		{

		}

		float                          mAmplitude;
		float                          mSteepnessFactor;
		float                          mWaveLength;
		float                          mSpeedOfWave;
		Maths::Vector::Vector2D<float> mDirectionOfWave;
		float                          padding1;
		float                          padding2;
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

		SingleGerstnerWaveData(float amplitude, Maths::Vector::Vector2D<float> direction, float speed, float wavelength, float steepness)
			: mAmplitude(amplitude)
			, mSteepness(steepness)
			, mSpeedOfWave(speed)
			, mWaveLength(wavelength)
			, mDirectionOfWave(direction)
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
			: mWindVelocity(0.0, 0.0f)
			, mGravity(9.81f)
			, mRepeatAfterTime(10.0f)
		{

		}

		Maths::Vector::Vector2D<float> mWindVelocity;
		float                          mGravity;
		float                          mRepeatAfterTime;
	};

	// ---------------------------------------

	enum class SimulationMethods 
	{
		Sine,
		Gerstner,
		Tessendorf
	};

	enum class SineWavePresets : char
	{
		Calm,
		Chopppy,
		Strange
	};

	enum class GerstnerWavePresets : char
	{
		Calm,
		Chopppy,
		Strange
	};

	class WaterSimulation final
	{
	public:
		WaterSimulation();
		~WaterSimulation();

		void RenderDebugMenu();

		void Update(const float deltaTime);
		void Render(Rendering::Camera* camera, Texture::CubeMapTexture* skybox);

		bool IsBelowSurface(Maths::Vector::Vector3D<float> position);

		Texture::Texture2D* GetPositionalBuffer() { return mPositionalBuffer; }
		Texture::Texture2D* GetNormalBuffer()     { return mNormalBuffer;     }
		Texture::Texture2D* GetTangentBuffer()    { return mTangentBuffer;    }
		Texture::Texture2D* GetBinormalBuffer()   { return mBiNormalBuffer;   }

		Texture::Texture2D* GetH0Buffer()                  { return mH0Buffer; }
		Texture::Texture2D* GetFourierDomainValuesBuffer() { return mFourierDomainValues; }

		void SetPreset(SimulationMethods approach, char preset);

	private:
		void PerformanceTesting();

		void SetupBuffers();
		void SetupShaders();
		void SetupTextures();

		void GenerateH0();

		void UpdateSineWaveDataSet();
		void UpdateGerstnerWaveDataSet();

		Maths::Vector::Vector2D<float>* GenerateVertexData(unsigned int dimensions, float distanceBetweenVertex);
		unsigned int* GenerateElementData(unsigned int dimensions);

		// --------------------- Modelling surface --------------------- //
		// Buffer holding the verticies of the water's surface
		Buffers::VertexBufferObject*   mWaterVBO;

		// Shader for modeling the movement of waves
		// Writes out the new X-Y-Z position of the verticies to an RGB buffer
		ShaderPrograms::ShaderProgram* mWaterMovementComputeShader_Sine;
		ShaderPrograms::ShaderProgram* mWaterMovementComputeShader_Gerstner;

		ShaderPrograms::ShaderProgram* mGenerateH0_ComputeShader;
		ShaderPrograms::ShaderProgram* mCreateFrequencyValues_ComputeShader;
		ShaderPrograms::ShaderProgram* mConvertToHeightValues_ComputeShader;

		SimulationMethods mModellingApproach;

		// Buffer that holds the world space X-Y-Z 
		Texture::Texture2D*            mPositionalBuffer;

		Texture::Texture2D*            mH0Buffer;
		Texture::Texture2D*            mFourierDomainValues;

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

		TessendorfWaveData                  mTessendorfData;

		// Level of detail - to allow for the ocean to go on forever
		int                                 mLevelOfDetailCount;
		bool                                mUsingLODs;

		float                               mHighestLODDimensions;
		unsigned int                        mDimensions;
		float                               mDistanceBetweenVerticies;

		unsigned int mTextureResolution;

		// --------------------- Rendering surface --------------------- //
		Buffers::VertexArrayObject*    mWaterVAO;

		Buffers::ElementBufferObjects* mWaterEBO;

		// Shader program used for rendering the surface of the water volume
		ShaderPrograms::ShaderProgram* mSurfaceRenderShaders;

		unsigned int mVertexCount;
		unsigned int mElementCount;

		// --------------------- Other --------------------- //

		// If the simuation is being updated
		bool                           mSimulationPaused;
		bool                           mWireframe;
	};

	// ---------------------------------------
}