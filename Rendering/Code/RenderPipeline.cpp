#include "RenderPipeline.h"

#include "Camera.h"
#include "GameWorld.h"

#include "RenderingResources/RenderingResources.h"
#include "Window.h"

#include "ProjectCube/Code/Video/Video.h"

#ifdef _DEBUG_BUILD
	#include "Include/imgui/imgui.h"
	#include "Include/imgui/imgui_impl_glfw.h"
	#include "Include/imgui/imgui_impl_opengl3.h"
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Rendering
{
	// ---------------------------------------

	RenderPipeline::RenderPipeline(RenderAPI api)
		: mActiveCamera(nullptr)
		, mGameCamera(nullptr)
		, mDebugCamera(nullptr)

		, mGameWorld(nullptr)

		, mVisualSettings()
		, mPostProcessingSettings()
		, mFinalRenderFBO(nullptr)

#ifdef _DEBUG_BUILD
		, mShowShadowMap(false)
#endif
		, mShadowMap(nullptr)
		, mShadowMapImageDimensions(3840, 2160)

		, mTextManager()
		, mAPI(api)

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
		Window::GetBufferStore().RemoveBuffer("ShadowMapData_SSBO", Rendering::Buffers::BufferType::SSBO);

		Texture::Texture2D* shadowMapImage = (Texture::Texture2D*)Rendering::Window::GetResourceCollection().GetResource(Rendering::ResourceType::Texture2D, "ShadowMap", false);
		Rendering::Window::GetResourceCollection().RemoveResource(Rendering::ResourceType::Texture2D, shadowMapImage);
		shadowMapImage = nullptr;

		delete mDebugCamera;
		mDebugCamera = nullptr;

		delete mGameCamera;
		mGameCamera = nullptr;

		//delete mGameWorld;
		mGameWorld = nullptr;
	}

	// ---------------------------------------

	void RenderPipeline::Init()
	{
		// Create the debug camera
		mDebugCamera = new DebugCamera(0.01f, 500.0f, 1920.0f, 1080.0f, 16.0f / 9.0f, 70.0f,
									   Maths::Vector::Vector3D<float>(0.0f, 1.0f, 0.0f),
									   Maths::Vector::Vector3D<float>(0.0f, 0.0f, -1.0f),
									   Maths::Vector::Vector3D<float>(0.0f, 0.0f, 1.0f));

		mActiveCamera = mDebugCamera;

		SetupUsefulVBOData();

		// ------------------------------------

		// Create the images and shadow FBO
		Rendering::Window::GetBufferStore().CreateFBO("ShadowMapFBO");

		Texture::Texture2D* shadowMapImage = new Texture::Texture2D();
		std::string         imageName      = "ShadowMap";

		shadowMapImage->InitEmpty(mShadowMapImageDimensions.x, mShadowMapImageDimensions.y, false, GL_FLOAT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, Rendering::TextureMinMagFilters(GL_LINEAR, GL_LINEAR), TextureWrappingSettings(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE));
		shadowMapImage->SetCompareMode(GL_COMPARE_REF_TO_TEXTURE, GL_LEQUAL);

		Rendering::Window::GetResourceCollection().AddResource(Rendering::ResourceType::Texture2D, imageName, shadowMapImage);

		// Shadow SSBO
		Window::GetBufferStore().CreateSSBO(nullptr, 0, GL_DYNAMIC_DRAW, "ShadowMapData_SSBO");

		// ------------------------------------

		std::vector<Rendering::Buffers::AttributePointerData> VAODataSetup;

		VAODataSetup.push_back(Buffers::AttributePointerData(0, 4, GL_FLOAT, false, 4 * sizeof(GL_FLOAT), 0, true, "VBO_full_FBO"));

		Window::GetBufferStore().CreateVAO("VAO_full_FBO", VAODataSetup);

		// ------------------------------------

		mTextManager.AddSupportedFont(FontSupported::Playfair_italic, 48);

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

	// ---------------------------------------

	void RenderPipeline::UpdateVideoFrames(const double deltaTime)
	{
		// Update all videos playing
		std::vector<std::pair<Resource*, unsigned int>> videos;
		Window::GetResourceCollection().GetResourceList(Rendering::ResourceType::VideoFrames, videos);

		unsigned int videoCount = (unsigned int)videos.size();
		for (unsigned int i = 0; i < videoCount; i++)
		{
			Rendering::VideoFrames* video = (Rendering::VideoFrames*)videos[i].first;

			if (video && video->GetActive())
			{
				video->Update(deltaTime);
			}
		}
	}

	// -------------------------------------------------

	void RenderPipeline::RenderUI()
	{
		// --------------------------------------------------------------------------------------------------

		// Videos
		std::vector<std::pair<Resource*, unsigned int>> videosLoaded;
		if (Window::GetResourceCollection().GetResourceList(ResourceType::VideoFrames, videosLoaded))
		{
			// There are videos so find which ones are active and add render them
			unsigned int videosLoadedCount = (unsigned int)videosLoaded.size();
			for (unsigned int i = 0; i < videosLoadedCount; i++)
			{
				if (videosLoaded[i].first &&
					videosLoaded[i].first->GetActive())
				{
					RenderVideo((VideoFrames*)videosLoaded[i].first);
				}
			}
		}

		// --------------------------------------------------------------------------------------------------

		// Text
		//mTextManager.AddTextToRender(Rendering::RenderTextData("Test render data to the screen!", { 100.0f,  100.0f }, 800.0f), FontSupported::Playfair_black);
		//mTextManager.AddTextToRender(Rendering::RenderTextData("This is a seperate line that loops around??!", { 100.0f, 300.0f }, 800.0f, Maths::Vector::Vector3D<float>(0.0f, 1.0f, 0.0f)), FontSupported::Playfair_italic);

		//mTextManager.Render(mActiveCamera);

		// --------------------------------------------------------------------------------------------------
	}

	// -------------------------------------------------

#ifdef _DEBUG_BUILD
	void RenderPipeline::DisplayDebugInfo()
	{
		if (mGameWorld)
		{
			mGameWorld->DisplayDebugInfoAboutWorld(mActiveCamera);
		}
	}

	// -------------------------------------------------

	void RenderPipeline::RenderDebugEditor()
	{
		ImGui::Begin("Visual Effects Editor");

		ImGui::Checkbox("Outlines", &mPostProcessingSettings.mLineOutline);
		ImGui::SliderFloat("Outline Thickness", &mPostProcessingSettings.mLineThickness, 0.0f, 1.0f);

		ImGui::Checkbox("Bloom",    &mPostProcessingSettings.mBloom);
		ImGui::SliderFloat("Bloom Factor", &mPostProcessingSettings.mBloomFactor, 0.0f, 1.0f);

		ImGui::Checkbox("Grain",    &mPostProcessingSettings.mGrain);
		ImGui::SliderFloat("Gain Factor", &mPostProcessingSettings.mGrainFactor, 0.0f, 1.0f);

		ImGui::Checkbox("Screen Space Reflections", &mPostProcessingSettings.mScreenSpaceRelfections);

		ImGui::Checkbox("Shadows",      &mVisualSettings.mShadows);
		ImGui::Checkbox("Reflections" , &mVisualSettings.mReflections);
		ImGui::Checkbox("Particles",    &mVisualSettings.mParticles);

		ImGui::Checkbox("Visualise Shadow Map", &mShowShadowMap);

		ImGui::End();
	}
#endif

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

		mDebugCamera->SetResolution((float)mScreenWidth, (float)mScreenHeight);
	}

	// -------------------------------------------------

	void RenderPipeline::SetupFinalRenderFBO()
	{
		mFinalRenderFBO = Window::GetBufferStore().GetFBO("FinalRender_FBO");

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