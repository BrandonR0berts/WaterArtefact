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
}