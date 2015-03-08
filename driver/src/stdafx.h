
#ifndef STDAFX_H
#define STDAFX_H

#pragma pack(1)

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#include <mmsystem.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "mmddk.h"
#ifdef __cplusplus
}
#endif  /* __cplusplus */

#include <mmreg.h>
#include <process.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "OPLSynth.h"
#include "MidiSynth.h"

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#endif /*STDAFX_H*/

