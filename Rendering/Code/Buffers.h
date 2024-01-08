#ifndef _BUFFERS_H_
#define _BUFFERS_H_

// This file contains definitions for the different types of buffers OpenGL requires, along with the attribute linking code
// This is all within the 'Buffers' namespace

#include "Maths/Code/AssertMsg.h"

#include "Rendering/Code/RenderingResourceTracking.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Rendering
{
	namespace Buffers
	{
		static float IdentityMatrix4x4[16] = {  1.0f, 0.0f, 0.0f, 0.0f,
												0.0f, 1.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 1.0f, 0.0f,
												0.0f, 0.0f, 0.0f, 1.0f };

		// ----------------------------------------------------
		// ====================================================

		class Buffer abstract
		{
		public:
			Buffer()
			{

			}

			~Buffer()
			{

			}

		private:
		};

		// ----------------------------------------------------
		// ====================================================

		class VertexBufferObject final : public Buffer
		{
		public:
			// --------------------------------

			VertexBufferObject()
				: Buffer()

				, mVBO(0)
				, mBytesInData(0)
				, mTarget(GL_ARRAY_BUFFER)
			{
				glGenBuffers(1, &mVBO);
			}

			// --------------------------------

			~VertexBufferObject()
			{
				Delete();
			}

			// --------------------------------

			// Binds to the stored target, which is GL_ARRAY_BUFFER by default
			void Bind()
			{
				glBindBuffer(mTarget, mVBO);

				ASSERTMSG(glGetError() != 0, "Error binding buffer.");
			}

			void Bind(GLenum target)
			{
				mTarget = target;

				Bind();
			}

			void UnBind()
			{
				glBindBuffer(mTarget, 0);

				GLenum error = glGetError();
				ASSERTMSG(error != 0, "Error binding buffer.");
			}

			void UnBind(GLenum target)
			{
				mTarget = target;
				 
				UnBind();
			}

			// --------------------------------

			void SetBufferData(const GLvoid* data, unsigned int bytesInData, GLenum usage)
			{
				if (mBytesInData != bytesInData)
				{
					Rendering::TrackingData::AdjustGPUMemoryUsed(bytesInData - mBytesInData);
				}

				Bind();
				mBytesInData = bytesInData;
				glBufferData(mTarget, mBytesInData, data, usage);

				GLenum error = glGetError();
				ASSERTMSG(error != 0, "Error setting buffer data.");
			}

			// --------------------------------

			void UpdateBufferData(const GLvoid* data, unsigned int bytesInData, GLenum usage)
			{
				Bind();

				// Set all of the prior data as being null
				glBufferData(mTarget, mBytesInData, NULL, usage);

				Rendering::TrackingData::AdjustGPUMemoryUsed(bytesInData - mBytesInData);

				// Set the new size and store the new data
				mBytesInData = bytesInData;
				glBufferData(mTarget, bytesInData, data, usage);

				ASSERTMSG(glGetError() != 0, "Error setting buffer data.");
			}

			void SubBufferUpdate(unsigned int offset, unsigned int bytesInData, const GLvoid* data)
			{
				Bind();

				// Error as we needed to have pre-assigned enough memory for this to work
				if (offset + bytesInData > mBytesInData)
					return;

				glBufferSubData(mTarget, offset, bytesInData, data);

				ASSERTMSG(glGetError() != 0, "Error setting buffer sub-data.");
			}

			void ClearAllDataInBuffer(GLenum usage)
			{
				Bind();
				glBufferData(mTarget, mBytesInData, NULL, usage);
				ASSERTMSG(glGetError() != 0, "Error clearing buffer data.");
				mBytesInData = 0;
			}

			void AllocateMemory(unsigned int bytes, GLenum usage)
			{
				Bind();

				// Clear the existing memory
				glBufferData(mTarget, mBytesInData, NULL, usage);

				ASSERTMSG(glGetError() != 0, "Error clearing buffer data.");

				// Alloctae the new memory count
				glBufferData(mTarget, bytes, NULL, usage);

				ASSERTMSG(glGetError() != 0, "Error setting buffer size.");

				// Store the count
				mBytesInData = bytes;

				Rendering::TrackingData::AdjustGPUMemoryUsed(bytes);
			}

			// --------------------------------

			unsigned int GetBytesInBuffer() const { return mBytesInData; }

			// --------------------------------

			void Delete()
			{
				Rendering::TrackingData::AdjustGPUMemoryUsed(-((int)mBytesInData));

				glDeleteBuffers(1, &mVBO);
				ASSERTMSG(glGetError() != 0, "Error deleteing buffers.");

				mBytesInData = 0;
			}

			// --------------------------------

			void SetTarget(GLenum target)
			{
				mTarget = target;
			}

		private:
			unsigned int mVBO;
			unsigned int mBytesInData;
			GLenum       mTarget;
		};

		// ----------------------------------------------------
		// ====================================================

		// Used for compute shaders and can support a lot more data than VBO's
		// OpenGL wiki: "The OpenGL spec guarantees that UBOs can be up to 16KB in size (implementations can allow them to be bigger). The spec guarantees that SSBOs can be up to 128MB. Most implementations will let you allocate a size up to the limit of GPU memory"
		class ShaderStorageBufferObject final : public Buffer
		{
		public:
			// --------------------------------			

			ShaderStorageBufferObject()
				: Buffer()

				, mSSBO(0)
				, mBytesInData(0)
			{
				glGenBuffers(1, &mSSBO);

				GLenum error = glGetError();

				ASSERTMSG(error != 0, "Error generating SSBO");
			}

			// --------------------------------

			~ShaderStorageBufferObject()
			{
				Delete();
			}

			// --------------------------------

			void Bind()
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSSBO);

				ASSERTMSG(glGetError() != 0, "Error binding SSBO");
			}

			// --------------------------------

			void SetBufferData(const GLvoid* data, unsigned int bytesInData, GLenum usage)
			{
				if (mBytesInData != bytesInData)
				{
					Rendering::TrackingData::AdjustGPUMemoryUsed(bytesInData - mBytesInData);
				}

				Bind();
				mBytesInData = bytesInData;
				glBufferData(GL_SHADER_STORAGE_BUFFER, mBytesInData, data, usage);

				ASSERTMSG(glGetError() != 0, "Error setting SSBO data");
			}

			// --------------------------------

			void UpdateBufferData(const GLvoid* data, unsigned int bytesInData, GLenum usage)
			{
				Bind();

				// Set all of the prior data as being null
				glBufferData(GL_SHADER_STORAGE_BUFFER, mBytesInData, NULL, usage);

				Rendering::TrackingData::AdjustGPUMemoryUsed(bytesInData - mBytesInData);

				// Set the new size and store the new data
				mBytesInData = bytesInData;
				glBufferData(GL_SHADER_STORAGE_BUFFER, bytesInData, data, usage);

				ASSERTMSG(glGetError() != 0, "Error setting SSBO data");
			}

			void SubBufferUpdate(unsigned int offset, unsigned int bytesInData, const GLvoid* data)
			{
				Bind();

				// Error as we needed to have pre-assigned enough memory for this to work
				if (offset + bytesInData > mBytesInData)
					return;

				glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, bytesInData, data);

				ASSERTMSG(glGetError() != 0, "Error setting SSBO sub-data");
			}

			void ClearAllDataInBuffer(GLenum usage)
			{
				Bind();
				glBufferData(GL_SHADER_STORAGE_BUFFER, mBytesInData, NULL, usage);

				ASSERTMSG(glGetError() != 0, "Error setting SSBO data");
				mBytesInData = 0;
			}

			void AllocateMemory(unsigned int bytes, GLenum usage)
			{
				Bind();

				// Clear the existing memory
				glBufferData(GL_SHADER_STORAGE_BUFFER, mBytesInData, NULL, usage);
				ASSERTMSG(glGetError() != 0, "Error setting SSBO data");

				// Alloctae the new memory count
				glBufferData(GL_SHADER_STORAGE_BUFFER, bytes, NULL, usage);
				ASSERTMSG(glGetError() != 0, "Error setting SSBO data");

				// Store the count
				mBytesInData = bytes;

				Rendering::TrackingData::AdjustGPUMemoryUsed(bytes);
			}

			// --------------------------------

			unsigned int GetBytesInBuffer() const { return mBytesInData; }

			// --------------------------------

			void BindToBufferIndex(unsigned int index)
			{
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, mSSBO);
			}

			// --------------------------------

			void Delete()
			{
				Rendering::TrackingData::AdjustGPUMemoryUsed(-((int)mBytesInData));

				glDeleteBuffers(1, &mSSBO);
				mBytesInData = 0;
			}

			// --------------------------------

		private:
			unsigned int mSSBO;
			unsigned int mBytesInData;
		};


		// ----------------------------------------------------
		// ====================================================

		class ElementBufferObjects final : public Buffer
		{
		public:
			// --------------------------------------------------------

			ElementBufferObjects()
				: Buffer()
			{
				glGenBuffers(1, &mEBO);
			}

			// --------------------------------------------------------

			~ElementBufferObjects()
			{
				Delete();
			}

			// --------------------------------------------------------

			void Bind()
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
			}

			// --------------------------------------------------------

			void SetBufferData(unsigned int  bytesForBuffer,
				const GLvoid* data,
				GLenum        usage)
			{
				Bind();

				glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytesForBuffer, data, usage);
				
				Rendering::TrackingData::AdjustGPUMemoryUsed(bytesForBuffer);
			}

			// --------------------------------------------------------

			void Delete()
			{
				glDeleteBuffers(1, &mEBO);
			}

			// --------------------------------------------------------

		private:
			unsigned int mEBO;
		};

		// ----------------------------------------------------

		// These hold calls to setting of vertex attrib array calls, as well as the setting of EBO's
		class VertexArrayObject final : public Buffer
		{
		public:
			// -----------------------------------------

			VertexArrayObject()
				: Buffer()

				, mVAO(0)
			{
				glGenVertexArrays(1, &mVAO);

				ASSERTMSG(glGetError() != 0, "Error generating VAO");
			}

			// -----------------------------------------

			~VertexArrayObject()
			{
				glDeleteVertexArrays(1, &mVAO);

				ASSERTMSG(glGetError() != 0, "Error deleting VAO");
			}

			// -----------------------------------------

			void Bind()
			{
				glBindVertexArray(mVAO);

				ASSERTMSG(glGetError() != 0, "Error binding VAO");
			}

			// -----------------------------------------

			void Unbind()
			{
				glBindVertexArray(0);
			}

			// -----------------------------------------

			void Delete()
			{
				glDeleteVertexArrays(1, &mVAO);

				ASSERTMSG(glGetError() != 0, "Error deleting VAO");
			}

			// -----------------------------------------

			void EnableVertexAttribArray(unsigned int index)
			{
				glEnableVertexAttribArray(index);

				ASSERTMSG(glGetError() != 0, "Error enabling VAO vertex attribute");
			}

			// -----------------------------------------

// This line disables the warning about casting from unisigned int to GLvoid*, which is completly unavoidable
// So I dont want to have this spammed every time I compile the program
#pragma warning( push )
#pragma warning( disable : 4312 )


			void SetVertexAttributePointers(unsigned int index,
				unsigned int floatsInSingleData,
				GLenum		 dataType,
				GLboolean    normalised,
				unsigned int stride,
				unsigned int offset,
				bool         perVertex)
			{
				glVertexAttribPointer(index, floatsInSingleData, dataType, normalised, stride, (GLvoid*)offset);

				ASSERTMSG(glGetError() != 0, "Error setting VAO attribute data");

				if (perVertex)
				{
					glVertexAttribDivisor(index, 0);
				}
				else
				{
					glVertexAttribDivisor(index, 1);
				}

				ASSERTMSG(glGetError() != 0, "Error setting VAO attribute data");
			}

#pragma warning( pop )

			// -----------------------------------------

			// Use this for GL_INT, GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT_ GL_UNSIGNED_SHORT, GL_UNSIGNED INT
			void SetVertexAttributePointers_int(unsigned int index,
				unsigned int floatsInSingleData,
				GLenum		 dataType,
				unsigned int stride,
				void*        offset,
				bool         perVertex)
			{
				glVertexAttribIPointer(index, floatsInSingleData, dataType, stride, offset);

				ASSERTMSG(glGetError() != 0, "Error setting VAO attribute data");

				if (perVertex)
				{
					glVertexAttribDivisor(index, 0);
				}
				else
				{
					glVertexAttribDivisor(index, 1);
				}
			}

			// -----------------------------------------

			// Use this for GL_DOUBLE
			void SetVertexAttributePointers_double(unsigned int index,
				unsigned int size,
				GLenum		 dataType,
				unsigned int stride,
				void*        offset,
				bool         perVertex)
			{
				glVertexAttribLPointer(index, size, dataType, stride, offset);

				ASSERTMSG(glGetError() != 0, "Error setting VAO attribute data");

				if (perVertex)
				{
					glVertexAttribDivisor(index, 0);
				}
				else
				{
					glVertexAttribDivisor(index, 1);
				}
			}

			// -----------------------------------------

		private:
			unsigned int mVAO;
		};

		// ----------------------------------------------------
		// ====================================================
	};
};

#endif