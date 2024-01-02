#pragma once

#ifdef _DEBUG_BUILD

#include "RenderingResourceTracking.h"

namespace Rendering
{
	unsigned int TrackingData::sGPUMemoryUsedBytes = 0;
}
#endif