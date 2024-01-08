#pragma once

#include "Maths/Code/Vector.h"

#include <string>
#include <GLFW/glfw3.h>

namespace Rendering
{
	class Camera;
	class Framebuffer;

	// ---------------------------------------

	// This is the main game render flow, which hooks into the current level to grab the data about what should be being rendered
	class RenderPipeline abstract
	{
	public:
		RenderPipeline();
		virtual ~RenderPipeline();

		// Functionality that needs to be defined by the child renderpipeline
		virtual void                     Update(const float deltaTime) = 0;
		virtual void                     Render()                      = 0;

		// ---------------------------------------------------------------- //

		void                ClearFinalRenderBuffer();
		void                OnScreenSizeChanged(std::string data);

		// ---------------------------------------------------------------- //

		void                Init();
		virtual bool        SetupGLFW() = 0;

		// ---------------------------------------------------------------- //

		virtual void        SetupImGui()    = 0;
		virtual void        ShutdownImGui() = 0;

		// ---------------------------------------------------------------- //

		GLFWwindow*         GetWindow()             { return mWindow; }

		unsigned int        GetScreenWidth()        { return mScreenWidth; }
		unsigned int        GetScreenHeight()       { return mScreenHeight; }

		unsigned int        GetMaxScreenWidth()     { return mMaxScreenWidth; }
		unsigned int        GetMaxScreenHeight()    { return mMaxScreenHeight; }

		void                SetWindowWidth(unsigned int width)   { mScreenWidth = width; }
		void                SetWindowHeight(unsigned int height) { mScreenHeight = height; }

		Camera*             GetActiveCamera() { return mActiveCamera; }

		// ---------------------------------------------------------------- //

	protected:
		// ---------------------------------------------------------------- //
		 
		// Cameras
		Camera* mActiveCamera;

		// ---------------------------------------------------------------- //

		// The framebuffer that is rendered to before going to the final screen
		Framebuffer* mFinalRenderFBO;

		// -------------------

		Framebuffer*                          mShadowMap;
		Maths::Vector::Vector2D<unsigned int> mShadowMapImageDimensions;

		// -------------------

		void SetupUsefulVBOData();

		GLFWwindow*  mWindow;

		unsigned int mScreenWidth;
		unsigned int mScreenHeight;

		unsigned int mMaxScreenHeight;
		unsigned int mMaxScreenWidth;
	};

	// ---------------------------------------
}