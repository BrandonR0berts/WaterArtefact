#ifndef _ENGINE_INPUT_H_
#define _ENGINE_INPUT_H_

#include "InputDevice.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Input
{
	// ----------------------------------------------------------

	namespace KeyboardInput
	{
		// ==========================================================
		// ----------------------------------------------------------

		enum KeyboardKeys : unsigned int
		{
			// Nothing
			NONE = 0,

			// Alphabet
			A = 1,
			B = 2,
			C = 3,
			D = 4,
			E = 5,
			F = 6,
			G = 7,
			H = 8,
			I = 9,
			J = 10,
			K = 11,
			L = 12,
			M = 13,
			N = 14,
			O = 15,
			P = 16,
			Q = 17,
			R = 18,
			S = 19,
			T = 20,
			U = 21,
			V = 22,
			W = 23,
			X = 24,
			Y = 25,
			Z = 26,

			LEFT = 27,
			RIGHT = 28,
			UP = 29,
			DOWN = 30,

			// Normal numbers
			ZERO = 40,
			ONE = 41,
			TWO = 41,
			THREE = 43,
			FOUR = 44,
			FIVE = 45,
			SIX = 46,
			SEVEN = 47,
			EIGHT = 48,
			NINE = 49,

			// F numbers
			F_ONE = 50,
			F_TWO = 51,
			F_THREE = 52,
			F_FOUR = 53,
			F_FIVE = 54,
			F_SIX = 55,
			F_SEVEN = 56,
			F_EIGHT = 57,
			F_NINE = 58,
			F_TEN = 59,
			F_ELEVEN = 60,
			F_TWELVE = 61,

			LEFT_SHIFT = 70,
			RIGHT_SHIFT = 71,

			LEFT_CONTROL = 72,
			RIGHT_CONTROL = 73,

			ENTER = 74,
			ESCAPE = 75,

			LEFT_ALT = 76,

			// Dot + comma
			DOT = 80,
			COMMA = 81,

			// Colons
			COLON = 82,
			SEMI_COLON = 83,

			// Slashes
			FORWARD_SLASH = 84,
			BACK_SLASH = 85,

			// Greater than and less than
			GREATER_THAN = 86,
			LESS_THAN = 87,

			// Backets
			OPEN_BRACKETS = 88,
			CLOSE_BRACKETS = 89,

			OPEN_SQUARE_BRACKETS = 90,
			CLOSE_SQUARE_BRACKETS = 91,

			OPEN_CURLEY_BRACKETS = 92,
			CLOSE_CURLEY_BRACKETS = 93,

			// End of sentance extras
			QUESTION_MARK = 94,
			EXCLAMATION_MARK = 95,

			SPEACH_MARKS = 96,
			QUOTATION_MARKS = 97,

			HASH = 98,
			TILDE = 99,

			SPACE = 100,

			ENUM_COUNT
		};

		// --------------------------------------------------------------

		class KeyboardInputDevice : public InputDevice
		{
		public:
			KeyboardInputDevice();
			~KeyboardInputDevice() override;

			void Update() override;

			static std::vector<std::pair<unsigned int, ButtonInputState>>& GetInputQueue() { return sInputQueue; }
			void AddInputEvent(unsigned int keyPressed, unsigned int inputState);

			static KeyboardInputDevice* Get() { return mInstance; }

		private:
			static KeyboardInputDevice* mInstance;

			void RegisterKeys();

			static std::vector<std::pair<unsigned int, ButtonInputState>> sInputQueue;
			static std::mutex                                             sInputMutex;
		};

		// --------------------------------------------------------------

#define CheckKeyPress(inputKeyID, glfwCase)\
{\
case glfwCase:\
	keyPressed = inputKeyID;\
break; \
}

		static void KeyboardCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
		{
			Input::ButtonInputState inputState = Input::ButtonInputState::NO_INPUT;

			switch (action)
			{
			case GLFW_PRESS:
				inputState = Input::ButtonInputState::PRESSED;
			break;

			case GLFW_RELEASE:
				inputState = Input::ButtonInputState::RELEASED;
			break;

			case GLFW_REPEAT:
				inputState = Input::ButtonInputState::HELD;
			break;

			default:
			break;
			}

			unsigned int keyPressed = (unsigned int)Input::KeyboardInput::KeyboardKeys::NONE;

			switch (key)
			{
				// Alphabet
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::A, GLFW_KEY_A);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::B, GLFW_KEY_B);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::C, GLFW_KEY_C);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::D, GLFW_KEY_D);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::E, GLFW_KEY_E);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F, GLFW_KEY_F);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::G, GLFW_KEY_G);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::H, GLFW_KEY_H);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::I, GLFW_KEY_I);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::J, GLFW_KEY_J);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::K, GLFW_KEY_L);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::M, GLFW_KEY_M);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::N, GLFW_KEY_N);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::O, GLFW_KEY_O);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::P, GLFW_KEY_P);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::Q, GLFW_KEY_Q);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::R, GLFW_KEY_R);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::S, GLFW_KEY_S);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::T, GLFW_KEY_T);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::U, GLFW_KEY_U);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::V, GLFW_KEY_V);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::W, GLFW_KEY_W);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::X, GLFW_KEY_X);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::Y, GLFW_KEY_Y);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::Z, GLFW_KEY_Z);

				// Space
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::SPACE, GLFW_KEY_SPACE);

				// Escape
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::ESCAPE, GLFW_KEY_ESCAPE);

				// Tidle
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::TILDE, GLFW_KEY_GRAVE_ACCENT);

				// Directions
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::RIGHT, GLFW_KEY_RIGHT);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::LEFT, GLFW_KEY_LEFT);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::UP, GLFW_KEY_UP);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::DOWN, GLFW_KEY_DOWN);

				// Numbers
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::ZERO, GLFW_KEY_0);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::ONE, GLFW_KEY_1);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::TWO, GLFW_KEY_2);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::THREE, GLFW_KEY_3);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::FOUR, GLFW_KEY_4);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::FIVE, GLFW_KEY_5);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::SIX, GLFW_KEY_6);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::SEVEN, GLFW_KEY_7);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::EIGHT, GLFW_KEY_8);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::NINE, GLFW_KEY_9);

				// Slashes
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::BACK_SLASH, GLFW_KEY_BACKSLASH);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::FORWARD_SLASH, GLFW_KEY_SLASH);

				// Brackets
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::OPEN_BRACKETS, GLFW_KEY_LEFT_BRACKET);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::CLOSE_BRACKETS, GLFW_KEY_RIGHT_BRACKET);

				// Left
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::LEFT_SHIFT, GLFW_KEY_LEFT_SHIFT);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::LEFT_CONTROL, GLFW_KEY_LEFT_CONTROL);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::LEFT_ALT, GLFW_KEY_LEFT_ALT);

				// Right
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::RIGHT_SHIFT, GLFW_KEY_RIGHT_SHIFT);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::RIGHT_CONTROL, GLFW_KEY_RIGHT_CONTROL);

				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::SEMI_COLON, GLFW_KEY_SEMICOLON);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::COMMA, GLFW_KEY_COMMA);

				// F-keys
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_ONE, GLFW_KEY_F1);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_TWO, GLFW_KEY_F2);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_THREE, GLFW_KEY_F3);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_FOUR, GLFW_KEY_F4);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_FIVE, GLFW_KEY_F5);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_SIX, GLFW_KEY_F6);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_SEVEN, GLFW_KEY_F7);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_EIGHT, GLFW_KEY_F8);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_NINE, GLFW_KEY_F9);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_TEN, GLFW_KEY_F10);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_ELEVEN, GLFW_KEY_F11);
				CheckKeyPress((unsigned int)Input::KeyboardInput::KeyboardKeys::F_TWELVE, GLFW_KEY_F12);

			default:
			break;
			}

			Input::KeyboardInput::KeyboardInputDevice::Get()->AddInputEvent(keyPressed, inputState);
		}

		static void RegisterKeyboardCallback(GLFWwindow* window)
		{
			glfwSetKeyCallback(window, KeyboardCallback);
		}
	};
};
#endif