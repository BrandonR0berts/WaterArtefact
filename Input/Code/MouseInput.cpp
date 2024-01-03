#include "MouseInput.h"

namespace Input
{
	namespace MouseInput
	{
		// --------------------------------------------------------------------

		std::vector<std::pair<unsigned int, ButtonInputState>> MouseInputDevice::sInputQueue = std::vector<std::pair<unsigned int, ButtonInputState>>();
		Maths::Vector::Vector2D<float> MouseInputDevice::sMousePosition                      = Maths::Vector::Vector2D<float>();
		Maths::Vector::Vector2D<float> MouseInputDevice::sMouseScroll                        = Maths::Vector::Vector2D<float>();
		Maths::Vector::Vector2D<float> MouseInputDevice::sMousePositionDelta                 = Maths::Vector::Vector2D<float>();

		std::mutex MouseInputDevice::sInputMutex;

		MouseInputDevice* MouseInputDevice::mInstance = nullptr;

		// --------------------------------------------------------------------

		MouseInputDevice::MouseInputDevice()
			: InputDevice(InputDeviceType::MOUSE)
		{
			RegisterKeys();

			mInstance = this;
		}

		// --------------------------------------------------------------------

		MouseInputDevice::~MouseInputDevice()
		{

		}

		// --------------------------------------------------------------------

		void MouseInputDevice::RegisterKeys()
		{
			RegisterInputKey((unsigned int)MouseInputBitfield::MIDDLE_MOUSE);

			RegisterInputKey((unsigned int)MouseInputBitfield::MOUSE_LEFT);
			RegisterInputKey((unsigned int)MouseInputBitfield::MOUSE_RIGHT);

			RegisterInputKey((unsigned int)MouseInputBitfield::MOUSE_SIDE_1);
			RegisterInputKey((unsigned int)MouseInputBitfield::MOUSE_SIDE_2);
			RegisterInputKey((unsigned int)MouseInputBitfield::MOUSE_SIDE_3);
			RegisterInputKey((unsigned int)MouseInputBitfield::MOUSE_SIDE_4);
		}

		// --------------------------------------------------------------------

		void MouseInputDevice::Update()
		{
			if (!sInputQueue.empty())
			{
				sInputMutex.lock();
					UpdateInputKeys(sInputQueue);
				sInputMutex.unlock();
			}
			else
			{
				// See if any buttons pressed need to be swapped to being held instead of just pressed
				for (std::unordered_map<unsigned int, ButtonInputState>::iterator iter = mInputMap.begin(); iter != mInputMap.end(); iter++)
				{
					if (iter->second == ButtonInputState::PRESSED)
					{
						iter->second = ButtonInputState::HELD;
					}
				}
			}
		}

		// --------------------------------------------------------------------
	};
};