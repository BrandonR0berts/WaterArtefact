#pragma once

#include "BufferStore.h"
#include "Engine/Code/Timer.h"

#ifdef _DEBUG_BUILD
	#include "ProjectCube/Code/LevelEditor.h"
	#include "ProjectCube/Code/EntityViewer.h"
#endif

#include "Maths/Code/Vector.h"
#include "Maths/Code/Matrix.h"

#include "RenderPipeline.h"

#include <thread>

namespace Rendering
{	
	namespace Texture
	{
		class Texture2D;
	}

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

		static inline ResourceCollection&   GetResourceCollection() { return mResourceCollection;  }
		static inline ShaderStore&          GetShaderStore()        { return mShaderStore;         }
		static inline Buffers::BufferStore& GetBufferStore()        { return mBufferStore;		   }

		static inline RenderPipeline*       GetRenderPipeline()     { return sRenderPipeline;	   }

		// ----

		bool                         GetHasQuitWindow()              const { return mWindowShouldClose; }
		void                         SetQuitWindow()                       { mWindowShouldClose = true; }

		// ----

		static bool                  GetInitialised()                      { return mInitialised; }
		static void                  SetFocused(bool focused)              { mWindowFocused = focused; }

		// ----

#ifdef _DEBUG_BUILD
		static bool                  UsingDebugOverlay();
		static void                  ToggleDebugOverlay();
		static bool                  GetLineMode();
#endif

		       void         OnScreenSizeChanged(std::string data);

		static void         SetWindowBeingResized(bool state) { sWindowBeingResized = state; }
		static bool         GetBeingResized()                 { return sWindowBeingResized;  }

		static unsigned int GetWindowWidth() 
		{
			if (!sRenderPipeline)
				return 0;

			return sRenderPipeline->GetScreenWidth();
		}

		static unsigned int GetWindowHeight()
		{
			if (!sRenderPipeline)
				return 0;

			return sRenderPipeline->GetScreenHeight();
		}

		static unsigned int GetMaxWindowWidth()
		{
			if (!sRenderPipeline)
				return 0;

			return sRenderPipeline->GetMaxScreenWidth();
		}

		// -------------------------------------------------

		static unsigned int GetMaxWindowHeight()
		{
			if (!sRenderPipeline)
				return 0;

			return sRenderPipeline->GetMaxScreenHeight();
		}

		// -------------------------------------------------

#ifdef _DEBUG_BUILD
		Game::LevelEditor* GetLevelEditor() { return mLevelEditor; }
#endif

	private:

		// ----

		std::thread mRenderThread;
		void RenderThread();

		void RenderUpdate();
		void Render(const float deltaTime);

		// ----

		// Backing framework
		void ShutdownGLFW();
		void SetupShaders();
		void ClearShaders();

		// ----

		void SwapBuffers();
		void SetBackgroundColour(const Maths::Vector::Vector4D<float>& colour);
		void ClearBuffers();
		void PollEvents();

		void SetupFinalRenderFBO();

		// -------------------------------

#ifdef _DEBUG_BUILD
		void RenderDebugMenu(const float deltaTime);
		void RenderRenderingDebugOverlay();
		void RenderPerformanceTimings();

		void CameraDebugWindow();
#endif

		// -------------------------------

		bool                mIsVsyncOn;
		bool                mWindowShouldClose;
		double              mOldTime;

		static RenderPipeline* sRenderPipeline;

#ifdef _DEBUG_BUILD
		Game::LevelEditor*  mLevelEditor;
		Game::EntityViewer  mEntityDebugViewer;
#endif

		// -------------------------------

		static ShaderStore          mShaderStore;
		static ResourceCollection   mResourceCollection;
		static Buffers::BufferStore mBufferStore;

		static bool                 mInitialised;
		static bool                 sWindowBeingResized;

		static bool                 mWindowFocused;

		// -----

#ifdef _DEBUG_BUILD
		static bool                 sUsingDebugOverlay;
		static Engine::Timer::Timer sDebugToggleTimer;
		static float                sTimeBetweenDebugToggles;

		static unsigned int         sFramesRenderedThisSecondSoFar;
		static unsigned int         sFramesRenderedLastSecond;

		bool                        mUsingRenderingOverlay;
		bool                        mUsingLevelEditorOverlay;
		bool                        mUsingVisualsEditor;

		bool                        mShowingTextureBindData;

		bool                        mShowPerformanceTimings;

		static bool                 mActiveModelLineMode;

		bool                        mDisplayCameraDebugInfo;
#endif		

		// -----
	};

	// ---------------------------------------------------------------
}