
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

#include "OPLSynth.h"
#include "MidiSynth.h"

#endif /*STDAFX_H*/

