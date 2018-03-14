/*
 * Just a helper class to abstract the hardware calls to any opl chip core, including hardware.
 */
#include "stdafx.h"
#include "opl3.h"

#ifndef OPLCHIP_INTERFACE_H
#define OPLCHIP_INTERFACE_H

#ifndef DISABLE_HW_SUPPORT
#include "opl_hw.h"
#endif //DISABLE_HW_SUPPORT

class OPLChipInterface {

public:
   OPLChipInterface(uint8_t numChips);
   void Opl3_ChipWrite(int chipNo, uint16_t idx, uint8_t val);
   void Opl3_GetSample(short *sample, int len);

private:
   uint8_t numSoftChips;

   // MAME
   // TODO: add MAME core here

   // DosBOX
   //OPL *dosboxChips;

   // Nuked OPL3
   std::vector<opl3_chip*> *nukeChips;

#ifndef DISABLE_HW_SUPPORT
   // Hardware OPL
   opl_hw *hardwareOut;
#endif //DISABLE_HOW_SUPPORT

#ifndef DISABLE_VGM_LOGGING
   //opl_vgmout *vgmOut;
//#include "vgm_logging.h"
#endif //DISABLE_VGM_LOGGING

};
#endif //OPLCHIP_INTERFACE_H

