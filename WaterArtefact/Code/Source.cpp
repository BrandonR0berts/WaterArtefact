#include "Artefact.h"

#include <chrono>

const float kFPSGoal       = 144.0f;
const float kTimePerUpdate = 1.0f / kFPSGoal;

int main()
{
	Artefact::ArtefactProgram* program = new Artefact::ArtefactProgram();

	bool                                               running            = true;
	float                                              deltaTime          = 0.0f;
	std::chrono::time_point<std::chrono::system_clock> oldTime            = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> now                = oldTime;
	float                                              timeTillNextUpdate = 0.0f;

	while (running && program)
	{
		now       = std::chrono::system_clock::now();
		deltaTime = std::chrono::duration<float>(now - oldTime).count();
		oldTime   = now;

		timeTillNextUpdate -= deltaTime;

		if (timeTillNextUpdate <= 0.0f)
		{
			program->Update(deltaTime);

			timeTillNextUpdate += kTimePerUpdate;
		}
	}

	return 0;
}