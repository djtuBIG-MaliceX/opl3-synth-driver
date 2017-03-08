:: Build DLL for MinGW (WIP)

del *.o
copy emscripten\mmddk.h .
::g++ -m32 -DUNICODE -DDISABLE_HW_SUPPORT *.cpp -c -std=c++11
::g++ -shared -Wl,--add-stdcall-alias *.o -lwinmm -o opl3emu.dll
g++ -O3 -m32 -DUNICODE -DDISABLE_HW_SUPPORT *.cpp -std=c++11 -shared -static -Wl,--add-stdcall-alias *.o -lwinmm -o opl3emu.dll

::clang++ -target=i386-pc-windows-msvc -O3 -DUNICODE -DDISABLE_HW_SUPPORT *.cpp -std=c++11 -shared -Wl,--add-stdcall-alias -lwinmm -o opl3emu.dll
del mmddk.h
pause

md bin
move opl3emu.dll bin/*
move opl3emu.dll bin/*
pause