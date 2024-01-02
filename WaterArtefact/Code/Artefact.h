#pragma once

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

	private:
		Rendering::Window* mRenderWindow;
	};

	// --------------------------------------------
}