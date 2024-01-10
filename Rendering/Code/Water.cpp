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
		, mWaterMovementComputeShader(nullptr)
		, mPositionalBuffer(nullptr)
		, mNormalBuffer(nullptr)
		, mTangentBuffer(nullptr)
		, mBiNormalBuffer(nullptr)
		, mFFTConfigurations()

		, mWaterVAO(nullptr)
		, mSurfaceRenderShaders(nullptr)

		, mSimulationPaused(false)
		, mWireframe(false)
	{
		// VBO and VAO
		SetupBuffers();

		// Compute and final render shaders
		SetupShaders();

		// Storage textures
		SetupTextures();
	}

	// ---------------------------------------------

	WaterSimulation::~WaterSimulation()
	{
		// --------------------------------------

		delete mSurfaceRenderShaders;
		mSurfaceRenderShaders = nullptr;

		delete mWaterMovementComputeShader;
		mWaterMovementComputeShader = nullptr;

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

		if(!mSurfaceRenderShaders)
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

			mSurfaceRenderShaders->SetInt("positionalBuffer", 0);
			mSurfaceRenderShaders->SetInt("normalBuffer",     1);
			mSurfaceRenderShaders->SetInt("tangentBuffer",    2);
			mSurfaceRenderShaders->SetInt("binormalBuffer",   3);
		}
		// --------------------------------------------------------------

		if(!mWaterMovementComputeShader)
		{
			mWaterMovementComputeShader = new ShaderPrograms::ShaderProgram();

			Shaders::ComputeShader* computeShader = new Shaders::ComputeShader("Code/Shaders/Compute/SurfaceUpdate.comp");

			mWaterMovementComputeShader->AttachShader(computeShader);

				mWaterMovementComputeShader->LinkShadersToProgram();

			mWaterMovementComputeShader->DetachShader(computeShader);

			delete computeShader;
		}
		// --------------------------------------------------------------
	}

	// ---------------------------------------------

	void WaterSimulation::SetupBuffers()
	{
		if (!mWaterVBO)
		{
			mWaterVBO = new Buffers::VertexBufferObject();

			unsigned int dimensions            = 100;
			float        distanceBetweenPoints = 1.0f;
			Maths::Vector::Vector2D<float>* vertexData = GenerateVertexData(dimensions, distanceBetweenPoints);

			mWaterVBO->SetBufferData((void*)vertexData, mVertexCount * sizeof(Maths::Vector::Vector2D<float>), GL_STATIC_DRAW);

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
		if (mSimulationPaused || !mWaterMovementComputeShader)
			return;

		// Update the texture holding the positional data for the water's surface
		// This is through dispatching the compute shader to compute the positions
		
		/*mWaterMovementComputeShader->UseProgram();

			mPositionalBuffer->BindForComputeShader(0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			mNormalBuffer->BindForComputeShader    (1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			mTangentBuffer->BindForComputeShader   (2, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			mBiNormalBuffer->BindForComputeShader  (3, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

		glDispatchCompute(1, 1, 1);*/
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
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		mWaterVAO->Bind();

		mSurfaceRenderShaders->UseProgram();

			OpenGLRenderPipeline* renderPipeline = ((OpenGLRenderPipeline*)Window::GetRenderPipeline());

			renderPipeline->SetLineModeEnabled(true);

			// Textures
			//renderPipeline->BindTextureToTextureUnit(mPositionalBuffer->GetTextureID(), 0, true);
			//renderPipeline->BindTextureToTextureUnit(mNormalBuffer->GetTextureID(), 1, true);
			//renderPipeline->BindTextureToTextureUnit(mTangentBuffer->GetTextureID(), 2, true);
			//renderPipeline->BindTextureToTextureUnit(mBiNormalBuffer->GetTextureID(), 3, true);

			// Matricies
			Maths::Matrix::Matrix4X4 identity = Maths::Matrix::Matrix4X4();
			mSurfaceRenderShaders->SetMat4("modelMat", &identity[0]);

			glm::mat4 viewMat = camera->GetViewMatrix();
			mSurfaceRenderShaders->SetMat4("viewMat", &viewMat[0][0]);

			glm::mat4 projectionMat = camera->GetPerspectiveMatrix();
			mSurfaceRenderShaders->SetMat4("projectionMat", &projectionMat[0][0]);

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
		mVertexCount = ((dimensions - 1) * (2 * dimensions));
		Maths::Vector::Vector2D<float>* newData     = new Maths::Vector::Vector2D<float>[mVertexCount];
			
		bool         evenSplit      = dimensions % 2 == 0;
		unsigned int halfDimensions = dimensions / 2;

		float                          startingDistanceFromCentre = evenSplit ? (halfDimensions * distanceBetweenVertex) + 0.5f : halfDimensions * distanceBetweenVertex;
		Maths::Vector::Vector2D<float> topLeftXPos                = Maths::Vector::Vector2D(-startingDistanceFromCentre, -startingDistanceFromCentre);

		unsigned int currentVertexID = 0;
		bool movementDirectionRight = true;

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
					if (x == dimensions)
					{
						// Flip the direction
						movementDirectionRight = !movementDirectionRight;
					}
				}
				else
				{
					newPos.x += distanceBetweenVertex * (dimensions - x);
					newPos.y += distanceBetweenVertex * (z + 1);

					newData[currentVertexID++] = newPos;

					// Now move back along the line
					newPos.x -= distanceBetweenVertex;
					newPos.y -= distanceBetweenVertex;

					newData[currentVertexID++] = newPos;

					// If this is the last point in the line
					if (x == dimensions)
					{
						// Flip the direction
						movementDirectionRight = !movementDirectionRight;
					}
				}
			}
		}

		return newData;
	}
}