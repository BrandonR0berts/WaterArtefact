#ifndef _SHADER_H_
#define _SHADER_H_

// This file contains definitions for a generic Vertex, Fragment, Geomery and Compute shader implementation

// Required OpenGL headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include <fstream>

namespace Rendering
{
	namespace Shaders
	{
		// --------------------------------------------------------------

		class Shader abstract
		{
		public:
			Shader() : mShaderID(0) { ; }

			~Shader() { ; }

			// --------------------------------------------------

			unsigned int GetShaderID()
			{
				return mShaderID;
			}

			// --------------------------------------------------

			void DeleteShader()
			{
				glDeleteShader(mShaderID);
			}

			// ==================================================
			// --------------------------------------------------

			unsigned int CreateShader(const std::string& filePath)
			{
				// Create the shader
				mShaderID = GenerateShaderID();

				// Set the shader source file
				std::string shaderSource;
				LoadInShaderFromFile(filePath, shaderSource);

				const char* sourceCode = shaderSource.c_str();

				glShaderSource(mShaderID, 1, &sourceCode, NULL);

				// Compile the shader
				glCompileShader(mShaderID);

				CompilationErrorChecking();

				return mShaderID;
			}

		protected:
			// -----------------------------------------------------------

			virtual unsigned int GenerateShaderID() = 0;

			void LoadInShaderFromFile(const std::string& filePath, std::string& fileContents)
			{
				std::string   shaderCode = "";
				std::ifstream shaderFile;

				shaderFile.open(filePath);

				if (shaderFile.is_open())
				{
					std::stringstream shaderStream;

					shaderStream << shaderFile.rdbuf();

					shaderFile.close();

					fileContents = shaderStream.str();
				}
				else
				{
					std::cout << "Failed to open shader: " << filePath << std::endl;
				}
			}

			// -----------------------------------------------------------

			void CompilationErrorChecking()
			{
				int  success = 0;
				char infoLog[512];

				// Get the error state
				glGetShaderiv(mShaderID, GL_COMPILE_STATUS, &success);

				if (!success)
				{
					glGetShaderInfoLog(mShaderID, 512, NULL, infoLog);

					std::cout << "Error loading shader: " + std::string(infoLog);
				}
			}

			// -----------------------------------------------------------

			unsigned int mShaderID;
		};

		// --------------------------------------------------------------

		class VertexShader final : public Shader
		{
		public:
			// --------------------------------------

			VertexShader() = default;

			VertexShader(const std::string& filePath)
			{
				CreateShader(filePath);
			}

			// --------------------------------------

			~VertexShader() {}

			// --------------------------------------

		private:
			unsigned int GenerateShaderID() override
			{
				return glCreateShader(GL_VERTEX_SHADER);
			}

			// --------------------------------------
		};

		// --------------------------------------------------------------

		class FragmentShader final : public Shader
		{
		public:
			// ------------------------------------

			FragmentShader() { ; }

			FragmentShader(const std::string& filePath)
			{
				CreateShader(filePath);
			}

			// ------------------------------------

			~FragmentShader() { }

			// ------------------------------------

		private:

			unsigned int GenerateShaderID() override
			{
				return glCreateShader(GL_FRAGMENT_SHADER);
			}

			// --------------------------------------
		};

		// --------------------------------------------------------------

		class GeometryShader final : public Shader
		{
		public:
			GeometryShader() { ; }

			GeometryShader(const std::string& filePath)
			{
				CreateShader(filePath);
			}

			~GeometryShader()
			{

			}

		private:
			unsigned int GenerateShaderID() override
			{
				return glCreateShader(GL_GEOMETRY_SHADER);
			}
		};

		// --------------------------------------------------------------

		class ComputeShader final : public Shader
		{
		public:
			ComputeShader() { ; }

			ComputeShader(const std::string & filePath)
			{
				CreateShader(filePath);
			}

			~ComputeShader()
			{

			}

		private:
			unsigned int GenerateShaderID() override
			{
				return glCreateShader(GL_COMPUTE_SHADER);
			}
		};

		// --------------------------------------------------------------
	};
};

#endif