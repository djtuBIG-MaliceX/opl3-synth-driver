cls
emcc -DDISABLE_HW_SUPPORT emscripten_wrapper.cpp OPLSynth.cpp opl3.cpp -s EXPORTED_FUNCTIONS="['_Initialize', '_Reset', '_WriteMidiData', '_GetSample', '_GetSampleTwo', '_PlaySysex', '_CloseInstance']" 
