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

namespace Rendering
{
	// ---------------------------------------------

	WaterSimulation::WaterSimulation()
		: mWaterVBO(nullptr)
		, mWaterMovementComputeShader_Sine(nullptr)
		, mWaterMovementComputeShader_Gerstner(nullptr)
		, mWaterMovementComputeShader_Tessendorf(nullptr)

		, mActiveWaterModellingApproach(nullptr)

		, mPositionalBuffer(nullptr)
		, mNormalBuffer(nullptr)
		, mTangentBuffer(nullptr)
		, mBiNormalBuffer(nullptr)

		, mFFTConfigurations()
		, mRunningTime(0.0f)

		, mSineWaveData()
		, mSineWaveSSBO()

		, mGersnterWaveData()
		, mGerstnerWaveSSBO()

		, mTessendorfData()

		, mLevelOfDetailCount(0)
		, mUsingLODs(true)

		, mWaterVAO(nullptr)

		, mSurfaceRenderShaders(nullptr)

		, mSimulationPaused(false)
		, mWireframe(false)

		, mWaterEBO(nullptr)
		, mElementCount(0)
	{
		// Compute and final render shaders
		SetupShaders();

		// VBO and VAO
		SetupBuffers();

		// Storage textures
		SetupTextures();
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

		delete mWaterMovementComputeShader_Tessendorf;
		mWaterMovementComputeShader_Tessendorf = nullptr;

		// --------------------------------------

		mActiveWaterModellingApproach = nullptr;

		// --------------------------------------

		delete mWaterVAO;
		mWaterVAO = nullptr;

		delete mWaterVBO;
		mWaterVBO = nullptr;

		// --------------------------------------

		mFFTConfigurations.clear();

		// --------------------------------------

		delete mPositionalBuffer;
		mPositionalBuffer = nullptr;

		delete mNormalBuffer;
		mNormalBuffer = nullptr;

		delete mBiNormalBuffer;
		mBiNormalBuffer = nullptr;

		delete mTangentBuffer;
		mTangentBuffer = nullptr;

		// --------------------------------------

		delete mSineWaveSSBO;
		mSineWaveSSBO = nullptr;

		delete mGerstnerWaveSSBO;
		mGerstnerWaveSSBO = nullptr;

		// --------------------------------------
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
				mSurfaceRenderShaders->SetInt("normalBuffer", 1);
				mSurfaceRenderShaders->SetInt("tangentBuffer", 2);
				mSurfaceRenderShaders->SetInt("binormalBuffer", 3);
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

		if (!mWaterMovementComputeShader_Tessendorf)
		{
			mWaterMovementComputeShader_Tessendorf = new ShaderPrograms::ShaderProgram();

			Shaders::ComputeShader* computeShader = new Shaders::ComputeShader("Code/Shaders/Compute/SurfaceUpdate_Tessendorf.comp");

			mWaterMovementComputeShader_Tessendorf->AttachShader(computeShader);

				mWaterMovementComputeShader_Tessendorf->LinkShadersToProgram();

			mWaterMovementComputeShader_Tessendorf->DetachShader(computeShader);

			delete computeShader;
		}

		mActiveWaterModellingApproach = mWaterMovementComputeShader_Sine;

		// --------------------------------------------------------------
	}

	// ---------------------------------------------

	void WaterSimulation::SetupBuffers()
	{
		unsigned int dimensions = 3;

		if (!mWaterVBO)
		{
			mWaterVBO = new Buffers::VertexBufferObject();

			float        distanceBetweenPoints = 2.5f;
			Maths::Vector::Vector2D<float>* vertexData = GenerateVertexData(dimensions, distanceBetweenPoints);

			mWaterVBO->SetBufferData((void*)vertexData, mVertexCount * sizeof(Maths::Vector::Vector2D<float>), GL_STATIC_DRAW);

			// ----------------

			bool         evenSplit                  = dimensions % 2 == 0;
			unsigned int halfDimensions             = dimensions / 2;
			float        startingDistanceFromCentre = evenSplit ? (halfDimensions * distanceBetweenPoints) + 0.5f : halfDimensions * distanceBetweenPoints;

			mHighestLODDimensions = 2.0f * startingDistanceFromCentre;

			if (mSurfaceRenderShaders)
			{
				mSurfaceRenderShaders->UseProgram();
					mSurfaceRenderShaders->SetFloat("maxDistanceFromOrigin", startingDistanceFromCentre);
			}

			if (mWaterMovementComputeShader_Tessendorf)
			{
				mWaterMovementComputeShader_Tessendorf->UseProgram();
					mWaterMovementComputeShader_Tessendorf->SetFloat("maxDistanceFromOrigin", startingDistanceFromCentre);
			}

			// ----------------

			delete[] vertexData;
		}

		if (!mWaterEBO)
		{
			mWaterEBO = new Buffers::ElementBufferObjects();

			unsigned int* elementData = GenerateElementData(dimensions);

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
		unsigned int width  = 1000;
		unsigned int height = 1000;

		if (!mPositionalBuffer)
		{
			mPositionalBuffer = new Texture::Texture2D();

			mPositionalBuffer->InitEmpty(width, height, true, GL_FLOAT, GL_RGBA32F, GL_RGBA, { GL_LINEAR, GL_NEAREST }, {});
		}

		if (!mNormalBuffer)
		{
			mNormalBuffer = new Texture::Texture2D();

			mNormalBuffer->InitEmpty(width, height, true, GL_FLOAT, GL_RGBA32F, GL_RGBA);
		}

		if (!mTangentBuffer)
		{
			mTangentBuffer = new Texture::Texture2D();

			mTangentBuffer->InitEmpty(width, height, true, GL_FLOAT, GL_RGBA32F, GL_RGBA);
		}

		if (!mBiNormalBuffer)
		{
			mBiNormalBuffer = new Texture::Texture2D();

			mBiNormalBuffer->InitEmpty(width, height, true, GL_FLOAT, GL_RGBA32F, GL_RGBA);
		}
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

			if (ImGui::CollapsingHeader("Modelling approach"))
			{
				if (ImGui::Button("Sine Waves"))
				{
					mActiveWaterModellingApproach = mWaterMovementComputeShader_Sine;
				}

				if (ImGui::Button("Gerstner Waves"))
				{
					mActiveWaterModellingApproach = mWaterMovementComputeShader_Gerstner;
				}

				if (ImGui::Button("Ocean simulation"))
				{
					mActiveWaterModellingApproach = mWaterMovementComputeShader_Tessendorf;
				}
			}

			if (ImGui::InputInt("LOD Count", &mLevelOfDetailCount))
			{
				if (mLevelOfDetailCount < 1)
					mLevelOfDetailCount = 1;
			}
		ImGui::End();

		if(mActiveWaterModellingApproach)
		{
			ImGui::Begin("Water properties");

				if (mActiveWaterModellingApproach == mWaterMovementComputeShader_Sine)
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
				}
				else if (mActiveWaterModellingApproach == mWaterMovementComputeShader_Gerstner)
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
				}
				else if (mActiveWaterModellingApproach == mWaterMovementComputeShader_Tessendorf)
				{
					ImGui::InputInt("Wave count##Tessendorf",          &mTessendorfData.mWaveCount);
					ImGui::InputFloat2("Wind Velocity##Tessendorf",    &mTessendorfData.mWindVelocity.x);
					ImGui::InputFloat("Gravity##Tessendorf",           &mTessendorfData.mGravity);
					ImGui::InputFloat("Repeat After Time##Tessendorf", &mTessendorfData.mRepeatAfterTime);

					if (ImGui::Button("Defaults##Tessendorf"))
					{
						mTessendorfData = TessendorfWaveData();
					}
				}

			ImGui::End();
		}
	}

	// ---------------------------------------------

	void WaterSimulation::Update(const float deltaTime)
	{
		if (mSimulationPaused || !mActiveWaterModellingApproach)
			return;

		mRunningTime += deltaTime;

		// Update the texture holding the positional data for the water's surface
		// This is through dispatching the compute shader to compute the positions
		mActiveWaterModellingApproach->UseProgram();

			mActiveWaterModellingApproach->SetFloat("time", mRunningTime);

			if(mActiveWaterModellingApproach == mWaterMovementComputeShader_Sine)
			{
				if(mSineWaveSSBO)
					mSineWaveSSBO->BindToBufferIndex(5);

				mActiveWaterModellingApproach->SetInt("waveCount", (int)mSineWaveData.size());
			}
			else if (mActiveWaterModellingApproach == mWaterMovementComputeShader_Gerstner)
			{
				if(mGerstnerWaveSSBO)
					mGerstnerWaveSSBO->BindToBufferIndex(5);

				mActiveWaterModellingApproach->SetInt("waveCount", (int)mGersnterWaveData.size());
			}
			else if (mActiveWaterModellingApproach == mWaterMovementComputeShader_Tessendorf)
			{
				mActiveWaterModellingApproach->SetFloat("gravity",         mTessendorfData.mGravity);
				mActiveWaterModellingApproach->SetFloat("repeatAfterTime", mTessendorfData.mRepeatAfterTime);
				mActiveWaterModellingApproach->SetVec2("windVelocity",     mTessendorfData.mWindVelocity);
				mActiveWaterModellingApproach->SetInt("waveCount",         mTessendorfData.mWaveCount);
			}

			mPositionalBuffer->BindForComputeShader(0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			mNormalBuffer->BindForComputeShader    (1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			mTangentBuffer->BindForComputeShader   (2, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			mBiNormalBuffer->BindForComputeShader  (3, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

		glDispatchCompute(1000, 1000, 1);
	}

	// ---------------------------------------------

	void WaterSimulation::Render(Rendering::Camera* camera, Texture::CubeMapTexture* skybox)
	{
		// Existance checks
		if (!mWaterVAO || !mWaterVBO || !mPositionalBuffer || !mNormalBuffer || !mTangentBuffer || !mBiNormalBuffer || !mSurfaceRenderShaders)
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

			mSurfaceRenderShaders->SetVec3("cameraPos", camera->GetPosition());

			// ------------------------------------------------------------------------------------------------

			// Now handle the LODs
			for (int i = 0; i <= mLevelOfDetailCount; i++)
			{
				float                          LODscaleFactor = std::powf(3, i);
				float                          dimensions     = mHighestLODDimensions * LODscaleFactor;

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
		mVertexCount = dimensions * dimensions;
		Maths::Vector::Vector2D<float>* newData = new Maths::Vector::Vector2D<float>[mVertexCount];

		bool         evenSplit      = (dimensions % 2 == 0);
		unsigned int halfDimensions = dimensions / 2;

		float                          startingDistanceFromCentre = evenSplit ? (halfDimensions * distanceBetweenVertex) + 0.5f : halfDimensions * distanceBetweenVertex;
		Maths::Vector::Vector2D<float> topLeftXPos                = Maths::Vector::Vector2D(-startingDistanceFromCentre, -startingDistanceFromCentre);

		unsigned int currentVertexID = 0;

		for (int z = dimensions - 1; z >= 0; z--)
		{
			for (int x = 0; x < dimensions; x++)
			{
				newData[currentVertexID++] = Maths::Vector::Vector2D<float>(topLeftXPos.x + (x * distanceBetweenVertex), topLeftXPos.y + (z * distanceBetweenVertex));
			}
		}

		return newData;
	}

	unsigned int* WaterSimulation::GenerateElementData(unsigned int dimensions)
	{
		mElementCount = (dimensions - 1) * (dimensions - 1) * 6;
		unsigned int* elementData = new unsigned int[mElementCount];

		unsigned int elementIndex = 0;
		for (unsigned int z = 0; z < dimensions - 1; z++)
		{
			unsigned int startOfRow     = dimensions * z;
			unsigned int startOfNextRow = startOfRow + dimensions;

			for (unsigned int x = 0; x < dimensions - 1; x++)
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
}