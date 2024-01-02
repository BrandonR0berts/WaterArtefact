#pragma once

#include <string>

#include "Text.h"

namespace Rendering
{
	class Camera;
	class VideoFrames;

	class Level;
	class LightCollection;

	class RainParticleSystem;

	class Framebuffer;

	class GameWorld;

	// ---------------------------------------

	struct ShadowImageSegment
	{
		ShadowImageSegment()
			: mBottomLeftUV(0.0f, 0.0f)
			, mTopRightUV(0.0f, 0.0f)
			, mMVPMatrix()
			, mLightPosition()
		{}

		ShadowImageSegment(Maths::Vector::Vector2D<float> bottomLeft, Maths::Vector::Vector2D<float> topRight)
			: mBottomLeftUV(bottomLeft)
			, mTopRightUV(topRight)
			, mMVPMatrix()
			, mLightPosition()
		{}

		ShadowImageSegment(Maths::Vector::Vector2D<float> bottomLeft, Maths::Vector::Vector2D<float> topRight, glm::mat4 matrix, Maths::Vector::Vector4D<float> pos)
			: mBottomLeftUV(bottomLeft)
			, mTopRightUV(topRight)
			, mMVPMatrix(matrix)
			, mLightPosition(pos)
		{}

		Maths::Vector::Vector2D<float> mBottomLeftUV;
		Maths::Vector::Vector2D<float> mTopRightUV;
		glm::mat4                      mMVPMatrix;
		Maths::Vector::Vector4D<float> mLightPosition;
		Maths::Vector::Vector4D<float> mLightDirection;
		Maths::Vector::Vector4D<float> mLightColour;
	};

	struct ShadowData
	{
		ShadowData()
			: mImageSegments()
		{}

		std::vector<ShadowImageSegment> mImageSegments;
	};

	// ---------------------------------------

	enum AntiAliasingType
	{
		none = 0
	};

	struct PostProcessingSettings
	{
		PostProcessingSettings()
			: mLineOutline(true)
			, mBloom(true)
			, mAntiAliasing(false)
			, mGrain(true)
			, mScreenSpaceRelfections(true)

			, mLineThickness(1.0f)
			, mBloomFactor(1.0)
			, mAAType(AntiAliasingType::none)
			, mGrainFactor(1.0f)
			, mScreenSpaceReflectionsClarity(1.0f)
		{

		}

		// Toggles
		bool mLineOutline;
		bool mBloom;
		bool mAntiAliasing;
		bool mGrain;
		bool mScreenSpaceRelfections;

		// Factors
		float            mLineThickness;
		float            mBloomFactor;
		AntiAliasingType mAAType;
		float            mGrainFactor;
		float            mScreenSpaceReflectionsClarity;
	};

	// ---------------------------------------

	enum ShadowType
	{
		Hard,
		Soft
	};

	struct VisualSettings
	{
		VisualSettings()
			: mShadows(true)
			, mParticles(true)
			, mReflections(true)

			, mShadowType(ShadowType::Hard)
		{
		
		}

		// Toggles
		bool mShadows;
		bool mParticles;
		bool mReflections;

		// Factors
		ShadowType mShadowType;
	};

	enum class RenderAPI
	{
		OpenGL,
		Vulkan,

		Count
	};

	// ---------------------------------------

	// This is the main game render flow, which hooks into the current level to grab the data about what should be being rendered
	class RenderPipeline abstract
	{
	public:
		RenderPipeline(RenderAPI API);
		virtual ~RenderPipeline();

		// Functionality that needs to be defined by the child renderpipeline
		virtual void                     Update(const float deltaTime) = 0;
		virtual void                     Render()                      = 0;

		// Useful for reflections - this is a very expensive function to call, so do it rarely
		virtual Texture::CubeMapTexture* RenderIntoCubemap(Maths::Vector::Vector3D<float> position, Maths::Vector::Vector2D<unsigned int> resolution, std::string newTextureName, bool generateMipMaps)                  = 0;
		virtual void                     RenderIntoCubemap(Maths::Vector::Vector3D<float> position, Texture::CubeMapTexture* textureToWriteInto, Maths::Vector::Vector2D<unsigned int> resolution, bool generateMipMaps) = 0;

		virtual void                     RenderVideo(VideoFrames* video) = 0;
		        void                     RenderUI();
		        void                     UpdateVideoFrames(const double deltaTime);

		// ---------------------------------------------------------------- //

#ifdef _DEBUG_BUILD
		void                DisplayDebugInfo();
		void                RenderDebugEditor();

		virtual void        RenderTextureBindInfo() = 0;
#endif

		// ---------------------------------------------------------------- //

		void                ClearFinalRenderBuffer();
		void                OnScreenSizeChanged(std::string data);

		void                SetWorldBeingPlayed(GameWorld* world) { mGameWorld = world; }
		GameWorld*          GetActiveWorld()                      { return mGameWorld; }

		Camera*             GetActiveCamera() const { return mActiveCamera; }
		RenderAPI           GetRenderAPI()    const { return mAPI; }

		// ---------------------------------------------------------------- //

		void                Init();
		virtual bool        SetupGLFW() = 0;

		// ---------------------------------------------------------------- //

#ifdef _DEBUG_BUILD
		virtual void        SetupImGui()    = 0;
		virtual void        ShutdownImGui() = 0;
#endif

		// ---------------------------------------------------------------- //

		GLFWwindow*         GetWindow()             { return mWindow; }

		unsigned int        GetScreenWidth()        { return mScreenWidth; }
		unsigned int        GetScreenHeight()       { return mScreenHeight; }

		unsigned int        GetMaxScreenWidth()     { return mMaxScreenWidth; }
		unsigned int        GetMaxScreenHeight()    { return mMaxScreenHeight; }

		void                SetWindowWidth(unsigned int width)   { mScreenWidth = width; }
		void                SetWindowHeight(unsigned int height) { mScreenHeight = height; }

		// ---------------------------------------------------------------- //

	protected:
		// ---------------------------------------------------------------- //
		 
		// Cameras
		Camera*                mActiveCamera;
		Camera*                mGameCamera;
		Camera*                mDebugCamera;

		// ---------------------------------------------------------------- //

		// The game world that is to be rendered
		GameWorld*             mGameWorld;

		// ---------------------------------------------------------------- //

		// The visual settings of the current game
		VisualSettings         mVisualSettings;
		PostProcessingSettings mPostProcessingSettings;

		// ---------------------------------------------------------------- //

		// The framebuffer that is rendered to before going to the final screen
		Framebuffer*           mFinalRenderFBO;

		// -------------------

#ifdef _DEBUG_BUILD
		bool                                  mShowShadowMap;
#endif

		Framebuffer*                          mShadowMap;
		Maths::Vector::Vector2D<unsigned int> mShadowMapImageDimensions;

		virtual void GenerateShadowMaps() = 0;
		virtual void RenderShadowIntoMap(ShadowImageSegment& segmentData, Rendering::LightCollection& lights, unsigned int index, float portion) = 0;

		        void SetupFinalRenderFBO();

		// -------------------

		void SetupUsefulVBOData();

		TextManager mTextManager;
		
		RenderAPI   mAPI;

		// -------------------

		virtual void RenderForReflections(Maths::Vector::Vector3D<float> positon, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, bool includeShadows = true, bool includeParticles = false) = 0;

		GLFWwindow*  mWindow;

		unsigned int mScreenWidth;
		unsigned int mScreenHeight;

		unsigned int mMaxScreenHeight;
		unsigned int mMaxScreenWidth;
	};

	// ---------------------------------------
}