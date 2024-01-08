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
		, mShadowMap(nullptr)
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
	}

	// ---------------------------------------

	void RenderPipeline::SetupUsefulVBOData()
	{

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
}