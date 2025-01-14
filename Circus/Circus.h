#pragma once
#include <iostream>
#include <vector>
#include <algorithm>

#ifdef CIRCUS_EXPORTS
#define CIRCUS_API __declspec(dllexport)
#else
#define CIRCUS_API __declspec(dllimport)
#endif

extern "C" CIRCUS_API void CompressionFuncCircus(int*, int, int, const int*);
