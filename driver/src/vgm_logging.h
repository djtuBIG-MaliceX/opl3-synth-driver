#include "stdafx.h"
#ifndef VGM_LOGGING_H
#define VGM_LOGGING_H

#include "VGMFile.h"

void VGMLog_Init();
inline void VGMLog_CmdWrite(BYTE Cmd, BYTE Reg, BYTE Data);
void VGMLog_IncrementSamples(int len);
void VGMLog_Close();
void VGMLog_MarkLoopStartNow();

#endif //VGM_LOGGING_H