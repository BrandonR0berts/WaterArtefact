#include "Timer.h"

namespace Engine
{
	namespace Timer
	{
		// ------------------------------------

		Timer::Timer()
			: mStartTime()
			, mPausedStartTime()
			, mPaused(false)
			, mRunning(false)
			, mPausedTime(0)
		{

		}

		// ------------------------------------

		Timer::~Timer() { }

		// ------------------------------------

		void Timer::Start()
		{
			// Get the current time
			mStartTime = std::chrono::high_resolution_clock::now();

			mPausedTime = 0.0f;
			mRunning    = true;
			mPaused     = false;
		}

		// ------------------------------------

		void Timer::Pause()
		{
			if (mRunning)
			{
				// Store the time pause was started
				mPausedStartTime = std::chrono::high_resolution_clock::now();

				// Set to be paused
				mPaused = true;
			}
		}

		// ------------------------------------

		void Timer::Resume()
		{
			if (mRunning && mPaused)
			{
				// Reset paused flag
				mPaused = false;

				// Get the current time
				std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

				// Calculate the amount of time between the last pause time and now
				float pausedTime = (float)std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mPausedStartTime).count();

				// Reset the pause time to make sure that we dont add pause time on multiple times
				mPausedStartTime = currentTime;

				// Add the time we were paused for onto the running count
				mPausedTime += pausedTime;
			}
		}

		// ------------------------------------

		void Timer::Reset()
		{
			mStartTime = std::chrono::high_resolution_clock::now();

			mRunning    = false;
			mPaused     = false;
			mPausedTime = 0.0f;
		}

		// ------------------------------------

		long long Timer::GetCurrentTime()
		{
			// Only valid to get the current time of a running timer
			if (mRunning)
			{
				// Get the current time
				std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

				// Update paused time if we are paused
				if (mPaused)
				{
					// Calculate the amount of time between the last pause time and now
					long long pausedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mPausedStartTime).count();

					// Reset the pause time to make sure that we dont add pause time on multiple times
					mPausedStartTime = currentTime;

					// Add the time we were paused for onto the running count
					mPausedTime += pausedTime;
				}

				// Take off start time from current time, making sure to take off time spent paused
				return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mStartTime).count() - (int64_t)mPausedTime;
			}

			// return 0 if the timer is not running
			return 0;
		}

		// ------------------------------------

		float Timer::GetCurrentTimeSeconds()
		{
			return (float)GetCurrentTime() / 1000.0f; 
		}

		// ------------------------------------
	}
}