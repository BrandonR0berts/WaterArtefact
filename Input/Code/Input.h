#ifndef _INPUT_GENERAL_H_
#define _INPUT_GENERAL_H_

#include <vector>

#include "InputDevice.h"

int ConvertDeviceTypeToGamepadID(Input::InputDeviceType type);

// A general access input handler that collates all of the different input systems together into an abstracted view
namespace Input
{
	class InputSingleton
	{
	public:
		std::vector<InputDevice*>& GetAllDevicesAttached()   { return mAttachedDevices; }

		std::vector<InputDevice*>& GetAllKeyboardsAttached() { return mAttachedKeyboards; }
		std::vector<InputDevice*>& GetAllMiceAttached()      { return mAttachedMice; }
		std::vector<InputDevice*>& GetAllGamepadsAttached()  { return mAttachedGamepads; }

		bool GetInputDevice(InputDeviceType type, std::vector<InputDevice*>& devicesFoundOfType);

		void ClearAllInput();

		void AddDevice(InputDevice*    device, InputDeviceType type);
		void RemoveDevice(InputDevice* device, InputDeviceType type);

		static InputSingleton* Get() { if (!mThis) mThis = new InputSingleton(); return mThis; }

		void UpdateAllDevices();

	private:
		InputSingleton();
		~InputSingleton();

		static InputSingleton* mThis;

		std::vector<InputDevice*> mAttachedDevices;

		std::vector<InputDevice*> mAttachedKeyboards;
		std::vector<InputDevice*> mAttachedMice;
		std::vector<InputDevice*> mAttachedGamepads;
	};
};

#endif