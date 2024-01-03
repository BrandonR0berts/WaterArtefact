#ifndef _INPUT_DEVICE_H_
#define _INPUT_DEVICE_H_

#include <unordered_map>

#include <mutex>

// --------------------------------------------------------------------

namespace Input
{
	// --------------------------------------------------------------------

	enum class InputDeviceType : unsigned int
	{
		MOUSE    = 1,
		KEYBOARD = 2,

		// GLFW has 16 joypad connections
		GAMEPAD_1 = 3,
		GAMEPAD_2 = 4,
		GAMEPAD_3 = 5,
		GAMEPAD_4 = 6,
		GAMEPAD_5 = 7,
		GAMEPAD_6 = 8,
		GAMEPAD_7 = 9,
		GAMEPAD_8 = 10,
		GAMEPAD_9 = 11,
		GAMEPAD_10 = 12,
		GAMEPAD_11 = 13,
		GAMEPAD_12 = 14,
		GAMEPAD_13 = 15,
		GAMEPAD_14 = 16,
		GAMEPAD_15 = 17,
		GAMEPAD_16 = 18,


		ENUM_COUNT
	};

	// --------------------------------------------------------------------

	enum ButtonInputState : unsigned int
	{
		PRESSED  = 1, // First frame that the button is down
		HELD     = 2, // If second onwards frame that the button is down
		RELEASED = 3, // First frame button is up
		NO_INPUT = 4, // Second frame onwards of no input

		ENUM_COUNT
	};

	// --------------------------------------------------------------------

	class InputDevice
	{
	public:
		explicit InputDevice(InputDeviceType type);
		virtual ~InputDevice();

		InputDeviceType  GetDeviceType()                                 const { return mDeviceType; }

		void             RegisterInputKey(unsigned int keyEnumValue);
		void             RegisterInputStick(unsigned int stickEnumValue);

		void             ClearAllInput();

		virtual void     Update() = 0;
		void             UpdateInputKeys(std::vector< std::pair<unsigned int, ButtonInputState>>& keysToUpdate);

		ButtonInputState QueryInput(unsigned int keyEnumValue) const;
		float            QueryAxis(unsigned int axisEnumValue) const;

	protected:
		InputDeviceType mDeviceType;

		// Binds an input enum ID to an enum state of pressed, held or released 
		std::unordered_map<unsigned int, ButtonInputState> mInputMap;

		// Binds an input enum ID to an enum state between -1.0 and 1.0
		std::unordered_map<unsigned int, float>            mAxisInputMap;

		// The amount of updates that must pass before a pressed input turns into a held input
		static unsigned int kUpdatesTillHeld;
	};
};

// --------------------------------------------------------------------

#endif