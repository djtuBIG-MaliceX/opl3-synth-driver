@echo off
:: Build DLL for MinGW (WIP)

del *.o
::g++ -m32 -DUNICODE -DDISABLE_HW_SUPPORT *.cpp -c -std=c++11
::g++ -shared -Wl,--add-stdcall-alias *.o -lwinmm -o opl3emu.dll
g++ -O3 -m32 -DUNICODE -DDISABLE_HW_SUPPORT -DDISABLE_VGM_LOGGING -DWIN32 *.cpp -std=c++11 -std=c++1y -shared -static -Wl,--add-stdcall-alias -lwinmm -o opl3emu.dll

::clang++ -target=i386-pc-windows-msvc -O3 -DUNICODE -DDISABLE_HW_SUPPORT *.cpp -std=c++11 -shared -Wl,--add-stdcall-alias -lwinmm -o opl3emu.dll
pause

md bin
move opl3emu.dll bin
pause