#pragma once

#include <iostream>

#define ABORT() *(char*)0 = 1;

// If we are in debug mode
#ifdef _DEBUG_BUILD

	#define ASSERTMSG(condition, failMessage)\
	{\
		if(!!(condition))\
		{\
			ABORT();\
		}\
	}

	#define ASSERTFAIL(failMessage)\
	{\
		ABORT();\
	}

#else // If we are in release mode

	// Make the asserts do nothing
	#define ASSERTMSG(condition, failMessage) 
	#define ASSERTFAIL(failMessage)
#endif