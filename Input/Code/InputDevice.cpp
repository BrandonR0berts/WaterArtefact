#include "InputDevice.h"

// ---------------------------------------------------------

namespace Input
{
	InputDevice::InputDevice(InputDeviceType type)
		: mDeviceType(type)
	{

	}

	// ---------------------------------------------------------

	InputDevice::~InputDevice()
	{
		mInputMap.clear();
	}

	// ---------------------------------------------------------

	void InputDevice::RegisterInputKey(unsigned int keyEnumValue)
	{
		// Check if the key already exists
		if (mInputMap.find(keyEnumValue) == mInputMap.end())
		{
			// Add the key to the map
			mInputMap.insert(std::pair<unsigned int, ButtonInputState>(keyEnumValue, ButtonInputState::NO_INPUT));
		}
	}

	// ---------------------------------------------------------

	void InputDevice::RegisterInputStick(unsigned int stickEnumValue)
	{
		// Check if the stick axis already exists
		if (mAxisInputMap.find(stickEnumValue) == mAxisInputMap.end())
		{
			// Add to the map
			mAxisInputMap.insert(std::pair<unsigned int, float>(stickEnumValue, 0.0f));
		}
	}

	// ---------------------------------------------------------

	void InputDevice::ClearAllInput()
	{
		for (std::unordered_map<unsigned int, ButtonInputState>::iterator iter = mInputMap.begin(); iter != mInputMap.end(); iter++)
		{
			iter->second  = ButtonInputState::NO_INPUT;
		}

		for (std::unordered_map<unsigned int, float>::iterator iter = mAxisInputMap.begin(); iter != mAxisInputMap.end(); iter++)
		{
			iter->second = 0.0f;
		}
	}

	// ---------------------------------------------------------

	ButtonInputState InputDevice::QueryInput(unsigned int keyEnumValue) const
	{
		// Check if the input key exists
		std::unordered_map<unsigned int, ButtonInputState>::const_iterator iter = mInputMap.find(keyEnumValue);

		if (iter != mInputMap.end())
		{
			return (ButtonInputState)iter->second;
		}

		return ButtonInputState::NO_INPUT;
	}

	// ---------------------------------------------------------

	float InputDevice::QueryAxis(unsigned int axisEnumValue) const
	{
		// Check if the input key exists
		std::unordered_map<unsigned int, float>::const_iterator iter = mAxisInputMap.find(axisEnumValue);

		if (iter != mAxisInputMap.end())
		{
			return iter->second;
		}

		return 0.0f;
	}

	// ---------------------------------------------------------

	void InputDevice::UpdateInputKeys(std::vector<std::pair<unsigned int, ButtonInputState>>& keysToUpdate)
	{
		if (keysToUpdate.empty())
			return;

		unsigned int sizeOfKeysToUpdate = (unsigned int)keysToUpdate.size();

		for (unsigned int i = 0; i < sizeOfKeysToUpdate; i++)
		{
			// Find the key
			std::unordered_map<unsigned int, ButtonInputState>::iterator iter = mInputMap.find(keysToUpdate[i].first);

			if (iter != mInputMap.end())
			{
				// Update the state
				iter->second = keysToUpdate[i].second;
			}
		}

		// Remove the ones we have handled
		keysToUpdate.erase(keysToUpdate.begin(), keysToUpdate.begin() + sizeOfKeysToUpdate);
	}

	// ---------------------------------------------------------
}