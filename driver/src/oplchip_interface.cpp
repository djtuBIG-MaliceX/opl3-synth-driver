
#include "oplchip_interface.h"

OPLChipInterface::OPLChipInterface(BYTE numChips) {
   this->nukeChips = new std::vector<opl3_chip*>(numChips, new opl3_chip());
};

void OPLChipInterface::Opl3_ChipWrite(int chipNo, uint16_t idx, uint8_t val) {
   opl3_chip *chip = (*nukeChips)[chipNo];
   OPL3_WriteReg(chip, idx, val);

#ifndef DISABLE_HW_SUPPORT
   OPL_HW_WriteReg(idx, val);
#endif
};

void OPLChipInterface::Opl3_GetSample(short *sample, int len) {
   opl3_chip *chip = (*nukeChips)[0]; // TODO sum all chips
   OPL3_GenerateStream(chip, sample, len);
};
