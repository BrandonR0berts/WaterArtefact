#pragma once

#include "Maths/Code/Timer.h"
#include "Rendering/Code/RenderPipeline.h"

#include "Water.h"

#include <thread>

namespace Rendering
{	
	class ShaderStore;
	class ResourceCollection;

	// ---------------------------------------------------------------

	class Window
	{
	public:
		Window();
		~Window();

		// ----

		void Update(const float deltaTime);

		// ----

		// The pipeline that stores the current state of the GPU and handlled swapping states
		static inline RenderPipeline* GetRenderPipeline()     { return sRenderPipeline;	   }

		// ----

		bool                         GetHasQuitWindow()              const { return mWindowShouldClose; }
		void                         SetQuitWindow()                       { mWindowShouldClose = true; }

		// ----

		static bool                  GetInitialised()                      { return mInitialised; }
		static void                  SetFocused(bool focused)              { mWindowFocused = focused; }

		// ----

		static bool                  UsingDebugOverlay();
		static void                  ToggleDebugOverlay();
		static bool                  GetLineMode();

		       void                  OnScreenSizeChanged(Maths::Vector::Vector2D<unsigned int> dimensions);

		static void                  SetWindowBeingResized(bool state) { sWindowBeingResized = state; }
		static bool                  GetBeingResized()                 { return sWindowBeingResized;  }

		static unsigned int          GetWindowWidth() 
		{
			if (!sRenderPipeline)
				return 0;

			return sRenderPipeline->GetScreenWidth();
		}

		static unsigned int         GetWindowHeight()
		{
			if (!sRenderPipeline)
				return 0;

			return sRenderPipeline->GetScreenHeight();
		}

		static unsigned int        GetMaxWindowWidth()
		{
			if (!sRenderPipeline)
				return 0;

			return sRenderPipeline->GetMaxScreenWidth();
		}

		// -------------------------------------------------

		static unsigned int       GetMaxWindowHeight()
		{
			if (!sRenderPipeline)
				return 0;

			return sRenderPipeline->GetMaxScreenHeight();
		}

		// -------------------------------------------------

	private:

		// ----

		std::thread mRenderThread;
		void RenderThread();

		void RenderUpdate();
		void Render(const float deltaTime);

		// ----

		// Backing framework
		void ShutdownGLFW();

		// ----

		void SwapBuffers();
		void SetBackgroundColour(const Maths::Vector::Vector4D<float>& colour);
		void ClearBuffers();
		void PollEvents();

		// -------------------------------

		void RenderDebugMenu(const float deltaTime);

		void CameraDebugWindow();

		// -------------------------------

		bool                mIsVsyncOn;
		bool                mWindowShouldClose;
		double              mOldTime;

		static RenderPipeline* sRenderPipeline;

		// -------------------------------

		static bool                 mInitialised;
		static bool                 sWindowBeingResized;

		static bool                 mWindowFocused;

		// -----

		static bool                 sUsingDebugOverlay;
		static Engine::Timer::Timer sDebugToggleTimer;
		static float                sTimeBetweenDebugToggles;

		static unsigned int         sFramesRenderedThisSecondSoFar;
		static unsigned int         sFramesRenderedLastSecond;

		bool                        mDisplayCameraDebugInfo;
		static bool                 mActiveModelLineMode;

		static float                kTimePerUpdate;

		// -----
	};

	// ---------------------------------------------------------------
}