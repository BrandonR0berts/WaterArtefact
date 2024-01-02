#include "OpenGLRenderPipeline.h"

#include "Camera.h"
#include "GameWorld.h"

#include "RenderingResources/RenderingResources.h"
#include "Window.h"

#include "ProjectCube/Code/Video/Video.h"
#include "Shaders/ShaderProgram.h"
#include "Shaders/ShaderTypes.h"
#include "Shaders/ShaderStore.h"

#include "GLTFModel.h"

#include "Input/Code/Input.h"
#include "Input/Code/KeyboardInput.h"
#include "Input/Code/MouseInput.h"
#include "Input/Code/GamepadInput.h"

#ifdef _DEBUG_BUILD
	#include "Include/imgui/imgui.h"
	#include "Include/imgui/imgui_impl_glfw.h"
	#include "Include/imgui/imgui_impl_opengl3.h"
#endif

#include "Skybox.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Rendering
{
	// ---------------------------------------

	OpenGLRenderPipeline::OpenGLRenderPipeline()
		: RenderPipeline(RenderAPI::OpenGL)
		, mTextureUnitBindings()
		, mShaderIDBound(0)
		, mLineModeEnabled(false)
		, mDepthTestEnabled(true)
		, mDepthFunction(GL_LESS)
		, mAlphaBlendingEnabled(false)
		, mAlphaBlendingSFactor(GL_SRC_ALPHA)
		, mAlphaBlendingDFactor(GL_ONE_MINUS_SRC_ALPHA)
		, mBackFaceCullingEnabled(true)

#ifdef _DEBUG_BUILD
		, mModelMeshRenderDepth(-1)
#endif
	{
	}

	// ---------------------------------------

	OpenGLRenderPipeline::~OpenGLRenderPipeline()
	{
	}

	// -------------------------------------------------

	bool OpenGLRenderPipeline::SetupGLFW()
	{
		// Init glfw
		glfwInit();

		// Set the version
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

		// Get the current monitor
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();

		// Get the current video mode
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		// Setup the colour bits
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		// Create the window of the correct resolution
		mScreenWidth     = mode->width;
		mScreenHeight    = mode->height;

		mMaxScreenWidth  = mScreenWidth;
		mMaxScreenHeight = mScreenHeight;

		//	mWindow = glfwCreateWindow(mScreenWidth, mScreenHeight, "Game Window", glfwGetPrimaryMonitor(), NULL);
		mWindow = glfwCreateWindow(mScreenWidth, mScreenHeight, "Game Window", NULL, NULL);
		if (mWindow == nullptr)
		{
			std::cout << "Error initialising GLFW window!" << std::endl;
			glfwTerminate();
			return false;
		}
		glfwMakeContextCurrent(mWindow);

		// -------

		// Turn vSync on
		glfwSwapInterval(1);

		// GLAD setup
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "GLAD failed to initialise." << std::endl;
			return false;
		}

		// Set the default framebuffer callback for the window
		glfwSetFramebufferSizeCallback(mWindow, Rendering::framebuffer_size_callback);
		CREATE_EVENT_NAME_INSTANCE(Engine::EventSystem::kOnScreenSizeChanged);

		SetWindowFocusCallback(Rendering::window_focus_change_callback);

		REGISTER_EVENT_CALLBACK_WITH_DATA(Engine::EventSystem::kOnScreenSizeChanged, OnScreenSizeChanged);

		Input::KeyboardInput::RegisterKeyboardCallback(mWindow);
		Input::MouseInput::RegisterMouseCallback(mWindow);
		Input::MouseInput::RegisterMousePositionCallback(mWindow);
		Input::MouseInput::RegisterMouseScrollCallback(mWindow);

		glfwSetJoystickCallback(Input::GamepadInput::joystick_callback);

		// Update the viewport
		glViewport(0, 0, mScreenWidth, mScreenHeight);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		// Enable depth testing by default
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		mDepthTestEnabled = true;
		mDepthFunction = GL_LESS;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);

		mAlphaBlendingEnabled = false;
		mAlphaBlendingSFactor = GL_SRC_ALPHA;
		mAlphaBlendingDFactor = GL_ONE_MINUS_SRC_ALPHA;

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		mLineModeEnabled = false;

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glFrontFace(GL_CCW);

		glfwMakeContextCurrent(mWindow);

		return true;
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::RenderVideo(VideoFrames* video)
	{
		if (!video || !mActiveCamera)
			return;

		if (video)
		{
			// Get the shader program we are to use for this render
			ShaderPrograms::ShaderProgram* program;

			if (Window::GetShaderStore().GetShaderProgram(ShaderProgramTypes::UI_Video, program))
			{
				program->UseProgram();

				ASSERTMSG(glGetError() != 0, "OpenGL error thrown!");

				BindTextureToTextureUnit(GL_TEXTURE0, video->GetTextureID());

				glm::mat4 model = glm::mat4(1.0f);

				program->SetInt("imageToRender", 0);
				program->SetMat4("projectionMat", &model[0][0]);
				program->SetMat4("modelMat", &model[0][0]);

				ASSERTMSG(glGetError() != 0, "OpenGL error thrown!");

				// Bind the VAO for the video data
				Buffers::BufferStore& bufferStore = Window::GetBufferStore();
				bufferStore.GetVAO("VAO_Video")->Bind();
				bufferStore.GetVBO("VBO_Video")->Bind();

				ASSERTMSG(glGetError() != 0, "OpenGL error thrown!");

				program->SetBool("flipV", true);

				// Draw the image
				glDrawArrays(GL_TRIANGLES, 0, 6);

				program->SetBool("flipV", false);

				GLenum error = glGetError();
				ASSERTMSG(error != 0, "OpenGL error thrown!");
			}
		}
	}

	// -------------------------------------------------

#ifdef _DEBUG_BUILD
	void OpenGLRenderPipeline::SetupImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
		ImGui_ImplOpenGL3_Init("#version 430");
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::ShutdownImGui()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::RenderTextureBindInfo()
	{
		ImGui::Begin("Texture Bindings");

		unsigned int textureBindingCount = (unsigned int)mTextureUnitBindings.size();

		for (unsigned int i = 0; i < textureBindingCount; i++)
		{
			std::string textureBindingName = ConvertTextureBindingToString(mTextureUnitBindings[i].first);
			std::string valueBoundTo = std::to_string(mTextureUnitBindings[i].second);

			ImGui::Text((textureBindingName + " : " + valueBoundTo).c_str());
		}

		ImGui::End();
	}
#endif

	// -------------------------------------------------

	std::string OpenGLRenderPipeline::ConvertTextureBindingToString(GLenum bindingPoint)
	{
		std::string returnValue = "";

		if (bindingPoint < GL_TEXTURE0 || bindingPoint > GL_TEXTURE31)
			return returnValue;

		bindingPoint -= GL_TEXTURE0;

		returnValue = "GL_TEXTURE" + std::to_string((unsigned int)bindingPoint);

		return returnValue;
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::Update(const float deltaTime)
	{
		if (!mFinalRenderFBO)
		{
			SetupFinalRenderFBO();
		}

		if (!mShadowMap)
		{
			mShadowMap = (Rendering::Framebuffer*)Rendering::Window::GetBufferStore().GetFBO("ShadowMapFBO");

			Rendering::Texture::Texture2D* depthBuffer = (Rendering::Texture::Texture2D*)Rendering::Window::GetResourceCollection().GetResource(Rendering::ResourceType::Texture2D, "ShadowMap");

			if (depthBuffer)
				mShadowMap->AttachDepthBuffer(depthBuffer);

			mShadowMap->CheckComplete();
		}

		if (mActiveCamera)
		{
			mActiveCamera->Update(deltaTime);
		}

		// Do the occlusion culling on the current level
		if (mGameWorld)
		{
			mGameWorld->Update(deltaTime, mActiveCamera);
		}

		UpdateVideoFrames(deltaTime);
	}

	// -------------------------------------------------

	bool OpenGLRenderPipeline::HandlePlayingVideos()
	{
		std::vector<std::pair<Resource*, unsigned int>> videosLoaded;
		Window::GetResourceCollection().GetResourceList(ResourceType::VideoFrames, videosLoaded);

		// See if any of the videos are active
		bool videoActive = false;

		unsigned int videoCount = (unsigned int)videosLoaded.size();
		for (unsigned int i = 0; i < videoCount; i++)
		{
			if (videosLoaded[i].first->GetActive())
			{
				videoActive = true;
				break;
			}
		}

		if (videoActive)
		{
			mFinalRenderFBO->SetActive(false, true);

			SetDepthTestEnabled(false);

			unsigned int videoCount = (unsigned int)videosLoaded.size();
			for (unsigned int i = 0; i < videoCount; i++)
			{
				if (videosLoaded[i].first &&
					videosLoaded[i].first->GetActive())
				{
					RenderVideo((VideoFrames*)videosLoaded[i].first);
				}
			}

			return true;
		}

		return false;
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::RenderGameWorld()
	{
		SetDepthTestEnabled(true);

		if (mGameWorld)
		{
			mGameWorld->Render(mActiveCamera);

#ifdef _DEBUG_BUILD
			mGameWorld->DebugRender(mActiveCamera);
#endif
		}

//		if (mCurrentGameLevel)
//		{
//			if (!mCurrentGameLevel->GetProbesHaveBeenInitialised())
//			{
//				mCurrentGameLevel->RenderCubemapProbesData();
//			}
//
//			// ---------------------------------------------
//
//			// Generate shadow maps for the scene
//			/*if (mVisualSettings.mShadows)
//			{
//				mFinalRenderFBO->SetActive(false , true);
//
//				GenerateShadowMaps();
//
//				mFinalRenderFBO->SetActive(true, true);
//			}*/
//
//			// ---------------------------------------------
//
//			// Render the static geometry in the area around the player
//			ShaderPrograms::ShaderProgram* shadersToUse = nullptr;
//			if (Window::GetShaderStore().GetShaderProgram(Rendering::ShaderProgramTypes::GLTF_Instance_Program, shadersToUse))
//			{
//				if (mVisualSettings.mShadows)
//				{
//					mCurrentGameLevel->Render(mActiveCamera, shadersToUse, false, (int)mShadowMap->GetDepthBuffer()->GetTextureID(), nullptr);// mCurrentGameLevel->GetClosestRenderProbeToCamera(mActiveCamera));
//				}
//				else
//				{
//					mCurrentGameLevel->Render(mActiveCamera, shadersToUse, false, -1, nullptr);// mCurrentGameLevel->GetClosestRenderProbeToCamera(mActiveCamera));
//				}
//			}
//
//#ifdef _DEBUG_BUILD
//			if (mCurrentGameLevel && mCurrentGameLevel->ShowingCubemapProbes())
//			{
//				mCurrentGameLevel->RenderCubemapProbes(mActiveCamera);
//			}
//#endif
//		}
	}
	// -------------------------------------------------

	void OpenGLRenderPipeline::Render()
	{
		// -----------------------------------------------------------

#ifdef _DEBUG_BUILD
		SetLineModeEnabled(Window::GetLineMode());
#endif

		// -----------------------------------------------------------

		// Video playing - takes priority as it is most likely a cutscene
		if (HandlePlayingVideos())
			return;
		
		// -----------------------------------------------------------

		// Make sure we are rendering to the offscreen buffer
		mFinalRenderFBO->SetActive(true, true);

		// Render the parts of the world that are not moving and can be rendered using instance rendering
		RenderGameWorld();

		// --------------------------------

		// Now render the UI to the screen on top of the frame so far
		RenderUI();

		// --------------------------------

#ifdef _DEBUG_BUILD	
		OpenGLRenderPipeline::SetLineModeEnabled(Window::GetLineMode());
#endif

		// --------------------------------

		// Now display the final buffer to the screen
		FinalRenderToScreen();

		// --------------------------------
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::FinalRenderToScreen() 
	{
		if (!mFinalRenderFBO)
			return;

		mFinalRenderFBO->SetActive(false, true);

		// Now render the colour buffer stored in the final render buffer to the screen as a quad 
		ShaderPrograms::ShaderProgram* finalRenderProgram;
		if (Window::GetShaderStore().GetShaderProgram(Rendering::ShaderProgramTypes::UI_Video, finalRenderProgram))
		{
			SetDepthTestEnabled(false);

			// Use this program
			finalRenderProgram->UseProgram();

#ifdef _DEBUG_BUILD
			if (mShowShadowMap)
			{
				BindTextureToTextureUnit(GL_TEXTURE0, mShadowMap->GetDepthBuffer()->GetTextureID());
			}
			else
#endif
			{
				BindTextureToTextureUnit(GL_TEXTURE0, mFinalRenderFBO->GetColourBuffer()->GetTextureID());
			}

			//float*    projMatrix = &GetActiveCamera()->GetOrthoMatrix()[0][0];
			glm::mat4 model = glm::mat4(1.0f);
			// model = glm::scale(model, glm::vec3((float)Window::GetWindowWidth(), (float)Window::GetWindowHeight(), 1.0f));

			finalRenderProgram->SetInt("imageToRender", 0);
			finalRenderProgram->SetMat4("projectionMat", &model[0][0]);
			finalRenderProgram->SetMat4("modelMat", &model[0][0]);

			// Bind the VAO for the video data
			Buffers::BufferStore& bufferStore = Window::GetBufferStore();
			Buffers::VertexArrayObject*  vao = bufferStore.GetVAO("VAO_Video");
			Buffers::VertexBufferObject* vbo = bufferStore.GetVBO("VBO_Video");

			if (vao && vbo)
			{
				vao->Bind();
				vbo->Bind();

				// Draw the image to the screen
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::GenerateShadowMaps()
	{
		//ShadowData shadowData;

		//// ---------------------

		//Rendering::LightCollection globalLights  = mGameWorld->GetGlobalLights();
		//Rendering::LightCollection segmentLights = mGameWorld->GetActiveSegmentLights();

		//unsigned int               globalsCount  = globalLights.size();
		//unsigned int               segmentCount  = segmentLights.size();

		//if (globalsCount == 0 && segmentCount == 0)
		//	return;

		//unsigned int directionalLightCount = (unsigned int)globalLights.GetAllDirectionalLights().size() + (unsigned int)segmentLights.GetAllDirectionalLights().size();
		//unsigned int pointLightCount       = (unsigned int)globalLights.GetAllPointLights().size()       + (unsigned int)segmentLights.GetAllPointLights().size();

		//// Clear the old image
		//mShadowMap->SetActive(true, true);
		//mShadowMap->ClearDepthBuffer();

		//// ---------------------

		//// See how is best to split the image and create the image segments for this
		//if (globalsCount == 0) // Taken up entirely by segment lights
		//{
		//	float sqrt    = std::sqrtf((float)segmentCount);
		//	float portion = 1.0f;

		//	if ((int)ceilf(sqrt) != 1)
		//	{
		//		// Increase the value by 1 to make sure we can definitially fit all of the maps in the image
		//		sqrt = floor(sqrt);
		//		sqrt += 1.0f;

		//		// Calculate the fraction each one will take up
		//		portion = 1.0f / sqrt;
		//	}

		//	for (unsigned int i = 0; i < segmentCount; i++)
		//	{
		//		unsigned int x = i % (unsigned int)sqrt;
		//		unsigned int y = i / (unsigned int)sqrt;

		//		ShadowImageSegment newSegment = ShadowImageSegment(Maths::Vector::Vector2D<float>(portion * x,       portion * y),
		//							                               Maths::Vector::Vector2D<float>(portion * (x + 1), portion * (y + 1)));

		//		RenderShadowIntoMap(newSegment, segmentLights, i, portion);

		//		shadowData.mImageSegments.push_back(newSegment);
		//	}
		//}
		//else if (segmentCount == 0) // Taken up entirely by global lights
		//{
		//	float sqrt    = std::sqrtf((float)globalsCount);
		//	float portion = 1.0f;

		//	if ((int)ceilf(sqrt) != 1)
		//	{
		//		// Increase the value by 1 to make sure we can definitially fit all of the maps in the image
		//		sqrt  = floor(sqrt);

		//		if (unsigned int(sqrt * sqrt) != globalsCount)
		//		{
		//			sqrt += 1.0f;
		//		}

		//		// Calculate the fraction each one will take up
		//		portion = 1.0f / sqrt;
		//	}

		//	for (unsigned int i = 0; i < globalsCount; i++)
		//	{
		//		unsigned int x = i % (unsigned int)sqrt;
		//		unsigned int y = i / (unsigned int)sqrt;

		//		ShadowImageSegment newSegment = ShadowImageSegment(Maths::Vector::Vector2D<float>(portion * x,       portion *  y),
		//			                                               Maths::Vector::Vector2D<float>(portion * (x + 1), portion * (y + 1)));

		//		RenderShadowIntoMap(newSegment, globalLights, i, portion);

		//		shadowData.mImageSegments.push_back(newSegment);
		//	}
		//}
		//else // Split between the two
		//{

		//}

		//// ---------------------

		//mShadowMap->SetActive(false, true);

		//// ---------------------

		//// Make sure to re-bind the final render FBO
		//if(mFinalRenderFBO)
		//	mFinalRenderFBO->SetActive(true, true);

		//// Reset the viewport
		//glViewport(0, 0, mScreenWidth, mScreenHeight);

		//// ---------------------

		//// Update the shadow SSBO with the new data
		//Buffers::ShaderStorageBufferObject* shadowSSBO = Window::GetBufferStore().GetSSBO("ShadowMapData_SSBO");

		//if (shadowSSBO)
		//{
		//	unsigned int                    vec4sInData = 1 + ((unsigned int)shadowData.mImageSegments.size() * 8); // * 6 as there is one mat4 and 4 vec4 per bit of shadow info
		//	Maths::Vector::Vector4D<float>* newData     = new Maths::Vector::Vector4D<float>[vec4sInData];

		//	// Populate the data - start with count
		//	newData[0] = Maths::Vector::Vector4D<float>((float)directionalLightCount, (float)pointLightCount, 0.0f, 0.0f);

		//	// now real data
		//	unsigned int currentID = 0;
		//	for (size_t i = 0; i < shadowData.mImageSegments.size(); i++)
		//	{
		//		newData[currentID + 1] = Maths::Vector::Vector4D<float>(shadowData.mImageSegments[i].mBottomLeftUV.x,
		//			                                                    shadowData.mImageSegments[i].mBottomLeftUV.y,

		//			                                                    shadowData.mImageSegments[i].mTopRightUV  .x,
		//			                                                    shadowData.mImageSegments[i].mTopRightUV  .y);

		//		newData[currentID + 2] = shadowData.mImageSegments[i].mMVPMatrix[0];
		//		newData[currentID + 3] = shadowData.mImageSegments[i].mMVPMatrix[1];
		//		newData[currentID + 4] = shadowData.mImageSegments[i].mMVPMatrix[2];
		//		newData[currentID + 5] = shadowData.mImageSegments[i].mMVPMatrix[3];

		//		newData[currentID + 6] = shadowData.mImageSegments[i].mLightPosition;

		//		newData[currentID + 7] = shadowData.mImageSegments[i].mLightDirection;
		//		newData[currentID + 8] = shadowData.mImageSegments[i].mLightColour;

		//		currentID += 8;
		//	}

		//	shadowSSBO->UpdateBufferData(newData, vec4sInData * sizeof(Maths::Vector::Vector4D<float>), GL_DYNAMIC_DRAW);

		//	// Make sure to clear up the memory allocated
		//	delete[] newData;
		//	newData = nullptr;
		//}

		// ---------------------
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::RenderShadowIntoMap(ShadowImageSegment& segmentData, Rendering::LightCollection& lights, unsigned int index, float imagePortion)
	{
		//if (!mShadowMap)
		//	return;

		//Maths::Vector::Vector2D<int> positionOnImage   = Maths::Vector::Vector2D<int>((int)(segmentData.mBottomLeftUV.x * mShadowMapImageDimensions.x), (int)(segmentData.mBottomLeftUV.y * mShadowMapImageDimensions.y));
		//Maths::Vector::Vector2D<int> segmentDimensions = Maths::Vector::Vector2D<int>((int)((float)mShadowMapImageDimensions.x * imagePortion), (int)((float)mShadowMapImageDimensions.y * imagePortion));

		//glViewport(positionOnImage.x, positionOnImage.y, segmentDimensions.x, segmentDimensions.y);

		//// Grab the shader program we are going to being using for this
		//Rendering::ShaderPrograms::ShaderProgram* program;
		//if (Window::GetShaderStore().GetShaderProgram(Rendering::ShaderProgramTypes::Shadow_Generation_Program, program))
		//{
		//	// Get the light
		//	unsigned int directionalLightCount = (unsigned int)lights.GetAllDirectionalLights().size();
		//	if (index >= directionalLightCount)
		//	{
		//		PointLight pointLight = lights.GetAllPointLights()[index - directionalLightCount];

		//		// Create a camera from the light's perspective
		//		DebugCamera newCamera = DebugCamera(0.01f, pointLight.mIntensity, 7.5f, 7.5f, 16.0f / 9.0f, 360.0f, Maths::Vector::Vector3D<float>(0.0f, 1.0f, 0.0f), Maths::Vector::Vector3D<float>(0.0f, 0.0f, 1.0f), pointLight.mPosition);

		//		mGameWorld->Render(&newCamera, program, false);

		//		glm::mat4 VPMatrix           = newCamera.GetPerspectiveMatrix() * newCamera.GetViewMatrix();
		//		segmentData.mMVPMatrix       = VPMatrix;
		//		segmentData.mLightPosition   = Maths::Vector::Vector4D<float>(pointLight.mPosition, 0.0f);
		//		segmentData.mLightDirection  = Maths::Vector::Vector4D<float>(1.0f);
		//		segmentData.mLightColour     = Maths::Vector::Vector4D<float>(pointLight.mColour * pointLight.mIntensity, 1.0f);

		//	}
		//	else // Directional light
		//	{				 
		//		DirectionalLight directionalLight = lights.GetAllDirectionalLights()[index];

		//		OrthographicCamera newCamera = OrthographicCamera(0.01f, 7.5f, 7.5f, 7.5f, Maths::Vector::Vector3D<float>(0.0f, 1.0f, 0.0f), directionalLight.mDirection, directionalLight.mPosition);

		//		glm::mat4 VPMatrix           = newCamera.GetPerspectiveMatrix() * newCamera.GetViewMatrix();
		//		segmentData.mMVPMatrix       = VPMatrix;
		//		segmentData.mLightPosition   = Maths::Vector::Vector4D<float>(directionalLight.mPosition, 0.0f);
		//		segmentData.mLightDirection  = Maths::Vector::Vector4D<float>(directionalLight.mDirection, 1.0f);
		//		segmentData.mLightColour     = Maths::Vector::Vector4D<float>(directionalLight.mColour, 1.0f);

		//		mGameWorld->Render(&newCamera, program, false);
		//	}
		//}
	}

	// -------------------------------------------------

	Texture::CubeMapTexture* OpenGLRenderPipeline::RenderIntoCubemap(Maths::Vector::Vector3D<float> position, Maths::Vector::Vector2D<unsigned int> resolution, std::string newTextureName, bool generateMipMaps)
	{
		Texture::CubeMapTexture* newTexture = new Texture::CubeMapTexture();
		newTexture->SetupInteralData(resolution.x, resolution.y, GL_UNSIGNED_BYTE, GL_RGB, GL_RGB, { GL_LINEAR, GL_LINEAR }, { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE });

		RenderIntoCubemap(position, newTexture, resolution, generateMipMaps);

		// Add to store
		Window::GetResourceCollection().AddResource(ResourceType::CubeMap, newTextureName, newTexture);

		return newTexture;
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::RenderIntoCubemap(Maths::Vector::Vector3D<float> position, Texture::CubeMapTexture* textureToWriteInto, Maths::Vector::Vector2D<unsigned int> resolution, bool generateMipMaps)
	{
		// Set the right viewport size
		glViewport(0, 0, resolution.x, resolution.y);

		// View matricies for the 6 different view angles
		static glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		// Projection matrix to make sure we fit the whole 90 degrees into view
		static glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

		Framebuffer* FBO = new Framebuffer();
		FBO->SetActive(true, true);

		// Loop through each face
		for (unsigned int i = 0; i < 6; i++)
		{
			// Bind the correct side of the cubemap to draw into
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, textureToWriteInto->GetTextureID(), 0);

			// Clear existing data
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Render the level parts we want to see into the texture
			RenderForReflections(position, captureViews[i], projectionMatrix);
		}

		if (generateMipMaps)
		{
			textureToWriteInto->Bind();
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

		// ----

		FBO->SetActive(false, true);
		delete FBO;

		// ----

		// Reset viewport back to the window size
		glViewport(0, 0, mScreenWidth, mScreenHeight);
	}

	// -------------------------------------------------

	// Assumes that everything has been setup correctly before calling - so this just calls the correct render functions
	void OpenGLRenderPipeline::RenderForReflections(Maths::Vector::Vector3D<float> positon, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, bool includeShadows, bool includeParticles)
	{
		// Render the level
		//ShaderPrograms::ShaderProgram* shadersToUse = nullptr;

		//if (Window::GetShaderStore().GetShaderProgram(Rendering::ShaderProgramTypes::GLTF_Instance_Program, shadersToUse))
		//{
		//	if (mVisualSettings.mShadows && includeShadows)
		//	{
		//		mGameWorld->RenderForReflections(positon, viewMatrix, projectionMatrix, shadersToUse, false, (int)mShadowMap->GetDepthBuffer()->GetTextureID());
		//	}
		//	else
		//	{
		//		mGameWorld->RenderForReflections(positon, viewMatrix, projectionMatrix, shadersToUse, false);
		//	}
		//}

		//// Now render particle systems
		//if (includeParticles)
		//{
		//	
		//}

		//// Render the skybox
		//Skybox* skybox = mGameWorld->GetSkybox();
		//if (skybox)
		//{
		//	skybox->Render(mActiveCamera);
		//}
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::BindTextureToTextureUnit(GLenum textureUnit, unsigned int textureUnitID, bool isTexture2D)
	{
		// Check if the texture unit is valid
		if (textureUnit < GL_TEXTURE0 || textureUnit > GL_TEXTURE31)
		{
			return;
		}

		unsigned int bindingsCount = (unsigned int)mTextureUnitBindings.size();
		for (unsigned int i = 0; i < bindingsCount; i++)
		{
			if (mTextureUnitBindings[i].first == textureUnit)
			{
				// Check for setting to same value
				if (mTextureUnitBindings[i].second == textureUnitID)
				{
					return;
				}

				mTextureUnitBindings[i].second = textureUnitID;

				glActiveTexture(textureUnit);

				if (isTexture2D)
				{
					glBindTexture(GL_TEXTURE_2D, textureUnitID);
				}
				else
				{
					glBindTexture(GL_TEXTURE_CUBE_MAP, textureUnitID);
				}

				GLenum error = glGetError();
				ASSERTMSG(error != 0, "Error binding texture");

				return;
			}
		}

		mTextureUnitBindings.push_back({ textureUnit, textureUnitID });

		glActiveTexture(textureUnit);
		if (isTexture2D)
		{
			glBindTexture(GL_TEXTURE_2D, textureUnitID);
		}
		else
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, textureUnitID);
		}

		GLenum error = glGetError();
		ASSERTMSG(error != 0, "Error binding texture");
	}

	// -------------------------------------------------

	unsigned int OpenGLRenderPipeline::QueryCurrentlyBoundTextureID(GLenum textureUnit)
	{
		unsigned int bindingsCount = (unsigned int)mTextureUnitBindings.size();
		for (unsigned int i = 0; i < bindingsCount; i++)
		{
			if (mTextureUnitBindings[i].first == textureUnit)
			{
				return mTextureUnitBindings[i].second;
			}
		}

		return 0;
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetActiveShader(unsigned int shaderID)
	{
		if (shaderID == mShaderIDBound)
			return;

		mShaderIDBound = shaderID;

		glUseProgram(shaderID);
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetLineModeEnabled(bool state)
	{
		if (state == mLineModeEnabled)
			return;

		mLineModeEnabled = state;

		if (state)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetDepthTestEnabled(bool state)
	{
		if (state == mDepthTestEnabled)
			return;

		mDepthTestEnabled = state;

		if (state)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
	}

	// -------------------------------------------------

    void OpenGLRenderPipeline::SetAlphaBlending(bool state)
	{
		if (state == mAlphaBlendingEnabled)
			return;

		mAlphaBlendingEnabled = state;

		if (state)
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetAlphaBlendingFunction(GLenum sFactor, GLenum dFactor)
	{
		if (sFactor == mAlphaBlendingSFactor && dFactor == mAlphaBlendingDFactor)
			return;

		mAlphaBlendingSFactor = sFactor;
		mAlphaBlendingDFactor = dFactor;

		glBlendFunc(sFactor, dFactor);
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetBackFaceCulling(bool state)
	{
		if (state == mBackFaceCullingEnabled)
			return;

		mBackFaceCullingEnabled = state;

		if (mBackFaceCullingEnabled)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetDepthTestFunction(GLenum state)
	{
		if (state == mDepthFunction)
			return;

		mDepthFunction = state;

		glDepthFunc(state);
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetWindowFocusCallback(void(*focusFunction)(GLFWwindow* window, int focused))
	{
		glfwSetWindowFocusCallback(mWindow, focusFunction);
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetKeypressCallback(GLFWkeyfun HandleKeyboardInputCallback)
	{
		glfwSetKeyCallback(mWindow, HandleKeyboardInputCallback);
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetCursorEnterCallback(GLFWcursorenterfun CursorEnterCallback)
	{
		glfwSetCursorEnterCallback(mWindow, CursorEnterCallback);
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetGetMousePositionCallback(GLFWcursorposfun MousePositionCallback)
	{
		glfwSetCursorPosCallback(mWindow, MousePositionCallback);
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetMouseButtonPressCallback(GLFWmousebuttonfun MouseButtonPressCallback)
	{
		glfwSetMouseButtonCallback(mWindow, MouseButtonPressCallback);
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetMouseScrollWheelCallback(GLFWscrollfun ScrollWheelCallback)
	{
		glfwSetScrollCallback(mWindow, ScrollWheelCallback);
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::ResetTextureBindingInfo()
	{
		mTextureUnitBindings.clear();
	}

	// -------------------------------------------------

	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);

		Window::GetRenderPipeline()->SetWindowWidth(width);
		Window::GetRenderPipeline()->SetWindowHeight(height);

		Window::SetWindowBeingResized(true);
	}

	// -------------------------------------------------

	void window_focus_change_callback(GLFWwindow* window, int focus)
	{
		Window::SetFocused(focus);
	}

	// -------------------------------------------------
}