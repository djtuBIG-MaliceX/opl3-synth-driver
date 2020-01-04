@echo off
cls
copy emscripten_wrapper.cpp ..
cd ..
call emcc -DDISABLE_HW_SUPPORT -DDISABLE_VGM_LOGGING -O3 emscripten_wrapper.cpp OPLSynth.cpp opl3.cpp -s EXPORTED_FUNCTIONS="['_Initialize', '_Reset', '_WriteMidiData', '_GetSample', '_GetSampleTwo', '_PlaySysex', '_CloseInstance']" 
del emscripten_wrapper.cpp
move a.out.js* emscripten
cd emscripten
pause