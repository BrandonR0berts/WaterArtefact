#include "Artefact.h"

#include "Rendering/Code/Window.h"

namespace Artefact
{
	// -----------------------------------------------------

	ArtefactProgram::ArtefactProgram()
		: mRenderWindow(nullptr)
	{
		mRenderWindow = new Rendering::Window();
	}

	// -----------------------------------------------------

	ArtefactProgram::~ArtefactProgram()
	{
		delete mRenderWindow;
		mRenderWindow = nullptr;
	}

	// -----------------------------------------------------

	void ArtefactProgram::Update(const float deltaTime)
	{

	}

	// -----------------------------------------------------

	bool ArtefactProgram::GetHasQuit()
	{
		if (!mRenderWindow)
			return false;

		return mRenderWindow->GetHasQuitWindow();
	}

	// -----------------------------------------------------
}