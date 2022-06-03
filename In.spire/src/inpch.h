#pragma once

#include <iostream>
#include <string>
#include <functional>

//#ifdef PLATFORM_WINDOWS //Client windows maybe?
	//#include <Windows.h>
	#ifdef DLL_BUILD
		#define IN_SPIRE __declspec(dllexport)
	#else
		#define IN_SPIRE __declspec(dllimport)
	#endif
//#endif
