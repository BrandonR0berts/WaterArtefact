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

		void                RenderDebugMenu();

		void                Update(const float deltaTime);
		void                Render(Rendering::Camera* camera, Texture::CubeMapTexture* skybox);

		bool                IsBelowSurface(Maths::Vector::Vector3D<float> position);

		Texture::Texture2D* GetPositionalBuffer()   { return mPositionalBuffer;   }
		Texture::Texture2D* GetPositionalBuffer2()  { return mSecondPositionalBuffer; }
		Texture::Texture2D* GetNormalBuffer()       { return mNormalBuffer;       }
		Texture::Texture2D* GetTangentBuffer()      { return mTangentBuffer;      }
		Texture::Texture2D* GetBinormalBuffer()     { return mBiNormalBuffer;     }
		Texture::Texture2D* GetRandomNumberBuffer() { return mRandomNumberBuffer; }
		Texture::Texture2D* GetButterflyTwiddleTexture() { return mButterflyTexture; }

		Texture::Texture2D* GetH0Buffer()                  { return mH0Buffer; }
		Texture::Texture2D* GetFourierDomainValuesBuffer() { return mFourierDomainValues; }

		void                SetPreset(SimulationMethods approach, char preset);

		int debugPassCount = 0;

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

		void RunInverseFFT();

		// this needs to be re-ran every time the resolution of the heightmap changes
		void CreateButterflyTexture();
		int* GenerateBitReversedIndicies();

		int ReverseBits(int input);

		// ------------------------------------------------------------- //
		// --------------------- Modelling surface --------------------- //
		// ------------------------------------------------------------- //
		 
		// Which approach we are currently displaying and updating
		SimulationMethods              mModellingApproach;

		// Parameters for the realistic ocean simulation
		TessendorfWaveData             mTessendorfData;

		// Buffer holding the verticies of the water's surface
		Buffers::VertexBufferObject*   mWaterVBO;

		// Shader for modeling the movement of waves
		// Writes out the new X-Y-Z position of the verticies to an RGB buffer
		ShaderPrograms::ShaderProgram* mWaterMovementComputeShader_Sine;
		ShaderPrograms::ShaderProgram* mWaterMovementComputeShader_Gerstner;

		// Tessendorf functionality
		ShaderPrograms::ShaderProgram* mGenerateH0_ComputeShader;                 // Create H0 texture
		ShaderPrograms::ShaderProgram* mCreateFrequencyValues_ComputeShader;      // Convert H0 to H(k, t)
		ShaderPrograms::ShaderProgram* mConvertToHeightValues_ComputeShader_FFT;  // Converts from H(k, t) to a height map

		ShaderPrograms::ShaderProgram* mGenerateButterflyFFTData;
		ShaderPrograms::ShaderProgram* mFFTFinalStageProgram;

		// Buffer that holds the world space X-Y-Z 
		Texture::Texture2D*            mPositionalBuffer;
		Texture::Texture2D*            mSecondPositionalBuffer; // Needed for the tessendorf FFT generation process

		// Buffer that holds the normal of the point given out by the computer shader
		Texture::Texture2D*            mNormalBuffer;

		// Buffer that holds the tangent of the point given out by the computer shader
		Texture::Texture2D*            mTangentBuffer;

		// Buffer that holds the BiNormal of the point given out by the computer shader
		Texture::Texture2D*            mBiNormalBuffer;

		// Buffer holding gaussian random numbers for H0 generation
		Texture::Texture2D*            mRandomNumberBuffer;

		Texture::Texture2D*            mH0Buffer;            // H0
		Texture::Texture2D*            mFourierDomainValues; // H(k, t)

		// Texture to hold the multipliers used during the FFT process
		Texture::Texture2D*			   mButterflyTexture;

		// Sine wave modelling data
		std::vector<SingleSineDataSet>      mSineWaveData;
		Buffers::ShaderStorageBufferObject* mSineWaveSSBO;

		// Gerstner wave modelling data
		std::vector<SingleGerstnerWaveData> mGersnterWaveData;
		Buffers::ShaderStorageBufferObject* mGerstnerWaveSSBO;

		// Level of detail - to allow for the ocean to go on forever
		int                                 mLevelOfDetailCount;
		bool                                mUsingLODs;

		float                               mHighestLODDimensions;
		unsigned int                        mDimensions;
		float                               mDistanceBetweenVerticies;

		unsigned int                        mTextureResolution;

		// --------------------- Rendering surface --------------------- //
		Buffers::VertexArrayObject*         mWaterVAO;
		Buffers::ElementBufferObjects*      mWaterEBO;

		// Shader program used for rendering the surface of the water volume
		ShaderPrograms::ShaderProgram*      mSurfaceRenderShaders;

		unsigned int                        mVertexCount;
		unsigned int                        mElementCount;

		RenderingWaterData                  mRenderingData;

		// --------------------- Other --------------------- //

		// If the simuation is being updated
		bool                           mSimulationPaused;
		bool                           mWireframe;

		float                          mRunningTime;

		unsigned int mMemoryBarrierBlockBits;

		const unsigned int             kComputeShaderThreadClusterSize;
	};

	// ---------------------------------------
}