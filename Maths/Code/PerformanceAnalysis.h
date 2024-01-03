#pragma once

#include <unordered_map>
#include <mutex>

namespace Engine
{
	namespace Timer
	{
		// -------------------------------------------------

		enum class PerformanceTimingAreas : unsigned int
		{
			// Whole frame data
			Update_WholeFrame_AmountCapped = 0,
			Update_WholeFrame_NotCapped,

			Render_WholeFrame_AmountCapped,
			Render_WholeFrame_NotCapped,
			Render_WholeFrame_NotCapped_WithoutImGUI,

			// Individual events data
			Update_Collisions,

			Setup_Whole,
			Setup_CompileShaders
		};

		// -------------------------------------------------

		class PerformanceTimings final
		{
		public:
			static float GetTiming(PerformanceTimingAreas area);
			static void  AddTiming(PerformanceTimingAreas area, float time);

			static void  Reset();

		private:
			PerformanceTimings();
			~PerformanceTimings();

			static std::unordered_map<PerformanceTimingAreas, std::pair<float, unsigned int>> mPerformanceStored;
			static std::mutex mAccessMutex;
		};

		// -------------------------------------------------
	}
}