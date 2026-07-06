#pragma once

#ifdef _WIN32
	#define EXPORT __declspec(dllexport)
	#include <windows.h>
	#include <winsock.h>
#else
	#define EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef struct MaxShockAndIntensityPerQuarter {
	public:
		int maxShockIntensity;
		int intensityPerQuarter;
		int intensityPerHealth;
		int durationMilliseconds;
} MaxShockAndIntensityPerQuarter;
EXPORT MaxShockAndIntensityPerQuarter getMaxShockAndIntensityPerQuarter();
#ifdef WIN32
EXPORT void __stdcall startShockSender(char* filePath);
EXPORT int __stdcall sendShock(int amount, bool useQuarters = true);
EXPORT bool __stdcall getIsRunning();
EXPORT int __stdcall stop();
#else
EXPORT void startShockSender(char* filePath);
EXPORT int sendShock(int amount, bool useQuarters = true);
EXPORT bool getIsRunning();
EXPORT int stop();
#endif
#ifdef __cplusplus
}
#endif