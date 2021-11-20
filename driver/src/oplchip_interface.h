/*
 * Just a helper class to abstract the hardware calls to any opl chip core, including hardware.
 */

#ifndef OPLCHIP_INTERFACE_H
#define OPLCHIP_INTERFACE_H

#include "stdafx.h"
#include "opl.h"
#include "opl3.h"

#define FSAMP                           (49716) //(3579545.0 / 72.0

#ifndef DISABLE_HW_SUPPORT
#include "opl_hw.h"
#endif //DISABLE_HW_SUPPORT

class OPLChipInterface {

public:
   OPLChipInterface();
   void Init(uint8_t numChips=1);
   void Opl3_ChipWrite(int chipNo, uint16_t idx, uint8_t val);
   void Opl3_GetSample(short *sample, int len);

private:
   uint8_t numSoftChips;

   // DosBOX
   std::vector< std::unique_ptr< OPL > > adlibEmuChips;

   // Nuked OPL3
   std::vector< std::unique_ptr< opl3_chip > > nukeChips;
   
   //opl3_chip* CreateNukedOPL3();

#ifndef DISABLE_HW_SUPPORT
   // Hardware OPL
   opl_hw *hardwareOut;
#endif //DISABLE_HW_SUPPORT

#ifndef DISABLE_VGM_LOGGING
   //opl_vgmout *vgmOut;
//#include "vgm_logging.h"
#endif //DISABLE_VGM_LOGGING

};
#endif //OPLCHIP_INTERFACE_H

