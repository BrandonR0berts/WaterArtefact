#pragma once

#include "Maths/Code/Vector.h"
#include "Rendering/Code/WaterStructures.h"

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

	class WaterSimulation final
	{
	public:
		WaterSimulation();
		~WaterSimulation();

		void               RenderDebugMenu();

		void                Update(const float deltaTime);
		void                Render(Rendering::Camera* camera, Texture::CubeMapTexture* skybox);

		bool                IsBelowSurface(Maths::Vector::Vector3D<float> position);

		Texture::Texture2D* GetPositionalBuffer()   { return mPositionalBuffer; }
		Texture::Texture2D* GetPositionalBuffer2()  { return mSecondPositionalBuffer; }
		Texture::Texture2D* GetNormalBuffer()       { return mNormalBuffer;     }
		Texture::Texture2D* GetTangentBuffer()      { return mTangentBuffer;    }
		Texture::Texture2D* GetBinormalBuffer()     { return mBiNormalBuffer;   }
		Texture::Texture2D* GetRandomNumberBuffer() { return mRandomNumberBuffer; }

		Texture::Texture2D* GetH0Buffer()                  { return mH0Buffer; }
		Texture::Texture2D* GetFourierDomainValuesBuffer() { return mFourierDomainValues; }

		void                SetPreset(SimulationMethods approach, char preset);

	private:
		void PerformanceTesting();

		void SetupBuffers();
		void SetupShaders();
		void SetupTextures();

		void GenerateH0();

		void UpdateSineWaveDataSet();
		void UpdateGerstnerWaveDataSet();

		Maths::Vector::Vector2D<float>*         GenerateVertexData(unsigned int dimensions, float distanceBetweenVertex);
		unsigned int*                           GenerateElementData(unsigned int dimensions);
		Maths::Vector::Vector4D<float>*         GenerateGaussianData();

		unsigned char ConvertToUnsignedChar(float value);

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
		ShaderPrograms::ShaderProgram* mBruteForceFFT;

		SimulationMethods              mModellingApproach;

		// Buffer that holds the world space X-Y-Z 
		Texture::Texture2D*            mPositionalBuffer;
		Texture::Texture2D*            mSecondPositionalBuffer; // Needed for the tessendorf FFT generation

		Texture::Texture2D*            mH0Buffer;
		Texture::Texture2D*            mFourierDomainValues;

		// Buffer that holds the normal of the point given out by the computer shader
		Texture::Texture2D*            mNormalBuffer;

		// Buffer that holds the tangent of the point given out by the computer shader
		Texture::Texture2D*            mTangentBuffer;

		// Buffer that holds the BiNormal of the point given out by the computer shader
		Texture::Texture2D*            mBiNormalBuffer;

		// Buffer holding gaussian random numbers for H0 generation
		Texture::Texture2D*            mRandomNumberBuffer;

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

		float        mPhilipsConstant;

		// --------------------- Rendering surface --------------------- //
		Buffers::VertexArrayObject*    mWaterVAO;

		Buffers::ElementBufferObjects* mWaterEBO;

		// Shader program used for rendering the surface of the water volume
		ShaderPrograms::ShaderProgram* mSurfaceRenderShaders;

		unsigned int mVertexCount;
		unsigned int mElementCount;

		RenderingWaterData        mRenderingData;

		// --------------------- Other --------------------- //

		// If the simuation is being updated
		bool                           mSimulationPaused;
		bool                           mWireframe;

		bool                           mBruteForce;

		Maths::Vector::Vector2D<float> mLxLz;
	};

	// ---------------------------------------
}