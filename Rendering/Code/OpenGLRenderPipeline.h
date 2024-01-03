#pragma once

#include "RenderPipeline.h"
#include <mutex>

namespace Rendering
{
	// This is the main game render flow, which hooks into the current level to grab the data about what should be being rendered
	class OpenGLRenderPipeline final : public RenderPipeline
	{
	public:
		OpenGLRenderPipeline();
		~OpenGLRenderPipeline() override;

		void Update(const float deltaTime) override;
		void Render()                      override;

		// -------------------------------------------- //

		// Static functionality specific to OpenGL
		void              BindTextureToTextureUnit(GLenum textureUnit, unsigned int textureUnitID, bool isTexture2D = true);
		unsigned int      QueryCurrentlyBoundTextureID(GLenum textureUnit);

		void              SetActiveShader(unsigned int shaderID);
		unsigned int      QueryCurrentlyActiveShaderID() { return mShaderIDBound; }

		void              SetLineModeEnabled(bool state);
		void              SetDepthTestEnabled(bool state);
		void              SetDepthTestFunction(GLenum state);

		void              SetAlphaBlending(bool state);
		void              SetAlphaBlendingFunction(GLenum sFactor, GLenum dFactor);

		void              SetBackFaceCulling(bool state);

		void              ResetTextureBindingInfo();

		// -------------------------------------------- //

		bool              SetupGLFW()     override;

		// -------------------------------------------- //

		void              SetupImGui()    override;
		void              ShutdownImGui() override;

		// -------------------------------------------- //

	private:
		// -------------------

		std::vector<std::pair<GLenum, unsigned int>> mTextureUnitBindings;
		unsigned int                                 mShaderIDBound;

		bool                                         mLineModeEnabled;
		bool                                         mDepthTestEnabled;
		GLenum                                       mDepthFunction;

		bool                                         mAlphaBlendingEnabled;
		GLenum                                       mAlphaBlendingSFactor;
		GLenum                                       mAlphaBlendingDFactor;

		bool                                         mBackFaceCullingEnabled;

		// ---------------------------------------------------------------- //

		std::string ConvertTextureBindingToString(GLenum bindingPoint);

		// ---------------------------------------------------------------- //

		void FinalRenderToScreen();		

		// -------------------------------------------- //

		// Callbacks
		void SetKeypressCallback(        void (*keyFunc)(       GLFWwindow* window, int    key,     int    scancode, int action, int mods));
		void SetCursorEnterCallback(     void (*enterFunc)(     GLFWwindow* window, int    entered));
		void SetGetMousePositionCallback(void (*cursorPos)(     GLFWwindow* window, double xpos,    double ypos));
		void SetMouseButtonPressCallback(void (*buttonFunction)(GLFWwindow* window, int    button,  int    action,   int mods));
		void SetMouseScrollWheelCallback(void (*scrollFunction)(GLFWwindow* window, double xoffset, double yoffset));
		void SetWindowFocusCallback(     void (*focusFunction) (GLFWwindow* window, int    focused));

		// -------------------------------------------- //
	};

	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void window_focus_change_callback(GLFWwindow* window, int focus);
}