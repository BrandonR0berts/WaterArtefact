#include "PerformanceAnalysis.h"

namespace Engine
{
	namespace Timer
	{
		std::unordered_map<PerformanceTimingAreas, std::pair<float, unsigned int>> PerformanceTimings::mPerformanceStored = std::unordered_map<PerformanceTimingAreas, std::pair<float, unsigned int>>();
		std::mutex                                                                 PerformanceTimings::mAccessMutex       = std::mutex();

		// ------------------------------------------------------------------

		PerformanceTimings::PerformanceTimings()
		{

		}

		// ------------------------------------------------------------------

		PerformanceTimings::~PerformanceTimings()
		{
			mAccessMutex.lock();
				mPerformanceStored.clear();
			mAccessMutex.unlock();
		}

		// ------------------------------------------------------------------

		float PerformanceTimings::GetTiming(PerformanceTimingAreas area)
		{
			mAccessMutex.lock();

			std::unordered_map<PerformanceTimingAreas, std::pair<float, unsigned int>>::iterator iter = mPerformanceStored.find(area);

			if (iter != mPerformanceStored.end())
			{
				float total             = iter->second.first;
				float measurementsTaken = (float)iter->second.second;

				mAccessMutex.unlock();

				// Calculate the average time from the stored values
				return total / measurementsTaken;
			}

			mAccessMutex.unlock();

			return 0.0f;
		}

		// ------------------------------------------------------------------

		void PerformanceTimings::AddTiming(PerformanceTimingAreas area, float time)
		{
			mAccessMutex.lock();

				std::unordered_map<PerformanceTimingAreas, std::pair<float, unsigned int>>::iterator iter = mPerformanceStored.find(area);

				if (iter != mPerformanceStored.end())
				{
					iter->second.first += time;
					iter->second.second++;
				}
				else
				{
					// Add the new area to the store
					mPerformanceStored.insert({ area, { time, 1 } });
				}

			mAccessMutex.unlock();
		}

		// ------------------------------------------------------------------

		void PerformanceTimings::Reset()
		{
			mAccessMutex.lock();
				mPerformanceStored.clear();
			mAccessMutex.unlock();
		}

		// ------------------------------------------------------------------
	}
}