#ifndef _SHADER_PROGRAM_H_
#define _SHADER_PROGRAM_H_

#include "Shader.h"

#include "Maths/Code/Vector.h"
#include "Maths/Code/AssertMsg.h"

#include "ShaderTypes.h"

namespace Rendering
{
	namespace ShaderPrograms
	{
		// -----------------------------------------------

		class ShaderProgram
		{
		public:
			// ----------------------------------------------------------

			ShaderProgram()
				: mAttachedCount(0)
				, mShaderProgramID(0)
				, mProgramType(ShaderProgramTypes::ProgramCount)
			{
				mShaderProgramID = glCreateProgram();
			}

			// ----------------------------------------------------------

			~ShaderProgram()
			{
				glDeleteProgram(mShaderProgramID);
			}

			// ----------------------------------------------------------

			// Attaching a shader to the program
			bool AttachShader(Shaders::Shader* shaderToAttach)
			{
				// 4 to allow 1 vertex, fragment, geometry and compute shaders
				if (mAttachedCount++ >= 4)
				{
					std::cout << "Too many shaders being attached to the program!" << std::endl;
					return false;
				}

				glAttachShader(mShaderProgramID, shaderToAttach->GetShaderID());

				int error = glGetError();
				ASSERTMSG(error != 0, "Failed to attach shader to program");

				return true;
			}

			// ----------------------------------------------------------

			// Function to link all attached shaders to the program
			bool LinkShadersToProgram()
			{
				glLinkProgram(mShaderProgramID);

				return LinkErrorChecking();
			}

			// ----------------------------------------------------------

			void UseProgram();

			// ==========================================================
			// ----------------------------------------------------------

			void SetBool(std::string name, bool value)
			{
				int uniformLocation = glGetUniformLocation(mShaderProgramID, name.c_str());

				if (uniformLocation != -1)
				{
					glUniform1i(uniformLocation, (int)value);

					ASSERTMSG(glGetError() != 0, "Error setting uniform data!");
				}
			}

			void SetInt(std::string name, int value)
			{
				int uniformLocation = glGetUniformLocation(mShaderProgramID, name.c_str());

				if (uniformLocation != -1)
				{
					glUniform1i(uniformLocation, value);

					ASSERTMSG(glGetError() != 0, "Error setting uniform data!");
				}
			}

			void SetUnsignedInt(std::string name, int value)
			{
				int uniformLocation = glGetUniformLocation(mShaderProgramID, name.c_str());

				if (uniformLocation != -1)
				{
					glUniform1ui(uniformLocation, value);

					ASSERTMSG(glGetError() != 0, "Error setting uniform data!");
				}
			}

			void SetFloat(std::string name, float value)
			{
				int uniformLocation = glGetUniformLocation(mShaderProgramID, name.c_str());

				if (uniformLocation != -1)
				{
					glUniform1f(uniformLocation, value);

					ASSERTMSG(glGetError() != 0, "Error setting uniform data!");
				}
			}

			void SetVec2(std::string name, Maths::Vector::Vector2D<float> value)
			{
				int uniformLocation = glGetUniformLocation(mShaderProgramID, name.c_str());

				if (uniformLocation != -1)
				{
					glUniform2f(uniformLocation, value.x, value.y);

					ASSERTMSG(glGetError() != 0, "Error setting uniform data!");
				}
			}

			void SetVec2(std::string name, float x, float y)
			{
				int uniformLocation = glGetUniformLocation(mShaderProgramID, name.c_str());

				if (uniformLocation != -1)
				{
					glUniform2f(uniformLocation, x, y);

					ASSERTMSG(glGetError() != 0, "Error setting uniform data!");
				}
			}

			void SetVec3(std::string name, Maths::Vector::Vector3D<float> value)
			{
				int uniformLocation = glGetUniformLocation(mShaderProgramID, name.c_str());

				if (uniformLocation != -1)
				{
					glUniform3f(uniformLocation, value.x, value.y, value.z);

					ASSERTMSG(glGetError() != 0, "Error setting uniform data!");
				}
			}

			void SetVec4(std::string name, Maths::Vector::Vector4D<float> value)
			{
				int uniformLocation = glGetUniformLocation(mShaderProgramID, name.c_str());

				if (uniformLocation != -1)
				{
					glUniform4f(uniformLocation, value.x, value.y, value.z, value.w);

					ASSERTMSG(glGetError() != 0, "Error setting uniform data!");
				}
			}

			void SetVec3(std::string name, float x, float y, float z)
			{
				int uniformLocation = glGetUniformLocation(mShaderProgramID, name.c_str());

				if (uniformLocation != -1)
				{
					glUniform3f(uniformLocation, x, y, z);

					ASSERTMSG(glGetError() != 0, "Error setting uniform data!");
				}
			}

			void SetMat4(std::string name, float* matrix)
			{
				int uniformLocation = glGetUniformLocation(mShaderProgramID, name.c_str());

				if (uniformLocation != -1)
				{
					glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, (const GLfloat*)matrix);

					int error = glGetError();
					ASSERTMSG(error != 0, "Error setting uniform data!");
				}
			}

			// ==========================================================
			// ----------------------------------------------------------

			unsigned int GetId() { return mShaderProgramID; }

			static unsigned int sThisShaderProgramCount;

			// ==========================================================
			// ----------------------------------------------------------

			void SetProgramType(ShaderProgramTypes type)
			{
				mProgramType = type;
			}


			ShaderProgramTypes GetType()
			{
				return mProgramType;
			}

		private:
			// ----------------------------------------------------------

			bool LinkErrorChecking()
			{
				int  success = 0;
				char infoLog[512];

				// Get the error state
				glGetProgramiv(mShaderProgramID, GL_LINK_STATUS, &success);

				if (!success)
				{
					glGetProgramInfoLog(mShaderProgramID, 512, NULL, infoLog);

					std::cout << "SHADER PROGRAM LINKING ERROR LOG: " << infoLog << std::endl;

					return false;
				}

				return true;
			}

			// ----------------------------------------------------------

			unsigned int mShaderProgramID;

			unsigned int mAttachedCount;

			ShaderProgramTypes mProgramType;
		};

		// -----------------------------------------------
	};
}

#endif