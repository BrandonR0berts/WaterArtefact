#include "Window.h"

#include "Camera.h"
#include "Textures/Texture.h"

#include "Include/imgui/imgui.h"
#include "Include/imgui/imgui_impl_glfw.h"
#include "Include/imgui/imgui_impl_opengl3.h"

#include "Maths/Code/PerformanceAnalysis.h"

#include "Rendering/Code/OpenGLRenderPipeline.h"

#include "Input/Code/KeyboardInput.h"
#include "Input/Code/MouseInput.h"

#include "Framebuffers.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <thread>

namespace Rendering
{
	// -------------------------------------------------

	bool                 Window::mInitialised                   = false;
	bool                 Window::sWindowBeingResized            = false;

	bool                 Window::mWindowFocused                 = true;

	RenderPipeline*      Window::sRenderPipeline                = nullptr;

	bool                 Window::sUsingDebugOverlay             = false;
	Engine::Timer::Timer Window::sDebugToggleTimer              = Engine::Timer::Timer();
	float                Window::sTimeBetweenDebugToggles       = 0.5f;
	unsigned int         Window::sFramesRenderedThisSecondSoFar = 0;
	unsigned int         Window::sFramesRenderedLastSecond      = 0;
	bool                 Window::mActiveModelLineMode           = false;

	// -------------------------------------------------

	Window::Window()
		: mIsVsyncOn(true)
		, mWindowShouldClose(false)
		, mOldTime(0.0)
		, mDisplayCameraDebugInfo(false)
	{		
		// Kick off the render thread
		mRenderThread = std::thread(&Window::RenderThread, this);
	}

	// -------------------------------------------------

	Window::~Window()
	{
		mWindowShouldClose = true;

		mRenderThread.join();
	}

	// -------------------------------------------------

	void Window::Update(const float deltaTime)
	{

	}

	// -------------------------------------------------

	void Window::ShutdownGLFW()
	{
		GLFWwindow* window = sRenderPipeline->GetWindow();
		glfwDestroyWindow(window);
	}

	// -------------------------------------------------

	void Window::SwapBuffers()
	{
		GLFWwindow* window = sRenderPipeline->GetWindow();
		glfwSwapBuffers(window);
	}

	// -------------------------------------------------

	void Window::SetBackgroundColour(const Maths::Vector::Vector4D<float>& colour)
	{
		glClearColor(colour.x, colour.y, colour.z, colour.w);
	}

	// -------------------------------------------------

	void Window::ClearBuffers()
	{
		if (sRenderPipeline)
			sRenderPipeline->ClearFinalRenderBuffer();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	// -------------------------------------------------

	void Window::PollEvents()
	{
		if (!mWindowShouldClose)
		{
			GLFWwindow* window = sRenderPipeline->GetWindow();
			mWindowShouldClose = (bool)glfwWindowShouldClose(window);
		}

		glfwPollEvents();
	}

	// -------------------------------------------------

	void Window::CameraDebugWindow()
	{
		ImGui::Begin("Camera Window");

		Camera*                        activeCamera = sRenderPipeline->GetActiveCamera();
		Maths::Vector::Vector3D<float> pos          = activeCamera->GetPosition();
		Maths::Vector::Vector3D<float> forwards     = activeCamera->GetForward();
		Maths::Vector::Vector3D<float> up           = activeCamera->GetUp();

		if (ImGui::InputFloat3("Camera Position: ", &pos.x))
		{
			activeCamera->SetPosition(pos);
		}

		ImGui::Text(std::string("Camera Forwards: " + std::to_string(forwards.x) + ", " + std::to_string(forwards.y) + ", " + std::to_string(forwards.z)).c_str());
		ImGui::Text(std::string("Camera Up: "       + std::to_string(up.x)       + ", " + std::to_string(up.y)       + ", " + std::to_string(up.z)).c_str());

		ImGui::Text("|||||||||||||||||||||||");

		ImGui::Text("Perspective Matrix");

		glm::mat4 projectionMatrix = activeCamera->GetPerspectiveMatrix();

		for (unsigned int i = 0; i < 4; i++)
		{
			std::string dataString = std::to_string(projectionMatrix[i][0]) + "\t" + std::to_string(projectionMatrix[i][1]) + "\t" + std::to_string(projectionMatrix[i][2]) + "\t" + std::to_string(projectionMatrix[i][3]);
			ImGui::Text((dataString).c_str());
		}

		ImGui::Text("|||||||||||||||||||||||");

		ImGui::Text("Orthographic Matrix");

		glm::mat4 orthoMatrix = activeCamera->GetOrthoMatrix();

		for (unsigned int i = 0; i < 4; i++)
		{
			std::string dataString = std::to_string(orthoMatrix[i][0]) + "\t" + std::to_string(orthoMatrix[i][1]) + "\t" + std::to_string(orthoMatrix[i][2]) + "\t" + std::to_string(orthoMatrix[i][3]);
			ImGui::Text((dataString).c_str());
		}

		ImGui::Text("|||||||||||||||||||||||");

		ImGui::Text("View Matrix");

		glm::mat4 viewMatrix = activeCamera->GetViewMatrix();

		for (unsigned int i = 0; i < 4; i++)
		{
			std::string dataString = std::to_string(viewMatrix[i][0]) + "\t" + std::to_string(viewMatrix[i][1]) + "\t" + std::to_string(viewMatrix[i][2]) + "\t" + std::to_string(viewMatrix[i][3]);
			ImGui::Text((dataString).c_str());
		}

		ImGui::Text("|||||||||||||||||||||||");

		float FOV = activeCamera->GetFOV();
		if (ImGui::InputFloat("FOV (Degrees):", &FOV))
		{
			activeCamera->SetFOV(FOV);
		}

		float FOVRadian = activeCamera->GetFOVAsRadians();
		if (ImGui::InputFloat("FOV (Radians):", &FOVRadian))
		{
			activeCamera->SetFOVAsRadians(FOVRadian);
		}

		float nearDistance = activeCamera->GetNearDistance();
		if (ImGui::InputFloat("Near distance:", &nearDistance))
		{
			activeCamera->SetNearDistance(nearDistance);
		}

		float farDistance = activeCamera->GetFarDistance();
		if (ImGui::InputFloat("Far distance:", &farDistance))
		{
			activeCamera->SetFarDistance(farDistance);
		}		

		float aspect = activeCamera->GetAspectRatio();
		if (ImGui::InputFloat("Aspect Ratio:", &aspect))
		{
			activeCamera->SetAspectRatio(aspect);
		}

		ImGui::Text("|||||||||||||||||||||||");

		ImGui::End();
	}

	// -------------------------------------------------

	void Window::RenderDebugMenu(const float deltaTime)
	{
		// If using the menu
		if (Rendering::Window::UsingDebugOverlay())
		{
			((OpenGLRenderPipeline*)sRenderPipeline)->RenderDebugMenu();
		}

		// Render the new ImGui frame
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	// -------------------------------------------------

	void Window::RenderUpdate()
	{
		// Counter for resetting FPS tracking with
		static double secondCounter  = 0.0;

		Engine::Timer::Timer wholeRenderFrameTimer_Capped;
		Engine::Timer::Timer wholeRenderFrameTimer_UnCapped;

		while (!mWindowShouldClose)
		{
			// ------                                                 ---- //

			wholeRenderFrameTimer_Capped.Reset();
			wholeRenderFrameTimer_Capped.Start();

			wholeRenderFrameTimer_UnCapped.Reset();
			wholeRenderFrameTimer_UnCapped.Start();

			// Calculate deltaTime for any processes that need to take time into account
			double currentTime = glfwGetTime();
			double deltaTime   = currentTime - mOldTime;
			mOldTime           = currentTime;

			secondCounter += deltaTime;

			if (sRenderPipeline)
			{
				((OpenGLRenderPipeline*)sRenderPipeline)->ResetTextureBindingInfo();
			}

			// ------                                                 ---- //
			 
			// If the player has not got the window selected then assume they dont want to have it be rendering at the same rate, so limit the framerate to a much lower number
			if (!mWindowFocused)
			{
				ClearBuffers();

				Render((float)deltaTime);

				Engine::Timer::PerformanceTimings::AddTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_AmountCapped, wholeRenderFrameTimer_UnCapped.GetCurrentTimeSeconds());

					SwapBuffers();

					PollEvents();

					std::this_thread::sleep_for(std::chrono::milliseconds(50));

				Engine::Timer::PerformanceTimings::AddTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_AmountCapped, wholeRenderFrameTimer_Capped.GetCurrentTimeSeconds());

				continue;
			}

			// -------

			if (GetBeingResized())
			{
				// See if the mouse has been released
				std::vector<Input::InputDevice*> mice;
				Input::InputSingleton::Get()->GetInputDevice(Input::InputDeviceType::MOUSE, mice);

				if (mice.size() > 0 && mice[0])
				{
					Input::MouseInput::MouseInputDevice* mouse = (Input::MouseInput::MouseInputDevice*)mice[0];

					if (mouse->QueryInput(Input::MouseInput::MouseInputBitfield::MOUSE_LEFT) == Input::ButtonInputState::RELEASED || 
						mouse->QueryInput(Input::MouseInput::MouseInputBitfield::MOUSE_LEFT) == Input::ButtonInputState::NO_INPUT)
					{
						SetWindowBeingResized(false);
					}
				}
			}

			// ------                                                 ---- //

			// Clear all buffers that need to reset first
			ClearBuffers();

			// ------   

			// Render the game
			Render((float)deltaTime);

			Engine::Timer::PerformanceTimings::AddTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_NotCapped_WithoutImGUI, wholeRenderFrameTimer_UnCapped.GetCurrentTimeSeconds());

			if (Rendering::Window::UsingDebugOverlay())
			{
				// Setup the next ImGui frame for rendering
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				RenderDebugMenu((float)deltaTime);
			}

			sFramesRenderedThisSecondSoFar++;

			if (secondCounter >= 1.0f)
			{
				std::cout << "FPS: " << sFramesRenderedThisSecondSoFar << "\n";

				sFramesRenderedLastSecond      = sFramesRenderedThisSecondSoFar;
				sFramesRenderedThisSecondSoFar = 0;
				secondCounter                  = 0.0f;
			}

			Engine::Timer::PerformanceTimings::AddTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_NotCapped, wholeRenderFrameTimer_UnCapped.GetCurrentTimeSeconds());

			// ------                                                 ---- //

			// Swap the new frame to being on the screen
			SwapBuffers();

			PollEvents();

			// ------                                                 ---- //

			Engine::Timer::PerformanceTimings::AddTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_AmountCapped, wholeRenderFrameTimer_Capped.GetCurrentTimeSeconds());
		}
	}

	// -------------------------------------------------

	void Window::RenderThread()
	{
		sRenderPipeline = new OpenGLRenderPipeline();
		sRenderPipeline->SetupGLFW();
		sRenderPipeline->Init();

		sRenderPipeline->SetupImGui();
		sDebugToggleTimer.Start();

		mInitialised = true;

		// Render loop
		RenderUpdate();

		sRenderPipeline->ShutdownImGui();

		ShutdownGLFW();

		delete sRenderPipeline;
		sRenderPipeline = nullptr;
	}

	// -------------------------------------------------	

	void Window::Render(const float deltaTime)
	{
		// --------------------------------

		if (!sRenderPipeline)
			return;

		sRenderPipeline->Update(deltaTime);
		sRenderPipeline->Render();

		// --------------------------------
	}

	// -------------------------------------------------

	void Window::OnScreenSizeChanged(std::string data)
	{
		if (sRenderPipeline)
		{
			sRenderPipeline->OnScreenSizeChanged(data);
		}
	}

	// -------------------------------------------------

	bool Window::UsingDebugOverlay()
	{
		return sUsingDebugOverlay; 
	}

	// -------------------------------------------------

	void Window::ToggleDebugOverlay()
	{ 
		if (sDebugToggleTimer.GetCurrentTimeSeconds() > sTimeBetweenDebugToggles)
		{
			// Restart the timer
			sDebugToggleTimer.Start();
			sUsingDebugOverlay = !sUsingDebugOverlay;
		}
	}

	// -------------------------------------------------

	bool Window::GetLineMode()
	{
		return mActiveModelLineMode;
	}

	// -------------------------------------------------
}