
#include "oplchip_interface.h"

OPLChipInterface::OPLChipInterface() :
   nukeChips(255),
   adlibEmuChips(255),
   numSoftChips(1)
{
   
}

void OPLChipInterface::Init(uint8_t numChips)
{
   numSoftChips = numChips;
   for (uint8_t i = 0; i < numSoftChips && i < 255; ++i)
   {
      // Nuked OPL3
      if (nukeChips[i] != nullptr)
      {
         nukeChips[i].reset();
         nukeChips[i] = nullptr;
      }
      nukeChips[i] = std::make_unique<opl3_chip>();
      OPL3_Reset(nukeChips[i].get(), (Bit32u)FSAMP);

      // DOSBox AdlibEmu
      if (adlibEmuChips[i] != nullptr)
      {
         adlibEmuChips[i].reset();
         adlibEmuChips[i] = nullptr;
      }
      adlibEmuChips[i] = std::make_unique<OPL>();
      adlibEmuChips[i].get()->adlib_init();
   }
}

void OPLChipInterface::Opl3_ChipWrite(int chipNo, uint16_t idx, uint8_t val)
{
   // Nuked OPL3
   opl3_chip *chip = nukeChips[chipNo].get();
   OPL3_WriteReg(chip, idx, val);

   // DOSBox AdlibEmu
   OPL *dosboxChip = adlibEmuChips[chipNo].get();
   dosboxChip->adlib_write(idx, val);

#ifndef DISABLE_HW_SUPPORT
   OPL_HW_WriteReg(idx, val);
#endif
}

void OPLChipInterface::Opl3_GetSample(short *sample, int len)
{
   // Nuked OPL3
   opl3_chip *chip = nukeChips[0].get(); // TODO sum all chips
   OPL3_GenerateStream(chip, sample, len);

   // DOSBox AdlibEmu
   OPL* dosboxChip = adlibEmuChips[0].get();
   dosboxChip->adlib_getsample(sample, len);
}
