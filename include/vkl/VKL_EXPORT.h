#pragma once
#ifdef WIN32
#ifdef VKL_LIB
#define VKL_EXPORT __declspec(dllexport)
#else
#define VKL_EXPORT __declspec(dllimport)
#endif
#else
#define VKL_EXPORT
#endif