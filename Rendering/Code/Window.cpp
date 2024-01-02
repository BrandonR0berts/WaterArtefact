#include "Window.h"

// Included needed for querying info about the system being ran on
#include <Windows.h>
#include <psapi.h>

#include <iostream>
#include <thread>

#include "Input/Code/Input.h"
#include "Input/Code/KeyboardInput.h"
#include "Input/Code/MouseInput.h"
#include "Input/Code/GamepadInput.h"

#include "Engine/Code/BaseGame.h"
#include "Engine/Code/Entity/Entity.h"
#include "Engine/Code/EventSystem.h"
#include "Engine/Code/HashedEventNames.h"

#include "Rendering/Code/Shaders/ShaderStore.h"
#include "RenderingResources/RenderingResources.h"

#include "Rendering/Code/GLTFImage.h"

#include "Maths/Code/Vector.h"

#include "Camera.h"
#include "Textures/Texture.h"

#ifdef _DEBUG_BUILD
	#include "Include/imgui/imgui.h"
	#include "Include/imgui/imgui_impl_glfw.h"
	#include "Include/imgui/imgui_impl_opengl3.h"

	#include "Rendering/Code/RenderingResourceTracking.h"

	#include "Engine/Code/PerformanceAnalysis.h"
#endif

#include "Engine/Code/EventSystem.h"
#include "Engine/Code/HashedEventNames.h"

#include "Maths/Code/Random.h"

#include "FileHandling/Code/ExcelHandling.h"

#include "Rendering/Code/OpenGLRenderPipeline.h"

#include "GameWorld.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Rendering
{
	// -------------------------------------------------

	ShaderStore          Window::mShaderStore                   = ShaderStore();
	ResourceCollection   Window::mResourceCollection            = ResourceCollection();
	Buffers::BufferStore Window::mBufferStore                   = Buffers::BufferStore();

	bool                 Window::mInitialised                   = false;
	bool                 Window::sWindowBeingResized            = false;

	bool                 Window::mWindowFocused                 = true;

	RenderPipeline*      Window::sRenderPipeline                = nullptr;

#ifdef _DEBUG_BUILD
	bool                 Window::sUsingDebugOverlay             = false;
	Engine::Timer::Timer Window::sDebugToggleTimer              = Engine::Timer::Timer();
	float                Window::sTimeBetweenDebugToggles       = 0.5f;
	unsigned int         Window::sFramesRenderedThisSecondSoFar = 0;
	unsigned int         Window::sFramesRenderedLastSecond      = 0;
	bool                 Window::mActiveModelLineMode           = false;
#endif

	// -------------------------------------------------

	Window::Window()
		: mIsVsyncOn(true)
		, mWindowShouldClose(false)
		, mOldTime(0.0)

#ifdef _DEBUG_BUILD
		, mLevelEditor()

		, mUsingRenderingOverlay(false)
		, mUsingLevelEditorOverlay(false)
		, mUsingVisualsEditor(false)
		, mShowingTextureBindData(false)
		, mShowPerformanceTimings(false)

		, mEntityDebugViewer()
		, mDisplayCameraDebugInfo(false)
#endif
	{		
		// Kick off the render thread
		mRenderThread = std::thread(&Window::RenderThread, this);
	}

	// -------------------------------------------------

	Window::~Window()
	{
		mWindowShouldClose = true;

		mRenderThread.join();

		Rendering::GLTFImage::ClearUpThreads();
	}

	// -------------------------------------------------

	void Window::Update(const float deltaTime)
	{
#ifdef _DEBUG_BUILD
		if (sRenderPipeline && mLevelEditor)
		{
			mLevelEditor->Update(deltaTime, sRenderPipeline->GetActiveCamera());
		}
#endif
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

#ifdef _DEBUG_BUILD
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

	void Window::RenderRenderingDebugOverlay()
	{
		ImGui::Begin("Rendering overlay");

		// Display debug data about the running game and show the toggles for debug options
		ImGui::Text(std::string("FPS:").append(std::to_string(sFramesRenderedLastSecond)).c_str());

		if (ImGui::Button("Toggle Line Mode"))
		{
			mActiveModelLineMode = !mActiveModelLineMode;
		}

		if (ImGui::Button("Close"))
		{
			mWindowShouldClose = true;
		}

		// --------------------------

		if (ImGui::Button("Toggle Vsync"))
		{
			mIsVsyncOn = !mIsVsyncOn;

			int newInterval = mIsVsyncOn ? 1 : 0;
			glfwSwapInterval(newInterval);
		}

		ImGui::Text(("Mesh render depth: " + std::to_string(((OpenGLRenderPipeline*)sRenderPipeline)->GetModelMeshRenderDepth())).c_str());

		if (ImGui::Button("Increase Mesh Render Depth"))
		{
			((OpenGLRenderPipeline*)sRenderPipeline)->AdjustMeshRenderDepth(1);
		}

		if (ImGui::Button("Decrease Mesh Render Depth"))
		{
			((OpenGLRenderPipeline*)sRenderPipeline)->AdjustMeshRenderDepth(-1);
		}

		if (ImGui::Button("Cameras"))
		{
			mDisplayCameraDebugInfo = !mDisplayCameraDebugInfo;
		}

		if (mDisplayCameraDebugInfo)
		{
			CameraDebugWindow();
		}

		// --------------------------

		// Grab the memory usage data for up to date info for displaying
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);

		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

		double GBConversionFactor        = 1024.0 * 1024.0 * 1024.0;

		double memoryAllocatedRAM        = double(memInfo.ullTotalPhys - memInfo.ullAvailPhys);
		memoryAllocatedRAM              /= GBConversionFactor;
		double memoryAllocatedRAMToThis  = (double)pmc.WorkingSetSize;
		memoryAllocatedRAMToThis        /= GBConversionFactor;
		double SystemMemory              = (double)memInfo.ullTotalPhys;
		SystemMemory                    /= GBConversionFactor;

		ImGui::Text("|||||||||||||||||||||||");
		ImGui::Text(std::string("RAM allocated to this program (GB's): ").append(std::to_string(memoryAllocatedRAMToThis)).c_str());
		ImGui::Text(std::string("Total RAM allocated (GB's): ")          .append(std::to_string(memoryAllocatedRAM)).c_str());
		ImGui::Text(std::string("Total System RAM (GB's): ")             .append(std::to_string(SystemMemory)).c_str());
		ImGui::Text("|||||||||||||||||||||||");

		// --------------------------

		// GPU data

		std::string vendor   = (const char*)glGetString(GL_VENDOR);
		std::string renderer = (const char*)glGetString(GL_RENDERER);
		std::string version  = (const char*)glGetString(GL_VERSION);

		ImGui::Text(std::string("GPU vendor: ")  .append(vendor).c_str());
		ImGui::Text(std::string("GPU renderer: ").append(renderer).c_str());
		ImGui::Text(std::string("GPU version: ") .append(version).c_str());

		ImGui::Text("|||||||||||||||||||||||");

		ImGui::Text(std::string("Total GPU memory allocated to this (GB's): ").append(std::to_string(Rendering::TrackingData::sGPUMemoryUsedBytes / GBConversionFactor)).c_str());

		ImGui::Text("|||||||||||||||||||||||");

		// --------------------------

		ImGui::End();
	}
#endif

	// -------------------------------------------------

#ifdef _DEBUG_BUILD
	void Window::RenderDebugMenu(const float deltaTime)
	{
		// If using the menu
		if (Rendering::Window::UsingDebugOverlay())
		{
			// Start the collection
			ImGui::Begin("Deug menu");

				// Rendering toggle
				if (ImGui::Button("Rendering"))
				{
					mUsingRenderingOverlay = !mUsingRenderingOverlay;
				}

				if (ImGui::Button("Visuals Editor"))
				{
					mUsingVisualsEditor = !mUsingVisualsEditor;
				}

				// Level editor toggle
				if (ImGui::Button("Level Editor"))
				{
					mUsingLevelEditorOverlay = !mUsingLevelEditorOverlay;
				}

				// Audio toggle
				if (ImGui::Button("Audio"))
				{
					RAISE_EVENT(Engine::EventSystem::kOnAudioOverlayToggled);
				}

				// Event system overlay
				if (ImGui::Button("Event System"))
				{
					RAISE_EVENT(Engine::EventSystem::kOnEventSystemOverlayToggled);
				}				

				if (ImGui::Button("Entity Viewer"))
				{
					mEntityDebugViewer.ToggleEnabled();
				}

				if (ImGui::Button("Texture Bindings"))
				{
					mShowingTextureBindData = !mShowingTextureBindData;
				}

				if (ImGui::Button("Performance Timings"))
				{
					mShowPerformanceTimings = !mShowPerformanceTimings;
				}

				// etc

			// End the collection
			ImGui::End();

			// --------------------------------------------------------------------------------------------------------------- //

			// Render the rendering overlay if it is enabled - this is in here as this is the window class
			// The specific overlays will be housed in their own part of the code base - but are called into from here to
			// keep everything in once place for the toggle
			if (mUsingRenderingOverlay)
			{
				RenderRenderingDebugOverlay();
			}

			if (mUsingLevelEditorOverlay)
			{
				mLevelEditor->RenderOverlay(sRenderPipeline->GetActiveCamera());

				if ( sRenderPipeline &&
					 sRenderPipeline->GetActiveWorld() !=            mLevelEditor->GetLevelBeingCreated() ||
					(sRenderPipeline->GetActiveWorld() == nullptr && mLevelEditor->GetLevelBeingCreated()))
				{ 
					sRenderPipeline->SetWorldBeingPlayed(mLevelEditor->GetLevelBeingCreated());
				}

				if (sRenderPipeline)
				{
					sRenderPipeline->DisplayDebugInfo();
				}
			}

			if (sRenderPipeline)
			{
				if (mUsingVisualsEditor)
					sRenderPipeline->RenderDebugEditor();

				if (mShowingTextureBindData)
				{
					sRenderPipeline->RenderTextureBindInfo();
				}
			}

			if (mShowPerformanceTimings)
			{
				RenderPerformanceTimings();
			}

			if (mEntityDebugViewer.GetEnabled())
			{
				mEntityDebugViewer.RenderImGUIMenu();
			}

			// --------------------------------------------------------------------------------------------------------------- //

			// Render the other overlays across the engine
			RAISE_EVENT(Engine::EventSystem::kOnDebugOverlayRender);
		}

		// Render the new ImGui frame
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
#endif

	// -------------------------------------------------

	void Window::RenderUpdate()
	{
		// Counter for resetting FPS tracking with
		static double secondCounter  = 0.0;

#ifdef _DEBUG_BUILD
		Engine::Timer::Timer wholeRenderFrameTimer_Capped;
		Engine::Timer::Timer wholeRenderFrameTimer_UnCapped;
#endif

		while (!mWindowShouldClose)
		{
			// ------                                                 ---- //

#ifdef _DEBUG_BUILD
			wholeRenderFrameTimer_Capped.Reset();
			wholeRenderFrameTimer_Capped.Start();

			wholeRenderFrameTimer_UnCapped.Reset();
			wholeRenderFrameTimer_UnCapped.Start();
#endif

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
				mResourceCollection.CreateQueuedResources();
				mBufferStore.Update();

				ClearBuffers();

				Render((float)deltaTime);

#ifdef _DEBUG_BUILD
				Engine::Timer::PerformanceTimings::AddTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_AmountCapped, wholeRenderFrameTimer_UnCapped.GetCurrentTimeSeconds());
#endif

				SwapBuffers();

				PollEvents();

				std::this_thread::sleep_for(std::chrono::milliseconds(50));

#ifdef _DEBUG_BUILD
				Engine::Timer::PerformanceTimings::AddTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_AmountCapped, wholeRenderFrameTimer_Capped.GetCurrentTimeSeconds());
#endif

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

						unsigned int screenWidth  = sRenderPipeline->GetScreenWidth();
						unsigned int screenHeight = sRenderPipeline->GetScreenHeight();

						std::string resizeData = std::to_string(screenWidth) + ", " + std::to_string(screenHeight);
						RAISE_EVENT_WITH_DATA(Engine::EventSystem::kOnScreenSizeChanged, resizeData);
					}
				}
			}

			// ------                                                 ---- //

			// Make sure to load in any queued resources pending
			mResourceCollection.CreateQueuedResources();
			mBufferStore.Update();

			// ------                                                 ---- //

			// Clear all buffers that need to reset first
			ClearBuffers();

			// ------   

			// Render the game
			Render((float)deltaTime);

#ifdef _DEBUG_BUILD
			Engine::Timer::PerformanceTimings::AddTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_NotCapped_WithoutImGUI, wholeRenderFrameTimer_UnCapped.GetCurrentTimeSeconds());
#endif

// Only want to do GUI stuff if we are not in the release version of the game
#ifdef _DEBUG_BUILD
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
#endif

#ifdef _DEBUG_BUILD
			Engine::Timer::PerformanceTimings::AddTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_NotCapped, wholeRenderFrameTimer_UnCapped.GetCurrentTimeSeconds());
#endif

			// ------                                                 ---- //

			// Swap the new frame to being on the screen
			SwapBuffers();

			PollEvents();

			// ------                                                 ---- //

#ifdef _DEBUG_BUILD
			Engine::Timer::PerformanceTimings::AddTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_AmountCapped, wholeRenderFrameTimer_Capped.GetCurrentTimeSeconds());
#endif
		}
	}

	// -------------------------------------------------

	void Window::RenderThread()
	{
		sRenderPipeline = new OpenGLRenderPipeline();
		sRenderPipeline->SetupGLFW();
		sRenderPipeline->Init();

		SetupShaders();

#ifdef _DEBUG_BUILD
		sRenderPipeline->SetupImGui();
		sDebugToggleTimer.Start();
#endif

		mInitialised = true;

		// Render loop
		RenderUpdate();

#ifdef _DEBUG_BUILD
		sRenderPipeline->ShutdownImGui();
#endif

		delete sRenderPipeline->GetActiveWorld();
		sRenderPipeline->SetWorldBeingPlayed(nullptr);

		ClearShaders();
		ShutdownGLFW();

		delete sRenderPipeline;
		sRenderPipeline = nullptr;
	}

	// -------------------------------------------------

	void Window::ClearShaders()
	{
#ifdef _DEBUG_BUILD
		delete mLevelEditor;
		mLevelEditor = nullptr;
#endif

		mShaderStore.Clear();
		mResourceCollection.Clear();
		mBufferStore.Clear();
	}

	// -------------------------------------------------

	void Window::SetupShaders()
	{
		// -----

		mShaderStore.Init();

		// -----

		mBufferStore.CreateFBO("FinalRender_FBO");

		unsigned int screenWidth  = sRenderPipeline->GetScreenWidth();
		unsigned int screenHeight = sRenderPipeline->GetScreenHeight();

		// Colour buffer
		std::string         colourTextureName = "FinalRenderbufferColourTexture";
		Texture::Texture2D* colourTexture     = new Texture::Texture2D();
		colourTexture->InitEmpty(screenWidth, screenHeight, false, GL_UNSIGNED_BYTE, GL_RGB, GL_RGB);
		mResourceCollection.AddResource(Rendering::ResourceType::Texture2D, colourTextureName, colourTexture);

		// Depth buffer
		std::string         depthTextureName = "FinalRenderbufferDepthTexture";
		Texture::Texture2D* depthTexture     = new Texture::Texture2D();
		depthTexture->InitEmpty(screenWidth, screenHeight, false, GL_UNSIGNED_BYTE, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
		mResourceCollection.AddResource(Rendering::ResourceType::Texture2D, depthTextureName, depthTexture);

		// -----

#ifdef _DEBUG_BUILD
		mLevelEditor = new Game::LevelEditor(sRenderPipeline);
#endif
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

#ifdef _DEBUG_BUILD
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

	void Window::RenderPerformanceTimings()
	{
		ImGui::Begin("Performance Timings");

			ImGui::Text("\n--------------------------------------\n");

			ImGui::Text(("Time for whole startup: " + std::to_string(Engine::Timer::PerformanceTimings::GetTiming(Engine::Timer::PerformanceTimingAreas::Setup_Whole))).c_str());

			ImGui::Text("\n--------------------------------------\n");

			ImGui::Text(("Time for whole frame Update (capped): " + std::to_string(Engine::Timer::PerformanceTimings::GetTiming(Engine::Timer::PerformanceTimingAreas::Update_WholeFrame_AmountCapped))).c_str());
			ImGui::Text(("Time for whole frame Update (uncapped): " + std::to_string(Engine::Timer::PerformanceTimings::GetTiming(Engine::Timer::PerformanceTimingAreas::Update_WholeFrame_NotCapped))).c_str());

			ImGui::Text("\n--------------------------------------\n");

			ImGui::Text(("Time for whole frame Render (capped): " + std::to_string(Engine::Timer::PerformanceTimings::GetTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_AmountCapped))).c_str());
			ImGui::Text(("Time for whole frame Render (uncapped): " + std::to_string(Engine::Timer::PerformanceTimings::GetTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_NotCapped))).c_str());
			ImGui::Text(("Time for whole frame Render (uncapped) (No ImGUI): " + std::to_string(Engine::Timer::PerformanceTimings::GetTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_NotCapped_WithoutImGUI))).c_str());

			ImGui::Text(("Time just for ImGui: " + std::to_string(Engine::Timer::PerformanceTimings::GetTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_NotCapped) - 
				                                                  Engine::Timer::PerformanceTimings::GetTiming(Engine::Timer::PerformanceTimingAreas::Render_WholeFrame_NotCapped_WithoutImGUI))).c_str());

			ImGui::Text("\n--------------------------------------\n");

		ImGui::End();
	}
#endif

	// -------------------------------------------------
}