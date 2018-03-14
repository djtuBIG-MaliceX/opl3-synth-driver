
#ifndef STDAFX_H
#define STDAFX_H

#pragma pack(1)

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <process.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "mmddk.h"
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#define ntohs(x) _byteswap_ushort(x)  // little endian assumed
#else //WIN32
#include <arpa/inet.h>  // ntohs
#endif //WIN32

#include <iostream>
#include <cmath>
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "OPLSynth.h"
#include "MidiSynth.h"

#ifdef _DEBUG
//#include <mutex>
#endif

#endif /*STDAFX_H*/

