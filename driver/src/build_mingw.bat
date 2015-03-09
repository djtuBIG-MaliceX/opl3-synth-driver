:: Build DLL for MinGW (WIP)

del *.o
g++ -DUNICODE -DDISABLE_HW_SUPPORT *.cpp -c -std=c++11
g++ -shared -Wl,--add-stdcall-alias *.o -lwinmm -o opl3emu.dll

pause

md bin
move opl3emu.dll bin\*
