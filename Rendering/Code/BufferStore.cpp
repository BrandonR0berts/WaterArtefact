#include "BufferStore.h"

#include "Engine/Code/StringHash.h"
#include "Engine/Code/AssertMsg.h"

#ifdef _DEBUG_BUILD
	#include "Include/imgui/imgui.h"
	#include "Include/imgui/imgui_impl_glfw.h"
	#include "Include/imgui/imgui_impl_opengl3.h"
#endif

namespace Rendering
{
	namespace Buffers
	{
		// ----------------------------------------------

		BufferStore::BufferStore()
		{

		}

		// ----------------------------------------------

		BufferStore::~BufferStore()
		{
			Clear();
		}

		// ----------------------------------------------

		void BufferStore::CreateVBO(void* data, unsigned int length, GLenum usage, std::string name, GLenum target)
		{
			QueuedVBOData* newVBO = new QueuedVBOData();

			newVBO->mData       = data;
			newVBO->mLength     = length;
			newVBO->mUsage      = usage;
			newVBO->mHashedName = Engine::StringHash::Hash(name);
			newVBO->mTarget     = target;

			mQueuedAdditions.push({BufferType::VBO, newVBO });
		}

		// ----------------------------------------------

		void BufferStore::CreateSSBO(void* data, unsigned int length, GLenum usage, std::string name)
		{
			QueuedSSBOData* newSSBO = new QueuedSSBOData();

			newSSBO->mData       = data;
			newSSBO->mLength     = length;
			newSSBO->mUsage      = usage;
			newSSBO->mHashedName = Engine::StringHash::Hash(name);

			mQueuedAdditions.push({ BufferType::SSBO, newSSBO });
		}

		// ----------------------------------------------

		void BufferStore::CreateEBO(float* data, unsigned int length, GLenum usage, std::string name)
		{
			QueuedEBOData* newEBO = new QueuedEBOData();

			newEBO->mData   = data;
			newEBO->mLength = length;
			newEBO->mUsage  = usage;
			newEBO->mHashedName = Engine::StringHash::Hash(name);

			mQueuedAdditions.push({BufferType::EBO, newEBO });
		}

		// ----------------------------------------------

		void BufferStore::CreateVAO(std::string name, std::vector<AttributePointerData> attributeSettingData, std::string EBOBufferToBind)
		{
			QueuedVAOData* newVAO      = new QueuedVAOData();			
			newVAO->mHashedName        = Engine::StringHash::Hash(name);
			newVAO->mAttributePointers = attributeSettingData;
			newVAO->mEBOBufferName     = EBOBufferToBind;

			mQueuedAdditions.push({BufferType::VAO, newVAO });
		}

		// ----------------------------------------------

		void BufferStore::CreateFBO(std::string name)
		{
			QueuedFBOData* newFBO = new QueuedFBOData();
			newFBO->mFBOName      = name;
			newFBO->mHashedName   = Engine::StringHash::Hash(name);

			mQueuedAdditions.push({ BufferType::FBO, newFBO });
		}

		// ----------------------------------------------

		void BufferStore::AddQueuedVBO()
		{
			QueuedVBOData* VBOData = (QueuedVBOData*)mQueuedAdditions.front().second;

			// Find if the name being added of this type already exists
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(BufferType::VBO);

			if (iter != mBuffers.end())
			{
				// See if a buffer by this name already exists
				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == VBOData->mHashedName)
					{
						// Clear up the memory used for the queued data
						delete VBOData;

						return;
					}
				}

				// Create the new buffer
				VertexBufferObject* newBuffer = new VertexBufferObject();
				newBuffer->SetTarget(VBOData->mTarget);
				newBuffer->SetBufferData(VBOData->mData, VBOData->mLength, VBOData->mUsage);

				// Add to the existing list
				iter->second.push_back({ VBOData->mHashedName, newBuffer });
			}
			else
			{
				// Create the new list
				std::vector<std::pair<unsigned int, Buffer*>> newList;

				// Create the new buffer
				VertexBufferObject* newBuffer = new VertexBufferObject();
				newBuffer->SetTarget(VBOData->mTarget);
				newBuffer->SetBufferData(VBOData->mData, VBOData->mLength, VBOData->mUsage);

				// Add the new element to the list
				newList.push_back({ VBOData->mHashedName, newBuffer });

				mBuffers.insert({ BufferType::VBO, newList });
			}

			// Clear up the memory used for the queued data
			delete VBOData;
		}

		// ----------------------------------------------

		void BufferStore::AddQueuedSSBO()
		{
			QueuedSSBOData* SSBOData = (QueuedSSBOData*)mQueuedAdditions.front().second;

			// Find if the name being added of this type already exists
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(BufferType::SSBO);

			if (iter != mBuffers.end())
			{
				// See if a buffer by this name already exists
				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == SSBOData->mHashedName)
					{
						// Clear up the memory used for the queued data
						delete SSBOData;

						return;
					}
				}

				// Create the new buffer
				ShaderStorageBufferObject* newBuffer = new ShaderStorageBufferObject();
				newBuffer->SetBufferData(SSBOData->mData, SSBOData->mLength, SSBOData->mUsage);

				// Add to the existing list
				iter->second.push_back({ SSBOData->mHashedName, newBuffer });
			}
			else
			{
				// Create the new list
				std::vector<std::pair<unsigned int, Buffer*>> newList;

				// Create the new buffer
				ShaderStorageBufferObject* newBuffer = new ShaderStorageBufferObject();
				newBuffer->SetBufferData(SSBOData->mData, SSBOData->mLength, SSBOData->mUsage);

				// Add the new element to the list
				newList.push_back({ SSBOData->mHashedName, newBuffer });

				mBuffers.insert({ BufferType::SSBO, newList });
			}

			// Clear up the memory used for the queued data
			delete SSBOData;
		}

		// ----------------------------------------------

		void BufferStore::AddQueuedEBO()
		{
			QueuedEBOData* EBOData = (QueuedEBOData*)mQueuedAdditions.front().second;

			// Find if the name being added of this type already exists
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(BufferType::EBO);

			if (iter != mBuffers.end())
			{
				// See if a buffer by this name already exists
				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == EBOData->mHashedName)
					{
						// Clear up the memory used for the queued data
						delete EBOData;

						return;
					}
				}

				// Create the new buffer
				ElementBufferObjects* newBuffer = new ElementBufferObjects();
				newBuffer->SetBufferData(EBOData->mLength, EBOData->mData, EBOData->mUsage);

				// Add to the existing list
				iter->second.push_back({ EBOData->mHashedName, newBuffer });
			}
			else
			{
				// Create the new list
				std::vector<std::pair<unsigned int, Buffer*>> newList;

				// Create the new buffer
				ElementBufferObjects* newBuffer = new ElementBufferObjects();
				newBuffer->SetBufferData(EBOData->mLength, EBOData->mData, EBOData->mUsage);

				// Add the new element to the list
				newList.push_back({ EBOData->mHashedName, newBuffer });

				mBuffers.insert({ BufferType::EBO, newList });
			}

			// Clear up the memory used for the queued data
			delete EBOData;
		}

		// ----------------------------------------------

		void BufferStore::AddQueuedVAO()
		{
			QueuedVAOData* VAOData = (QueuedVAOData*)mQueuedAdditions.front().second;

			// Find if the name being added of this type already exists
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(BufferType::VAO);

			// See the list exists
			if (iter != mBuffers.end())
			{
				// See if a buffer by this name already exists
				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == VAOData->mHashedName)
					{
						// Clear up the memory used for the queued data
						delete VAOData;

						return;
					}
				}

				// Create the new buffer
				VertexArrayObject* newBuffer = new VertexArrayObject();

				HandleSettingVAOParams(newBuffer, VAOData);

				// Add to the existing list
				iter->second.push_back({ VAOData->mHashedName, newBuffer });
			}
			else
			{
				// Create the new list
				std::vector<std::pair<unsigned int, Buffer*>> newList;

				// Create the new buffer
				VertexArrayObject* newBuffer = new VertexArrayObject();

				// Go through and setup all vertex pointers
				HandleSettingVAOParams(newBuffer, VAOData);

				// Add the new element to the list
				newList.push_back({ VAOData->mHashedName, newBuffer });

				mBuffers.insert({ BufferType::VAO, newList });
			}

			// Clear up the memory used for the queued data
			delete VAOData;
		}

		// ----------------------------------------------

		void BufferStore::AddQueuedFBO()
		{
			QueuedFBOData* FBOData = (QueuedFBOData*)mQueuedAdditions.front().second;

			// Find if the name being added of this type already exists
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(BufferType::FBO);

			// See the list exists
			if (iter != mBuffers.end())
			{
				// See if a buffer by this name already exists
				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == FBOData->mHashedName)
					{
						// Clear up the memory used for the queued data
						delete FBOData;

						return;
					}
				}

				// Create the new buffer
				Framebuffer* newBuffer = new Framebuffer();

				// Add to the existing list
				iter->second.push_back({ FBOData->mHashedName, newBuffer });
			}
			else
			{
				// Create the new list
				std::vector<std::pair<unsigned int, Buffer*>> newList;

				// Create the new buffer
				Framebuffer* newBuffer = new Framebuffer();

				// Add the new element to the list
				newList.push_back({ FBOData->mHashedName, newBuffer });

				mBuffers.insert({ BufferType::FBO, newList });
			}

			// Clear up the memory used for the queued data
			delete FBOData;
		}

		// ----------------------------------------------

		void BufferStore::HandleSettingVAOParams(VertexArrayObject* newBuffer, QueuedVAOData* VAOData)
		{
			newBuffer->Bind();
			Buffers::VertexBufferObject* buffer = nullptr;

			// Go through and setup all vertex pointers
			unsigned int attributeSize = (unsigned int)VAOData->mAttributePointers.size();
			for (unsigned int i = 0; i < attributeSize; i++)
			{
				AttributePointerData& currentAttribute = VAOData->mAttributePointers[i];

				buffer = GetVBO(currentAttribute.mVBONameToBind);

				if (!buffer)
				{
					ASSERTFAIL("Buffer to be bound before setting up VAO has not been created yet - may be an ordering problem.");
					return;
				}

				// Bind the buffer that the vertex attribute array will be pointing into
				buffer->Bind();

				// Enable the vertex array
				newBuffer->EnableVertexAttribArray(currentAttribute.mIndex);			

				// Now set the attibutes of this vertex pointer
				newBuffer->SetVertexAttributePointers(  currentAttribute.mIndex, 
														currentAttribute.mFloatsInSingleData,
														currentAttribute.mDataType,
														currentAttribute.mNormalised, 
														currentAttribute.mStride, 
														currentAttribute.mOffset,
														currentAttribute.mPerVertex);
			}

			if (VAOData->mEBOBufferName != "")
			{
				// Now bind the buffer passed in into the EBO slot of this VAO
				Buffers::VertexBufferObject* EBOBuffer = nullptr;
				EBOBuffer = GetVBO(VAOData->mEBOBufferName);

				if (EBOBuffer)
				{
					EBOBuffer->SetTarget(GL_ELEMENT_ARRAY_BUFFER);
					EBOBuffer->Bind();
				}
			}

			// Unbind the VAO as we are now done with it
			newBuffer->Unbind();
		}

		// ----------------------------------------------

		void BufferStore::Update()
		{
			// Go through each addition
			while(!mQueuedAdditions.empty())
			{
				// See what type we are adding to the map
				switch (mQueuedAdditions.front().first)
				{
				case BufferType::VBO:  AddQueuedVBO();  break;
				case BufferType::EBO:  AddQueuedEBO();  break;
				case BufferType::VAO:  AddQueuedVAO();  break;
				case BufferType::SSBO: AddQueuedSSBO(); break;
				case BufferType::FBO:  AddQueuedFBO();  break;

				default: break; 
				}

				mQueuedAdditions.pop(); // The pop here prevents an infinite loop
			}
		}

		// ----------------------------------------------

		Framebuffer* BufferStore::GetFBO(std::string name)
		{
			// Hash the name
			unsigned int hashedName = Engine::StringHash::Hash(name);

			// Find if the name being added of this type already exists
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(BufferType::FBO);

			if (iter != mBuffers.end())
			{
				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == hashedName)
					{
						return (Framebuffer*)iter->second[i].second;
					}
				}
			}

			return nullptr;
		}

		// ----------------------------------------------

		VertexArrayObject* BufferStore::GetVAO(std::string name)
		{
			// Hash the name
			unsigned int hashedName = Engine::StringHash::Hash(name);

			// Find if the name being added of this type already exists
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(BufferType::VAO);

			if (iter != mBuffers.end())
			{
				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == hashedName)
					{
						return (VertexArrayObject*)iter->second[i].second;
					}
				}
			}

			return nullptr;
		}

		// ----------------------------------------------

		VertexBufferObject* BufferStore::GetVBO(std::string name)
		{
			// Hash the name
			unsigned int hashedName = Engine::StringHash::Hash(name);

			// Find if the name being added of this type already exists
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(BufferType::VBO);

			if (iter != mBuffers.end())
			{
				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == hashedName)
					{
						return (VertexBufferObject*)iter->second[i].second;
					}
				}
			}

			return nullptr;
		}

		// ----------------------------------------------

		ShaderStorageBufferObject* BufferStore::GetSSBO(std::string name)
		{
			// Hash the name
			unsigned int hashedName = Engine::StringHash::Hash(name);

			// Find if the name being added of this type already exists
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(BufferType::SSBO);

			if (iter != mBuffers.end())
			{
				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == hashedName)
					{
						return (ShaderStorageBufferObject*)iter->second[i].second;
					}
				}
			}

			return nullptr;
		}

		// ----------------------------------------------

		ElementBufferObjects* BufferStore::GetEBO(std::string name)
		{
			// Find if the name being added of this type already exists
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(BufferType::EBO);

			if (iter != mBuffers.end())
			{
				// Hash the name
				unsigned int hashedName = Engine::StringHash::Hash(name);

				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == hashedName)
					{
						return (ElementBufferObjects*)iter->second[i].second;
					}
				}
			}

			return nullptr;
		}

		// ----------------------------------------------

		void BufferStore::Clear()
		{
			// Remove any queued for adding
			unsigned int size = (unsigned int)mQueuedAdditions.size();

			for (unsigned int i = 0; i < size; i++)
			{
				delete mQueuedAdditions.front().second;
				mQueuedAdditions.front().second = nullptr;

				mQueuedAdditions.pop();
			}

			// Remove any that currently exist
			for (auto iter = mBuffers.begin(); iter != mBuffers.end(); ++iter)
			{
				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					delete iter->second[i].second;
					iter->second[i].second = nullptr;
				}
				iter->second.clear();
			}
			mBuffers.clear();
		}

		// ----------------------------------------------

		void BufferStore::RemoveBuffer(std::string name, BufferType type)
		{
			std::unordered_map<BufferType, std::vector<std::pair<unsigned int, Buffer*>>>::iterator iter = mBuffers.find(type);

			if (iter != mBuffers.end())
			{
				// A buffer of this type actually exists so find the one requested to be removed
				unsigned int hashedName = Engine::StringHash::Hash(name);

				unsigned int bufferCount = (unsigned int)iter->second.size();
				for (unsigned int i = 0; i < bufferCount; i++)
				{
					if (iter->second[i].first == hashedName)
					{
						// Delete the buffer
						delete iter->second[i].second;
						iter->second[i].second = nullptr;

						// Remove this index and return
						iter->second.erase(iter->second.begin() + i);

						return;
					}
				}
			}
		}

		// ----------------------------------------------

#ifdef _DEBUG_BUILD
		void BufferStore::ShowImGuiData()
		{
			ImGui::Begin("Buffer Data Store");

				

			ImGui::End();
		}
#endif

		// ----------------------------------------------
	}
}