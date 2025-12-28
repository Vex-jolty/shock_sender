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
	int maxShockIntensity;
	int intensityPerQuarter;
	int durationMilliseconds;
} MaxShockAndIntensityPerQuarter;
EXPORT MaxShockAndIntensityPerQuarter getMaxShockAndIntensityPerQuarter();
EXPORT int run(char* filePath);
EXPORT int sendShock(int quarters);
EXPORT bool getIsRunning();
EXPORT int stop();
#ifdef __cplusplus
}
#endif