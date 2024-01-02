#include "RenderingResources.h"

#include "Rendering/Code/Textures/Texture.h"
#include "Rendering/Code/GLTFModel.h"
#include "ProjectCube/Code/Video/Video.h"

#include "Rendering/Code/Window.h"

#include "Engine/Code/StringHash.h"

#ifdef _DEBUG_BUILD
	#include "Include/imgui/imgui.h"
	#include "Include/imgui/imgui_impl_glfw.h"
	#include "Include/imgui/imgui_impl_opengl3.h"
#endif

namespace Rendering
{
	// --------------------------------------------------------	

	ResourceCollection::ResourceCollection()
	{

	}

	// --------------------------------------------------------	

	ResourceCollection::~ResourceCollection()
	{

	}

	// --------------------------------------------------------	

	// This is a waiting function - do not call from within the graphics thread or the program will likely hang
	void ResourceCollection::AddResource(ResourceType type, std::string& name, std::vector<std::string> params)
	{
		// Add the resource to the queue for adding
		QueueAddingResource({ type, name, params });
	}

	// --------------------------------------------------------	

	void ResourceCollection::AddResource(ResourceType type, std::string& name, Resource* resource)
	{
		std::unordered_map<ResourceType, std::vector<std::pair<Resource*, unsigned int>>>::iterator iter = mCollection.find(type);

		// Set the identifying name
		unsigned int hashedName = Engine::StringHash::Hash(name);
		resource->SetHashedName(hashedName);

#ifdef _DEBUG_BUILD
		resource->SetName(name);
#endif

		if (iter != mCollection.end())
		{
			// See if the resource already exists
			unsigned int resourceSize = (unsigned int)iter->second.size();
			for (unsigned int i = 0; i < resourceSize; i++)
			{
				if (iter->second[i].first == resource)
				{
					iter->second[i].second++;
					return;
				}
			}

			// Not found in the list so create a new element
			iter->second.push_back({resource, 1});
		}
		else
		{
			// if the collection does not already exist then create a new one for the type
			mCollection.insert({ type, std::vector<std::pair<Resource*, unsigned int>>{ { resource, 1 }}});
		}
	}

	// --------------------------------------------------------	

	void ResourceCollection::QueueAddingResource(QueueDataCollection data)
	{
		mAddtionMutex.lock();
			mQueuedAdditions.push_back(data);
		mAddtionMutex.unlock();
	}

	// --------------------------------------------------------	

	void ResourceCollection::CreateQueuedResource(ResourceType type, std::string& name, std::vector<std::string>& params)
	{
		// If the resource already exists then leave the function as we dont need to create it
		if (GetResource(type, name, true))
		{
			return;
		}

		std::unordered_map<ResourceType, std::vector<std::pair<Resource*, unsigned int>>>::iterator iter = mCollection.find(type);

		Resource*    newObject  = nullptr;
		unsigned int hashedName = Engine::StringHash::Hash(name);

		switch (type)
		{
		case ResourceType::Texture2D:
			newObject = new Texture::Texture2D();

			((Texture::Texture2D*)newObject)->LoadTextureFromFile(params[0]);
		break;

		case ResourceType::Texture3D:
			newObject = new Texture::Texture3D();
		break;

		case ResourceType::GLTFModel:		
			newObject = new GLTFModel(params[0]);
		break;

		case ResourceType::VideoFrames:
		{
			newObject = new VideoFrames(params[0], std::stof(params[1]));
		}
		break;

		default:
		return;
		}

		// Set the identifying name
#ifdef _DEBUG_BUILD
		newObject->SetName(name);
#endif

		newObject->SetHashedName(hashedName);

		if (iter != mCollection.end())
		{
			iter->second.push_back({ newObject, 1});
		}
		else
		{
			// if the collection does not already exist then create a new one for the type
			mCollection.insert({ type, std::vector<std::pair<Resource*, unsigned int>>{ {newObject, 1}} });
		}
	}

	// --------------------------------------------------------	

	bool ResourceCollection::GetResourceList(ResourceType type, std::vector<std::pair<Resource*, unsigned int>>& out)
	{
		std::unordered_map<ResourceType, std::vector<std::pair<Resource*, unsigned int>>>::iterator iter = mCollection.find(type);

		if (iter != mCollection.end())
		{
			out = iter->second;
			return true;
		}

		return false;
	}

	// --------------------------------------------------------	

	void ResourceCollection::Clear()
	{
		for (std::unordered_map<ResourceType, std::vector<std::pair<Resource*, unsigned int>>>::iterator iter = mCollection.begin(); iter != mCollection.end(); iter++)
		{
			Clear(iter->first);
		}
		mCollection.clear();
	}

	// --------------------------------------------------------	

	void ResourceCollection::Clear(ResourceType type)
	{
		std::unordered_map<ResourceType, std::vector<std::pair<Resource*, unsigned int>>>::iterator iter = mCollection.find(type);

		unsigned int resourceCount = (unsigned int)iter->second.size();
		for (unsigned int i = 0; i < resourceCount; i++)
		{
			delete iter->second[i].first;
			iter->second[i].first = nullptr;
		}
		iter->second.clear();
	}

	// --------------------------------------------------------	

	void ResourceCollection::RemoveResource(ResourceType type, std::string& name)
	{
		std::unordered_map<ResourceType, std::vector<std::pair<Resource*, unsigned int>>>::iterator iter = mCollection.find(type);
		unsigned int hashedName = Engine::StringHash::Hash(name);

		unsigned int resourceCount = (unsigned int)iter->second.size();
		for (unsigned int i = 0; i < resourceCount; i++)
		{
			if (iter->second[i].first && iter->second[i].first->GetHashedName() == hashedName)
			{
				// Lower the count by one
				iter->second[i].second--;

				// If none left then delete it
				if (iter->second[i].second == 0)
				{
					delete iter->second[i].first;
					iter->second[i].first = nullptr;

					// Remove the index from the vector
					iter->second.erase(iter->second.begin() + i);
				}

				return;
			}
		}
	}

	// --------------------------------------------------------	

	void ResourceCollection::RemoveResource(ResourceType type, Resource* pointer)
	{
		if (pointer == nullptr)
			return;

		std::unordered_map<ResourceType, std::vector<std::pair<Resource*, unsigned int>>>::iterator iter = mCollection.find(type);

		if (iter != mCollection.end())
		{
			unsigned int resourceCount = (unsigned int)iter->second.size();
			for (unsigned int i = 0; i < resourceCount; i++)
			{
				// See if it is this resource
				if (iter->second[i].first == pointer)
				{
					// Remove one from the count
					iter->second[i].second--;

					// If none left
					if (iter->second[i].second == 0)
					{
						delete iter->second[i].first;
						iter->second[i].first = nullptr;

						// Remove the index from the vector
						iter->second.erase(iter->second.begin() + i);
					}

					return;
				}
			}
		}
	}

	// --------------------------------------------------------	

	void ResourceCollection::CreateQueuedResources()
	{
		mAddtionMutex.lock();

			unsigned int queueCount = (unsigned int)mQueuedAdditions.size();
			for (unsigned int i = 0; i < queueCount; i++)
			{
				CreateQueuedResource(mQueuedAdditions[i].mType, mQueuedAdditions[i].mName, mQueuedAdditions[i].mParams);
			}
			mQueuedAdditions.clear();

		mAddtionMutex.unlock();
	}

	// --------------------------------------------------------	

	Resource* ResourceCollection::GetResource(ResourceType type, const std::string& resourceName, bool toAdd)
	{
		std::unordered_map<ResourceType, std::vector<std::pair<Resource*, unsigned int>>>::iterator iter = mCollection.find(type);

		if (iter != mCollection.end())
		{
			// Now loop through the list to find the name stored
			unsigned int hashedName = Engine::StringHash::Hash(resourceName);

			unsigned int listSize = (unsigned int)iter->second.size();
			for (unsigned int i = 0; i < listSize; i++)
			{
				if (iter->second[i].first &&
					iter->second[i].first->GetHashedName() == hashedName)
				{
					if (toAdd)
					{
						iter->second[i].second++;
					}

					return iter->second[i].first;
				}
			}
		}

		return nullptr;
	}

	// --------------------------------------------------------	

#ifdef _DEBUG_BUILD

	// --------------------------------------------------------	

	void ResourceCollection::RenderTextureImGuiOverlay()
	{
		// Start the table
		ImGui::Begin("Texture Overlay Menu");

			// Get the list of texture2D's
			std::vector<std::pair<Resource*, unsigned int>> texture2Ds;
			if (GetResourceList(Rendering::ResourceType::Texture2D, texture2Ds))
			{
				// Start the table of data
				ImGuiTableFlags localFlags = ImGuiTableFlags_Resizable + ImGuiTableFlags_Borders;
				ImGui::BeginTable("Texture2D Table Data", 4, localFlags, ImVec2(0,0), 0);

				ImGui::TableSetupColumn("File Path");
				ImGui::TableSetupColumn("Stored Name");
				ImGui::TableSetupColumn("Size (MB)");
				ImGui::TableSetupColumn("Dimensions");
				ImGui::TableHeadersRow();

				unsigned int texture2DCount = (unsigned int)texture2Ds.size();
				for (unsigned int i = 0; i < texture2DCount; i++)
				{
					ImGui::TableNextColumn();

					Texture::Texture2D* textureCast = (Texture::Texture2D*)texture2Ds[i].first;
					ImGui::Text(textureCast->GetFilePath().c_str());
					ImGui::TableNextColumn();

					ImGui::Text(textureCast->GetName().c_str());
					ImGui::TableNextColumn();

					float bytesIntexture = textureCast->GetDataSizeMegaBytes();
					ImGui::Text(std::to_string(bytesIntexture).c_str());

					ImGui::TableNextColumn();

					unsigned int height = textureCast->GetTextureHeight();
					unsigned int width  = textureCast->GetTextureWidth();
					ImGui::Text(std::string(std::to_string(width) + ", " + std::to_string(height)).c_str());

				}

				ImGui::EndTable();
			}

		ImGui::End();
	}

	// --------------------------------------------------------	

	void ResourceCollection::RenderVideosImGuiOverlay()
	{
		// Start the collection
		ImGui::Begin("Videos Overlay Menu");

		// Get the list of videos
		std::vector<std::pair<Resource*, unsigned int>> videos;
		if (GetResourceList(Rendering::ResourceType::VideoFrames, videos))
		{
			// Start the table of data
			ImGuiTableFlags localFlags = ImGuiTableFlags_Resizable + ImGuiTableFlags_Borders;
			ImGui::BeginTable("Video Table Data", 4, localFlags, ImVec2(0, 0), 0);

			ImGui::TableSetupColumn("File Path");
			ImGui::TableSetupColumn("Stored Name");
			ImGui::TableSetupColumn("Total Run Time");
			ImGui::TableSetupColumn("Playing");

			ImGui::TableHeadersRow();

			unsigned int videosCount = (unsigned int)videos.size();
			for (unsigned int i = 0; i < videosCount; i++)
			{
				ImGui::TableNextColumn();

				VideoFrames* videoCast = (VideoFrames*)videos[i].first;
				ImGui::Text(videoCast->GetFilePath().c_str());
				ImGui::TableNextColumn();

				ImGui::Text(videoCast->GetName().c_str());
				ImGui::TableNextColumn();

				ImGui::Text(std::to_string(videoCast->GetTotalRuntime()).c_str());

				ImGui::TableNextColumn();

				bool playing;
				playing = !videoCast->GetPaused();

				if(playing)
					ImGui::Text("true");
				else
					ImGui::Text("false");

			}

			ImGui::EndTable();
		}

		ImGui::End();
	}

	// --------------------------------------------------------	

	void ResourceCollection::RenderModelsImGuiOverlay()
	{
		// Start the collection
		ImGui::Begin("Render Models Overlay");

		// Get the list of models
		std::vector<std::pair<Resource*, unsigned int>> models;
		if (GetResourceList(Rendering::ResourceType::GLTFModel, models))
		{
			// Start the table of data
			ImGuiTableFlags localFlags = ImGuiTableFlags_Resizable + ImGuiTableFlags_Borders;
			ImGui::BeginTable("Models Table Data", 4, localFlags, ImVec2(0, 0), 0);

			ImGui::TableSetupColumn("File Path");
			ImGui::TableSetupColumn("Stored Name");
			ImGui::TableSetupColumn("Triangle Count");
			ImGui::TableSetupColumn("Data Size (MB)");

			ImGui::TableHeadersRow();

			unsigned int modelsCount = (unsigned int)models.size();
			for (unsigned int i = 0; i < modelsCount; i++)
			{
				ImGui::TableNextColumn();

				GLTFModel* modelCast = (GLTFModel*)models[i].first;
				ImGui::Text(modelCast->GetFilePath().c_str());
				ImGui::TableNextColumn();

				ImGui::Text(modelCast->GetName().c_str());
				ImGui::TableNextColumn();

				ImGui::Text(std::to_string(modelCast->GetVertexCount()).c_str());
				ImGui::TableNextColumn();

				ImGui::Text(std::to_string(float(modelCast->GetDataSizeMB())).c_str());
				ImGui::TableNextColumn();
			}

			ImGui::EndTable();
		}

		ImGui::End();
	}

	// --------------------------------------------------------	
#endif
}