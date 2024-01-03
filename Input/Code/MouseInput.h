#ifndef _MOUSE_INPUT_H_
#define _MOUSE_INPUT_H_

#include "InputDevice.h"

#include "Input.h"
#include "Maths/Code/Vector.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Input
{
	namespace MouseInput
	{
		enum MouseInputBitfield : unsigned int
		{
			NONE = 0,

			MOUSE_LEFT = 1,
			MOUSE_RIGHT = 2,

			MIDDLE_MOUSE = 3,

			MOUSE_SIDE_1 = 4,
			MOUSE_SIDE_2 = 5,
			MOUSE_SIDE_3 = 6,
			MOUSE_SIDE_4 = 7,

			ENUM_COUNT
		};

		class MouseInputDevice : public InputDevice
		{
		public:
			MouseInputDevice();
			~MouseInputDevice() override;

			void Update() override;

			static void                           SetMousePosition(double x, double y) 
			{ 
				Maths::Vector::Vector2D<float> newPos = Maths::Vector::Vector2D<float>((float)x, float(y));

				sMousePositionDelta = newPos - sMousePosition;
				sMousePosition      = newPos; 
			}
			static void                           SetMouseScroll(double x, double y)   { sMouseScroll   = Maths::Vector::Vector2D<float>((float)x, float(y));}

			static Maths::Vector::Vector2D<float> GetMousePosition()                   { return sMousePosition; }
			static Maths::Vector::Vector2D<float> GetMousePositionDelta()
			{
				Maths::Vector::Vector2D<float> returnDelta = sMousePositionDelta;

				sMousePositionDelta.x = 0.0f;
				sMousePositionDelta.y = 0.0f;

				return returnDelta;
			}

			static bool                           GetMouseOnScreen()                   { return sMouseOnScreen; }
			static Maths::Vector::Vector2D<float> GetMouseScroll()                     { return sMouseScroll; }

			//static std::vector<std::pair<unsigned int, ButtonInputState>>& GetInputQueue() { return sInputQueue; }

			static MouseInputDevice* Get() { return mInstance; }

			void AddInputEvent(std::pair<unsigned int, Input::ButtonInputState> eventToAdd)
			{
				sInputMutex.lock();
					sInputQueue.push_back(eventToAdd);
				sInputMutex.unlock();
			}

		private:
			static MouseInputDevice* mInstance;

			void RegisterKeys();

			static Maths::Vector::Vector2D<float> sMousePosition;
			static Maths::Vector::Vector2D<float> sMousePositionDelta;
			static Maths::Vector::Vector2D<float> sMouseScroll;
			static bool                           sMouseOnScreen;

			static std::vector<std::pair<unsigned int, ButtonInputState>> sInputQueue;
			static std::mutex                                             sInputMutex;
		};

#define CheckKeyPressMouse(mouseKeyID, glfwCase)case glfwCase:\
	keyPressed = mouseKeyID;\
break;

		static void MouseCallback(GLFWwindow* window, int button, int action, int mods)
		{
			Input::ButtonInputState inputState = Input::ButtonInputState::NO_INPUT;

			// There is no GLFW_REPEAT for mice

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

			unsigned int keyPressed = (unsigned int)Input::MouseInput::MouseInputBitfield::NONE;

			switch (button)
			{
				CheckKeyPressMouse((unsigned int)MouseInputBitfield::MOUSE_RIGHT,  GLFW_MOUSE_BUTTON_RIGHT);
				CheckKeyPressMouse((unsigned int)MouseInputBitfield::MOUSE_LEFT,   GLFW_MOUSE_BUTTON_LEFT);
				CheckKeyPressMouse((unsigned int)MouseInputBitfield::MIDDLE_MOUSE, GLFW_MOUSE_BUTTON_MIDDLE);

				CheckKeyPressMouse((unsigned int)MouseInputBitfield::MOUSE_SIDE_1, GLFW_MOUSE_BUTTON_4);
				CheckKeyPressMouse((unsigned int)MouseInputBitfield::MOUSE_SIDE_2, GLFW_MOUSE_BUTTON_5);
				CheckKeyPressMouse((unsigned int)MouseInputBitfield::MOUSE_SIDE_3, GLFW_MOUSE_BUTTON_6);
				CheckKeyPressMouse((unsigned int)MouseInputBitfield::MOUSE_SIDE_4, GLFW_MOUSE_BUTTON_7);

			default:
			break;
			}

			Input::MouseInput::MouseInputDevice::Get()->AddInputEvent(std::pair<unsigned int, Input::ButtonInputState>(keyPressed, inputState));
		}

		static void RegisterMouseCallback(GLFWwindow* window)
		{
			glfwSetMouseButtonCallback(window, MouseCallback);
		}

		static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
		{
			MouseInputDevice::SetMousePosition(xpos, ypos);
		}

		static void RegisterMousePositionCallback(GLFWwindow* window)
		{
			glfwSetCursorPosCallback(window, cursor_position_callback);
		}

		static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
		{
			MouseInputDevice::SetMouseScroll(xOffset, yOffset);
		}

		static void RegisterMouseScrollCallback(GLFWwindow* window)
		{
			glfwSetScrollCallback(window, scroll_callback);
		}
	};
};

#endif