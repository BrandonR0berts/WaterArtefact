#include "Water.h"

#include "Include/imgui/imgui.h"
#include "Include/imgui/imgui_impl_glfw.h"
#include "Include/imgui/imgui_impl_opengl3.h"

#include "Textures/Texture.h"
#include "Shaders/ShaderProgram.h"
#include "Shaders/Shader.h"

#include "Buffers.h"

#include "Maths/Code/Matrix.h"
#include "Camera.h"

#include "Window.h"
#include "OpenGLRenderPipeline.h"

#include <GLFW/glfw3.h>
#include <random>

namespace Rendering
{
	// ---------------------------------------------

	WaterSimulation::WaterSimulation()
		: mModellingApproach(SimulationMethods::Sine)

		, mTessendorfData()

		, mWaterVBO(nullptr)
		, mWaterMovementComputeShader_Sine(nullptr)
		, mWaterMovementComputeShader_Gerstner(nullptr)

		, mGenerateH0_ComputeShader(nullptr)
		, mCreateFrequencyValues_ComputeShader(nullptr)
		, mConvertToHeightValues_ComputeShader_FFT(nullptr)

		, mPositionalBuffer(nullptr)
		, mSecondPositionalBuffer(nullptr)
		, mNormalBuffer(nullptr)
		, mTangentBuffer(nullptr)
		, mBiNormalBuffer(nullptr)
		, mRandomNumberBuffer(nullptr)
		, mH0Buffer(nullptr)
		, mFourierDomainValues(nullptr)

		, mSineWaveData()
		, mSineWaveSSBO()

		, mGersnterWaveData()
		, mGerstnerWaveSSBO()

		, mLevelOfDetailCount(0)
		, mUsingLODs(true)

		, mHighestLODDimensions(0.0f)

		, mDimensions(100)
		, mDistanceBetweenVerticies(0.3)

		, mTextureResolution(1024) // 1024

		, mWaterVAO(nullptr)
		, mWaterEBO(nullptr)

		, mSurfaceRenderShaders(nullptr)

		, mVertexCount(0)
		, mElementCount(0)

		, mRenderingData()

		, mSimulationPaused(false)
		, mWireframe(false)

		, mRunningTime(0.0f)

		, kComputeShaderThreadClusterSize(32)
	{
		// Compute and final render shaders
		SetupShaders();

		// VBO and VAO
		SetupBuffers();

		// Storage textures
		SetupTextures();

		GenerateH0();
	}

	// ---------------------------------------------

	WaterSimulation::~WaterSimulation()
	{
		// --------------------------------------

		delete mSurfaceRenderShaders;
		mSurfaceRenderShaders = nullptr;

		// --------------------------------------

		delete mWaterMovementComputeShader_Sine;
		mWaterMovementComputeShader_Sine = nullptr;

		delete mWaterMovementComputeShader_Gerstner;
		mWaterMovementComputeShader_Gerstner = nullptr;

		// --------------------------------------

		delete mGenerateH0_ComputeShader;
		mGenerateH0_ComputeShader = nullptr;

		delete mCreateFrequencyValues_ComputeShader;
		mCreateFrequencyValues_ComputeShader = nullptr;

		delete mConvertToHeightValues_ComputeShader_FFT;
		mConvertToHeightValues_ComputeShader_FFT = nullptr;

		// --------------------------------------

		delete mWaterVAO;
		mWaterVAO = nullptr;

		delete mWaterVBO;
		mWaterVBO = nullptr;

		// --------------------------------------

		delete mPositionalBuffer;
		mPositionalBuffer = nullptr;

		delete mSecondPositionalBuffer;
		mSecondPositionalBuffer = nullptr;

		delete mNormalBuffer;
		mNormalBuffer = nullptr;

		delete mBiNormalBuffer;
		mBiNormalBuffer = nullptr;

		delete mTangentBuffer;
		mTangentBuffer = nullptr;

		delete mFourierDomainValues;
		mFourierDomainValues = nullptr;

		delete mRandomNumberBuffer;
		mRandomNumberBuffer = nullptr;

		// --------------------------------------

		delete mSineWaveSSBO;
		mSineWaveSSBO = nullptr;

		delete mGerstnerWaveSSBO;
		mGerstnerWaveSSBO = nullptr;

		// --------------------------------------
	}

	// ---------------------------------------------

	void WaterSimulation::GenerateH0()
	{
		if (!mH0Buffer || !mGenerateH0_ComputeShader || !mRandomNumberBuffer)
			return;

		mGenerateH0_ComputeShader->UseProgram();
			mH0Buffer          ->BindForComputeShader(0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			mRandomNumberBuffer->BindForComputeShader(1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

			mGenerateH0_ComputeShader->SetFloat("gravity",          mTessendorfData.mGravity);
			mGenerateH0_ComputeShader->SetVec2("windVelocity",      mTessendorfData.mWindVelocity);
			mGenerateH0_ComputeShader->SetFloat("phillipsConstant", mTessendorfData.mPhilipsConstant);
			mGenerateH0_ComputeShader->SetVec2("LxLz",              mTessendorfData.mLxLz);

			glDispatchCompute(mTextureResolution / kComputeShaderThreadClusterSize, mTextureResolution / kComputeShaderThreadClusterSize, 1);
	}

	// ---------------------------------------------

	void WaterSimulation::SetupShaders()
	{
		// --------------------------------------------------------------

		if (!mSurfaceRenderShaders)
		{
			mSurfaceRenderShaders = new ShaderPrograms::ShaderProgram();

			Shaders::VertexShader*   vertexShader   = new Shaders::VertexShader("Code/Shaders/Vertex/WaterSurface.vert");
			Shaders::FragmentShader* fragmentShader = new Shaders::FragmentShader("Code/Shaders/Fragment/WaterSurface.frag");

			mSurfaceRenderShaders->AttachShader(vertexShader);
			mSurfaceRenderShaders->AttachShader(fragmentShader);

				mSurfaceRenderShaders->LinkShadersToProgram();

			mSurfaceRenderShaders->DetachShader(vertexShader);
			mSurfaceRenderShaders->DetachShader(fragmentShader);

			delete vertexShader;
			delete fragmentShader;

			mSurfaceRenderShaders->UseProgram();
				mSurfaceRenderShaders->SetInt("positionalBuffer", 0);
				mSurfaceRenderShaders->SetInt("normalBuffer",     1);
				mSurfaceRenderShaders->SetInt("tangentBuffer",    2);
				mSurfaceRenderShaders->SetInt("binormalBuffer",   3);

				mSurfaceRenderShaders->SetVec3("ambientColour", { 0.7765f, 0.902f, 0.9255f });
		}

		// --------------------------------------------------------------

		if(!mWaterMovementComputeShader_Sine)
		{
			mWaterMovementComputeShader_Sine = new ShaderPrograms::ShaderProgram();

			Shaders::ComputeShader* computeShader = new Shaders::ComputeShader("Code/Shaders/Compute/SurfaceUpdate_Sine.comp");

			mWaterMovementComputeShader_Sine->AttachShader(computeShader);

				mWaterMovementComputeShader_Sine->LinkShadersToProgram();

			mWaterMovementComputeShader_Sine->DetachShader(computeShader);

			delete computeShader;
		}

		if(!mWaterMovementComputeShader_Gerstner)
		{
			mWaterMovementComputeShader_Gerstner = new ShaderPrograms::ShaderProgram();

			Shaders::ComputeShader* computeShader = new Shaders::ComputeShader("Code/Shaders/Compute/SurfaceUpdate_Gerstner.comp");

			mWaterMovementComputeShader_Gerstner->AttachShader(computeShader);

				mWaterMovementComputeShader_Gerstner->LinkShadersToProgram();

			mWaterMovementComputeShader_Gerstner->DetachShader(computeShader);

			delete computeShader;
		}

		if (!mGenerateH0_ComputeShader)
		{
			mGenerateH0_ComputeShader = new ShaderPrograms::ShaderProgram();

			Shaders::ComputeShader* computeShader = new Shaders::ComputeShader("Code/Shaders/Compute/GenerateH0_Tessendorf.comp");

			mGenerateH0_ComputeShader->AttachShader(computeShader);

				mGenerateH0_ComputeShader->LinkShadersToProgram();

			mGenerateH0_ComputeShader->DetachShader(computeShader);

			delete computeShader;
		}

		if (!mGenerateH0_ComputeShader)
		{
			mGenerateH0_ComputeShader = new ShaderPrograms::ShaderProgram();

			Shaders::ComputeShader* computeShader = new Shaders::ComputeShader("Code/Shaders/Compute/GenerateH0_Tessendorf.comp");

			mGenerateH0_ComputeShader->AttachShader(computeShader);

				mGenerateH0_ComputeShader->LinkShadersToProgram();

			mGenerateH0_ComputeShader->DetachShader(computeShader);

			delete computeShader;
		}

		if (!mCreateFrequencyValues_ComputeShader)
		{
			mCreateFrequencyValues_ComputeShader = new ShaderPrograms::ShaderProgram();

			Shaders::ComputeShader* computeShader = new Shaders::ComputeShader("Code/Shaders/Compute/GenerateHeight_Tessendorf.comp");

			mCreateFrequencyValues_ComputeShader->AttachShader(computeShader);

				mCreateFrequencyValues_ComputeShader->LinkShadersToProgram();

			mCreateFrequencyValues_ComputeShader->DetachShader(computeShader);

			delete computeShader;
		}

		if (!mConvertToHeightValues_ComputeShader_FFT)
		{
			mConvertToHeightValues_ComputeShader_FFT = new ShaderPrograms::ShaderProgram();

			Shaders::ComputeShader* computeShader = new Shaders::ComputeShader("Code/Shaders/Compute/ConvertFrequencyToWorldHeight.comp");

			mConvertToHeightValues_ComputeShader_FFT->AttachShader(computeShader);

				mConvertToHeightValues_ComputeShader_FFT->LinkShadersToProgram();

			mConvertToHeightValues_ComputeShader_FFT->DetachShader(computeShader);

			delete computeShader;
		}

		mModellingApproach = SimulationMethods::Sine;

		// --------------------------------------------------------------
	}

	// ---------------------------------------------

	void WaterSimulation::SetupBuffers()
	{
		if (!mWaterVBO)
		{
			mWaterVBO = new Buffers::VertexBufferObject();

			Maths::Vector::Vector2D<float>* vertexData = GenerateVertexData(mDimensions, mDistanceBetweenVerticies);

			mWaterVBO->SetBufferData((void*)vertexData, mVertexCount * sizeof(Maths::Vector::Vector2D<float>), GL_STATIC_DRAW);

			// ----------------

			unsigned int halfDimensions             = mDimensions / 2;
			float        startingDistanceFromCentre = (float)halfDimensions * mDistanceBetweenVerticies;

			mHighestLODDimensions = startingDistanceFromCentre;

			if (mSurfaceRenderShaders)
			{
				mSurfaceRenderShaders->UseProgram();
					mSurfaceRenderShaders->SetFloat("maxDistanceFromOrigin", startingDistanceFromCentre);
			}

			// ----------------

			delete[] vertexData;
		}

		if (!mWaterEBO)
		{
			mWaterEBO = new Buffers::ElementBufferObjects();

			unsigned int* elementData = GenerateElementData(mDimensions);

			mWaterEBO->SetBufferData(mElementCount * sizeof(unsigned int), elementData, GL_STATIC_DRAW);

			delete[] elementData;
		}

		if (!mWaterVAO)
		{
			mWaterVAO = new Buffers::VertexArrayObject();

			mWaterVAO->Bind();
			mWaterVBO->Bind();
			mWaterEBO->Bind();

			// Positional data
			mWaterVAO->EnableVertexAttribArray(0);
			mWaterVAO->SetVertexAttributePointers(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0, true);

			mWaterVAO->Unbind();
			mWaterVBO->UnBind();
		}

		if (!mSineWaveSSBO)
		{
			mSineWaveSSBO = new Buffers::ShaderStorageBufferObject();

			UpdateSineWaveDataSet();
		}

		if (!mGerstnerWaveSSBO)
		{
			mGerstnerWaveSSBO = new Buffers::ShaderStorageBufferObject();

			UpdateGerstnerWaveDataSet();
		}
	}

	// ---------------------------------------------

	void WaterSimulation::UpdateSineWaveDataSet()
	{
		         int waveCount   = (int)mSineWaveData.size();
		unsigned int bytesInData = sizeof(SingleSineDataSet) * waveCount;

		SingleSineDataSet* newData = new SingleSineDataSet[waveCount];

		for (int i = 0; i < waveCount; i++)
		{
			newData[i] = mSineWaveData[i];
		}
		
		mSineWaveSSBO->SetBufferData(newData, bytesInData, GL_DYNAMIC_DRAW);

		delete[] newData;
	}

	// ---------------------------------------------

	void WaterSimulation::UpdateGerstnerWaveDataSet()
	{
		         int waveCount   = (int)mGersnterWaveData.size();
		unsigned int bytesInData = sizeof(SingleGerstnerWaveData) * waveCount;

		SingleGerstnerWaveData* newData = new SingleGerstnerWaveData[waveCount];

		for (int i = 0; i < waveCount; i++)
		{
			newData[i] = mGersnterWaveData[i];
		}

		mGerstnerWaveSSBO->SetBufferData(newData, bytesInData, GL_DYNAMIC_DRAW);

		delete[] newData;
	}

	// ---------------------------------------------

	void WaterSimulation::SetupTextures()
	{
		if (!mPositionalBuffer)
		{
			mPositionalBuffer = new Texture::Texture2D();

			mPositionalBuffer->InitEmpty(mTextureResolution, mTextureResolution, true, GL_FLOAT, GL_RGBA32F, GL_RGBA, { GL_LINEAR, GL_NEAREST }, {});
		}

		if (!mSecondPositionalBuffer)
		{
			mSecondPositionalBuffer = new Texture::Texture2D();

			mSecondPositionalBuffer->InitEmpty(mTextureResolution, mTextureResolution, true, GL_FLOAT, GL_RGBA32F, GL_RGBA, { GL_LINEAR, GL_NEAREST }, {});
		}

		if (!mNormalBuffer)
		{
			mNormalBuffer = new Texture::Texture2D();

			mNormalBuffer->InitEmpty(mTextureResolution, mTextureResolution, true, GL_FLOAT, GL_RGBA32F, GL_RGBA);
		}

		if (!mTangentBuffer)
		{
			mTangentBuffer = new Texture::Texture2D();

			mTangentBuffer->InitEmpty(mTextureResolution, mTextureResolution, true, GL_FLOAT, GL_RGBA32F, GL_RGBA);
		}

		if (!mBiNormalBuffer)
		{
			mBiNormalBuffer = new Texture::Texture2D();

			mBiNormalBuffer->InitEmpty(mTextureResolution, mTextureResolution, true, GL_FLOAT, GL_RGBA32F, GL_RGBA);
		}

		if (!mH0Buffer)
		{
			mH0Buffer = new Texture::Texture2D();

			mH0Buffer->InitEmpty(mTextureResolution, mTextureResolution, true, GL_FLOAT, GL_RGBA32F, GL_RGBA);
		}

		if (!mFourierDomainValues)
		{
			mFourierDomainValues = new Texture::Texture2D();

			mFourierDomainValues->InitEmpty(mTextureResolution, mTextureResolution, true, GL_FLOAT, GL_RGBA32F, GL_RGBA);
		}

		if (!mRandomNumberBuffer)
		{
			mRandomNumberBuffer = new Texture::Texture2D();

			Maths::Vector::Vector4D<float>* randomNumberData = GenerateGaussianData();

			mRandomNumberBuffer->InitWithData(mTextureResolution, mTextureResolution, randomNumberData, true, GL_FLOAT, GL_RGBA32F, GL_RGBA);

			delete[] randomNumberData;
		}
	}

	// ---------------------------------------------

	void WaterSimulation::RenderDebugMenu()
	{
		ImGui::Begin("Water Simulation Debug");

			// Pause
			if (ImGui::Button("Toggle Simulation Pause"))
			{
				mSimulationPaused = !mSimulationPaused;
			}

			// Wireframe
			if (ImGui::Button("Toggle Wireframe"))
			{
				mWireframe = !mWireframe;
			}

			// Testing
			if (ImGui::Button("Run performance tests"))
			{
				PerformanceTesting();
			}

			// Swapping approach
			if (ImGui::CollapsingHeader("Modelling approach"))
			{
				if (ImGui::Button("Sine Waves"))
				{
					mModellingApproach = SimulationMethods::Sine;

					mSurfaceRenderShaders->SetBool("renderingSineGeneration", true);
				}

				if (ImGui::Button("Gerstner Waves"))
				{
					mModellingApproach = SimulationMethods::Gerstner;

					mSurfaceRenderShaders->SetBool("renderingSineGeneration", false);
				}

				if (ImGui::Button("Ocean simulation"))
				{
					mModellingApproach = SimulationMethods::Tessendorf;

					mSurfaceRenderShaders->SetBool("renderingSineGeneration", false);
				}
			}

			// LOD
			if (ImGui::InputInt("LOD Count", &mLevelOfDetailCount))
			{
				if (mLevelOfDetailCount < 0)
					mLevelOfDetailCount = 0;
			}

			if (ImGui::DragFloat3("Directional light direction", &mRenderingData.mLightDirection.x, 0.01f, -1.0f, 1.0f))
			{
				mRenderingData.mLightDirection.Normalise();
			}

			// Water colour
			ImGui::DragFloat3("Water colour", &mRenderingData.mWaterColour.x, 0.001f, 0.0f, 2.0f);

			ImGui::DragFloat("Reflection Proportion", &mRenderingData.mReflectionFactor, 0.001f, 0.0f, 1.0f);

			ImGui::DragFloat3("Ambient colour", &mRenderingData.mAmbientColour.x, 0.001f, 0.0f, 1.0f);
		ImGui::End();

		
		ImGui::Begin("Water properties");

			if (mModellingApproach == SimulationMethods::Sine)
			{
				bool changed = false;

				if (ImGui::CollapsingHeader("Sine wave data"))
				{
					unsigned int waveCount = (unsigned int)mSineWaveData.size();
					for (unsigned int i = 0; i < waveCount; i++)
					{
						ImGui::Text("-------------------------------------------------------");

						std::string stringI = std::to_string(i);

						if (ImGui::InputFloat(("Amplitude##Sine" + stringI).c_str(), &mSineWaveData[i].mAmplitude))
							changed = true;

						if (ImGui::InputFloat2(("Direction##Sine" + stringI).c_str(), &mSineWaveData[i].mDirectionOfWave.x))
							changed = true;

						if (ImGui::InputFloat(("Speed##Sine" + stringI).c_str(), &mSineWaveData[i].mSpeedOfWave))
							changed = true;

						if (ImGui::InputFloat(("Wavelength##Sine" + stringI).c_str(), &mSineWaveData[i].mWaveLength))
							changed = true;

						if (ImGui::Button(("Remove wave##Sine" + stringI).c_str()))
						{
							mSineWaveData.erase(mSineWaveData.begin() + i);
							UpdateSineWaveDataSet();

							changed = false;

							break;
						}

						ImGui::Text("-------------------------------------------------------");
					}

					if (ImGui::Button("Add new wave##Sine"))
					{
						mSineWaveData.push_back(SingleSineDataSet());

						changed = true;
					}

					if (changed)
					{
						UpdateSineWaveDataSet();
					}
				}

				if (ImGui::CollapsingHeader("Presets##Sine"))
				{
					if (ImGui::Button("Calm##sine"))
					{
						SetPreset(Rendering::SimulationMethods::Sine, (char)SineWavePresets::Calm);

						mSurfaceRenderShaders->SetBool("renderingSineGeneration", true);
					}

					if (ImGui::Button("Choppy##sine"))
					{
						SetPreset(Rendering::SimulationMethods::Sine, (char)SineWavePresets::Chopppy);

						mSurfaceRenderShaders->SetBool("renderingSineGeneration", true);
					}

					if (ImGui::Button("Strange##sine"))
					{
						SetPreset(Rendering::SimulationMethods::Sine, (char)SineWavePresets::Strange);

						mSurfaceRenderShaders->SetBool("renderingSineGeneration", true);
					}
				}
			}
			else if (mModellingApproach == SimulationMethods::Gerstner)
			{
				bool changed = false;

				if (ImGui::CollapsingHeader("Gerstner wave data"))
				{
					unsigned int waveCount = (unsigned int)mGersnterWaveData.size();
					for (unsigned int i = 0; i < waveCount; i++)
					{
						ImGui::Text("-------------------------------------------------------");

						std::string stringI = std::to_string(i);

						if (ImGui::InputFloat(("Amplitude##Gerstner" + stringI).c_str(), &mGersnterWaveData[i].mAmplitude))
							changed = true;

						if (ImGui::InputFloat2(("Direction##Gerstner" + stringI).c_str(), &mGersnterWaveData[i].mDirectionOfWave.x))
							changed = true;

						if (ImGui::InputFloat(("Speed##Gerstner" + stringI).c_str(), &mGersnterWaveData[i].mSpeedOfWave))
							changed = true;

						if (ImGui::InputFloat(("Wavelength##Gerstner" + stringI).c_str(), &mGersnterWaveData[i].mWaveLength))
							changed = true;

						if (ImGui::InputFloat(("Steepness##Gerstner" + stringI).c_str(), &mGersnterWaveData[i].mSteepness))
							changed = true;

						if (ImGui::Button(("Remove wave##Gerstner" + stringI).c_str()))
						{
							mGersnterWaveData.erase(mGersnterWaveData.begin() + i);
							UpdateGerstnerWaveDataSet();

							changed = false;

							break;
						}

						ImGui::Text("-------------------------------------------------------");
					}

					if (ImGui::Button("Add new wave##Gerstner"))
					{
						mGersnterWaveData.push_back(SingleGerstnerWaveData());

						changed = true;
					}

					if (changed)
					{
						UpdateGerstnerWaveDataSet();
					}
				}

				if (ImGui::CollapsingHeader("Presets##Gerstner"))
				{
					if (ImGui::Button("Calm##Gerstner"))
					{
						SetPreset(Rendering::SimulationMethods::Gerstner, (char)GerstnerWavePresets::Calm);

						mSurfaceRenderShaders->SetBool("renderingSineGeneration", false);
					}

					if (ImGui::Button("Choppy##Gerstner"))
					{
						SetPreset(Rendering::SimulationMethods::Gerstner, (char)GerstnerWavePresets::Chopppy);

						mSurfaceRenderShaders->SetBool("renderingSineGeneration", false);
					}

					if (ImGui::Button("Strange##Gerstner"))
					{
						SetPreset(Rendering::SimulationMethods::Gerstner, (char)GerstnerWavePresets::Strange);

						mSurfaceRenderShaders->SetBool("renderingSineGeneration", false);
					}
				}
			}
			if (mModellingApproach == SimulationMethods::Tessendorf)
			{
				if (ImGui::InputFloat2("Wind Velocity##Tessendorf", &mTessendorfData.mWindVelocity.x))
				{
					GenerateH0();
				}

				if (ImGui::InputFloat("Phillips Constant", &mTessendorfData.mPhilipsConstant))
				{
					GenerateH0();
				}

				ImGui::InputFloat("Gravity##Tessendorf",           &mTessendorfData.mGravity);
				ImGui::InputFloat("Repeat After Time##Tessendorf", &mTessendorfData.mRepeatAfterTime);

				if (ImGui::Button("Defaults##Tessendorf"))
				{
					mTessendorfData = TessendorfWaveData();

					mSurfaceRenderShaders->SetBool("renderingSineGeneration", false);
				}

				if (ImGui::InputFloat2("LxLz", &mTessendorfData.mLxLz.x))
				{
					GenerateH0();
				}
			}

			ImGui::End();
	}

	// ---------------------------------------------

	void WaterSimulation::Update(const float deltaTime)
	{
		if (mSimulationPaused)
			return;

		mRunningTime += deltaTime;

		switch(mModellingApproach)
		{
			case SimulationMethods::Sine:

				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				mWaterMovementComputeShader_Sine->UseProgram();

				mWaterMovementComputeShader_Sine->SetFloat("time", mRunningTime);
			
				if(mSineWaveSSBO)
					mSineWaveSSBO->BindToBufferIndex(5);

				mWaterMovementComputeShader_Sine->SetInt("waveCount", (int)mSineWaveData.size());

				mPositionalBuffer->BindForComputeShader(0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				mNormalBuffer    ->BindForComputeShader(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				mTangentBuffer   ->BindForComputeShader(2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				mBiNormalBuffer  ->BindForComputeShader(3, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

				glDispatchCompute(mTextureResolution / kComputeShaderThreadClusterSize, mTextureResolution / kComputeShaderThreadClusterSize, 1);
			break;

			case SimulationMethods::Gerstner:

				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				mWaterMovementComputeShader_Gerstner->UseProgram();

				if (mGerstnerWaveSSBO)
					mGerstnerWaveSSBO->BindToBufferIndex(5);

				mWaterMovementComputeShader_Gerstner->SetFloat("time", mRunningTime);
				mWaterMovementComputeShader_Gerstner->SetInt("waveCount", (int)mGersnterWaveData.size());

				mPositionalBuffer->BindForComputeShader(0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				mNormalBuffer    ->BindForComputeShader(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				mTangentBuffer   ->BindForComputeShader(2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				mBiNormalBuffer  ->BindForComputeShader(3, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

				glDispatchCompute(mTextureResolution / kComputeShaderThreadClusterSize, mTextureResolution / kComputeShaderThreadClusterSize, 1);

			break;

			case SimulationMethods::Tessendorf:

				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				// Generate the frequency values
				mCreateFrequencyValues_ComputeShader->UseProgram();

					mCreateFrequencyValues_ComputeShader->SetFloat("time",            mRunningTime);
					mCreateFrequencyValues_ComputeShader->SetFloat("gravity",         mTessendorfData.mGravity);
					mCreateFrequencyValues_ComputeShader->SetFloat("repeatAfterTime", mTessendorfData.mRepeatAfterTime);
					mCreateFrequencyValues_ComputeShader->SetVec2("LxLz",             mTessendorfData.mLxLz);

					mFourierDomainValues->BindForComputeShader(0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Output fourier domain values
					mNormalBuffer       ->BindForComputeShader(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Normals
					mTangentBuffer      ->BindForComputeShader(2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Tangent
					mBiNormalBuffer     ->BindForComputeShader(3, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Binormals
					 
					mH0Buffer           ->BindForComputeShader(4, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);  // H0 values created at startup

				glDispatchCompute(mTextureResolution / kComputeShaderThreadClusterSize, mTextureResolution / kComputeShaderThreadClusterSize, 1);

				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				RunInverseFFT();				
				
			break;
		}
	}

	void WaterSimulation::RunInverseFFT()
	{
		// Now convert to world space heights
		// Determine how many passess are needed
		unsigned int passCount = std::log2(mTextureResolution); // * 2 as this is a 2D texture, not a 1D set of data

		mConvertToHeightValues_ComputeShader_FFT->UseProgram();

		mFourierDomainValues   ->BindForComputeShader(0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		mPositionalBuffer      ->BindForComputeShader(1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		mSecondPositionalBuffer->BindForComputeShader(2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		mConvertToHeightValues_ComputeShader_FFT->SetBool("horizontal", true);

		// Horizontal passes
		for (unsigned int i = 0; i < passCount; i++)
		{
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			// Divide by 2 as each shader call stores 2 values
			glDispatchCompute(mTextureResolution / kComputeShaderThreadClusterSize, mTextureResolution / 2 * kComputeShaderThreadClusterSize, 1);
		}

		mConvertToHeightValues_ComputeShader_FFT->SetBool("horizontal", false);

		// Vertical passes
		for (unsigned int i = 0; i < passCount; i++)
		{
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



			// Divide by 2 as each shader call stores 2 values
			glDispatchCompute(mTextureResolution / kComputeShaderThreadClusterSize, mTextureResolution / 2 * kComputeShaderThreadClusterSize, 1);
		}
	}

	// ---------------------------------------------

	void WaterSimulation::Render(Rendering::Camera* camera, Texture::CubeMapTexture* skybox)
	{
		// Existance checks
		if (!mWaterVAO || !mWaterVBO || !mPositionalBuffer || !mSecondPositionalBuffer || !mNormalBuffer || !mTangentBuffer || !mBiNormalBuffer || !mSurfaceRenderShaders)
			return;

		// Rendering of the surface is done through passing the positional texture into the vertex shader to create the final world position
		// Then the fragment shader used information given to it from the other textures output by the compute shader

		// Make sure the compute shader has finished before reading from the textures
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		mWaterVAO->Bind();

		mSurfaceRenderShaders->UseProgram();

			OpenGLRenderPipeline* renderPipeline = ((OpenGLRenderPipeline*)Window::GetRenderPipeline());

			renderPipeline->SetBackFaceCulling(true);

			renderPipeline->SetLineModeEnabled(mWireframe);

			// Textures

			// If we are using the tessendorf approach, and the amount of passes required to fully finish the FFTs is odd, then the final data will be in the second buffer instead of the main one 
			//if(mModellingApproach == SimulationMethods::Tessendorf && (int)std::log2(mTextureResolution) % 2 == 1)
			//	renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mSecondPositionalBuffer->GetTextureID(), true);
			//else
				renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mPositionalBuffer->GetTextureID(), true);

			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE1, mNormalBuffer->GetTextureID(),     true);
			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE2, mTangentBuffer->GetTextureID(),    true);
			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE3, mBiNormalBuffer->GetTextureID(),   true);

			if (skybox)
			{
				renderPipeline->BindTextureToTextureUnit(GL_TEXTURE4, skybox->GetTextureID(), false);
			}

			glm::mat4 identity = glm::mat4(1.0f);
			mSurfaceRenderShaders->SetMat4("modelMat", &identity[0][0]);

			// view and projection matricies from the camera
			glm::mat4 viewMat = camera->GetViewMatrix();
			mSurfaceRenderShaders->SetMat4("viewMat", &viewMat[0][0]);

			glm::mat4 projectionMat = camera->GetPerspectiveMatrix();
			mSurfaceRenderShaders->SetMat4("projectionMat", &projectionMat[0][0]);

			mSurfaceRenderShaders->SetVec3("cameraPosition", camera->GetPosition());

			mSurfaceRenderShaders->SetVec3("directionalLightDirection", mRenderingData.mLightDirection);
			mSurfaceRenderShaders->SetFloat("reflectionProportion",     mRenderingData.mReflectionFactor);
			mSurfaceRenderShaders->SetVec3("waterColour",               mRenderingData.mWaterColour);
			mSurfaceRenderShaders->SetVec3("ambientColour",             mRenderingData.mAmbientColour);

			// ------------------------------------------------------------------------------------------------

			// Determine if the camera is below the surface of the water, and if so then we need to flip the culling order
			if(IsBelowSurface(camera->GetPosition()))
			{
				glCullFace(GL_FRONT);
			}
			else
			{
				glCullFace(GL_BACK);
			}

			// Now handle the LODs
			for (int i = 0; i <= mLevelOfDetailCount; i++)
			{
				float                          LODscaleFactor = std::powf(3.0f, i);
				float                          dimensions     = (mHighestLODDimensions * LODscaleFactor) * 2.0f;

				Maths::Vector::Vector2D<float> backLeftPos    = Maths::Vector::Vector2D<float>(-dimensions, -dimensions);

				for (int j = 0; j < 9; j++)
				{
					// Skip the middle one
					if (j == 4 && i != 0)
						continue;

					// See if this is visible to the camera

					// Create the model matrix for this LOD so that it is the right scale
					Maths::Matrix::Matrix4X4 modelMat = Maths::Matrix::Matrix4X4();

					modelMat.scaleX(LODscaleFactor);
					modelMat.scaleZ(LODscaleFactor);

					unsigned int row    = j / 3;
					unsigned int column = j % 3;

					modelMat.transform(Maths::Vector::Vector3D<float>(backLeftPos.x + (column * dimensions), 0.0f, backLeftPos.y + (row * dimensions)));

					mSurfaceRenderShaders->SetMat4("modelMat", &modelMat[0]);

					mSurfaceRenderShaders->SetFloat("textureCoordScale", LODscaleFactor);

					// Draw the LOD
					glDrawElements(GL_TRIANGLES, mElementCount, GL_UNSIGNED_INT, 0);

					ASSERTMSG(glGetError() != 0, "?");
				}
			}

			// ------------------------------------------------------------------------------------------------

			renderPipeline->SetLineModeEnabled(false);
			renderPipeline->SetBackFaceCulling(true);

		mWaterVAO->Unbind();
	}

	// ---------------------------------------------

	void WaterSimulation::PerformanceTesting()
	{

	}

	// ---------------------------------------------

	bool WaterSimulation::IsBelowSurface(Maths::Vector::Vector3D<float> position)
	{
		if (!mPositionalBuffer)
			return false;

		// See if the position is out of bounds of the data

		// Get the offset stored in the buffer

		// See if the position passed in is below or above the value from the buffer
		

		return false;
	}

	// ---------------------------------------------

	Maths::Vector::Vector2D<float>* WaterSimulation::GenerateVertexData(unsigned int dimensions, float distanceBetweenVertex)
	{		
		mVertexCount = (dimensions + 1) * (dimensions + 1);
		Maths::Vector::Vector2D<float>* newData = new Maths::Vector::Vector2D<float>[mVertexCount];

		unsigned int halfDimensions = dimensions / 2;

		float                          startingDistanceFromCentre = halfDimensions * distanceBetweenVertex;
		Maths::Vector::Vector2D<float> topLeftXPos                = Maths::Vector::Vector2D(-startingDistanceFromCentre, -startingDistanceFromCentre);

		unsigned int currentVertexID = 0;

		// Looped in reverse to get the winding order correct
		for (int z = dimensions; z >= 0; z--)
		{
			for (int x = 0; x <= dimensions; x++)
			{
				newData[currentVertexID++] = Maths::Vector::Vector2D<float>(topLeftXPos.x + (x * distanceBetweenVertex), topLeftXPos.y + (z * distanceBetweenVertex));
			}
		}

		return newData;
	}

	// ---------------------------------------------

	unsigned int* WaterSimulation::GenerateElementData(unsigned int dimensions)
	{
		mElementCount = (dimensions) * (dimensions) * 6;
		unsigned int* elementData = new unsigned int[mElementCount];

		unsigned int elementIndex = 0;
		for (unsigned int z = 0; z < dimensions; z++)
		{
			unsigned int startOfRow     = (dimensions + 1) * z;
			unsigned int startOfNextRow = startOfRow + (dimensions + 1);

			for (unsigned int x = 0; x < dimensions; x++)
			{
				elementData[elementIndex++] = startOfRow     + x;
				elementData[elementIndex++] = startOfRow     + (x + 1);
				elementData[elementIndex++] = startOfNextRow + x;

				elementData[elementIndex++] = startOfRow     + (x + 1);
				elementData[elementIndex++] = startOfNextRow + (x + 1);
				elementData[elementIndex++] = startOfNextRow + x;
			}
		}

		return elementData;
	}

	// ---------------------------------------------

	void WaterSimulation::SetPreset(SimulationMethods approach, char preset)
	{
		switch (approach)
		{
		case SimulationMethods::Sine:
			mModellingApproach = SimulationMethods::Sine;

			mSineWaveData.clear();

			switch ((SineWavePresets)preset)
			{
			case SineWavePresets::Calm:
				mSineWaveData.push_back(SingleSineDataSet(0.1f, {1.0f, 0.0f}, 10.0f, 30.0f));
				mSineWaveData.push_back(SingleSineDataSet(0.1f, { 0.05f, 1.0f }, 15.0f, 15.0f));
				mSineWaveData.push_back(SingleSineDataSet(0.03f, { 0.6f, 0.4f }, 50.0f, 13.0f));
			break;
			
			case SineWavePresets::Chopppy:
				mSineWaveData.push_back(SingleSineDataSet(0.2f, { 0.5f, 0.5f }, 5.0f, 60.0f));
				mSineWaveData.push_back(SingleSineDataSet(0.25f, { 0.05f, 1.0f }, 15.0f, 100.0f));
				mSineWaveData.push_back(SingleSineDataSet(0.05f, { 0.6f, 0.4f }, 50.0f, 13.0f));
			break;

			case SineWavePresets::Strange:
				mSineWaveData.push_back(SingleSineDataSet(0.6f, { 1.0f, 0.0f }, 5.0f, 60.0f));
				mSineWaveData.push_back(SingleSineDataSet(0.5f, { 0.05f, 1.0f }, 30.0f, 126.0f));
				mSineWaveData.push_back(SingleSineDataSet(0.05f, { 0.2f, 0.6f }, 12.0f, 15.0f));
			break;

			default:
			return;
			}

			UpdateSineWaveDataSet();
		break;

		case SimulationMethods::Gerstner:
			mModellingApproach = SimulationMethods::Gerstner;

			mGersnterWaveData.clear();

			switch ((GerstnerWavePresets)preset)
			{
			case GerstnerWavePresets::Calm:
				mGersnterWaveData.push_back(SingleGerstnerWaveData(0.13f, {0.5f, 0.1f}, 15.0f, 50.0f, 0.2f));
				mGersnterWaveData.push_back(SingleGerstnerWaveData(0.1f, {0.3f, 0.4f}, 6.0f, 100.0f, 0.1f));
				mGersnterWaveData.push_back(SingleGerstnerWaveData(0.1f, {1.0f, 0.0f}, 30.0f, 150.0f, 5.0f));
			break;
			
			case GerstnerWavePresets::Chopppy:
				mGersnterWaveData.push_back(SingleGerstnerWaveData(0.2f, { 0.5f, 0.1f }, 6.0f, 15.0f, 0.1f));
				mGersnterWaveData.push_back(SingleGerstnerWaveData(0.1f, { 1.0f, 1.0f}, 30.0f, 15.0f, 0.3f));
				mGersnterWaveData.push_back(SingleGerstnerWaveData(0.15f, { 0.5f, 0.1f }, 6.0f, 20.0f, 0.3f));
			break;
				
			case GerstnerWavePresets::Strange:
				mGersnterWaveData.push_back(SingleGerstnerWaveData(0.2f, { 0.5f, 0.1f }, 3.0f, 30.0f, 0.5f));
				mGersnterWaveData.push_back(SingleGerstnerWaveData(0.3f, { 0.0f, 1.0f }, 15.0f, 40.0f, 0.1f));
				mGersnterWaveData.push_back(SingleGerstnerWaveData(0.15f, { 0.0f, 1.0f }, 6.0f, 15.0f, 1.0f));
			break;

			default:
			return;
			}

			UpdateGerstnerWaveDataSet();
		break;

		case SimulationMethods::Tessendorf:
			mModellingApproach = SimulationMethods::Tessendorf;
		break;

		default:
		return;
		}
	}

	// ---------------------------------------------

	Maths::Vector::Vector4D<float>* WaterSimulation::GenerateGaussianData()
	{
		unsigned int                    pixelsOnScreen = mTextureResolution * mTextureResolution;
		Maths::Vector::Vector4D<float>* returnData     = new Maths::Vector::Vector4D<float>[pixelsOnScreen];

		// Used for generating a properly random seed
		std::random_device rd;

		// This is the seed
		std::mt19937 gen(rd());

		// Set the mean to 0.5 and standard deviation to being 0.5, so most values will fall within the 0.0 -> 1.0 range
		// the ones that dont will be capped to 0 and 1
		std::normal_distribution<float> normalDistibution{0.5, 0.5};

		float randomValue1, randomValue2;
		unsigned int currentIndex = 0;

		for (unsigned int y = 0; y < mTextureResolution; y++)
		{
			for (unsigned int x = 0; x < mTextureResolution; x++)
			{
				currentIndex = x + (y * mTextureResolution);

				// Generate the two random numbers for this pixel
				randomValue1 = normalDistibution(gen);
				randomValue2 = normalDistibution(gen);

				returnData[currentIndex] = Maths::Vector::Vector4D<float>(randomValue1, randomValue2, 0.0f, 0.0f);
			}
		}

		return returnData;
	}

	// ---------------------------------------------

}