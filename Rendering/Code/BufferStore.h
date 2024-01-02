#pragma once

#include "Rendering/Code/Buffers.h"
#include "Rendering/Code/Framebuffers.h"

#include <unordered_map>
#include <string>
#include <queue>

namespace Rendering
{
	namespace Buffers
	{
		// ----------------------------------------------

		enum class BufferType : unsigned int
		{
			VBO,
			SSBO,
			EBO,
			VAO,

			FBO
		};

		// ----------------------------------------------

		struct AttributePointerData
		{
			AttributePointerData(unsigned int index,
								 unsigned int floatsInSingleData,
								 GLenum       dataType,
								 GLboolean    normalised,
								 unsigned int stride,
								 unsigned int offset,
								 bool         perVertex,
								 std::string  vboNameToBind)
				: mIndex(index),
			      mFloatsInSingleData(floatsInSingleData),
			      mDataType(dataType),
			      mNormalised(normalised),
			      mStride(stride),
			      mOffset(offset),
			      mPerVertex(perVertex),
				  mVBONameToBind(vboNameToBind)
			{

			}

			unsigned int mIndex;
			unsigned int mFloatsInSingleData;
			GLenum       mDataType;
			GLboolean    mNormalised;
			unsigned int mStride;
			unsigned int mOffset;
			bool         mPerVertex;
			std::string  mVBONameToBind;
		};

		// ----------------------------------------------

		struct QueuedBufferData abstract
		{			
			unsigned int mHashedName;
		};

		struct QueuedVBOData final : public QueuedBufferData
		{
			void*        mData;
			unsigned int mLength;
			GLenum       mUsage;
			GLenum       mTarget;
		};

		struct QueuedSSBOData final : public QueuedBufferData
		{
			void*        mData;
			unsigned int mLength;
			GLenum       mUsage;
		};

		struct QueuedEBOData final : public QueuedBufferData
		{
			void*        mData;
			unsigned int mLength;
			GLenum       mUsage;
		};

		struct QueuedVAOData final : public QueuedBufferData
		{
			std::string mEBOBufferName;
			std::vector<AttributePointerData> mAttributePointers;
		};

		struct QueuedFBOData final : public QueuedBufferData
		{
			std::string mFBOName;
		};

		// ----------------------------------------------

		class BufferStore final
		{
		public:
			BufferStore();
			~BufferStore();

			void Update();
		
			void CreateVBO(void* data, unsigned int length, GLenum usage, std::string name, GLenum target = GL_ARRAY_BUFFER);
			void CreateEBO(float* data, unsigned int length, GLenum usage, std::string name);
			void CreateVAO(std::string name, std::vector<AttributePointerData> attributeSettingData, std::string EBOBufferToBind = "");
			void CreateSSBO(void* data, unsigned int length, GLenum usage, std::string name);
			void CreateFBO(std::string name);

			VertexArrayObject*         GetVAO(std::string name);
			VertexBufferObject*        GetVBO(std::string name);
			ElementBufferObjects*      GetEBO(std::string name);
			ShaderStorageBufferObject* GetSSBO(std::string name);
			Framebuffer*               GetFBO(std::string name);

			void RemoveBuffer(std::string name, Buffers::BufferType type);

			void Clear();

#ifdef _DEBUG_BUILD
			void ShowImGuiData();
#endif

		private:
			void AddQueuedVBO();
			void AddQueuedSSBO();
			void AddQueuedEBO();
			void AddQueuedVAO();
			void AddQueuedFBO();

			void HandleSettingVAOParams(VertexArrayObject* newBuffer, QueuedVAOData* VAOData);

			std::queue<std::pair<BufferType, QueuedBufferData*>> mQueuedAdditions;
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>  mBuffers;
		};

		// ----------------------------------------------
	}
}