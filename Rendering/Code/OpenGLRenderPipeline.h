#pragma once

#include "RenderPipeline.h"

#include "Water.h"

#include <mutex>
#include <glad/glad.h>

namespace Rendering
{
	namespace Buffers
	{
		class VertexArrayObject;
		class VertexBufferObject;
	}

	namespace ShaderPrograms
	{
		class ShaderProgram;
	}

	namespace Texture
	{
		class Texture2D;
	}

	class Skybox;

	// -----------------------------------------

	enum class BufferViewOverrideTypes
	{
		None = 0,
		Position,
		Normal,
		Tangent,
		Binormal,
		H0,
		Fourier,
		Depth,
		Position2,
		RandomNumbers,
		ButterflyTexture,
	};

	// -----------------------------------------

	// This is the main game render flow, which hooks into the current level to grab the data about what should be being rendered
	class OpenGLRenderPipeline final : public RenderPipeline
	{
	public:
		OpenGLRenderPipeline();
		~OpenGLRenderPipeline() override;

		void Update(const float deltaTime, bool updateWater, float delayedUpdateDeltaTime) override;
		void Render()                                                                      override;

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

		void              RenderDebugMenu();

		// -------------------------------------------- //

		bool              SetupGLFW()     override;

		// -------------------------------------------- //

		void              SetupImGui()    override;
		void              ShutdownImGui() override;

		// -------------------------------------------- //

	private:
		std::vector<std::pair<GLenum, unsigned int>> mTextureUnitBindings;
		unsigned int                                 mShaderIDBound;

		bool                                         mLineModeEnabled;
		bool                                         mDepthTestEnabled;
		GLenum                                       mDepthFunction;

		bool                                         mAlphaBlendingEnabled;
		GLenum                                       mAlphaBlendingSFactor;
		GLenum                                       mAlphaBlendingDFactor;

		bool                                         mBackFaceCullingEnabled;

		ShaderPrograms::ShaderProgram*               mFinalRenderProgram;

		Buffers::VertexArrayObject*                  mVAOVideo;
		Buffers::VertexBufferObject*                 mVBOVideo;

		Texture::Texture2D*                          mColourTexture;
		Texture::Texture2D*                          mDepthStencilTexture;

		// ---------------------------------------------------------------- //

		WaterSimulation*    mWaterSimulation;
		Skybox*             mSkybox;

		BufferViewOverrideTypes mDebugVisualisationOverride;

		// ---------------------------------------------------------------- //

		std::string ConvertTextureBindingToString(GLenum bindingPoint);

		// ---------------------------------------------------------------- //

		void FinalRenderToScreen();		
		void SetupShaders();

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