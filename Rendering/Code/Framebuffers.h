#pragma once

#include "Rendering/Code/AssertMsg.h"
#include "Rendering/Code/Buffers.h"
#include "Maths/Code/Vector.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Rendering
{
	namespace Texture
	{
		class Texture2D;
	}

	class RenderBuffer;

	// -------------------------------------------

	class Framebuffer : public Buffers::Buffer
	{
	public:
		Framebuffer();
		~Framebuffer();

		// --------------

		void SetActive(bool activeState, bool drawing);
		void BindAllColourBuffers();

		// --------------

		void AttachColourBuffer(Texture::Texture2D* colourTexture, unsigned int attachmentID = 0, bool drawing = true);
		void AttachDepthBuffer(Texture::Texture2D* depthTexture, bool drawing = true);
		void AttachStencilBuffer(Texture::Texture2D* stencilTexture, bool drawing = true);

		void AttachDepthStencilBuffer(Texture::Texture2D* depthStencilTexture, bool drawing = true);

		// --------------

		void ClearAll(float x, float y, float z, float w);

		void ClearColourBuffer(unsigned int attachmentID = 0);
		void ClearDepthBuffer();
		void ClearStencilBuffer();
		void ClearDepthStencilBuffer();

		// --------------

		Texture::Texture2D* GetColourBuffer(unsigned int ID = 0);
		Texture::Texture2D* GetDepthBuffer()                      const { return mDepthBuffer; }
		Texture::Texture2D* GetStencilBuffer()                    const { return mStencilBuffer; }

		void RemoveColourBuffer(unsigned int attachmentID);

		// --------------

		bool CheckComplete(bool drawing = true);

		// Format = GL_STENCIL_INDEX, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL, GL_RED, GL_GREEN, GL_BLUE, GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA.
		// type = GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_HALF_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV,
		// GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_INT_8_8_8_8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_24_8, GL_UNSIGNED_INT_10F_11F_11F_REV,
		// GL_UNSIGNED_INT_5_9_9_9_REV, or GL_FLOAT_32_UNSIGNED_INT_24_8_REV
		Maths::Vector::Vector4D<unsigned int> GetPixelColour(Maths::Vector::Vector2D<float> coord, GLenum format, GLenum type, unsigned int attachmentID = 0);

		void ResizeBuffers(unsigned int width, unsigned int height);

		void AssignRenderBuffer(RenderBuffer* bufffer, GLenum mode);

	private:
		GLuint mFBO;

		std::vector<std::pair<unsigned int, Texture::Texture2D*>> mColourBuffers; // The buffers bound to the draw buffers
		Texture::Texture2D*              mDepthBuffer;
		Texture::Texture2D*              mStencilBuffer;
	};

	// -------------------------------------------

	class RenderBuffer : public Buffers::Buffer
	{
	public:
		RenderBuffer();
		~RenderBuffer();

		void Bind();

		void SetStorageData(GLenum format, unsigned int width, unsigned int height);

		unsigned int GetID() const { return mRBOID; }

	private:
		unsigned int mRBOID;
	};

	// -------------------------------------------
}