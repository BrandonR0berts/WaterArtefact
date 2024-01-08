#pragma once

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