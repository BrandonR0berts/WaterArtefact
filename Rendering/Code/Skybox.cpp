#include "Skybox.h"

#include "Window.h"

#include "OpenGLRenderPipeline.h"
#include "Shaders/Shader.h"
#include "Shaders/ShaderProgram.h"

#include "Camera.h"

namespace Rendering
{
	Maths::Vector::Vector3D<float> Skybox::mCubeData[36] = { { -1.0f,  1.0f, -1.0f },
															 { -1.0f, -1.0f, -1.0f },
															 {  1.0f, -1.0f, -1.0f },
															 {  1.0f, -1.0f, -1.0f },
		                                                     {  1.0f,  1.0f, -1.0f },
		                                                     { -1.0f,  1.0f, -1.0f },

		                                                     { -1.0f, -1.0f,  1.0f },
		                                                     { -1.0f, -1.0f, -1.0f },
		                                                     { -1.0f,  1.0f, -1.0f },
		                                                     { -1.0f,  1.0f, -1.0f },
		                                                     { -1.0f,  1.0f,  1.0f },
		                                                     { -1.0f, -1.0f,  1.0f },

		                                                     {  1.0f, -1.0f, -1.0f },
		                                                     {  1.0f, -1.0f,  1.0f },
															 {  1.0f,  1.0f,  1.0f },
		                                                     {  1.0f,  1.0f,  1.0f },
		                                                     {  1.0f,  1.0f, -1.0f },
		                                                     {  1.0f, -1.0f, -1.0f },

		                                                     { -1.0f, -1.0f,  1.0f },
		                                                     { -1.0f,  1.0f,  1.0f },
		                                                     {  1.0f,  1.0f,  1.0f },
		                                                     {  1.0f,  1.0f,  1.0f },
		                                                     {  1.0f, -1.0f,  1.0f },
		                                                     { -1.0f, -1.0f,  1.0f },

		                                                     { -1.0f,  1.0f, -1.0f },
		                                                     {  1.0f,  1.0f, -1.0f },
		                                                     {  1.0f,  1.0f,  1.0f },
		                                                     {  1.0f,  1.0f,  1.0f },
		                                                     { -1.0f,  1.0f,  1.0f },
		                                                     { -1.0f,  1.0f, -1.0f },

		                                                     { -1.0f, -1.0f, -1.0f },
		                                                     { -1.0f, -1.0f,  1.0f },
		                                                     {  1.0f, -1.0f, -1.0f },
		                                                     {  1.0f, -1.0f, -1.0f },
		                                                     { -1.0f, -1.0f,  1.0f },
		                                                     {  1.0f, -1.0f,  1.0f } };

	// -----------------------------------------

	Skybox::Skybox(std::string name, std::string filePaths[6])
		: mCubeMapTexture(nullptr)
		, mFilePaths{ filePaths[0], filePaths[1], filePaths[2], filePaths[3], filePaths[4], filePaths[5] }
		, mInternalName("Skybox_Cubemap_" + name)
		, mName(name)
		, mCubeVAO(nullptr)
		, mSkyBoxProgram(nullptr)
		, mShowingIrradianceMap(false)
	{
		LoadCubeMapTextures(mFilePaths);

		if (!mCubeVAO)
		{
			mCubeVAO = new Buffers::VertexArrayObject();

			if (!mCubeVAO)
				SetupBufferData();
		}

		SetupShaders();
	}

	// -----------------------------------------

	Skybox::Skybox(std::string name, std::string filePath1, std::string filePath2, std::string filePath3, std::string filePath4, std::string filePath5, std::string filePath6)
		: mCubeMapTexture(nullptr)
		, mFilePaths{ filePath1, filePath2, filePath3, filePath4, filePath5, filePath6 }
		, mInternalName("Skybox_Cubemap_" + name)
		, mName(name)
		, mCubeVAO(nullptr)
	{
		LoadCubeMapTextures(mFilePaths);

		if (!mCubeVAO)
		{
			mCubeVAO = new Buffers::VertexArrayObject();

			if (!mCubeVAO)
				SetupBufferData();
		}

		SetupShaders();
	}

	// -----------------------------------------

	Skybox::~Skybox()
	{
		// Clear up the cubemap resources used
		delete mCubeMapTexture;
		delete mConvolutedVersion;

		mCubeMapTexture    = nullptr;
		mConvolutedVersion = nullptr;
	}

	// -----------------------------------------

	void Skybox::SetupShaders()
	{
		if (!mSkyBoxProgram)
		{
			mSkyBoxProgram = new ShaderPrograms::ShaderProgram();

			Shaders::VertexShader*   vertexShader   = new Shaders::VertexShader("Code/Shaders/Vertex/Skybox.vert");
			Shaders::FragmentShader* fragmentShader = new Shaders::FragmentShader("Code/Shaders/Fragment/Skybox.frag");

			mSkyBoxProgram->AttachShader(vertexShader);
			mSkyBoxProgram->AttachShader(fragmentShader);

			mSkyBoxProgram->LinkShadersToProgram();

			delete vertexShader;
			delete fragmentShader;
		}
	}

	// -----------------------------------------

	void Skybox::SetupBufferData()
	{
		// Create the VBO and set its data
		mCubeVBO = new Buffers::VertexBufferObject();
		mCubeVBO->SetBufferData(mCubeData, sizeof(Maths::Vector::Vector3D<float>) * 36, GL_STATIC_DRAW);

		// Setup the vao
		mCubeVAO->SetVertexAttributePointers(0, 3, GL_FLOAT, false, 3 * sizeof(GL_FLOAT), 0, true);
	}

	// -----------------------------------------

	void Skybox::LoadCubeMapTextures(std::string filePaths[6])
	{
		mCubeMapTexture = new Texture::CubeMapTexture();

		mCubeMapTexture->LoadInTextures(filePaths);
	}

	// -----------------------------------------

	void Skybox::Render(Camera* camera, Texture::CubeMapTexture* textureToReplaceSkybox)
	{
		OpenGLRenderPipeline* renderPipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

		if (!renderPipeline)
			return;

		// First turn off blending and turn on back face culling
		renderPipeline->SetAlphaBlending(false);
		renderPipeline->SetBackFaceCulling(false);

		if (!mConvolutedVersion)
		{
			// This is used within the PBR irradiance mapping to give the illusion of global illumination
			// It is essentially a blurred version
			mConvolutedVersion = mCubeMapTexture->ConvoluteTexture(mCubeVAO);
		}

		if (!mCubeMapTexture || !camera || !mCubeVAO || !mSkyBoxProgram)
			return;

		mCubeVAO->Bind();

		mSkyBoxProgram->UseProgram();

		// Allow the early out depth test to work
		renderPipeline->SetDepthTestFunction(GL_LEQUAL);

		// Bind the cube map
		mSkyBoxProgram->SetInt("skyboxImage", 0);

		glm::mat4 view = glm::mat4(glm::mat3(camera->GetViewMatrix()));
		mSkyBoxProgram->SetMat4("viewMat", &view[0][0]);

		glm::mat4 projection = camera->GetPerspectiveMatrix();
		mSkyBoxProgram->SetMat4("projectionMat", &projection[0][0]);

		if (textureToReplaceSkybox)
		{
			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, textureToReplaceSkybox->GetTextureID(), false);
		}
		else
		{
			if (mShowingIrradianceMap && mConvolutedVersion)
			{
				renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mConvolutedVersion->GetTextureID(), false);
			}
			else
			{
				renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mCubeMapTexture->GetTextureID(), false);
			}
		}

		// Draw
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Reset depth func back to default
		renderPipeline->SetDepthTestFunction(GL_LESS);
	}

	// -----------------------------------------

	void Skybox::Render(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, Texture::CubeMapTexture* textureToReplaceSkybox)
	{
		OpenGLRenderPipeline* renderPipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

		if (!renderPipeline)
			return;

		renderPipeline->SetAlphaBlending(false);
		renderPipeline->SetBackFaceCulling(false);

		if (!mCubeMapTexture || !mCubeVAO || !mSkyBoxProgram)
			return;

		mCubeVAO->Bind();

		mSkyBoxProgram->UseProgram();

		// Allow the early out depth test to work
		renderPipeline->SetDepthTestFunction(GL_LEQUAL);

		// Bind the cube map
		mSkyBoxProgram->SetInt("skyboxImage", 0);

		glm::mat4 view = glm::mat4(glm::mat3(viewMatrix));
		mSkyBoxProgram->SetMat4("viewMat", &view[0][0]);

		mSkyBoxProgram->SetMat4("projectionMat", &projectionMatrix[0][0]);

		if (textureToReplaceSkybox)
		{
			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, textureToReplaceSkybox->GetTextureID(), false);
		}
		else
		{
			if (mShowingIrradianceMap && mConvolutedVersion)
			{
				renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mConvolutedVersion->GetTextureID(), false);
			}
			else
			{
				renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mCubeMapTexture->GetTextureID(), false);
			}
		}

		// Draw
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Reset depth func back to default
		renderPipeline->SetDepthTestFunction(GL_LESS);
	}

	// -----------------------------------------

	void Skybox::ConvoluteTexture()
	{
		if (!mConvolutedVersion)
		{
			// This is used within the PBR irradiance mapping to give the illusion of global illumination
			// It is essentially a blurred version
			mConvolutedVersion = mCubeMapTexture->ConvoluteTexture(mCubeVAO);
		}
	}

	// -----------------------------------------

	Texture::CubeMapTexture* Skybox::GetConvolutedTexture()
	{
		if (!mConvolutedVersion) 
			ConvoluteTexture(); 
		
		return mConvolutedVersion; 
	}

	// -----------------------------------------
}