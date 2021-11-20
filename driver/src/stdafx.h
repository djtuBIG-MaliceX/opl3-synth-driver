#pragma once

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <tchar.h>

#include "mmddk.h"

#define ntohs(x) _byteswap_ushort(x)  // little endian assumed
#else //WIN32
#include <arpa/inet.h>  // ntohs
#endif //WIN32

#include <iostream>
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#ifndef M_PI // because VS2019 does not define this unless it's compiled in C style (ie: #include <math.h>)
#define M_PI 3.14159265358979323846 // pi
#endif

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <memory>
#include <cinttypes>

#ifdef _DEBUG
//#include <mutex>
#endif
