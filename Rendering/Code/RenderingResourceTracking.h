#pragma once

#ifdef _DEBUG_BUILD

namespace Rendering
{
	struct TrackingData
	{
		static unsigned int sGPUMemoryUsedBytes;

		static void AdjustGPUMemoryUsed(int countToAdjustBy)
		{
			sGPUMemoryUsedBytes += countToAdjustBy;
		}
	};
}

#endif