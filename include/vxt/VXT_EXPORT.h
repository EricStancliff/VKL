#pragma once
#ifdef WIN32
#ifdef VXT_LIB
#define VXT_EXPORT __declspec(dllexport)
#else
#define VXT_EXPORT __declspec(dllimport)
#endif
#else
#define VXT_EXPORT
#endif