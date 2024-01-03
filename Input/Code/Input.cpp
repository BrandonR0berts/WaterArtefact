#include "input.h"

#include "KeyboardInput.h"
#include "MouseInput.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// --------------------------------------------------------------------

int ConvertDeviceTypeToGamepadID(Input::InputDeviceType type)
{
	if (type == Input::InputDeviceType::KEYBOARD   ||
		type == Input::InputDeviceType::MOUSE      || 
		type == Input::InputDeviceType::ENUM_COUNT)
	{
		return -1;
	}

	// Assumes that all gamepads are defined in the enum one after each other
	return (unsigned int(type)) - (unsigned int)Input::InputDeviceType::GAMEPAD_1;
}

// --------------------------------------------------------------------

namespace Input
{
	InputSingleton* InputSingleton::mThis = nullptr;

	// --------------------------------------------------------------------

	InputSingleton::InputSingleton()
	{
		KeyboardInput::KeyboardInputDevice* newKeyboard = new KeyboardInput::KeyboardInputDevice();
		MouseInput::MouseInputDevice*       newMouse    = new MouseInput::MouseInputDevice();

		AddDevice(newKeyboard, Input::InputDeviceType::KEYBOARD);
		AddDevice(newMouse,    Input::InputDeviceType::MOUSE);
	}

	// --------------------------------------------------------------------

	InputSingleton::~InputSingleton()
	{
		unsigned int count = (unsigned int)mAttachedDevices.size();
		for (unsigned int i = 0; i < count; i++)
		{
			delete mAttachedDevices[i];
			mAttachedDevices[i] = nullptr;
		}
		mAttachedDevices.clear();
	}

	// --------------------------------------------------------------------

	bool InputSingleton::GetInputDevice(InputDeviceType type, std::vector<InputDevice*>& devicesFoundOfType)
	{
		unsigned int count = (unsigned int)mAttachedDevices.size();
		for (unsigned int ID = 0; ID < count; ++ID)
		{
			InputDevice* currentDevice = mAttachedDevices[ID];
			if (currentDevice)
			{
				if (currentDevice->GetDeviceType() == type)
				{
					devicesFoundOfType.emplace_back(currentDevice);
				}
			}
		}

		return !devicesFoundOfType.empty();
	}

	// --------------------------------------------------------------------

	void InputSingleton::ClearAllInput()
	{
		unsigned int count = (unsigned int)mAttachedDevices.size();
		for (unsigned int ID = 0; ID < count; ++ID)
		{
			if (mAttachedDevices[ID])
			{
				mAttachedDevices[ID]->ClearAllInput();
			}
		}
	}

	// --------------------------------------------------------------------

	void InputSingleton::AddDevice(InputDevice* device, InputDeviceType type)
	{
		bool found = false;

		switch (type)
		{
		case InputDeviceType::KEYBOARD:
		{
			unsigned int count = (unsigned int)mAttachedDevices.size();
			for (unsigned int i = 0; i < count; i++)
			{
				if (mAttachedKeyboards[i] == device)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				mAttachedKeyboards.push_back(device);
			}
		}
		break;

		case InputDeviceType::MOUSE:
		{
			unsigned int count = (unsigned int)mAttachedMice.size();
			for (unsigned int i = 0; i < count; i++)
			{
				if (mAttachedMice[i] == device)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				mAttachedMice.push_back(device);
			}
		}
		break;

		default:
		{
			unsigned int count = (unsigned int)mAttachedGamepads.size();
			for (unsigned int i = 0; i < count; i++)
			{
				if (mAttachedGamepads[i] == device)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				mAttachedGamepads.push_back(device);
			}
		}
		break;

		case InputDeviceType::ENUM_COUNT: return;
		}


		// Add the device to the overall store
		mAttachedDevices.push_back(device);
	}

	// --------------------------------------------------------------------

	void InputSingleton::RemoveDevice(InputDevice* device, InputDeviceType type)
	{
		switch (type)
		{
		case InputDeviceType::KEYBOARD:
		{
			unsigned int count = (unsigned int)mAttachedKeyboards.size();
			for (unsigned int i = 0; i < count; i++)
			{
				if (mAttachedKeyboards[i] == device)
				{
					mAttachedKeyboards[i] = nullptr;
					mAttachedKeyboards.erase(mAttachedKeyboards.begin() + i);
					break;
				}
			}
		}
		break;

		case InputDeviceType::MOUSE:
		{
			unsigned int count = (unsigned int)mAttachedMice.size();
			for (unsigned int i = 0; i < count; i++)
			{
				if (mAttachedMice[i] == device)
				{
					mAttachedMice[i] = nullptr;
					mAttachedMice.erase(mAttachedMice.begin() + i);
					break;
				}
			}
		}
		break;

		default:
		{
			unsigned int count = (unsigned int)mAttachedGamepads.size();
			for (unsigned int i = 0; i < count; i++)
			{
				if (mAttachedGamepads[i] == device)
				{
					mAttachedGamepads[i] = nullptr;
					mAttachedGamepads.erase(mAttachedGamepads.begin() + i);
					break;
				}
			}
		}
		break;

		case InputDeviceType::ENUM_COUNT: return;
		}

		unsigned int count = (unsigned int)mAttachedDevices.size();
		for (unsigned int i = 0; i < count; i++)
		{
			if (mAttachedDevices[i] == device)
			{
				delete mAttachedDevices[i];
				mAttachedDevices[i] = nullptr;

				mAttachedDevices.erase(mAttachedDevices.begin() + i);
			}
		}
	}

	// --------------------------------------------------------------------

	void InputSingleton::UpdateAllDevices()
	{
		unsigned int count = (unsigned int)mAttachedDevices.size();
		for (unsigned int i = 0; i < count; i++)
		{
			if (mAttachedDevices[i])
			{
				mAttachedDevices[i]->Update();
			}
		}
	}

	// --------------------------------------------------------------------
};