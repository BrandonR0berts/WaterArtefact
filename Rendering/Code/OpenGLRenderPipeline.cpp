#include "OpenGLRenderPipeline.h"

#include "Camera.h"
#include "Window.h"

#include "Shaders/ShaderProgram.h"
#include "Shaders/ShaderTypes.h"

#include "Input/Code/Input.h"
#include "Input/Code/KeyboardInput.h"
#include "Input/Code/MouseInput.h"

#include "Include/imgui/imgui.h"
#include "Include/imgui/imgui_impl_glfw.h"
#include "Include/imgui/imgui_impl_opengl3.h"

#include "Skybox.h"
#include "Framebuffers.h"

#include "Rendering/Code/Skybox.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Rendering
{
	// ---------------------------------------

	OpenGLRenderPipeline::OpenGLRenderPipeline()
		: RenderPipeline()
		, mShaderIDBound(0)
		, mLineModeEnabled(false)
		, mDepthTestEnabled(true)
		, mDepthFunction(GL_LESS)
		, mAlphaBlendingEnabled(false)
		, mAlphaBlendingSFactor(GL_SRC_ALPHA)
		, mAlphaBlendingDFactor(GL_ONE_MINUS_SRC_ALPHA)
		, mBackFaceCullingEnabled(true)

		, mVAOVideo(nullptr)
		, mVBOVideo(nullptr)

		, mColourTexture(nullptr)
		, mDepthStencilTexture(nullptr)

		, mFinalRenderProgram(nullptr)

		, mWaterSimulation(nullptr)
		, mSkybox(nullptr)
	{

	}

	// ---------------------------------------

	OpenGLRenderPipeline::~OpenGLRenderPipeline()
	{
		//delete mWaterSimulation;
		//mWaterSimulation = nullptr;

		//delete mSkybox;
		//mSkybox = nullptr;
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

		SetWindowFocusCallback(Rendering::window_focus_change_callback);


		Input::KeyboardInput::RegisterKeyboardCallback(mWindow);
		Input::MouseInput::RegisterMouseCallback(mWindow);
		Input::MouseInput::RegisterMousePositionCallback(mWindow);
		Input::MouseInput::RegisterMouseScrollCallback(mWindow);

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

		SetupShaders();

		mWaterSimulation = new WaterSimulation();
		mSkybox          = new Skybox("Sky", "Skybox/Day/Right.bmp", "Skybox/Day/Left.bmp", "Skybox/Day/Top.bmp", "Skybox/Day/Bottom.bmp", "Skybox/Day/Front.bmp", "Skybox/Day/Back.bmp");

		return true;
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::SetupShaders()
	{
		// -------------------------------------------------------------

		mFinalRenderProgram = new ShaderPrograms::ShaderProgram();

		Shaders::VertexShader*   vertexShader   = new Shaders::VertexShader("Code/Shaders/Vertex/VideoVertexShader.vert");
		Shaders::FragmentShader* fragmentShader = new Shaders::FragmentShader("Code/Shaders/Fragment/VideoFragmentShader.frag");

		mFinalRenderProgram->AttachShader(vertexShader);
		mFinalRenderProgram->AttachShader(fragmentShader);

		mFinalRenderProgram->LinkShadersToProgram();

		mFinalRenderProgram->DetachShader(vertexShader);
		mFinalRenderProgram->DetachShader(fragmentShader);

		delete vertexShader;
		delete fragmentShader;

		// -------------------------------------------------------------

		mVAOVideo = new Buffers::VertexArrayObject();
		mVAOVideo->Bind();

			mVBOVideo = new Buffers::VertexBufferObject();
			static float renderingData[24] = { -1.0f, 1.0f, 0.0f, 1.0f,
											   -1.0f, -1.0f, 0.0f, 0.0f,
												1.0f, -1.0f, 1.0f, 0.0f,

											   -1.0f, 1.0f, 0.0f, 1.0f,
												1.0f, -1.0f, 1.0f, 0.0f,
												1.0f, 1.0f, 1.0f, 1.0f };

			mVBOVideo->SetTarget(GL_ARRAY_BUFFER);
			mVBOVideo->SetBufferData((void*)renderingData, 96, GL_STATIC_DRAW);

			mVBOVideo->Bind();
				mVAOVideo->EnableVertexAttribArray(0);
				mVAOVideo->SetVertexAttributePointers(0, 4, GL_FLOAT, false, 4 * sizeof(GL_FLOAT), 0, true);

			mVAOVideo->Unbind();
		mVBOVideo->UnBind();

		// -------------------------------------------------------------

		if (!mFinalRenderFBO)
		{
			mFinalRenderFBO = new Framebuffer();

			unsigned int screenWidth  = GetScreenWidth();
			unsigned int screenHeight = GetScreenHeight();

			// Colour buffer
			mColourTexture = new Texture::Texture2D();
			mColourTexture->InitEmpty(screenWidth, screenHeight, false, GL_UNSIGNED_BYTE, GL_RGB, GL_RGB);

			// Depth buffer
			mDepthStencilTexture = new Texture::Texture2D();
			mDepthStencilTexture->InitEmpty(screenWidth, screenHeight, false, GL_UNSIGNED_INT_24_8, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL);

			mFinalRenderFBO->AttachColourBuffer(mColourTexture);
			mFinalRenderFBO->AttachDepthStencilBuffer(mDepthStencilTexture);

			mFinalRenderFBO->CheckComplete();
		}
	}

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
		if (mActiveCamera)
		{
			mActiveCamera->Update(deltaTime);
		}

		if (mWaterSimulation)
		{
			mWaterSimulation->Update(deltaTime);
		}
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::Render()
	{
		if (!mFinalRenderFBO || !mWaterSimulation)
			return;

		// -----------------------------------------------------------

		SetLineModeEnabled(Window::GetLineMode());

		// -----------------------------------------------------------

		// Make sure we are rendering to the offscreen buffer
		mFinalRenderFBO->SetActive(true, true);

		if (mSkybox)
		{
			mSkybox->Render(mActiveCamera);
		}

		// --------------------------------

		OpenGLRenderPipeline::SetLineModeEnabled(Window::GetLineMode());

		// --------------------------------

		// See if the camera is above or below the surface
		if (mWaterSimulation->IsBelowSurface(mActiveCamera->GetPosition()))
		{

		}
		else
		{
			// Render the sub-surface models

			// Render the water's surface
			mWaterSimulation->Render(mActiveCamera, mSkybox->GetCubeMapTexture());

			// Render everything above the surface
		}

		// --------------------------------

		mFinalRenderFBO->SetActive(false, true);

		// --------------------------------

		// Now display the final buffer to the screen
		FinalRenderToScreen();

		// --------------------------------
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline:: RenderDebugMenu()
	{
		if (mWaterSimulation)
			mWaterSimulation->RenderDebugMenu();

		ImGui::Begin("Buffer visualisations");

			if (ImGui::Button("View positional buffer"))
			{
				mDebugVisualisationOverride = BufferViewOverrideTypes::Position;
			}

			if (ImGui::Button("View normal buffer"))
			{
				mDebugVisualisationOverride = BufferViewOverrideTypes::Normal;
			}

			if (ImGui::Button("View tangent buffer"))
			{
				mDebugVisualisationOverride = BufferViewOverrideTypes::Tangent;
			}

			if (ImGui::Button("View binormal buffer"))
			{
				mDebugVisualisationOverride = BufferViewOverrideTypes::Binormal;
			}

			if (ImGui::Button("View H0 buffer"))
			{
				mDebugVisualisationOverride = BufferViewOverrideTypes::H0;
			}

			if (ImGui::Button("Reset"))
			{
				mDebugVisualisationOverride = BufferViewOverrideTypes::None;
			}

		ImGui::End();

		ImGui::Begin("Camera");

			float farDistance = mActiveCamera->GetFarDistance();
			if (ImGui::InputFloat("Far Distance", &farDistance))
			{
				mActiveCamera->SetFarDistance(farDistance);
			}

			Maths::Vector::Vector3D<float> cameraPos = mActiveCamera->GetPosition();
			if (ImGui::InputFloat3("Camera Position", &cameraPos.x))
			{
				mActiveCamera->SetPosition(cameraPos);
			}

		ImGui::End();
	}

	// -------------------------------------------------

	void OpenGLRenderPipeline::FinalRenderToScreen() 
	{
		if (!mFinalRenderFBO || !mFinalRenderProgram)
			return;

		mFinalRenderFBO->SetActive(false, true);

		// Now render the colour buffer stored in the final render buffer to the screen as a quad 
		SetDepthTestEnabled(false);

		// Use this program
		mFinalRenderProgram->UseProgram();

		switch (mDebugVisualisationOverride)
		{
		case BufferViewOverrideTypes::None:
			BindTextureToTextureUnit(GL_TEXTURE0, mFinalRenderFBO->GetColourBuffer()->GetTextureID());
		break;

		case BufferViewOverrideTypes::Position:
			BindTextureToTextureUnit(GL_TEXTURE0, mWaterSimulation->GetPositionalBuffer()->GetTextureID());
		break;

		case BufferViewOverrideTypes::Normal:
			BindTextureToTextureUnit(GL_TEXTURE0, mWaterSimulation->GetNormalBuffer()->GetTextureID());
			break;

		case BufferViewOverrideTypes::Tangent:
			BindTextureToTextureUnit(GL_TEXTURE0, mWaterSimulation->GetTangentBuffer()->GetTextureID());
			break;

		case BufferViewOverrideTypes::Binormal:
			BindTextureToTextureUnit(GL_TEXTURE0, mWaterSimulation->GetBinormalBuffer()->GetTextureID());
			break;

		case BufferViewOverrideTypes::H0:
			BindTextureToTextureUnit(GL_TEXTURE0, mWaterSimulation->GetH0Buffer()->GetTextureID());
		break;
		}

		//float*    projMatrix = &GetActiveCamera()->GetOrthoMatrix()[0][0];
		glm::mat4 model = glm::mat4(1.0f);
		// model = glm::scale(model, glm::vec3((float)Window::GetWindowWidth(), (float)Window::GetWindowHeight(), 1.0f));

		mFinalRenderProgram->SetInt("imageToRender",  0);
		mFinalRenderProgram->SetMat4("projectionMat", &model[0][0]);
		mFinalRenderProgram->SetMat4("modelMat",      &model[0][0]);

		if (mVAOVideo && mVBOVideo)
		{
			mVAOVideo->Bind();
			mVBOVideo->Bind();

			// Draw the image to the screen
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
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