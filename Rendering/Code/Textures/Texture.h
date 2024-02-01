#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <string>

#include <glm/matrix.hpp>

#include "TextureSettings.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// This file contains definitions for Texture2D and Texture3D within the texture namespace
namespace Rendering
{
	namespace Buffers
	{
		class VertexArrayObject;
	}

	namespace ShaderPrograms
	{
		class ShaderProgram;
	}

	namespace Texture
	{
		// ---------------------------------------------------

		class Texture2D final
		{
		public:
			Texture2D();
			~Texture2D();

			// -------

			// Texture data handling
			bool LoadTextureFromFile(std::string             filePath, 
									 TextureMinMagFilters    minMagFilters       = TextureMinMagFilters(), 
									 TextureWrappingSettings textureWrapSettings = TextureWrappingSettings());

			bool           ReplaceTextureData(unsigned char* data);
			bool           ReplaceTextureData(unsigned char* data, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int mipMapLevel = 0);

			unsigned char* GetPixelData();
			unsigned char* GetPixelData(unsigned int xOffset, unsigned int yOffset);

			// -------

			// Init
			bool InitEmpty(unsigned int            width, 
						   unsigned int            height, 
						   bool                    willHaveAlpha, 
 				           GLenum                  internalDataType    = GL_UNSIGNED_BYTE,
				           GLenum                  internalDataFormat  = GL_RGBA8,
				           GLenum                  format              = GL_RGBA,
						   TextureMinMagFilters    minMagFilters       = TextureMinMagFilters(),
						   TextureWrappingSettings textureWrapSettings = TextureWrappingSettings());

			bool InitWithData(unsigned int            width, 
							  unsigned int            height, 
							  void*                   data, 
							  bool                    hasAlpha, 
							  GLenum                  internalDataType,
							  GLenum                  internalFormat,
						      GLenum                  format,
							  TextureMinMagFilters    minMagFilters = TextureMinMagFilters(),
							  TextureWrappingSettings textureWrapSettings = TextureWrappingSettings());

			// -------

			// Texture2D binders
			void           Bind(GLenum unitToBindTo = GL_TEXTURE0);
			void           BindForComputeShader(GLuint unit, GLint level, bool layered, GLint layer, GLenum access, GLenum format);
			void           UnBind(GLenum unitToBindTo = GL_TEXTURE0);

			// -------

			// Getters
			unsigned int   GetTextureID()      const { return mTextureID;   }
			bool           GetIsInitialised()  const { return mInitialised; }

			unsigned int   GetTextureWidth()   const { return mWidth;  }
			unsigned int   GetTextureHeight()  const { return mHeight; }

			std::string    GetFilePath()       const { return mFilePath; }
			unsigned int   GetDataSize();
			float          GetDataSizeMegaBytes()    { return float(GetDataSize()) / 1048576.0f; } // 1024 ^^ 2

			// -------

			// Setters
			void           SetIsForVideo() { mForVideo = true; }

			void           SetTextureMinMagFilters(TextureMinMagFilters minMagFilters);
			void           SetTextureWrappingSettings(TextureWrappingSettings settings);
			void           SetCompareMode(GLenum mode, GLenum function);

			// -------

			bool           LoadInImageData(std::string filePath);
			void           LoadInDataFromBuffer(std::string typeToMimic, const unsigned char* dataBuffer, unsigned int bufferLength, unsigned int offsetIntoBuffer);

			bool           SendTextureDataToGPU();

			void           FreeCachedImageData();

			void           Resize(unsigned int width, unsigned int height);

		private:

			std::string    mFilePath;

			unsigned int   mTextureID;
			unsigned int   mPBO; // This is used for getting pixels back to the CPU from the GPU

			unsigned int   mWidth;
			unsigned int   mHeight;
			int            mFormat;

			bool           mInitialised;
			bool           mForVideo;
			bool           mHasAlpha;

			bool           mLastDataInvalid;
			unsigned char* mLastPixelDataFromGPU; // The pixel store of what we have last requested from the GPU - will not be up-to date pixel data, mainly here to prevent memory leaks

			unsigned char* mLoadedImageData;

			TextureMinMagFilters    mMinMagFilters;
			TextureWrappingSettings mTextureWrapSettings;

			GLenum        mInternalFormat;
			GLenum        mInternalDataType;
			GLenum        mExternalFormat;
		};

		// ---------------------------------------------------

		class CubeMapTexture final
		{
		public:
			CubeMapTexture();
			~CubeMapTexture();

			void            Bind(GLenum unitToBindTo = GL_TEXTURE0);
			void            UnBind(GLenum unitToBindTo = GL_TEXTURE0);
			unsigned int    GetTextureID() const { return mTextureID; }

			void            LoadInTextures(std::string filePaths[6], TextureMinMagFilters minMagFilters = TextureMinMagFilters(), TextureWrappingSettings wrapSettings = TextureWrappingSettings());

			// Returns a new texture that has been convoluted
			CubeMapTexture* ConvoluteTexture(Buffers::VertexArrayObject* cubeVAO);

			// Convolutes this texture, but as it is roughness based the only blurred parts are the mip map levels
			void            ConvoluteTexture_Roughness(Buffers::VertexArrayObject* cubeVAO);

			void            SetTextureMinMagFilters(TextureMinMagFilters minMagFilters);
			void            SetTextureWrappingSettings(TextureWrappingSettings settings);

			void            SetupInteralData(unsigned int width, unsigned int height, GLenum dataType, GLenum internalFormat, GLenum externalFormat, TextureMinMagFilters minMagFilters = TextureMinMagFilters(), TextureWrappingSettings wrapSettings = TextureWrappingSettings(), bool generateMipMaps = false);

			glm::mat4       GetCaptureView(unsigned int ID) { return mCaptureViews[ID]; }

		private:
			static glm::mat4 mCaptureViews[];

			unsigned int mTextureID;

			unsigned int mWidth;
			unsigned int mHeight;

			static ShaderPrograms::ShaderProgram* mConvolutionShader;
			static ShaderPrograms::ShaderProgram* mRoughnessConvolutionShader;
		};

		// ---------------------------------------------------
	};
}

#endif