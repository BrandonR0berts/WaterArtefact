#include "Texture.h"

#include "Rendering/Code/STB_Image/STB_ImageInit.h"
#include "Rendering/Code/Window.h"
#include "Rendering/Code/Shaders/ShaderProgram.h"

#include "Rendering/Code/RenderingResourceTracking.h"

#include "Rendering/Code/Framebuffers.h"

#include "Rendering/Code/OpenGLRenderPipeline.h"
#include "Rendering/Code/Shaders/ShaderProgram.h"
#include "Rendering/Code/Shaders/Shader.h"

#include <iostream>

namespace Rendering
{
	namespace Texture
	{
		// ----------------------------------------------------------------------------------------------------------

		Texture2D::Texture2D() 
			:  mFilePath("")
			, mTextureID(0)
			, mPBO(0)

			, mWidth(0)
			, mHeight(0)
			, mFormat(0)

			, mInitialised(false)
			, mForVideo(false)
			, mHasAlpha(false)
			, mLastDataInvalid(true)
			, mLastPixelDataFromGPU(nullptr)

			, mInternalDataType(GL_UNSIGNED_BYTE)
			, mInternalFormat(GL_RGB)
			, mExternalFormat(GL_RGB)

			, mLoadedImageData(nullptr)
		{
			glGenTextures(1, &mTextureID);
		}

		// ----------------------------------------------------------------------------------------------------------

		Texture2D::~Texture2D()
		{
			FreeCachedImageData();

			glDeleteTextures(1, &mTextureID);
			mTextureID   = 0;

			if (mPBO != 0)
			{
				glDeleteBuffers(1, &mPBO);
				mPBO = 0;
			}

			if (mLastPixelDataFromGPU)
			{
				// I dont know if this is needed or not to prevent a memory leak
			//	delete[] mLastPixelDataFromGPU;
				mLastPixelDataFromGPU = nullptr;
			}

			mInitialised = false;
		}

		// ----------------------------------------------------------------------------------------------------------

		// TODO specify format based on file type
		bool Texture2D::LoadTextureFromFile(std::string             filePath,
											TextureMinMagFilters    minMagFilters, 
											TextureWrappingSettings textureWrapSettings)
		{
			if (filePath == "")
				return false;

			mLastDataInvalid     = true;
			mMinMagFilters       = minMagFilters;
			mTextureWrapSettings = textureWrapSettings;

			if (!LoadInImageData(filePath))
			{
				return false;
			}

			return SendTextureDataToGPU();
		}

		// ----------------------------------------------------------------------------------------------------------

		void Texture2D::Bind(GLenum unitToBindTo)
		{			
			OpenGLRenderPipeline* pipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

			pipeline->BindTextureToTextureUnit(unitToBindTo, mTextureID, true);

			GLenum error = glGetError();
			ASSERTMSG(error  != 0, "Error binding texture2D.");
		}

		// ----------------------------------------------------------------------------------------------------------

		void Texture2D::BindForComputeShader(GLuint unit, GLint level, bool layered, GLint layer, GLenum access, GLenum format)
		{
			glBindImageTexture(unit, mTextureID, level, layered, layer, access, format);

			GLenum error = glGetError();
			ASSERTMSG(error != 0, "Error binding texture2D.");
		}

		// ----------------------------------------------------------------------------------------------------------

		void Texture2D::UnBind(GLenum unitToBindTo)
		{
			OpenGLRenderPipeline* pipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

			pipeline->BindTextureToTextureUnit(unitToBindTo, 0, true);
		}

		// ----------------------------------------------------------------------------------------------------------

		bool Texture2D::InitEmpty(unsigned int width, unsigned int height, bool willHaveAlpha, GLenum internalDataType, GLenum internalFormat, GLenum format, TextureMinMagFilters minMagFilters, TextureWrappingSettings textureWrapSettings)
		{
			mWidth            = width;
			mHeight           = height;
			mHasAlpha         = willHaveAlpha;

			mInternalDataType = internalDataType;
			mInternalFormat   = internalFormat;
			mExternalFormat   = format;

			OpenGLRenderPipeline* renderPipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

			if (!renderPipeline)
				return false;

			// Bind the texture
			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mTextureID);

				SetTextureMinMagFilters(minMagFilters);
				SetTextureWrappingSettings(textureWrapSettings);

				if (willHaveAlpha)
				{
#ifdef _DEBUG_BUILD
					Rendering::TrackingData::AdjustGPUMemoryUsed(width * height * 4);
#endif
					mFormat = 4;
				}
				else
				{
#ifdef _DEBUG_BUILD
					Rendering::TrackingData::AdjustGPUMemoryUsed(width * height * 3);
#endif

					mFormat = 3;
				}

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0, format, internalDataType, nullptr);

			int error = glGetError();
			ASSERTMSG(error != 0, "Error with setting texture data");

			return true;
		}

		// ----------------------------------------------------------------------------------------------------------

		bool Texture2D::InitWithData(unsigned int width, unsigned int height, unsigned char* data, bool hasAlpha, TextureMinMagFilters minMagFilters, TextureWrappingSettings textureWrapSettings)
		{
			mLastDataInvalid = true;

			mWidth    = width;
			mHeight   = height;
			mHasAlpha = hasAlpha;

			Bind();

				SetTextureMinMagFilters(minMagFilters);
				SetTextureWrappingSettings(textureWrapSettings);

				if (mHasAlpha)
				{
#ifdef _DEBUG_BUILD
					Rendering::TrackingData::AdjustGPUMemoryUsed(width * height * 4);
#endif
					mInternalFormat = GL_RGBA;
					mExternalFormat = mInternalFormat;

					glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mExternalFormat, mInternalDataType, (const void*)data);
				}
				else
				{
#ifdef _DEBUG_BUILD
					Rendering::TrackingData::AdjustGPUMemoryUsed(width * height * 3);
#endif
					mInternalFormat = GL_RGB;
					mExternalFormat = mInternalFormat;

					glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mExternalFormat, mInternalDataType, (const void*)data);
				}

			UnBind();

			return true;
		}

		// ----------------------------------------------------------------------------------------------------------

		bool Texture2D::ReplaceTextureData(unsigned char* data)
		{
			mLastDataInvalid = true;

			Bind();
			
			// Replace the data with the new data
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, mInternalFormat, mInternalDataType, (const void*)data);

			UnBind();

			return true;
		}

		// ----------------------------------------------------------------------------------------------------------

		bool Texture2D::ReplaceTextureData(unsigned char* data, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int mipMapLevel)
		{
			if ((width == 0 && height == 0) || data == nullptr)
			{
				return false;
			}

			mLastDataInvalid = true;

			// Bind the texture
			Bind();

			// Replace the data with the new data
			glTexSubImage2D(GL_TEXTURE_2D, mipMapLevel, x, y, width, height, mInternalFormat, mInternalDataType, (const void*)data);

			// Unbind
			UnBind();

			GLenum error = glGetError();
			ASSERTMSG(error != 0, "Failure setting texture data");

			return true;
		}

		// ----------------------------------------------------------------------------------------------------------

		unsigned char* Texture2D::GetPixelData()
		{
			if (!mLastDataInvalid)
			{
				return mLastPixelDataFromGPU;
			}

			OpenGLRenderPipeline* renderPipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

			if (!renderPipeline)
				return mLastPixelDataFromGPU;

			// -----------------

			// See if the PBO has been created yet
			if (mPBO == 0)
			{
				glGenBuffers(1, &mPBO);
			}

			// -----------------

			// Bind the PBO
			glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);

			// Setup the internal data
			if(mHasAlpha)
				glBufferData(GL_PIXEL_PACK_BUFFER, mWidth * mHeight * 4, NULL, GL_STREAM_COPY);
			else
				glBufferData(GL_PIXEL_PACK_BUFFER, mWidth * mHeight * 3, NULL, GL_STREAM_COPY);

			// -----------------

			// Bind the texture to be read from
			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mTextureID);

			// -----------------

			// Kick off the read
			if (mHasAlpha)
			{
				//mInternalFormat = GL_RGBA;
				glGetTexImage(GL_TEXTURE_2D, 0, mInternalFormat, mInternalDataType, NULL);
			}
			else
			{
				//mInternalFormat = GL_RGB;
				glGetTexImage(GL_TEXTURE_2D, 0, mInternalFormat, mInternalDataType, NULL);
			}

			// -----------------

			GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

			// -----------------

			// Wait for the result to be avaliable - this is not advised, but when it becomes a problem I will rework it to be async
			GLint result;
			do
			{
				glGetSynciv(sync, GL_SYNC_STATUS, sizeof(result), NULL, &result);
			} while (result != GL_SIGNALED);

			// Grab the result
			mLastPixelDataFromGPU = (unsigned char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

			// Unmap as part of clean up
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

			// -----------------

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

			mLastDataInvalid = false;

			return mLastPixelDataFromGPU;
		}

		// ----------------------------------------------------------------------------------------------------------

		unsigned char* Texture2D::GetPixelData(unsigned int xOffset, unsigned int yOffset)
		{
			// --------

			// Make sure that the most recent data is cached
			GetPixelData();

			// --------

			unsigned int offset = (yOffset * mWidth) + xOffset;

			// Offset by the correct size of the pixels
			if (mHasAlpha)
			{
				offset *= 4;
			}
			else
			{
				offset *= 3;
			}

			// --------

			return &mLastPixelDataFromGPU[offset];

			// --------
		}

		// ----------------------------------------------------------------------------------------------------------

		// The texture needs to be bound before calling this
		void Texture2D::SetTextureMinMagFilters(TextureMinMagFilters minMagFilters)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minMagFilters.mMinFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, minMagFilters.mMagFilter);
		}

		// ----------------------------------------------------------------------------------------------------------

		// The texture needs to be bound before calling this
		void Texture2D::SetTextureWrappingSettings(TextureWrappingSettings settings)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, settings.mSSetting);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, settings.mTSetting);
		}

		// ----------------------------------------------------------------------------------------------------------

		void Texture2D::SetCompareMode(GLenum mode, GLenum function)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, mode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, function);
		}

		// ----------------------------------------------------------------------------------------------------------

		unsigned int Texture2D::GetDataSize()
		{
			if (mHasAlpha)
			{
				// Width * height * 4 components (RGBA)
				return mWidth * mHeight * 4;
			}

			// Width * height * 3 components (RGB)
			return mWidth * mHeight * 3;

		}

		// ----------------------------------------------------------------------------------------------------------

		bool Texture2D::LoadInImageData(std::string filePath)
		{
			int width, height, format;

			// Load the image data
			unsigned char* image = stbi_load(filePath.c_str(), &width, &height, &format, 0);

			if (image == nullptr)
			{
				std::cout << "Failed to load texture file: " << filePath << std::endl;

				return false;
			}

			mFilePath = filePath;

			mWidth    = width;
			mHeight   = height;
			mFormat   = format;

			mLoadedImageData = image;

			return true;
		}

		// ----------------------------------------------------------------------------------------------------------

		bool Texture2D::SendTextureDataToGPU()
		{
			// Bind the texture
			Bind();

			SetTextureMinMagFilters(mMinMagFilters);
			SetTextureWrappingSettings(mTextureWrapSettings);

			if (mFormat == 3)
			{
#ifdef _DEBUG_BUILD
				Rendering::TrackingData::AdjustGPUMemoryUsed(mWidth * mHeight * 3);
#endif

				mHasAlpha       = false;
				//mInternalFormat = GL_RGB;
				glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mExternalFormat, mInternalDataType, (const GLvoid*)mLoadedImageData);
			}
			else if (mFormat == 4)
			{
#ifdef _DEBUG_BUILD
				Rendering::TrackingData::AdjustGPUMemoryUsed(mWidth * mHeight * 4);
#endif

				mHasAlpha = true;
				//mInternalFormat = GL_RGBA;

				if (mInternalFormat == GL_RGB)
				{
					mInternalFormat = GL_RGBA;
				}

				if (mExternalFormat == GL_RGB)
				{
					mExternalFormat = GL_RGBA;
				}

				glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mExternalFormat, mInternalDataType, (const GLvoid*)mLoadedImageData);
			}
			else
			{
				std::cout << "Unsupported image format: " << mFilePath << std::endl;

				return false;
			}

			// Unbind
			UnBind();

			// Free the memory allocated
			stbi_image_free(mLoadedImageData);
			mLoadedImageData = nullptr;

			mInitialised = true;
			return true;
		}

		// ----------------------------------------------------------------------------------------------------------

		void Texture2D::FreeCachedImageData()
		{
			if (mLoadedImageData)
			{
				stbi_image_free(mLoadedImageData);
				mLoadedImageData = nullptr;

				mLastDataInvalid = true;

				delete[] mLastPixelDataFromGPU;
				mLastPixelDataFromGPU = nullptr;
			}
		}

		// ----------------------------------------------------------------------------------------------------------

		void Texture2D::Resize(unsigned int width, unsigned int height)
		{
			OpenGLRenderPipeline* renderPipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

			if (!renderPipeline)
				return;

			// Bind the texture
			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mTextureID);

#ifdef _DEBUG_BUILD
			if (mFormat == 3)
			{
				Rendering::TrackingData::AdjustGPUMemoryUsed(-1 * mWidth * mHeight * 3);
				Rendering::TrackingData::AdjustGPUMemoryUsed(     width  * height  * 3);
			}
			else
			{
				Rendering::TrackingData::AdjustGPUMemoryUsed(-1 * mWidth * mHeight * 4);
				Rendering::TrackingData::AdjustGPUMemoryUsed(     width  * height  * 4);
			}
#endif

			glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, (GLsizei)width, (GLsizei)height, 0, mExternalFormat, mInternalDataType, nullptr);

			GLenum error = glGetError();
			ASSERTMSG(error != 0, "Error resizing image");

			mWidth  = width;
			mHeight = height;
		}

		// ----------------------------------------------------------------------------------------------------------

		void Texture2D::LoadInDataFromBuffer(std::string typeToMimic, const unsigned char* dataBuffer, unsigned int bufferLength, unsigned int offsetIntoBuffer)
		{
			// See which format to mimic
			if (typeToMimic == "image/jpeg")
			{
				int width, height, numberOfChannels;
				mLoadedImageData = stbi_load_from_memory(dataBuffer + offsetIntoBuffer, bufferLength, &width, &height, &numberOfChannels, 3);

				mWidth  = (unsigned int)width;
				mHeight = (unsigned int)height;
				mFormat = 3;
			}
			else if (typeToMimic == "image/png")
			{
				int width, height, numberOfChannels;
				mLoadedImageData = stbi_load_from_memory(dataBuffer + offsetIntoBuffer, bufferLength, &width, &height, &numberOfChannels, 4);

				mWidth  = (unsigned int)width;
				mHeight = (unsigned int)height;
				mFormat = 4;


				// Make sure opengl knows this is an rgba texture instead of just rgb
				mInternalFormat = GL_RGBA;
				mExternalFormat = GL_RGBA;
			}

			mLastDataInvalid = true;
		}

		// ----------------------------------------------------------------------------------------------------------
		// ----------------------------------------------------------------------------------------------------------
		// ----------------------------------------------------------------------------------------------------------

		ShaderPrograms::ShaderProgram* CubeMapTexture::mConvolutionShader          = nullptr;
		ShaderPrograms::ShaderProgram* CubeMapTexture::mRoughnessConvolutionShader = nullptr;

		glm::mat4 CubeMapTexture::mCaptureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		CubeMapTexture::CubeMapTexture()
			: mTextureID(0)
			, mWidth(0)
			, mHeight(0)
		{
			glGenTextures(1, &mTextureID);

			if (!mConvolutionShader)
			{
				mConvolutionShader = new ShaderPrograms::ShaderProgram();

				Shaders::VertexShader*   vertexShader   = new Shaders::VertexShader("Code/Shaders/Vertex/ConvoluteCubeMap.vert");
				Shaders::FragmentShader* fragmentShader = new Shaders::FragmentShader("Code/Shaders/Fragment/ConvoluteCubeMap.frag");

				mConvolutionShader->AttachShader(vertexShader);
				mConvolutionShader->AttachShader(fragmentShader);

				mConvolutionShader->LinkShadersToProgram();

				mConvolutionShader->DetachShader(vertexShader);
				mConvolutionShader->DetachShader(fragmentShader);

				delete vertexShader;
				delete fragmentShader;
			}

			if (!mRoughnessConvolutionShader)
			{
				mRoughnessConvolutionShader = new ShaderPrograms::ShaderProgram();

				Shaders::VertexShader*   vertexShader   = new Shaders::VertexShader("Code/Shaders/Vertex/ConvoluteCubeMap_Reflections.vert");
				Shaders::FragmentShader* fragmentShader = new Shaders::FragmentShader("Code/Shaders/Fragment/ConvoluteCubeMap_Reflections.frag");

				mRoughnessConvolutionShader->AttachShader(vertexShader);
				mRoughnessConvolutionShader->AttachShader(fragmentShader);

				mRoughnessConvolutionShader->LinkShadersToProgram();

				mRoughnessConvolutionShader->DetachShader(vertexShader);
				mRoughnessConvolutionShader->DetachShader(fragmentShader);

				delete vertexShader;
				delete fragmentShader;
			}
		}

		// ----------------------------------------------------------------------------------------------------------

		CubeMapTexture::~CubeMapTexture()
		{
			glDeleteTextures(1, &mTextureID);
			mTextureID = 0;

			// This assumes that all images in the cube map are of the same size
			Rendering::TrackingData::AdjustGPUMemoryUsed(mWidth * mHeight * 3 * 6);
		}

		// ----------------------------------------------------------------------------------------------------------

		void CubeMapTexture::Bind(GLenum unitToBindTo)
		{
			OpenGLRenderPipeline* pipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

			pipeline->BindTextureToTextureUnit(unitToBindTo, mTextureID, false);

			ASSERTMSG(glGetError() != 0, "Error binding cubemap");
		}

		// ----------------------------------------------------------------------------------------------------------

		void CubeMapTexture::UnBind(GLenum unitToBindTo)
		{
			OpenGLRenderPipeline* pipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

			pipeline->BindTextureToTextureUnit(unitToBindTo, 0, false);
		}

		// ----------------------------------------------------------------------------------------------------------

		void CubeMapTexture::LoadInTextures(std::string filePaths[6], TextureMinMagFilters minMagFilters, TextureWrappingSettings wrapSettings)
		{
			Bind();

			int width, height, nrChannels;

			// Loop through all 6 sides of the image
			for (unsigned int i = 0; i < 6; i++)
			{
				unsigned char* data = stbi_load(filePaths[i].c_str(), &width, &height, &nrChannels, 0);

				if (data)
				{
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

					Rendering::TrackingData::AdjustGPUMemoryUsed(width * height * 3);

					mHeight = height;
					mWidth  = width;
				}
				else
				{
					ASSERTFAIL("Failed to load image for cubemap");
				}
				stbi_image_free(data);
				data = nullptr;
			}

			// ----------

			SetTextureMinMagFilters(minMagFilters);
			SetTextureWrappingSettings(wrapSettings);

			UnBind();

			// ----------
		}

		// ----------------------------------------------------------------------------------------------------------

		CubeMapTexture* CubeMapTexture::ConvoluteTexture(Buffers::VertexArrayObject* cubeVAO)
		{
			OpenGLRenderPipeline* renderPipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

			if (!renderPipeline || !mConvolutionShader)
				return nullptr;
			
			// ----
				 
			// Create the output cubemap
			CubeMapTexture* newCubemap = new CubeMapTexture();

			newCubemap->SetupInteralData(32, 32, GL_FLOAT, GL_RGB16F, GL_RGB, { GL_LINEAR, GL_LINEAR }, { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE });

			// ----

			mConvolutionShader->UseProgram();

			mConvolutionShader->SetInt("environmentMap", 0);

			glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			mConvolutionShader->SetMat4("projectionMatrix", &projectionMatrix[0][0]);

			// Bind this cubemap to the active texture ID
			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mTextureID, false);

			// Set the viewport to the right size for the texture
			glViewport(0, 0, 32, 32);

			Framebuffer* FBO = new Framebuffer();
			FBO->SetActive(true, true);

			cubeVAO->Bind();

			// Loop through each face
			for (unsigned int i = 0; i < 6; i++)
			{
				mConvolutionShader->SetMat4("viewMatrix", &mCaptureViews[i][0][0]);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, newCubemap->GetTextureID(), 0);

				GLenum error = glGetError();
				if (error != 0)
				{
					std::cout << "we have problems";
				}

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Render the cube
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}

			// ----

			FBO->SetActive(false, true);
			delete FBO;

			cubeVAO->Unbind();

			// ----

			// Reset the viewport to the screen size
			glViewport(0, 0, Window::GetWindowWidth(), Window::GetWindowHeight());

			return newCubemap;
		}

		// ----------------------------------------------------------------------------------------------------------

		void CubeMapTexture::SetTextureMinMagFilters(TextureMinMagFilters minMagFilters)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minMagFilters.mMinFilter);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, minMagFilters.mMagFilter);
		}

		// ----------------------------------------------------------------------------------------------------------

		// The texture needs to be bound before calling this
		void CubeMapTexture::SetTextureWrappingSettings(TextureWrappingSettings settings)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, settings.mSSetting);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, settings.mTSetting);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, settings.mRSetting);
		}

		// ----------------------------------------------------------------------------------------------------------

		void CubeMapTexture::SetupInteralData(unsigned int width, unsigned int height, GLenum dataType, GLenum internalFormat, GLenum externalFormat, TextureMinMagFilters minMagFilters, TextureWrappingSettings wrapSettings, bool generateMipMaps)
		{
			Bind();

			mWidth  = width;
			mHeight = height;

			for (unsigned int i = 0; i < 6; ++i)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, externalFormat, dataType, nullptr);
			}

			SetTextureMinMagFilters(minMagFilters);
			SetTextureWrappingSettings(wrapSettings);

			if (generateMipMaps)
			{
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			}
		}

		// ----------------------------------------------------------------------------------------------------------

		void CubeMapTexture::ConvoluteTexture_Roughness(Buffers::VertexArrayObject* cubeVAO)
		{
			OpenGLRenderPipeline* renderPipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

			if (!renderPipeline || !mRoughnessConvolutionShader)
				return;
			
			// Set the texture to convolute
			mRoughnessConvolutionShader->UseProgram();

			renderPipeline->BindTextureToTextureUnit(GL_TEXTURE0, mTextureID, false);

			mRoughnessConvolutionShader->SetInt("environmentMap", 0);

			// Set the projection matrix
			glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			mRoughnessConvolutionShader->SetMat4("projectionMatrix", &projectionMatrix[0][0]);

			// Now loop through each mip map layer
			unsigned int mipMapLevels = 5;

			// Grab the cube VAO
			cubeVAO->Bind();

			Framebuffer  FBO = Framebuffer();
			RenderBuffer RBO = RenderBuffer();

			FBO.SetActive(true, true);

			RBO.SetStorageData(GL_DEPTH_COMPONENT24, 128, 128);
			FBO.AssignRenderBuffer(&RBO, GL_DEPTH_ATTACHMENT);

			for (unsigned int mipLevel = 0; mipLevel < mipMapLevels; mipLevel++)
			{
				// Calculate the texture size of this mip level
				unsigned int textureWidth  = (unsigned int)((double)mWidth  * std::pow(0.5f, mipLevel));
				unsigned int textureHeight = (unsigned int)((double)mHeight * std::pow(0.5f, mipLevel));

				RBO.Bind();
				RBO.SetStorageData(GL_DEPTH_COMPONENT24, textureWidth, textureHeight);

				// Now set the viewport
				glViewport(0, 0, textureWidth, textureHeight);

				// Calculate the roughness value for this level and set in the shader
				float roughness = (float)mipLevel / (float)(mipMapLevels - 1);
				mRoughnessConvolutionShader->SetFloat("roughness", roughness);

				// Loop through all 6 faces
				for (unsigned int j = 0; j < 6; j++)
				{
					// Set the view matrix
					mRoughnessConvolutionShader->SetMat4("viewMatrix", &mCaptureViews[j][0][0]);

					// Bind the texture to the framebuffer
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, mTextureID, mipLevel);

					// Clear the existing data
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					// Draw the cube
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
			}

			FBO.SetActive(false, true);

			cubeVAO->Unbind();

			// Reset the viewport
			glViewport(0, 0, Window::GetWindowWidth(), Window::GetWindowHeight());
		}

		// ----------------------------------------------------------------------------------------------------------
	};
}