#pragma once

#ifdef CIRCUS_EXPORTS
#define CIRCUS_API __declspec(dllexport)
#else
#define CIRCUS_API __declspec(dllimport)
#endif

extern "C" CIRCUS_API void Circus(long long x, long long y);
extern "C" CIRCUS_API int CompressionFuncCircus(int x, int y);