@echo off
cls
copy emscripten_wrapper.cpp ..
cd ..
call emcc -DDISABLE_HW_SUPPORT -DDISABLE_VGM_LOGGING -O3 -std=c++1z emscripten_wrapper.cpp OPLSynth.cpp opl3.cpp oplchip_interface.cpp opl.cpp -s EXPORTED_FUNCTIONS="['_Initialize', '_Reset', '_WriteMidiData', '_GetSample', '_GetSampleTwo', '_PlaySysex', '_CloseInstance']" 
del emscripten_wrapper.cpp
move a.out.js* emscripten
cd emscripten
pause