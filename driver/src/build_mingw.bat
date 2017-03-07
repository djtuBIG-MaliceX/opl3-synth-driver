:: Build DLL for MinGW (WIP)

del *.o
::g++ -m32 -DUNICODE -DDISABLE_HW_SUPPORT *.cpp -c -std=c++11
g++ -O3 -m32 -DUNICODE -DDISABLE_HW_SUPPORT *.cpp -c -std=c++11
::g++ -shared -Wl,--add-stdcall-alias *.o -lwinmm -o opl3emu.dll
g++ -shared -static -Wl,--add-stdcall-alias *.o -lwinmm -o opl3emu.dll

pause

md bin
mv opl3emu.dll bin/*
move opl3emu.dll bin/*
