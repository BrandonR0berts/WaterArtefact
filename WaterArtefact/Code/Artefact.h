#pragma once

#include "Rendering/Code/Water.h"

namespace Rendering
{
	class Window;
}

namespace Artefact
{
	// --------------------------------------------

	class ArtefactProgram
	{
	public:
		ArtefactProgram();
		~ArtefactProgram();

		void Update(const float deltaTime);

		bool GetHasQuit();

	private:
		Rendering::Window* mRenderWindow;
	};

	// --------------------------------------------
}