
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
#endif //WIN32

#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <thread>
//#include <mutex>

#include "OPLSynth.h"
#include "MidiSynth.h"

#ifdef _DEBUG
#endif

#endif /*STDAFX_H*/

