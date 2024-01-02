#include "ShaderStore.h"

#include "Engine/Code/BaseGame.h"
#include "FileHandling/Code/JSONFile.h"
#include "Rendering/Code/Shaders/Shader.h"
#include "Rendering/Code/Shaders/ShaderTypes.h"

#include <iostream>

namespace Rendering
{
	// ------------------------------------------------------

	ShaderStore::ShaderStore()
	{
		
	}

	// ------------------------------------------------------

	ShaderStore::~ShaderStore()
	{
		Clear();
	}

	// ------------------------------------------------------

	void ShaderStore::LoadInShadersFromFile(const std::string& filePath, ShaderTypes shaderTypeBeingLoaded)
	{
		// -----------------------------

		// Load in the shaders from file
		JSONFile file(filePath);

		JSONObject& root = file.GetRootNode();

		std::vector<std::any> shaders;
		root.GetList("Shaders", shaders);

		std::string path;
		std::string type;

		// -----------------------------

		unsigned int shaderCount = (unsigned int)shaders.size();
		for (unsigned int i = 0; i < shaderCount; i++)
		{
			JSONObject& currentObject = std::any_cast<JSONObject&>(shaders[i]);

			currentObject.GetVariableValue("Path", path);
			currentObject.GetVariableValue("Type", type);

			if (path.empty() || type.empty())
				continue;

			Engine::BaseGame::DebugLog("Loading shader: " + path + "\n");

			unsigned int typeID = std::stoul(type);

			switch (shaderTypeBeingLoaded)
			{
			case ShaderTypes::VertexShader:
			{
				Rendering::VertexShaderTypes shaderType = (Rendering::VertexShaderTypes)typeID;

				// Check to see if the shader already exists
				std::unordered_map<Rendering::VertexShaderTypes, Shaders::VertexShader*>::iterator iter = mVertexShaders.find(shaderType);

				if (iter != mVertexShaders.end())
					continue;

				// Create the shader as it doesnt already exist
				Shaders::VertexShader* newShader = new Shaders::VertexShader(path);

				mVertexShaders.insert({ shaderType, newShader });
			}
			break;

			case ShaderTypes::FragmentShader:
			{
				Rendering::FragmentShaderTypes shaderType = (Rendering::FragmentShaderTypes)typeID;

				// Check to see if the shader already exists
				std::unordered_map<Rendering::FragmentShaderTypes, Shaders::FragmentShader*>::iterator iter = mFragmentShaders.find(shaderType);

				if (iter != mFragmentShaders.end())
					continue;

				// Create the shader as it doesnt already exist
				Shaders::FragmentShader* newShader = new Shaders::FragmentShader(path);

				mFragmentShaders.insert({ shaderType, newShader });
			}
			break;

			case ShaderTypes::GeometryShader:
			{
				Rendering::GeometryShaderTypes shaderType = (Rendering::GeometryShaderTypes)typeID;

				// Check to see if the shader already exists
				std::unordered_map<Rendering::GeometryShaderTypes, Shaders::GeometryShader*>::iterator iter = mGeometryShaders.find(shaderType);

				if (iter != mGeometryShaders.end())
					continue;

				// Create the shader as it doesnt already exist
				Shaders::GeometryShader* newShader = new Shaders::GeometryShader(path);

				mGeometryShaders.insert({ shaderType, newShader });
			}
			break;

			case ShaderTypes::ComputeShader:
			{
				Rendering::ComputeShaderTypes shaderType = (Rendering::ComputeShaderTypes)typeID;

				// Check to see if the shader already exists
				std::unordered_map<Rendering::ComputeShaderTypes, Shaders::ComputeShader*>::iterator iter = mComputeShaders.find(shaderType);

				if (iter != mComputeShaders.end())
					continue;

				// Create the shader as it doesnt already exist
				Shaders::ComputeShader* newShader = new Shaders::ComputeShader(path);

				mComputeShaders.insert({ shaderType, newShader });
			}
			break;

			default:
			break;
			}
		}
	}

	// ------------------------------------------------------

	bool ShaderStore::GetVertexShader(VertexShaderTypes   shaderType, Shaders::VertexShader*& shader)
	{
		std::unordered_map<VertexShaderTypes, Shaders::VertexShader*>::iterator iter = mVertexShaders.find(shaderType);

		if (iter != mVertexShaders.end())
		{
			shader = iter->second;
			return true;
		}

		shader = nullptr;
		return false;
	}

	// ------------------------------------------------------

	bool ShaderStore::GetFragmentShader(FragmentShaderTypes shaderType, Shaders::FragmentShader*& shader)
	{
		std::unordered_map<FragmentShaderTypes, Shaders::FragmentShader*>::iterator iter = mFragmentShaders.find(shaderType);

		if (iter != mFragmentShaders.end())
		{
			shader = iter->second;
			return true;
		}

		shader = nullptr;
		return false;
	}

	// ------------------------------------------------------

	bool ShaderStore::GetGeometryShader(GeometryShaderTypes shaderType, Shaders::GeometryShader*& shader)
	{
		std::unordered_map<GeometryShaderTypes, Shaders::GeometryShader*>::iterator iter = mGeometryShaders.find(shaderType);

		if (iter != mGeometryShaders.end())
		{
			shader = iter->second;
			return true;
		}

		shader = nullptr;
		return false;
	}

	// ------------------------------------------------------

	bool ShaderStore::GetComputeShader(ComputeShaderTypes  shaderType, Shaders::ComputeShader*& shader)
	{
		std::unordered_map<ComputeShaderTypes, Shaders::ComputeShader*>::iterator iter = mComputeShaders.find(shaderType);

		if (iter != mComputeShaders.end())
		{
			shader = iter->second;
			return true;
		}

		shader = nullptr;
		return false;
	}

	// ------------------------------------------------------

	bool ShaderStore::GetShaderProgram(ShaderProgramTypes programToGet, ShaderPrograms::ShaderProgram*& program)
	{
		std::unordered_map<ShaderProgramTypes, ShaderPrograms::ShaderProgram*>::iterator iter = mShaderPrograms.find(programToGet);

		if (iter != mShaderPrograms.end())
		{
			program = iter->second;
			return true;
		}

		program = nullptr;
		return false;
	}

	ShaderPrograms::ShaderProgram* ShaderStore::GetShaderProgram(ShaderProgramTypes programToGet)
	{
		std::unordered_map<ShaderProgramTypes, ShaderPrograms::ShaderProgram*>::iterator iter = mShaderPrograms.find(programToGet);

		if (iter != mShaderPrograms.end())
		{
			return iter->second;
		}

		return nullptr;
	}

	// ------------------------------------------------------

	void ShaderStore::Clear()
	{
		ClearAllInMap<VertexShaderTypes,   Shaders::VertexShader>(mVertexShaders);
		ClearAllInMap<FragmentShaderTypes, Shaders::FragmentShader>(mFragmentShaders);
		ClearAllInMap<GeometryShaderTypes, Shaders::GeometryShader>(mGeometryShaders);
		ClearAllInMap<ComputeShaderTypes,  Shaders::ComputeShader>(mComputeShaders);

		ClearAllInMap<ShaderProgramTypes, ShaderPrograms::ShaderProgram>(mShaderPrograms);
	}

	// ------------------------------------------------------

	void ShaderStore::Init()
	{
		// Load in the shaders from file for future use
		LoadInShadersFromFile("Shaders/Vertex/VertexShaders.json", ShaderTypes::VertexShader);
		LoadInShadersFromFile("Shaders/Fragment/FragmentShaders.json", ShaderTypes::FragmentShader);
		LoadInShadersFromFile("Shaders/Geometry/GeometryShaders.json", ShaderTypes::GeometryShader);
		LoadInShadersFromFile("Shaders/Compute/ComputeShaders.json", ShaderTypes::ComputeShader);

		LoadInShaderPrograms("Shaders/Programs/ShaderPrograms.json");
	}

	// ------------------------------------------------------

	void ShaderStore::LoadInShaderPrograms(const std::string& filePath)
	{
		Engine::BaseGame::DebugLog("Loading in shader programs.");

		// Load in the shaders from file
		JSONFile file(filePath);

		JSONObject& root = file.GetRootNode();

		std::vector<std::any> programs;
		root.GetList("Programs", programs);

		std::string vertexShaderString;
		std::string fragmentShaderString;
		std::string computeShaderString;
		std::string geometryShaderString;

		std::string programID;

		Shaders::VertexShader*   vertexShader;
		Shaders::FragmentShader* fragmentShader;
		Shaders::ComputeShader*  computeShader;
		Shaders::GeometryShader* geometryShader;

		unsigned int programCount = (unsigned int)programs.size();
		for (unsigned int i = 0; i < programCount; i++)
		{
			bool hasShader = false;

			// Reset the shaders being pointed to
			vertexShader   = nullptr;
			fragmentShader = nullptr;
			computeShader  = nullptr;
			geometryShader = nullptr;

			JSONObject& currentObject = std::any_cast<JSONObject&>(programs[i]);

			// Create the program
			ShaderPrograms::ShaderProgram* newProgram = new ShaderPrograms::ShaderProgram();

			if (currentObject.GetVariableValue("Vertex", vertexShaderString))
			{
				VertexShaderTypes vertexShaderID = (VertexShaderTypes)std::stoul(vertexShaderString);

				if (GetVertexShader(vertexShaderID, vertexShader))
				{
					newProgram->AttachShader(vertexShader);

					hasShader = true;
				}
			}
			
			if (currentObject.GetVariableValue("Fragment", fragmentShaderString))
			{
				FragmentShaderTypes fragmentShaderID = (FragmentShaderTypes)std::stoul(fragmentShaderString);

				if (GetFragmentShader(fragmentShaderID, fragmentShader))
				{
					newProgram->AttachShader(fragmentShader);

					hasShader = true;
				}
			}

			if (currentObject.GetVariableValue("Compute", computeShaderString))
			{
				ComputeShaderTypes computeShaderID = (ComputeShaderTypes)std::stoul(computeShaderString);

				if (GetComputeShader(computeShaderID, computeShader))
				{
					newProgram->AttachShader(computeShader);

					hasShader = true;
				}
			}

			if (currentObject.GetVariableValue("Geometry", geometryShaderString))
			{
				GeometryShaderTypes geometryShaderID = (GeometryShaderTypes)std::stoul(geometryShaderString);

				if (GetGeometryShader(geometryShaderID, geometryShader))
				{
					newProgram->AttachShader(geometryShader);

					hasShader = true;
				}
			}

			if (!hasShader)
			{
				delete newProgram;
				newProgram = nullptr;

				continue;
			}

			currentObject.GetVariableValue("ID", programID);
			unsigned int ID = std::stoul(programID);

			Rendering::ShaderProgramTypes  programType = (Rendering::ShaderProgramTypes)ID;

			newProgram->LinkShadersToProgram();
			newProgram->SetProgramType(programType);

			mShaderPrograms.insert({ programType, newProgram });
		}
	}

	// ------------------------------------------------------
}