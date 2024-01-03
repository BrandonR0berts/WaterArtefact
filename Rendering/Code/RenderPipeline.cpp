#include "RenderPipeline.h"

#include "Camera.h"

#include "Window.h"

#include "Framebuffers.h"
#include "Textures/Texture.h"

#include "Include/imgui/imgui.h"
#include "Include/imgui/imgui_impl_glfw.h"
#include "Include/imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Rendering
{
	// ---------------------------------------

	RenderPipeline::RenderPipeline()
		: mActiveCamera(nullptr)

		, mFinalRenderFBO(nullptr)

		, mScreenWidth(1920)
		, mScreenHeight(1080)

		, mMaxScreenWidth(1920)
		, mMaxScreenHeight(1080)

		, mWindow(nullptr)
	{
		
	}

	// ---------------------------------------

	RenderPipeline::~RenderPipeline()
	{
		delete mActiveCamera;
		mActiveCamera = nullptr;
	}

	// ---------------------------------------

	void RenderPipeline::Init()
	{
		// Create the debug camera
		mActiveCamera = new DebugCamera(0.01f, 500.0f, 1920.0f, 1080.0f, 16.0f / 9.0f, 70.0f,
									   Maths::Vector::Vector3D<float>(0.0f, 1.0f, 0.0f),
									   Maths::Vector::Vector3D<float>(0.0f, 0.0f, -1.0f),
									   Maths::Vector::Vector3D<float>(0.0f, 0.0f, 1.0f));

		SetupUsefulVBOData();
		
		// ------------------------------------

		std::vector<Rendering::Buffers::AttributePointerData> VAODataSetup;

		VAODataSetup.push_back(Buffers::AttributePointerData(0, 4, GL_FLOAT, false, 4 * sizeof(GL_FLOAT), 0, true, "VBO_full_FBO"));

		Window::GetBufferStore().CreateVAO("VAO_full_FBO", VAODataSetup);

		// ------------------------------------

		// Create empty texture for use when we dont have a real texture to bind, but still want the shaders to work
		Texture::Texture2D* emptyTexture = new Texture::Texture2D();
		unsigned char       emptyData[]  = { 255, 255, 255 };
		emptyTexture->InitWithData(1, 1, emptyData, false);

		std::string         emptyTextureName = "Empty";
		Rendering::Window::GetResourceCollection().AddResource(Rendering::ResourceType::Texture2D, emptyTextureName, emptyTexture);

		// ------------------------------------
	}

	// ---------------------------------------

	void RenderPipeline::SetupUsefulVBOData()
	{
		// ----

		static float renderingData[24] = { -1.0f, 1.0f, 0.0f, 1.0f,
										   -1.0f, -1.0f, 0.0f, 0.0f,
										    1.0f, -1.0f, 1.0f, 0.0f,

										   -1.0f, 1.0f, 0.0f, 1.0f,
											1.0f, -1.0f, 1.0f, 0.0f,
										    1.0f, 1.0f, 1.0f, 1.0f };

		Window::GetBufferStore().CreateVBO(renderingData, 96, GL_STATIC_DRAW, "VBO_Video");

		// ----

		static float renderingDataFBO[24] = { -1.0f, -1.0f, 0.0f, 0.0f,
										      -1.0f,  1.0f, 0.0f, 1.0f,
										       1.0f,  1.0f, 1.0f, 1.0f,

										       1.0f,  1.0f, 1.0f, 1.0f,
										       1.0f, -1.0f, 1.0f, 0.0f,
										      -1.0f, -1.0f, 0.0f, 0.0f };

		Window::GetBufferStore().CreateVBO(renderingDataFBO, 96, GL_STATIC_DRAW, "VBO_full_FBO");

		// ----
	}

	// -------------------------------------------------

	void RenderPipeline::ClearFinalRenderBuffer()
	{
		if (mFinalRenderFBO)
		{
			float intensity = 0.6f;
			mFinalRenderFBO->ClearAll(intensity, intensity, intensity, 1.0f);
		}
	}

	// -------------------------------------------------

	void RenderPipeline::OnScreenSizeChanged(std::string data)
	{
		// Resize the image attachments in the FBO
		std::string stringWidth  = data.substr(0, data.find_first_of(','));
		std::string stringHeight = data.substr(data.find_first_of(',') + 1);

		unsigned int width  = std::stoul(stringWidth);
		unsigned int height = std::stoul(stringHeight);

		mFinalRenderFBO->ResizeBuffers(width, height);

		mScreenWidth  = width;
		mScreenHeight = height;

		if(mActiveCamera)
			mActiveCamera->SetResolution((float)mScreenWidth, (float)mScreenHeight);
	}

	// -------------------------------------------------

	void RenderPipeline::SetupFinalRenderFBO()
	{
		if (mFinalRenderFBO)
		{
			mFinalRenderFBO->SetActive(true, true);

			Texture::Texture2D* colourBuffer = (Texture::Texture2D*)Window::GetResourceCollection().GetResource(Rendering::ResourceType::Texture2D, "FinalRenderbufferColourTexture");

			if (colourBuffer)
				mFinalRenderFBO->AttachColourBuffer(colourBuffer);

			Texture::Texture2D* depthBuffer = (Texture::Texture2D*)Window::GetResourceCollection().GetResource(Rendering::ResourceType::Texture2D, "FinalRenderbufferDepthTexture");

			if (depthBuffer)
				mFinalRenderFBO->AttachDepthBuffer(depthBuffer);

			mFinalRenderFBO->CheckComplete();
		}
	}

	// -------------------------------------------------
}