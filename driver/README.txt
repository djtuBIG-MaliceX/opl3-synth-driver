OPL3 software synthesizer WinMM driver
======================================

Original code (C) 2011-2013 Alex Khokolov
Additions (C) 2013 dj.tuBIG/MaliceX (http://www.codingchords.com)
Distributed under LGPL


Git Repository: https://bitbucket.org/djtubig-malicex/opl3-synth-driver


Changelog
---------
[30-DEC-2013]
 * Added ARM target to project solution (for jailbroken Windows RT 8.0)
 * Implemented Mono Legato mode (CC126) and Poly switch (CC127)
 * Tweaked converted banks to use pitch offsets (still doesn't sound right due to patches being designed for the OPL Rhythm Mode channels)
 * Fixed sustain bug where it was not releasing repeated sustained notes.
 * Fixed long-release pitch bend behaviour
 * Refactored note off functionality to allow for instant voice cut.
 
[31-OCT-2013]
 * Recompiled binaries to Release w/ XP compatibility (project solution upgrade changed default target)
 * Added Jamie O'Connell's FMSynth default bank
 * Tweaked pitch bend range handling to update freq based on semitone range change (exists in EMU10k-based MIDI synths)
 * Changed driver's MIDI Mapper name  (future-proofing consideration)


[30-OCT-2013]
 * Adapted ValleyBell's Pitch Bend range extender support from MidiPlay (thanks!)
 * Added CC11 Expression support
 * Reorganised patch.h to also include percussion bank mapping
 * Included a conversion of mauifm.ibk+mauiperc.ibk -> mauipatch.h (you can always change it back to the original patch.h Fat Man bank)


TODO
----
 * Bank switching (eg: changing patch banks, something Jamie O'Connell's custom driver could do)
 * Support for 4-op patch banks (eg: Voyetra Super SAPI!)
 * Patch editor or more patch converters (looking at .SBI and Freq Monster 801)
 * Considering special build for channel-fixed support (may be necessary for true 4op)
 * VGM logging?
 

Special Thanks
---------------
- ValleyBell, for significant assistance and allowing me to see the source code 
  of MidiPlay.
  
- DJBouche, for advice in handling MIDI pitch bend calculations.

- khokh2001, for creating the original OPL3 windows driver code base.

- DOSBox team, for creating an excellent emulator, with an excellent OPL3 chip
  emulation engine.


Note
----

Use either install-fatman.bat or install-maui.bat to install.
Alternatively, if using XP or earlier, rename either to opl3emu.dll and
install via Add New Hardware.


(Below instructions from Sergey V. Mikayev)



OPL3 WinMM Driver. Based on Munt WinMM Driver.
********************************


First-time Installation
-----------------------

Run drvsetup.exe to perform installation. This will copy opl3emu.dll into
C:\WINDOWS\SYSTEM32 (C:\WINDOWS\SYSWOW64 for x64 systems) directory.
Use the main UI-enabled application to configure driver settings.

--------------------------------------------------------------------------------

You can also use the fail-safe approach "Add Hardware wizard" if you like.
The information file opl3emu.inf is provided. However, this way works
on x86 Windows only.

 1) Unpack opl3emu.inf and opl3emu.dll. Remember the location of this directory.
 2) Open Control Panel.
 3) Double-click on "Add Hardware".
 4) Click "Next" until you come to a message asking you whether you have
    already installed the hardware.
 5) Select the "Yes" option and click "Next".
 6) A list of installed hardware will appear. Scroll to the bottom of the list
    and select the last entry, which should be something like "New Hardware".
    Click "Next".
 7) Select "Choose hardware manually from a list" and click "Next".
 8) Select "Sound, Video and Game Controllers" and click "Next".
 9) Click "Have Disk...".
10) In the window that pops up, click "Browse..." and choose the directory to
    which you unpacked the opl3emu.inf and opl3emu.dll files. Click "OK".
11) If a window pops up complaining about the lack of Windows Logo testing,
    click "Install Anyway" or similar.
12) "YMF262 Synth Emulator" should have appeared selected in a list.
    Click "Next" twice.
13) The driver *still* isn't Windows Logo tested, so click "Install Anyway" if
    necessary.
14) The driver should now have been installed; click "Finish".
15) A dialog box will recommend that you reboot. Go ahead if you enjoy that
    sort of thing, but it shouldn't be necessary for a fresh installation.

--------------------------------------------------------------------------------

To begin playing back MIDI through the emulator, you need to configure
the corresponding MIDI device in your MIDI application. In order to set
the emulator as the default MIDI device in the system you may use Control
Panel or a MIDI switcher like Putzlowitschs Vista-MIDIMapper.

The driver tries to find out whether the main UI-enabled application is running, and if
it is, the driver directs all the incoming MIDI messages to the main application
for processing. If the application is not running, the driver operates in self-contained
mode.

If you start the main synth application and / or change the emulation parameters you need
to either restart MIDI playback or reopen your MIDI application for the new
parameters to take effect and for the driver to make an attempt to communicate
with the main application.


Upgrading
---------

Run drvsetup.exe to upgrade the driver. You may also need to check
the configuration file opl3emu.ini for new configuration variables.

                             OR

 1) Click on "Start", then "Run...".
 2) Type "devmgmt.msc" and press enter.
 3) Click on the "+" Next to "Sound, Video and Game Controllers".
 4) Right-click on "YMF262 Synth Emulator" and select "Update Driver".
 5) If asked whether you'd like to check Windows Update, select "No, not this time" and click "Next".
 6) Select "Select software from a list" and click "Next".
 7) Select "Don't search, let me choose from a list" and click "Next".
 8) Click "Browse...".
 9) Choose the address of the unzipped stuff and click "OK".
10) Click "Next".
11) Click "Continue installation" if asked about Windows Logo stuff.
12) If a dialog pops up asking for the ROM files, navigate *again* to the directory to show it where the ROMs are and click OK.
13) If asked about change in languages, confirm that you want the file replaced.
14) You almost certainly *will* need to reboot the computer for the change to take effect.


License
-------

LGPL parts:

Copyright (C) 2003, 2004, 2005, 2011 Dean Beeler, Jerome Fisher
Copyright (C) 2011, 2012 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
Copyright (C) 2002-2013  The DOSBox Team

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

MS-LPL parts:

Copyright (c) 1996-2000 Microsoft Corporation.

Microsoft Limited Public License

This license governs use of code marked as “sample” or “example” available on this web site without a license agreement, as provided under the section above titled “NOTICE SPECIFIC TO SOFTWARE AVAILABLE ON THIS WEB SITE.” If you use such code (the “software”), you accept this license. If you do not accept the license, do not use the software.
1.Definitions

  The terms “reproduce,” “reproduction,” “derivative works,” and “distribution” have the same meaning here as under U.S. copyright law.

  A “contribution” is the original software, or any additions or changes to the software.

  A “contributor” is any person that distributes its contribution under this license.

  “Licensed patents” are a contributor’s patent claims that read directly on its contribution.
  
2.Grant of Rights

  A.Copyright Grant - Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non-exclusive, worldwide, royalty-free copyright license to reproduce its contribution, prepare derivative works of its contribution, and distribute its contribution or any derivative works that you create.
  
  B.Patent Grant - Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non-exclusive, worldwide, royalty-free license under its licensed patents to make, have made, use, sell, offer for sale, import, and/or otherwise dispose of its contribution in the software or derivative works of the contribution in the software.

3.Conditions and Limitations 
  
  A.No Trademark License- This license does not grant you rights to use any contributors’ name, logo, or trademarks.
  B.If you bring a patent claim against any contributor over patents that you claim are infringed by the software, your patent license from such contributor to the software ends automatically.
  C.If you distribute any portion of the software, you must retain all copyright, patent, trademark, and attribution notices that are present in the software.
  D.If you distribute any portion of the software in source code form, you may do so only under this license by including a complete copy of this license with your distribution.  If you distribute any portion of the software in compiled or object code form, you may only do so under a license that complies with this license.
  E.The software is licensed “as-is.” You bear the risk of using it. The contributors give no express warranties, guarantees or conditions.  You may have additional consumer rights under your local laws which this license cannot change. To the extent permitted under your local laws, the contributors exclude the implied warranties of merchantability, fitness for a particular purpose and non-infringement.
  F.Platform Limitation - The licenses granted in sections 2(A) and 2(B) extend only to the software or derivative works that you create that run on a Microsoft Windows operating system product.
  