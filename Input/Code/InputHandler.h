#ifndef _INPUT_HANDLER_H_
#define _INPUT_HANDLER_H_

#include "StringHash.h"
#include "Input.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace Input
{
	// ----------------------------------------------------

	// A setup of button presses to event triggers, with the device the button is from stored along-side
	struct InputSetup final
	{
		InputSetup()
			: mName("Unnamed")
		{}

		InputSetup(std::string name) 
			: mName(name)
		{}

		std::string mName;
		std::vector<std::pair<Input::InputDeviceType, std::pair<unsigned int, std::string>>> mButtonToEventMaps;
	};

	// ----------------------------------------------------

	class InputHandler final
	{
	public:
		InputHandler();
		~InputHandler();

		bool SwapToInputSetup(std::string& inputSetupName);
		void AddCombinationToEventMap(const std::string& eventMapName, Input::InputDeviceType deviceType, unsigned int buttonEnum, const std::string& eventToTrigger);

		void AddInputSetup(const InputSetup& setup) { mInputSetups.insert({ Engine::StringHash::Hash(setup.mName), setup }); }

		bool GetInputSetup(std::string setupName, InputSetup& inputOut);

		void Update();

	private:
		std::unordered_map<unsigned int, InputSetup> mInputSetups;
		InputSetup*                                  mCurrentInputSetup;
	};

	// ----------------------------------------------------
}

#endif