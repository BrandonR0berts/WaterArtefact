#pragma once

#include "Engine/Code/StringHash.h"

#include <unordered_map>
#include <mutex>

namespace Engine
{
	class Entity;
}

namespace Rendering
{
	// --------------------------------------------------------

	enum class ResourceType : unsigned int
	{
		VideoFrames,
		Texture2D,
		Texture3D,
		GLTFModel,
		CubeMap,

		Count
	};

	// --------------------------------------------------------

	class Resource abstract
	{
	public:
		Resource()
			: mActive(false)
			, mResourceType(ResourceType::Count)
			, mEntityAssociated(nullptr)
			, mHashedName(0)

#ifdef _DEBUG_BUILD
			, mName("")
#endif
		{}

		Resource(ResourceType type, bool active, Engine::Entity* entity, const std::string& name)
			: mResourceType(type)
			, mActive(active)
			, mEntityAssociated(entity)
			, mHashedName(Engine::StringHash::Hash(name))
		{  }

		virtual ~Resource() { mEntityAssociated  = nullptr; }

		void            SetActive(bool state)               { mActive = state; }
		void            SetResourceType(ResourceType type)  { mResourceType = type; }
		void            SetHashedName(unsigned int newName) { mHashedName = newName; }

#ifdef _DEBUG_BUILD
		void            SetName(std::string name)           { mName = name; }
		std::string     GetName()                     const { return mName; }
#endif

		bool            GetActive()       const { return mActive; }
		ResourceType    GetResourceType() const { return mResourceType; }
		Engine::Entity* GetEntity()       const { return mEntityAssociated; }
		unsigned int    GetHashedName()   const { return mHashedName; }

	protected:
		bool            mActive;
		ResourceType    mResourceType;
		Engine::Entity* mEntityAssociated;
		unsigned int    mHashedName;

#ifdef _DEBUG_BUILD
		std::string     mName;
#endif
	};

	// --------------------------------------------------------	

	struct QueueDataCollection
	{
		ResourceType mType;
		std::string  mName;
		std::vector<std::string> mParams;
	};

	class ResourceCollection final
	{
	public:
		ResourceCollection();
		~ResourceCollection();

		// This is a waiting function - do not call from within the graphics thread or the program will likely hang
		void AddResource(ResourceType type, std::string& name, std::vector<std::string> params);

		// This one is NOT a hanging function
		void      AddResource(ResourceType type, std::string& name, Resource* resource);

		bool      GetResourceList(ResourceType type, std::vector<std::pair<Resource*, unsigned int>>& out);
		Resource* GetResource(ResourceType type, const std::string& resourceName, bool toAdd = false);

		void      Clear();
		void      Clear(ResourceType type);

		void      RemoveResource(ResourceType type, std::string& name);
		void      RemoveResource(ResourceType type, Resource* pointer);

		void      CreateQueuedResources();

#ifdef _DEBUG_BUILD
		void RenderTextureImGuiOverlay();
		void RenderVideosImGuiOverlay();
		void RenderModelsImGuiOverlay();
#endif

	private:
		void QueueAddingResource(QueueDataCollection data);
		void CreateQueuedResource(ResourceType type, std::string& name, std::vector<std::string>& params);
		
		std::vector<QueueDataCollection>                                                  mQueuedAdditions;
		std::unordered_map<ResourceType, std::vector<std::pair<Resource*, unsigned int>>> mCollection;

		std::mutex                                                                        mAddtionMutex;
	};

	// --------------------------------------------------------	
}