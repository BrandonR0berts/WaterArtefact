#include "Framebuffers.h"

#include "Textures/Texture.h"

namespace Rendering
{
	// ----------------------------------

	Framebuffer::Framebuffer()
		: Buffer::Buffer()
		, mFBO(0)
		, mColourBuffers()
		, mDepthBuffer(nullptr)
		, mStencilBuffer(nullptr)
	{
		glGenFramebuffers(1, &mFBO);

		ASSERTMSG(glGetError() != 0, "Error creating framebuffer");
	}

	// ----------------------------------

	Framebuffer::~Framebuffer()
	{
		SetActive(true, true);

		// Unbind the buffers
		unsigned int bufferCount = (unsigned int)mColourBuffers.size();
		for (unsigned int i = 0; i < bufferCount; i++)
		{
			if (mColourBuffers[i].second)
			{
				mColourBuffers[i].second = nullptr;

				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
			}
		}
		mColourBuffers.clear();

		if (mDepthBuffer)
		{
			mDepthBuffer = nullptr;

			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
		}

		if (mStencilBuffer)
		{
			mStencilBuffer = nullptr;

			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
		}

		SetActive(false, true);

		// Delete the FBO
		glDeleteFramebuffers(1, &mFBO);

		ASSERTMSG(glGetError() != 0, "Error deleting framebuffer");
	}

	// ----------------------------------

	void Framebuffer::SetActive(bool activeState, bool drawing)
	{
		int error = 0;
		if (activeState)
		{
			if (drawing)
			{
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			}
			else
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			}
		}
		else
		{
			if (drawing)
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			else
				glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		}

		error = glGetError();
		ASSERTMSG(error != 0, "Error binding framebuffer to OpenGL!");
	}

	// ----------------------------------
	 
	void Framebuffer::AttachColourBuffer(Texture::Texture2D* colourTexture, unsigned int attachmentID, bool drawing)
	{
		if (!colourTexture)
			return;

		SetActive(true, drawing);

		GLuint textureID = colourTexture->GetTextureID();

		if (drawing)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentID, GL_TEXTURE_2D, textureID, 0);
		}
		else
		{
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentID, GL_TEXTURE_2D, textureID, 0);
		}

		int error = glGetError();
		ASSERTMSG(error != 0, "Error attaching colour buffer to framebuffer!");

		CheckComplete();

		RemoveColourBuffer(attachmentID);
		mColourBuffers.push_back({ attachmentID, colourTexture });

		BindAllColourBuffers();
	}

	// ----------------------------------

	void Framebuffer::RemoveColourBuffer(unsigned int attachmentID)
	{
		unsigned int bufferCount = (unsigned int)mColourBuffers.size();
		for (unsigned int i = 0; i < bufferCount; i++)
		{
			if (mColourBuffers[i].first == attachmentID)
			{
				mColourBuffers.erase(mColourBuffers.begin() + i);
				return;
			}
		}
	}

	// ----------------------------------

	Texture::Texture2D* Framebuffer::GetColourBuffer(unsigned int ID)
	{
		unsigned int bufferCount = (unsigned int)mColourBuffers.size();
		for (unsigned int i = 0; i < bufferCount; i++)
		{
			if (mColourBuffers[i].first == ID)
				return mColourBuffers[i].second;
		}

		return nullptr;
	}

	// ----------------------------------

	void Framebuffer::AttachDepthBuffer(Texture::Texture2D* depthTexture, bool drawing)
	{
		if (!depthTexture)
			return;

		SetActive(true, drawing);

		GLuint textureID = depthTexture->GetTextureID();

		if(drawing)
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
		else
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);

		ASSERTMSG(glGetError() != 0, "Error attaching depth buffer to framebuffer!");

		mDepthBuffer = depthTexture;
	}

	// ----------------------------------

	void Framebuffer::AttachStencilBuffer(Texture::Texture2D* stencilTexture, bool drawing)
	{
		if (!stencilTexture)
			return;

		SetActive(true, drawing);

		GLuint textureID = stencilTexture->GetTextureID();

		if(drawing)
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
		else
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);

		ASSERTMSG(glGetError() != 0, "Error attaching stencil buffer to framebuffer!");

		mStencilBuffer = stencilTexture;
	}

	// ----------------------------------

	void Framebuffer::AttachDepthStencilBuffer(Texture::Texture2D* depthStencilTexture, bool drawing)
	{
		if (!depthStencilTexture)
			return;

		SetActive(true, drawing);

		GLuint textureID = depthStencilTexture->GetTextureID();

		if(drawing)
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
		else
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);

		ASSERTMSG(glGetError() != 0, "Error attaching depth + stencil buffer to framebuffer!");

		mDepthBuffer   = depthStencilTexture;
		mStencilBuffer = depthStencilTexture;
	}

	// ----------------------------------

	void Framebuffer::ClearAll(float x, float y, float z, float w)
	{
		SetActive(true, true);

		glClearColor(x, y, z, w);

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		GLenum error = glGetError();
		ASSERTMSG(error != 0, "Error clearing texture");
	}

	// ----------------------------------

	void Framebuffer::ClearColourBuffer(unsigned int attachmentID)
	{
		SetActive(true, true);

		glClear(GL_COLOR_BUFFER_BIT);

		ASSERTMSG(glGetError() != 0, "Error clearing texture");
	}

	// ----------------------------------

	void Framebuffer::ClearDepthBuffer()
	{
		SetActive(true, true);

		glClear(GL_DEPTH_BUFFER_BIT);

		ASSERTMSG(glGetError() != 0, "Error clearing texture");
	}

	// ----------------------------------

	void Framebuffer::ClearStencilBuffer()
	{
		SetActive(true, true);

		glClear(GL_STENCIL_BUFFER_BIT);

		ASSERTMSG(glGetError() != 0, "Error clearing texture");
	}

	// ----------------------------------

	void Framebuffer::ClearDepthStencilBuffer()
	{
		SetActive(true, true);

		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ASSERTMSG(glGetError() != 0, "Error clearing texture");
	}

	// ----------------------------------

	bool Framebuffer::CheckComplete(bool drawing)
	{
		if (drawing)
		{
			if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				ASSERTFAIL("Drawing framebuffer is not complete");

				return false;
			}
		}
		else
		{
			if(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				ASSERTFAIL("Reading framebuffer is not complete");

				return false;
			}
		}

		return true;
	}

	// ----------------------------------

	void Framebuffer::BindAllColourBuffers()
	{
		if (mColourBuffers.empty())
			return;

		// Find the max value of the attachment ids
		unsigned int maxAttachmentID = 0;

		unsigned int bufferCount = (unsigned int)mColourBuffers.size();
		for (unsigned int i = 0; i < bufferCount; i++)
		{
			if (mColourBuffers[i].first > maxAttachmentID)
			{
				maxAttachmentID = mColourBuffers[i].first;
			}
		}

		// Allocate memory and default to GL_NONE (0)
		unsigned int* bufferState = new unsigned int[maxAttachmentID + 1]; // Plus 1 because we need a count not an index
		memset(bufferState, GL_NONE, sizeof(unsigned int) * (maxAttachmentID + 1));

		for (unsigned int i = 0; i < bufferCount; i++)
		{
			bufferState[mColourBuffers[i].first] = GL_COLOR_ATTACHMENT0 + mColourBuffers[i].first;
		}

		glDrawBuffers(maxAttachmentID + 1, bufferState);
	}

	// ----------------------------------

	Maths::Vector::Vector4D<unsigned int> Framebuffer::GetPixelColour(Maths::Vector::Vector2D<float> coord, GLenum format, GLenum type, unsigned int attachmentID)
	{
		// Set active for reading
		SetActive(true, false);

		// Set the draw buffer
		unsigned int data[4] = { 0 };
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentID);

			// Read the pixel from the image
			glReadPixels((int)coord.x, (int)coord.y, 1, 1, format, type, &data);

		int error = glGetError();
		ASSERTMSG(error != 0, "error");

		// Remove the buffer we are reading from
		glReadBuffer(0);

		// Disable the FBO for reading
		SetActive(false, false);

		return Maths::Vector::Vector4D<unsigned int>(data[0], data[1], data[2], data[3]);
	}

	// ----------------------------------

	void Framebuffer::ResizeBuffers(unsigned int width, unsigned int height)
	{
		// This can happen on minimise, which will get overridden when opening the window again
		// So this is not problematic to set
		if (width == 0 && height == 0)
		{
			width  = 1;
			height = 1;
		}

		SetActive(true, true);

		unsigned int bufferCount = (unsigned int)mColourBuffers.size();
		for (unsigned int i = 0; i < bufferCount; i++)
		{
			if (mColourBuffers[i].second)
			{
				mColourBuffers[i].second->Resize(width, height);

				GLenum error = glGetError();
				ASSERTMSG(error != 0, "Error resizing the buffers for a framebuffer");
			}
		}

		if (mDepthBuffer)
		{
			mDepthBuffer->Resize(width, height);

			GLenum error = glGetError();
			ASSERTMSG(error != 0, ">");
		}

		if (mStencilBuffer)
		{
			mStencilBuffer->Resize(width, height);

			GLenum error = glGetError();
			ASSERTMSG(error != 0, ">");
		}

		CheckComplete();

		SetActive(false, true);
	}

	// ----------------------------------

	void Framebuffer::AssignRenderBuffer(RenderBuffer* buffer, GLenum attachmentMode)
	{
		if (!buffer)
			return;

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentMode, GL_RENDERBUFFER, buffer->GetID());

		GLenum error = glGetError();

		ASSERTMSG(error != 0, "Error binding renderbuffer to framebuffer");
	}

	// ----------------------------------
	// ----------------------------------
	// ----------------------------------

	RenderBuffer::RenderBuffer()
		: mRBOID(0)
	{
		glGenRenderbuffers(1, &mRBOID);

		ASSERTMSG(glGetError() != 0, "Error creating renderbuffer");
	}

	// ----------------------------------

	RenderBuffer::~RenderBuffer()
	{
		glDeleteRenderbuffers(1, &mRBOID);

		ASSERTMSG(glGetError() != 0, "Error deleting renderbuffer");
	}

	// ----------------------------------

	void RenderBuffer::Bind()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, mRBOID);

		ASSERTMSG(glGetError() != 0, "Error binding renderbuffer");
	}

	// ----------------------------------

	void RenderBuffer::SetStorageData(GLenum format, unsigned int width, unsigned int height)
	{
		Bind();

		glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);

		ASSERTMSG(glGetError() != 0, "Error setting renderbuffer data");
	}

	// ----------------------------------
}