
#include "vgm_logging.h"

const double FMToVGMSamples = 0.887038377986966;

VGM_HEADER VGMHead;
DWORD ClockAdd, LastVgmDelay;
FILE *hFileVGM = NULL;
DWORD VGMSmplPlayed;
WCHAR TempLogPath[BUFSIZ];

void VGMLog_Init()
{
   memset(&VGMHead, 0x00, sizeof(VGM_HEADER));
   /*switch(DROInf.iHardwareType)
   {
   case 0x00:	// Single OPL2 Chip
      VGMHead.lngHzYM3812 = 3579545;
      break;
   case 0x01:	// Dual OPL2 Chip
      VGMHead.lngHzYM3812 = 3579545 | 0xC0000000;
      break;
   case 0x02:	// Single OPL3 Chip*/
      VGMHead.lngHzYMF262 = 14318180;
      /*break;
   default:
      VGMHead.lngHzYM3812 = 3579545 | 0x40000000;
      break;
   }*/
		

   //if (OPL_CHIPS > 1)
   //   ClockAdd = 0x40000000;
   //else
   //   ClockAdd = 0x00;
   VGMHead.fccVGM = FCC_VGM;
   VGMHead.lngVersion = 0x00000151;
   VGMHead.lngEOFOffset = 0xC0;
   VGMHead.lngDataOffset = sizeof(VGM_HEADER) - 0x34;
   //if (VGMHead.lngHzYM3526)
   //   VGMHeadL.lngHzYM3526 = VGMHead.lngHzYM3526 | ClockAdd;
   //if (VGMHead.lngHzYM3812)
   //   VGMHeadL.lngHzYM3812 = VGMHead.lngHzYM3812 | ClockAdd;
   if (VGMHead.lngHzYMF262)
      VGMHead.lngHzYMF262 = VGMHead.lngHzYMF262 | ClockAdd;
   ExpandEnvironmentStrings(L"%TEMP%\\opl3vgmlog.vgm", (LPWSTR)&TempLogPath, BUFSIZ);
   //strcpy_s(TempLogPath, getenv("TEMP"));
   //fopen_s(&hFileVGM, "opl3SynthLog.vgm", "wb");
   _wfopen_s(&hFileVGM, TempLogPath, L"wb");
   //hFileVGM = fopen("D:\\Downloads\\testlog.vgm", "wb");
   if (hFileVGM == NULL)
   {
      //hFileVGM = NULL;
      MessageBox(NULL, L"Handle for opl3SynthLog.vgm failed.", L"VGM Logger", MB_OK | MB_ICONEXCLAMATION);
      return;
   }
   fwrite(&VGMHead, 0x01, sizeof(VGM_HEADER), hFileVGM);
   LastVgmDelay = 0;
   VGMSmplPlayed = 0;
}

/*OPL_Write() / adlib_wite() etc. guide
static inline void OPL_Write(BYTE ChipID, WORD Register, BYTE Data)
{
	//if (ChipID >= OPL_VCHIPS)
	//{
	//	if (ErrMsg && ! SkipMode)
	//		printf("Illegal Chip Write!\n");
	//	return;
	//}
	
	BYTE RegSet;
	WORD RegAddr;
	
	RegAddr = (ChipID << 8) | Register;
	RegAddr %= OPL_VCHIPS << 8;
   switch(OPL_MODE)
   {
   case 0x01:	// OPL 1
      ym3526_w(ChipID, 0x00, Register & 0xFF);
      ym3526_w(ChipID, 0x01, Data);
      if (LogVGM)
         VGMLog_CmdWrite(0x5B + ChipID * 0x50, Register & 0xFF, Data);
      break;
   case 0x02:	// OPL 2
      ym3812_w(ChipID, 0x00, Register & 0xFF);
      ym3812_w(ChipID, 0x01, Data);
      if (LogVGM)
         VGMLog_CmdWrite(0x5A + ChipID * 0x50, Register & 0xFF, Data);
      break;
   case 0x03:	// OPL 3
      RegSet = Register >> 8;
      RegSet |= (ChipID & 0x01);
      ChipID >>= 1;
      
      ymf262_w(ChipID, 0x00 | (RegSet << 1), Register & 0xFF);d
      ymf262_w(ChipID, 0x01 | (RegSet << 1), Data);
      if (LogVGM)
         VGMLog_CmdWrite((0x5E | RegSet) + ChipID * 0x50, Register & 0xFF, Data);
      break;
   }
}*/

// helper function
inline void VGMLog_CmdWrite(BYTE Cmd, BYTE Reg, BYTE Data)
{
	DWORD DelayDiff, CurTime;
	WORD WrtDly;
	
   if (hFileVGM == NULL) return;

   CurTime = VGMSmplPlayed;

	//DelayDiff = PlayingTime - LastVgmDelay;
	DelayDiff = CurTime - LastVgmDelay;
	while(DelayDiff)
	{
		if (DelayDiff > 0xFFFF)
			WrtDly = 0xFFFF;
		else
			WrtDly = (unsigned short int)DelayDiff;
		fputc(0x61, hFileVGM);
		fwrite(&WrtDly, 0x02, 0x01, hFileVGM);
		DelayDiff -= WrtDly;
	}
	LastVgmDelay = CurTime;	//PlayingTime;
	
	fputc(Cmd, hFileVGM);
	if (Cmd == 0x66)
		return;
	fputc(Reg, hFileVGM);
	fputc(Data, hFileVGM);
	
	return;
}

void VGMLog_IncrementSamples(int len)
{
   VGMSmplPlayed += (DWORD)(len * FMToVGMSamples);
   //VGMSmplPlayed += len;
}

void VGMLog_Close()
{
   if (hFileVGM == NULL) return;

   VGMLog_CmdWrite(0x66, 0x00, 0x00);
   UINT32 AbsVol = (UINT32)(ftell(hFileVGM) - 0x04);
   fseek(hFileVGM, 0x04, SEEK_SET);
   fwrite(&AbsVol, sizeof(UINT32), 0x01, hFileVGM);
   fseek(hFileVGM, 0x18, SEEK_SET);
   fwrite(&VGMSmplPlayed, 0x04, 0x01, hFileVGM);
   fclose(hFileVGM);

   MessageBox(NULL, L"File opl3SynthLog.vgm saved.", L"VGM Logger", MB_OK | MB_ICONINFORMATION);
}
