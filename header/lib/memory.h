#pragma once
#include "stdafx.h"
#include "struct/mt4/MqlStr.h"


uint WINAPI GetBoolsAddress(const BOOL values[]);
uint WINAPI GetDoublesAddress(const double values[]);
uint WINAPI GetIntsAddress(const int values[]);
uint WINAPI GetStringAddress(const TCHAR* value);
uint WINAPI GetStringsAddress(const MqlStr values[]);

uint WINAPI GetPointedAddress(const void* value);
BOOL WINAPI MemCompare(const void* a, const void* b, size_t size);
