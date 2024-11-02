#pragma once
#include <iostream>
#include <vector>
#include <algorithm>

#ifdef CIRCUS_EXPORTS
#define CIRCUS_API __declspec(dllexport)
#else
#define CIRCUS_API __declspec(dllimport)
#endif

extern "C" CIRCUS_API void Circus(long long x, long long y);
extern "C" CIRCUS_API void CompressionFuncCircus(int* pixelData, int size, int width, int height);