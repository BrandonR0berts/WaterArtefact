#ifndef _TIMER_H_
#define _TIMER_H_

#include <chrono>

namespace Engine
{
	namespace Timer
	{
		// --------------------------------------------

		class Timer final
		{
		public:
			Timer();
			~Timer();

			// Sets the timer to zero
			void Start();

			// Pauses a currently running timer
			void Pause();

			// Resumes a currently running timer
			void Resume();

			// Sets the current time to being zero and pauses the timer
			void Reset();

			// Returns the number of miliseconds the timer has been running
			long long GetCurrentTime();
			float     GetCurrentTimeSeconds();

			bool      GetIsPaused()  const { return mPaused; }
			bool      GetIsRunning() const { return mRunning; }

		private:
			std::chrono::steady_clock::time_point mStartTime;
			std::chrono::steady_clock::time_point mPausedStartTime;

			float mPausedTime;

			bool mRunning;
			bool mPaused;
		};

		// --------------------------------------------
	}
}

#endif