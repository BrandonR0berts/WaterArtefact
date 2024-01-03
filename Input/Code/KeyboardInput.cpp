#include "KeyboardInput.h"

// --------------------------------------------------------------------

namespace Input
{
	namespace KeyboardInput
	{
		std::vector<std::pair<unsigned int, ButtonInputState>> KeyboardInputDevice::sInputQueue = std::vector<std::pair<unsigned int, ButtonInputState>>();
		std::mutex KeyboardInputDevice::sInputMutex;

		KeyboardInputDevice* KeyboardInputDevice::mInstance = nullptr;

		// --------------------------------------------------------------------

		KeyboardInputDevice::KeyboardInputDevice()
			: InputDevice(InputDeviceType::KEYBOARD)
		{
			RegisterKeys();

			mInstance = this;
		}

		// --------------------------------------------------------------------

		KeyboardInputDevice::~KeyboardInputDevice()
		{

		}

		// --------------------------------------------------------------------

		void KeyboardInputDevice::RegisterKeys()
		{
			RegisterInputKey((unsigned int)KeyboardKeys::A);
			RegisterInputKey((unsigned int)KeyboardKeys::B);
			RegisterInputKey((unsigned int)KeyboardKeys::C);
			RegisterInputKey((unsigned int)KeyboardKeys::D);
			RegisterInputKey((unsigned int)KeyboardKeys::E);
			RegisterInputKey((unsigned int)KeyboardKeys::F);
			RegisterInputKey((unsigned int)KeyboardKeys::G);
			RegisterInputKey((unsigned int)KeyboardKeys::H);
			RegisterInputKey((unsigned int)KeyboardKeys::I);
			RegisterInputKey((unsigned int)KeyboardKeys::J);
			RegisterInputKey((unsigned int)KeyboardKeys::K);
			RegisterInputKey((unsigned int)KeyboardKeys::L);
			RegisterInputKey((unsigned int)KeyboardKeys::M);
			RegisterInputKey((unsigned int)KeyboardKeys::N);
			RegisterInputKey((unsigned int)KeyboardKeys::O);
			RegisterInputKey((unsigned int)KeyboardKeys::P);
			RegisterInputKey((unsigned int)KeyboardKeys::Q);
			RegisterInputKey((unsigned int)KeyboardKeys::R);
			RegisterInputKey((unsigned int)KeyboardKeys::S);
			RegisterInputKey((unsigned int)KeyboardKeys::T);
			RegisterInputKey((unsigned int)KeyboardKeys::U);
			RegisterInputKey((unsigned int)KeyboardKeys::V);
			RegisterInputKey((unsigned int)KeyboardKeys::W);
			RegisterInputKey((unsigned int)KeyboardKeys::X);
			RegisterInputKey((unsigned int)KeyboardKeys::Y);
			RegisterInputKey((unsigned int)KeyboardKeys::Z);

			RegisterInputKey((unsigned int)KeyboardKeys::ONE);
			RegisterInputKey((unsigned int)KeyboardKeys::TWO);
			RegisterInputKey((unsigned int)KeyboardKeys::THREE);
			RegisterInputKey((unsigned int)KeyboardKeys::FOUR);
			RegisterInputKey((unsigned int)KeyboardKeys::FIVE);
			RegisterInputKey((unsigned int)KeyboardKeys::SIX);
			RegisterInputKey((unsigned int)KeyboardKeys::SEVEN);
			RegisterInputKey((unsigned int)KeyboardKeys::EIGHT);
			RegisterInputKey((unsigned int)KeyboardKeys::NINE);

			RegisterInputKey((unsigned int)KeyboardKeys::CLOSE_BRACKETS);
			RegisterInputKey((unsigned int)KeyboardKeys::OPEN_BRACKETS);

			RegisterInputKey((unsigned int)KeyboardKeys::CLOSE_CURLEY_BRACKETS);
			RegisterInputKey((unsigned int)KeyboardKeys::OPEN_CURLEY_BRACKETS);

			RegisterInputKey((unsigned int)KeyboardKeys::CLOSE_SQUARE_BRACKETS);
			RegisterInputKey((unsigned int)KeyboardKeys::OPEN_SQUARE_BRACKETS);

			RegisterInputKey((unsigned int)KeyboardKeys::LESS_THAN);
			RegisterInputKey((unsigned int)KeyboardKeys::GREATER_THAN);

			RegisterInputKey((unsigned int)KeyboardKeys::BACK_SLASH);
			RegisterInputKey((unsigned int)KeyboardKeys::FORWARD_SLASH);

			RegisterInputKey((unsigned int)KeyboardKeys::COLON);
			RegisterInputKey((unsigned int)KeyboardKeys::SEMI_COLON);

			RegisterInputKey((unsigned int)KeyboardKeys::DOT);
			RegisterInputKey((unsigned int)KeyboardKeys::COMMA);

			RegisterInputKey((unsigned int)KeyboardKeys::DOWN);
			RegisterInputKey((unsigned int)KeyboardKeys::UP);
			RegisterInputKey((unsigned int)KeyboardKeys::RIGHT);
			RegisterInputKey((unsigned int)KeyboardKeys::LEFT);

			RegisterInputKey((unsigned int)KeyboardKeys::ENTER);
			RegisterInputKey((unsigned int)KeyboardKeys::ESCAPE);

			RegisterInputKey((unsigned int)KeyboardKeys::HASH);
			RegisterInputKey((unsigned int)KeyboardKeys::EXCLAMATION_MARK);

			RegisterInputKey((unsigned int)KeyboardKeys::LEFT_ALT);
			RegisterInputKey((unsigned int)KeyboardKeys::RIGHT_CONTROL);
			RegisterInputKey((unsigned int)KeyboardKeys::LEFT_CONTROL);
			RegisterInputKey((unsigned int)KeyboardKeys::RIGHT_SHIFT);
			RegisterInputKey((unsigned int)KeyboardKeys::LEFT_SHIFT);

			RegisterInputKey((unsigned int)KeyboardKeys::SPACE);
			RegisterInputKey((unsigned int)KeyboardKeys::SPEACH_MARKS);

			RegisterInputKey((unsigned int)KeyboardKeys::F_ONE);
			RegisterInputKey((unsigned int)KeyboardKeys::F_TWO);
			RegisterInputKey((unsigned int)KeyboardKeys::F_THREE);
			RegisterInputKey((unsigned int)KeyboardKeys::F_FOUR);
			RegisterInputKey((unsigned int)KeyboardKeys::F_FIVE);
			RegisterInputKey((unsigned int)KeyboardKeys::F_SIX);
			RegisterInputKey((unsigned int)KeyboardKeys::F_SEVEN);
			RegisterInputKey((unsigned int)KeyboardKeys::F_EIGHT);
			RegisterInputKey((unsigned int)KeyboardKeys::F_NINE);
			RegisterInputKey((unsigned int)KeyboardKeys::F_TEN);
			RegisterInputKey((unsigned int)KeyboardKeys::F_ELEVEN);
			RegisterInputKey((unsigned int)KeyboardKeys::F_TWELVE);

			RegisterInputKey((unsigned int)KeyboardKeys::TILDE);
		}

		// --------------------------------------------------------------------

		void KeyboardInputDevice::Update()
		{
			sInputMutex.lock();
				UpdateInputKeys(sInputQueue);
			sInputMutex.unlock();
		}

		// --------------------------------------------------------------------

		void KeyboardInputDevice::AddInputEvent(unsigned int keyPressed, unsigned int inputState)
		{
			sInputMutex.lock();

			// Check to see if there is already a registered event for this key and if so override it
			unsigned int inputQueueSize = (unsigned int)sInputQueue.size();

			for (unsigned int i = 0; i < inputQueueSize; i++)
			{
				if (sInputQueue[i].first == keyPressed)
				{
					sInputQueue[i].second = (Input::ButtonInputState)inputState;
					sInputMutex.unlock();
					return;
				}
			}

			sInputQueue.push_back(std::pair<unsigned int, Input::ButtonInputState>(keyPressed, (Input::ButtonInputState)inputState));

			sInputMutex.unlock();
		}

		// --------------------------------------------------------------------
	}
}