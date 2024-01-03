#include "InputHandler.h"

namespace Input
{
	// ---------------------------------------------

	InputHandler::InputHandler()
		: mCurrentInputSetup(nullptr)
	{

	}

	// ---------------------------------------------

	InputHandler::~InputHandler()
	{

	}

	// ---------------------------------------------

	void InputHandler::AddCombinationToEventMap(const std::string& eventMapName, Input::InputDeviceType deviceType, unsigned int buttonEnum, const std::string& eventToTrigger)
	{
		// Find the setup in the map
		const unsigned int hashedName = Engine::StringHash::Hash(eventMapName);
		std::unordered_map<unsigned int, InputSetup>::iterator iter = mInputSetups.find(hashedName);

		if (iter != mInputSetups.end())
		{
			// Exists so add it to the map
			iter->second.mButtonToEventMaps.push_back(std::make_pair(deviceType, std::make_pair(buttonEnum, eventToTrigger)));
		}
		else
		{
			// Does not already exist so create a new one
			InputSetup newSetup;
			newSetup.mName = eventMapName;
			newSetup.mButtonToEventMaps.push_back(std::make_pair(deviceType, std::make_pair(buttonEnum, eventToTrigger)));

			mInputSetups.insert({ hashedName, newSetup });
		}
	}

	// ---------------------------------------------

	bool InputHandler::SwapToInputSetup(std::string& inputName)
	{
		std::unordered_map<unsigned int, InputSetup>::iterator iter = mInputSetups.find(Engine::StringHash::Hash(inputName));

		if (iter != mInputSetups.end())
		{
			mCurrentInputSetup = &iter->second;
			return true;
		}

		return false;
	}

	// ---------------------------------------------

	bool InputHandler::GetInputSetup(std::string setupName, InputSetup& setupOut)
	{
		std::unordered_map<unsigned int, InputSetup>::iterator iter = mInputSetups.find(Engine::StringHash::Hash(setupName));

		if (iter != mInputSetups.end())
		{
			setupOut = iter->second;
			return true;
		}

		return false;
	}

	// ---------------------------------------------

	void InputHandler::Update()
	{
		// See what input devices the current input system uses
		std::vector<InputDeviceType> devicesToCheck;

		if (mCurrentInputSetup)
		{
			std::vector<std::pair<Input::InputDeviceType, std::pair<unsigned int, std::string>>>& buttonEventMaps = mCurrentInputSetup->mButtonToEventMaps;
			unsigned int buttonMapCount = (unsigned int)buttonEventMaps.size();
			for (unsigned int i = 0; i < buttonMapCount; i++)
			{
				bool added = false;

				// See if the device is already in the list
				unsigned int devicesCount = (unsigned int)devicesToCheck.size();
				for (unsigned int j = 0; j < devicesCount; j++)
				{
					if (buttonEventMaps[i].first == devicesToCheck[j])
					{
						added = true;
						break;
					}
				}

				if (!added)
				{
					devicesToCheck.push_back(buttonEventMaps[i].first);
				}
			}
		}

		// Now go through all devices we need to check and see if any of the buttons stored have been pressed
		std::vector<InputDevice*>& devices = InputSingleton::Get()->GetAllDevicesAttached();

		unsigned int deviceToCheckCount = (unsigned int)devicesToCheck.size();
		for (unsigned int i = 0; i < deviceToCheckCount; i++)
		{
			unsigned int devicesCount = (unsigned int)devices.size();
			for (unsigned int j = 0; j < devicesCount; j++)
			{
				// See if we have one of those attached
				if (devices[j]->GetDeviceType() == devicesToCheck[i])
				{
					// Now see if any of the keys pressed match currently pressed keys
					InvokeEventsMapped(devices[j]);

					break;
				}
			}
		}
	}
}