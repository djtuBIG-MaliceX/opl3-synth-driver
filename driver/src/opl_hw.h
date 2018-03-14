/*
 * Header for OPL Hardware Passthrough support
 * (C) 2013-2014 James Alan Nguyen
 * Original code adapted from MidiPlay/VGMPlay (C) 201X ValleyBell
 * 
 * Released under LGPL
 */
#include "stdafx.h"
#ifndef OPL_HW_H
#define OPL_HW_H

#ifndef DISABLE_HW_SUPPORT
#include "stdafx.h"
#include "InpOut32Helper.h"

#define DELAY_OPL2_REG	 3.3f
#define DELAY_OPL2_DATA	23.0f
#define DELAY_OPL3_REG	 0.0f
#define DELAY_OPL3_DATA	 0.28f

class OPLHardwareCore {
   void OPL_Hardware_Detection(void);
   void OPL_HW_WriteReg(WORD Reg, BYTE Data);
   void OPL_HW_Init();
   void OPL_HW_Close();
   inline UINT8 OPL_HW_GetStatus(void);
   inline void OPL_HW_WaitDelay(INT64 StartTime, float Delay);
};

#endif // DISABLE_HW_SUPPORT
#endif /*OPL_HW_H*/