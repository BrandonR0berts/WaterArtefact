#pragma once

#include "ShaderTypes.h"
#include "ShaderProgram.h"

#include <unordered_map>
#include <string>

namespace Rendering
{
	// ---------------------------------------------------------------------

	class ShaderStore
	{
	public:
		ShaderStore();
		~ShaderStore();

		// Loading
		void Init();

		// Getters
		bool GetVertexShader(  VertexShaderTypes   shaderType, Shaders::VertexShader*&   shader);
		bool GetFragmentShader(FragmentShaderTypes shaderType, Shaders::FragmentShader*& shader);
		bool GetGeometryShader(GeometryShaderTypes shaderType, Shaders::GeometryShader*& shader);
		bool GetComputeShader( ComputeShaderTypes  shaderType, Shaders::ComputeShader*&  shader);

		bool                           GetShaderProgram(ShaderProgramTypes programToGet, ShaderPrograms::ShaderProgram*&  program);
		ShaderPrograms::ShaderProgram* GetShaderProgram(ShaderProgramTypes programToGet);

		void Clear();

	private:
		// Make this class non-copyable
		const ShaderStore& operator=(const ShaderStore&) = delete;
		ShaderStore(const ShaderStore&) = delete;

		// ------------------------------------

		void LoadInShadersFromFile(const std::string& filePath, ShaderTypes shaderTypeBeingLoaded);
		void LoadInShaderPrograms(const std::string& filePath);

		// ------------------------------------

		// The unordered map shader stores
		std::unordered_map<VertexShaderTypes,   Shaders::VertexShader*>   mVertexShaders;
		std::unordered_map<FragmentShaderTypes, Shaders::FragmentShader*> mFragmentShaders;
		std::unordered_map<GeometryShaderTypes, Shaders::GeometryShader*> mGeometryShaders;
		std::unordered_map<ComputeShaderTypes,  Shaders::ComputeShader*>  mComputeShaders;

		std::unordered_map<ShaderProgramTypes, ShaderPrograms::ShaderProgram*>  mShaderPrograms;

		// ------------------------------------

		template <typename enumType, typename mapContentsType>
		inline void ClearAllInMap(std::unordered_map<enumType, mapContentsType*>& map)
		{
			for (auto iter = map.begin(); iter != map.end(); ++iter)
			{
				delete iter->second;
				iter->second = nullptr;
			}
			map.clear();
		}

		// ------------------------------------
	};

	// ---------------------------------------------------------------------
}