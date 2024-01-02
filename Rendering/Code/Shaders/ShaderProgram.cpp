#include "ShaderProgram.h"

#include "Rendering/Code/OpenGLRenderPipeline.h"
#include "Rendering/Code/Window.h"

namespace Rendering
{
	namespace ShaderPrograms
	{
		unsigned int ShaderPrograms::ShaderProgram::sThisShaderProgramCount = 0;

		void ShaderProgram::UseProgram()
		{
			OpenGLRenderPipeline* renderPipeline = (OpenGLRenderPipeline*)Window::GetRenderPipeline();

			if (!renderPipeline)
				return;

			renderPipeline->SetActiveShader(mShaderProgramID);
		}
	}
}