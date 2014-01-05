/*
 * inpout32.dll helper functions
 * 2013 James Alan Nguyen
 * http://www.codingchords.com
 *
 * Do whatever you want with it.  Made this while trying to port the OPL passthrough
 * routines on VGMPlay, since PortTalk does not work under 64-bit Windows.
 */

#include "InpOut32Helper.h"

typedef void	(__stdcall *lpOut32)(short, short);
typedef short	(__stdcall *lpInp32)(short);
typedef BOOL	(__stdcall *lpIsInpOutDriverOpen)(void);
typedef BOOL	(__stdcall *lpIsXP64Bit)(void);

lpOut32 gfpOut32 = 0;
lpInp32 gfpInp32 = 0;
lpIsInpOutDriverOpen gfpIsInpOutDriverOpen = 0;
lpIsXP64Bit gfpIsXP64Bit = 0;

HINSTANCE hInpOutDll ;

/*
 * Returns true if inpout32 failed to open (necessary for applications checking for non-zero failures)
 */
BOOL OpenInpOut32(void)
{
	hInpOutDll = LoadLibrary ( L"InpOut32.DLL" ) ;	//The 32bit DLL. If we are building x64 C++ 
													//applicaiton then use InpOutx64.dll
	if ( hInpOutDll != NULL )
	{
		gfpOut32 = (lpOut32)GetProcAddress(hInpOutDll, "Out32");
		gfpInp32 = (lpInp32)GetProcAddress(hInpOutDll, "Inp32");
		gfpIsInpOutDriverOpen = (lpIsInpOutDriverOpen)GetProcAddress(hInpOutDll, "IsInpOutDriverOpen");
		gfpIsXP64Bit = (lpIsXP64Bit)GetProcAddress(hInpOutDll, "IsXP64Bit");

		if (gfpIsInpOutDriverOpen())
         return FALSE; 
   }

    MessageBox(NULL, L"Cannot load inpout32.dll for hardware OPL playback.", L"OPL3_HW", MB_OK | MB_ICONEXCLAMATION);
   return TRUE;
}

void CloseInpOut32(void)
{
   FreeLibrary(hInpOutDll);
   gfpOut32 = NULL;
   gfpInp32 = NULL;
	gfpIsInpOutDriverOpen = NULL;
	gfpIsXP64Bit = NULL;
}

void outportb(unsigned short PortAddress, unsigned short Data)
{
   if (gfpOut32 == NULL)
   {
#ifdef DEBUG 
      MessageBox(NULL, L"Cannot write to register %d; inpout32.dll not loaded.", L"DEBUG", MB_OK | MB_ICONERROR);
#endif
      return;
   }
   gfpOut32(PortAddress, Data);
}

unsigned char inportb(unsigned short PortAddress)
{
   if (gfpInp32 == NULL)
   {
#ifdef DEBUG
      MessageBox(NULL, L"Cannot read register; inpout32.dll not loaded.", L"DEBUG", MB_OK | MB_ICONERROR);
#endif
      return ~0;
   }
   return (char)gfpInp32(PortAddress);
}
