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
		, mActiveWaterRenderingApproach(nullptr)

		, mPositionalBuffer(nullptr)
		, mNormalBuffer(nullptr)
		, mTangentBuffer(nullptr)
		, mBiNormalBuffer(nullptr)
		, mFFTConfigurations()
		, mRunningTime(0.0f)
		, mSineWaveData()
		, mGersnterWaveData()

		, mWaterVAO(nullptr)

		, mSurfaceRenderShaders_Sine(nullptr)
		, mSurfaceRenderShaders_Gersnter(nullptr)
		, mSurfaceRenderShaders_Tessendorf(nullptr)

		, mSimulationPaused(false)
		, mWireframe(false)
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

		delete mSurfaceRenderShaders_Sine;
		mSurfaceRenderShaders_Sine = nullptr;

		delete mSurfaceRenderShaders_Gersnter;
		mSurfaceRenderShaders_Gersnter = nullptr;

		delete mSurfaceRenderShaders_Tessendorf;
		mSurfaceRenderShaders_Tessendorf = nullptr;

		// --------------------------------------

		delete mWaterMovementComputeShader_Sine;
		mWaterMovementComputeShader_Sine = nullptr;

		delete mWaterMovementComputeShader_Gerstner;
		mWaterMovementComputeShader_Gerstner = nullptr;

		delete mWaterMovementComputeShader_Tessendorf;
		mWaterMovementComputeShader_Tessendorf = nullptr;

		// --------------------------------------

		mActiveWaterModellingApproach = nullptr;
		mActiveWaterRenderingApproach = nullptr;

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
	}

	// ---------------------------------------------

	void WaterSimulation::SetupShaders()
	{
		// --------------------------------------------------------------

		if(!mSurfaceRenderShaders_Sine)
		{
			mSurfaceRenderShaders_Sine = new ShaderPrograms::ShaderProgram();

			Shaders::VertexShader*   vertexShader   = new Shaders::VertexShader("Code/Shaders/Vertex/WaterSurface_Sine.vert");
			Shaders::FragmentShader* fragmentShader = new Shaders::FragmentShader("Code/Shaders/Fragment/WaterSurface.frag");

			mSurfaceRenderShaders_Sine->AttachShader(vertexShader);
			mSurfaceRenderShaders_Sine->AttachShader(fragmentShader);

				mSurfaceRenderShaders_Sine->LinkShadersToProgram();

			mSurfaceRenderShaders_Sine->DetachShader(vertexShader);
			mSurfaceRenderShaders_Sine->DetachShader(fragmentShader);

			delete vertexShader;
			delete fragmentShader;

			mSurfaceRenderShaders_Sine->UseProgram();
				mSurfaceRenderShaders_Sine->SetInt("positionalBuffer", 0);
				mSurfaceRenderShaders_Sine->SetInt("normalBuffer",     1);
				mSurfaceRenderShaders_Sine->SetInt("tangentBuffer",    2);
				mSurfaceRenderShaders_Sine->SetInt("binormalBuffer",   3);
		}

		if (!mSurfaceRenderShaders_Gersnter)
		{
			mSurfaceRenderShaders_Gersnter = new ShaderPrograms::ShaderProgram();

			Shaders::VertexShader*   vertexShader   = new Shaders::VertexShader("Code/Shaders/Vertex/WaterSurface_Gersnter.vert");
			Shaders::FragmentShader* fragmentShader = new Shaders::FragmentShader("Code/Shaders/Fragment/WaterSurface.frag");

			mSurfaceRenderShaders_Gersnter->AttachShader(vertexShader);
			mSurfaceRenderShaders_Gersnter->AttachShader(fragmentShader);

				mSurfaceRenderShaders_Gersnter->LinkShadersToProgram();

			mSurfaceRenderShaders_Gersnter->DetachShader(vertexShader);
			mSurfaceRenderShaders_Gersnter->DetachShader(fragmentShader);

			delete vertexShader;
			delete fragmentShader;

			mSurfaceRenderShaders_Gersnter->UseProgram();
				mSurfaceRenderShaders_Gersnter->SetInt("positionalBuffer", 0);
				mSurfaceRenderShaders_Gersnter->SetInt("normalBuffer", 1);
				mSurfaceRenderShaders_Gersnter->SetInt("tangentBuffer", 2);
				mSurfaceRenderShaders_Gersnter->SetInt("binormalBuffer", 3);
		}

		if (!mSurfaceRenderShaders_Tessendorf)
		{
			mSurfaceRenderShaders_Tessendorf = new ShaderPrograms::ShaderProgram();

			Shaders::VertexShader*   vertexShader   = new Shaders::VertexShader("Code/Shaders/Vertex/WaterSurface_Tessendorf.vert");
			Shaders::FragmentShader* fragmentShader = new Shaders::FragmentShader("Code/Shaders/Fragment/WaterSurface.frag");

			mSurfaceRenderShaders_Tessendorf->AttachShader(vertexShader);
			mSurfaceRenderShaders_Tessendorf->AttachShader(fragmentShader);

				mSurfaceRenderShaders_Tessendorf->LinkShadersToProgram();

			mSurfaceRenderShaders_Tessendorf->DetachShader(vertexShader);
			mSurfaceRenderShaders_Tessendorf->DetachShader(fragmentShader);

			delete vertexShader;
			delete fragmentShader;

			mSurfaceRenderShaders_Tessendorf->UseProgram();
				mSurfaceRenderShaders_Tessendorf->SetInt("positionalBuffer", 0);
				mSurfaceRenderShaders_Tessendorf->SetInt("normalBuffer", 1);
				mSurfaceRenderShaders_Tessendorf->SetInt("tangentBuffer", 2);
				mSurfaceRenderShaders_Tessendorf->SetInt("binormalBuffer", 3);
		}

		mActiveWaterRenderingApproach = mSurfaceRenderShaders_Sine;

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
		if (!mWaterVBO)
		{
			mWaterVBO = new Buffers::VertexBufferObject();

			unsigned int dimensions            = 200;
			float        distanceBetweenPoints = 0.1f;
			Maths::Vector::Vector2D<float>* vertexData = GenerateVertexData(dimensions, distanceBetweenPoints);

			mWaterVBO->SetBufferData((void*)vertexData, mVertexCount * sizeof(Maths::Vector::Vector2D<float>), GL_STATIC_DRAW);

			// ----------------

			bool         evenSplit                  = dimensions % 2 == 0;
			unsigned int halfDimensions             = dimensions / 2;
			float        startingDistanceFromCentre = evenSplit ? (halfDimensions * distanceBetweenPoints) + 0.5f : halfDimensions * distanceBetweenPoints;

			if (mSurfaceRenderShaders_Sine)
			{
				mSurfaceRenderShaders_Sine->UseProgram();
					mSurfaceRenderShaders_Sine->SetFloat("maxDistanceFromOrigin", startingDistanceFromCentre);
			}

			if (mSurfaceRenderShaders_Gersnter)
			{
				mSurfaceRenderShaders_Gersnter->UseProgram();
					mSurfaceRenderShaders_Gersnter->SetFloat("maxDistanceFromOrigin", startingDistanceFromCentre);
			}

			if (mSurfaceRenderShaders_Tessendorf)
			{
				mSurfaceRenderShaders_Tessendorf->UseProgram();
					mSurfaceRenderShaders_Tessendorf->SetFloat("maxDistanceFromOrigin", startingDistanceFromCentre);
			}

			// ----------------

			delete[] vertexData;
		}

		if (!mWaterVAO)
		{
			mWaterVAO = new Buffers::VertexArrayObject();

			mWaterVAO->Bind();
			mWaterVBO->Bind();

			// Positional data
			mWaterVAO->EnableVertexAttribArray(0);
			mWaterVAO->SetVertexAttributePointers(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0, true);

			mWaterVAO->Unbind();
			mWaterVBO->UnBind();
		}
	}

	// ---------------------------------------------

	void WaterSimulation::SetupTextures()
	{
		if (!mPositionalBuffer)
		{
			mPositionalBuffer = new Texture::Texture2D();

			unsigned int width  = 1000;
			unsigned int height = 1000;

			mPositionalBuffer->InitEmpty(width, height, true, GL_FLOAT, GL_RGBA32F, GL_RGBA);
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
					mActiveWaterRenderingApproach = mSurfaceRenderShaders_Sine;
				}

				if (ImGui::Button("Gerstner Waves"))
				{
					mActiveWaterModellingApproach = mWaterMovementComputeShader_Gerstner;
					mActiveWaterRenderingApproach = mSurfaceRenderShaders_Gersnter;
				}

				if (ImGui::Button("Ocean simulation"))
				{
					mActiveWaterModellingApproach = mWaterMovementComputeShader_Tessendorf;
					mActiveWaterRenderingApproach = mSurfaceRenderShaders_Tessendorf;
				}
			}
		ImGui::End();

		if(mActiveWaterModellingApproach)
		{
			ImGui::Begin("Water properties");

				if (mActiveWaterModellingApproach == mWaterMovementComputeShader_Sine)
				{
					ImGui::InputFloat("Amplitude##Sine",  &mSineWaveData.mAmplitude);
					ImGui::InputFloat2("Direction##Sine", &mSineWaveData.mDirectionOfWave.x);
					ImGui::InputFloat("Speed##Sine",      &mSineWaveData.mSpeedOfWave);
					ImGui::InputFloat("Wavelength##Sine", &mSineWaveData.mWaveLength);

					if (ImGui::Button("Defaults##Sine"))
					{
						mSineWaveData = SingleSineDataSet();
					}
				}
				else if (mActiveWaterModellingApproach == mWaterMovementComputeShader_Gerstner)
				{
					ImGui::InputFloat("Amplitude##Gersnter",  &mGersnterWaveData.mAmplitude);
					ImGui::InputFloat2("Direction##Gersnter", &mGersnterWaveData.mDirectionOfWave.x);
					ImGui::InputFloat("Speed##Gersnter",      &mGersnterWaveData.mSpeedOfWave);
					ImGui::InputFloat("Wavelength##Gersnter", &mGersnterWaveData.mWaveLength);

					if (ImGui::Button("Defaults##Gersnter"))
					{
						mGersnterWaveData = SingleGerstnerWaveData();
					}
				}
				else if (mActiveWaterModellingApproach == mWaterMovementComputeShader_Tessendorf)
				{

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
				mActiveWaterModellingApproach->SetFloat("amplitude",      mSineWaveData.mAmplitude);
				mActiveWaterModellingApproach->SetVec2("directionOfWave", mSineWaveData.mDirectionOfWave);
				mActiveWaterModellingApproach->SetFloat("speedOfWave",    mSineWaveData.mSpeedOfWave);
				mActiveWaterModellingApproach->SetFloat("waveLength",     mSineWaveData.mWaveLength);
			}
			else if (mActiveWaterModellingApproach == mWaterMovementComputeShader_Gerstner)
			{
				mActiveWaterModellingApproach->SetFloat("amplitude",      mGersnterWaveData.mAmplitude);
				mActiveWaterModellingApproach->SetVec2("directionOfWave", mGersnterWaveData.mDirectionOfWave);
				mActiveWaterModellingApproach->SetFloat("speedOfWave",    mGersnterWaveData.mSpeedOfWave);
				mActiveWaterModellingApproach->SetFloat("waveLength",     mGersnterWaveData.mWaveLength);
			}
			else if (mActiveWaterModellingApproach == mWaterMovementComputeShader_Tessendorf)
			{

			}

			mPositionalBuffer->BindForComputeShader(0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			//mNormalBuffer->BindForComputeShader    (1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			//mTangentBuffer->BindForComputeShader   (2, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			//mBiNormalBuffer->BindForComputeShader  (3, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

		glDispatchCompute(1000, 1000, 1);
	}

	// ---------------------------------------------

	void WaterSimulation::Render(Rendering::Camera* camera)
	{
		// Existance checks
		//if (!mWaterVAO || !mWaterVBO || !mPositionalBuffer || !mNormalBuffer || !mTangentBuffer || !mBiNormalBuffer || !mSurfaceRenderShaders)
		//	return;

		// Rendering of the surface is done through passing the positional texture into the vertex shader to create the final world position
		// Then the fragment shader used information given to it from the other textures output by the compute shader

		// Make sure the compute shader has finished before reading from the textures
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		mWaterVAO->Bind();

		mActiveWaterRenderingApproach->UseProgram();

			OpenGLRenderPipeline* renderPipeline = ((OpenGLRenderPipeline*)Window::GetRenderPipeline());

			renderPipeline->SetLineModeEnabled(mWireframe);

			// Textures
			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mPositionalBuffer->GetTextureID(), true);
			//renderPipeline->BindTextureToTextureUnit(mNormalBuffer->GetTextureID(), 1, true);
			//renderPipeline->BindTextureToTextureUnit(mTangentBuffer->GetTextureID(), 2, true);
			//renderPipeline->BindTextureToTextureUnit(mBiNormalBuffer->GetTextureID(), 3, true);

			// Matricies
			Maths::Matrix::Matrix4X4 identity = Maths::Matrix::Matrix4X4();
			mActiveWaterRenderingApproach->SetMat4("modelMat", &identity[0]);

			glm::mat4 viewMat = camera->GetViewMatrix();
			mActiveWaterRenderingApproach->SetMat4("viewMat", &viewMat[0][0]);

			glm::mat4 projectionMat = camera->GetPerspectiveMatrix();
			mActiveWaterRenderingApproach->SetMat4("projectionMat", &projectionMat[0][0]);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, mVertexCount);

			renderPipeline->SetLineModeEnabled(false);

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
		bool         evenSplit      = dimensions % 2 == 0;
		unsigned int halfDimensions = dimensions / 2;

		float                          startingDistanceFromCentre = evenSplit ? (halfDimensions * distanceBetweenVertex) + 0.5f : halfDimensions * distanceBetweenVertex;
		Maths::Vector::Vector2D<float> topLeftXPos                = Maths::Vector::Vector2D(-startingDistanceFromCentre, -startingDistanceFromCentre);

		unsigned int currentVertexID        = 0;
		bool         movementDirectionRight = true;

		// ------------------------------------------------------- //

		unsigned int totalVertexCount = 0;

		for (unsigned int z = 0; z < dimensions - 1; z++)
		{
			for (unsigned int x = 0; x < dimensions; x++)
			{
				if (movementDirectionRight)
				{
					totalVertexCount += 2;

					// If this is the last point in the line
					if (x == dimensions - 1)
					{
						movementDirectionRight = false;
						totalVertexCount += 2;
					}
				}
				else
				{
					totalVertexCount += 2;

					// If this is the last point in the line
					if (x == dimensions - 1)
					{
						// Flip the direction
						movementDirectionRight = true;
					}
				}
			}
		}

		// ------------------------------------------------------- //

		                                mVertexCount = totalVertexCount;
		Maths::Vector::Vector2D<float>* newData      = new Maths::Vector::Vector2D<float>[mVertexCount];

		// ------------------------------------------------------- //

		for (unsigned int z = 0; z < dimensions - 1; z++)
		{
			for (unsigned int x = 0; x < dimensions; x++)
			{
				if (currentVertexID > mVertexCount)
					ASSERTFAIL("error");

				Maths::Vector::Vector2D<float> newPos = topLeftXPos;

				if (movementDirectionRight)
				{
					// Do the current position
					newPos.x += distanceBetweenVertex * x;
					newPos.y += distanceBetweenVertex * z;

					newData[currentVertexID++] = newPos;

					// Go down on the z
					newPos.y += distanceBetweenVertex;

					newData[currentVertexID++] = newPos;

					// If this is the last point in the line
					if (x == dimensions - 1)
					{	
						// Flip the direction
						movementDirectionRight = false;

						newData[currentVertexID] = newData[currentVertexID - 1];
						currentVertexID++;

						newData[currentVertexID] = newData[currentVertexID - 1];
						currentVertexID++;
					}
				}
				else
				{
					newPos.x += distanceBetweenVertex * (dimensions - x - 1);
					newPos.y += distanceBetweenVertex * (z + 1);

					newData[currentVertexID++] = newPos;

					// If this is the last point in the line
					if (x == dimensions - 1)
					{
						// Flip the direction
						movementDirectionRight = true;
						
						newData[currentVertexID] = newData[currentVertexID - 1];
						currentVertexID++;
					}
					else
					{
						// Now move back along the line
						newPos.x -= distanceBetweenVertex;
						newPos.y -= distanceBetweenVertex;

						newData[currentVertexID++] = newPos;
					}
				}
			}
		}

		// ------------------------------------------------------- //

		return newData;
	}
}