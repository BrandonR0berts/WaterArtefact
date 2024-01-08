#include "Artefact.h"

#include "Rendering/Code/Window.h"

#include "Input/Code/KeyboardInput.h"
#include "Input/Code/Input.h"

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
		Input::InputSingleton::Get()->UpdateAllDevices();

		std::vector<Input::InputDevice*> attachedKeyboards = Input::InputSingleton::Get()->GetAllKeyboardsAttached();

		if (!attachedKeyboards.empty() && mRenderWindow)
		{
			Input::KeyboardInput::KeyboardInputDevice* keyboard = (Input::KeyboardInput::KeyboardInputDevice*)attachedKeyboards[0];

			if (keyboard->QueryInput(Input::KeyboardInput::TILDE) == Input::ButtonInputState::PRESSED)
			{
				mRenderWindow->ToggleDebugOverlay();
			}
		}
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