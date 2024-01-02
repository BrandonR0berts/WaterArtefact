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

		// Useful for reflections - this is a very expensive function to call, so do it rarely
		Texture::CubeMapTexture* RenderIntoCubemap(Maths::Vector::Vector3D<float> position, Maths::Vector::Vector2D<unsigned int> resolution, std::string newTextureName, bool generateMipMaps) override;
		void                     RenderIntoCubemap(Maths::Vector::Vector3D<float> position, Texture::CubeMapTexture* textureToWriteInto, Maths::Vector::Vector2D<unsigned int> resolution, bool generateMipMaps) override;

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

#ifdef _DEBUG_BUILD
		void              SetupImGui()    override;
		void              ShutdownImGui() override;

		int               GetModelMeshRenderDepth()         { return mModelMeshRenderDepth;    }
		void              AdjustMeshRenderDepth(int amount) { mModelMeshRenderDepth += amount; if (mModelMeshRenderDepth < -1) mModelMeshRenderDepth = -1; }

		void              RenderTextureBindInfo() override;
#endif

		// -------------------------------------------- //

	private:
		// -------------------

		// Shadows
		void              GenerateShadowMaps() override;
		void              RenderShadowIntoMap(ShadowImageSegment& segmentData, Rendering::LightCollection& lights, unsigned int index, float portion) override;

		// -------------------

		// UI
		bool              HandlePlayingVideos();
		void              RenderVideo(VideoFrames* video) override;

		// -------------------

		void              RenderForReflections(Maths::Vector::Vector3D<float> positon, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, bool includeShadows = true, bool includeParticles = false) override;

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

#ifdef _DEBUG_BUILD
		int                                          mModelMeshRenderDepth;
#endif

		// ---------------------------------------------------------------- //

		std::string ConvertTextureBindingToString(GLenum bindingPoint);

		// ---------------------------------------------------------------- //

		void FinalRenderToScreen();		

		void RenderGameWorld();

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