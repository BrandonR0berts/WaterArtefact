#pragma once

#include "Textures/Texture.h"
#include "Maths/Code/Vector.h"
#include "Rendering/Code/Buffers.h"

#include <string>

namespace Rendering
{
	// --------------------------------------

	class Camera;

	// --------------------------------------

	class Skybox final
	{
	public:
		Skybox(std::string name, std::string filePaths[6]);
		Skybox(std::string name, std::string filePaths1, std::string filePaths2, std::string filePaths3, std::string filePaths4, std::string filePaths5, std::string filePaths6);
		~Skybox();

		Texture::CubeMapTexture* GetCubeMapTexture()     const { return mCubeMapTexture;    }
		Texture::CubeMapTexture* GetConvolutedTexture();

		void                     ConvoluteTexture();

		// Make sure to call this when everything else has already been drawn, 
		// Otherwise it would ruin the draw rate
		void Render(Camera* camera, Texture::CubeMapTexture* textureToReplaceSkybox = nullptr);

		void Render(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, Texture::CubeMapTexture* textureToReplaceSkybox = nullptr);

		std::string* GetFilePaths()                  { return mFilePaths; }
		std::string  GetName()                       { return mName; }

		void         ToggleDisplayingIrradianceMap() { mShowingIrradianceMap = !mShowingIrradianceMap; }

	private:

		void SetupShaders();

		static Maths::Vector::Vector3D<float> mCubeData[36];

		void LoadCubeMapTextures(std::string filePaths[6]);
		void SetupBufferData();

		// Array of 6 textures, one for each side
		Texture::CubeMapTexture* mCubeMapTexture;
		Texture::CubeMapTexture* mConvolutedVersion;

		std::string              mFilePaths[6];
		std::string              mName;
		std::string              mInternalName;

		Buffers::VertexArrayObject* mCubeVAO;
		ShaderPrograms::ShaderProgram* mSkyBoxProgram;

		Buffers::VertexBufferObject* mCubeVBO;

		bool mShowingIrradianceMap;
	};

	// --------------------------------------
}