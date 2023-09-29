@echo off
cls
copy emscripten_wrapper.cpp ..
cd ..
call emcc -DDISABLE_HW_SUPPORT -DDISABLE_VGM_LOGGING -O3 -std=c++17 emscripten_wrapper.cpp OPLSynth.cpp opl3.cpp oplchip_interface.cpp opl.cpp -s EXPORTED_FUNCTIONS="['_Initialize', '_Reset', '_WriteMidiData', '_GetSample', '_GetSampleTwo', '_PlaySysex', '_CloseInstance', '_malloc']" -s EXPORTED_RUNTIME_METHODS=ccall,cwrap
del emscripten_wrapper.cpp
move a.out.* emscripten
cd emscripten
pause