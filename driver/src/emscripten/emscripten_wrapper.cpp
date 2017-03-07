
#include "OPLSynth.h"
#include <emscripten.h>

extern "C" {

   static OPLSynth *instance = nullptr;

   void Initialize() 
   {
      if (instance == nullptr)
      {
         std::cout << "New instance for OPLSynth()" << std::endl;
         instance = new OPLSynth();
      }
      
      instance->Init();
   }
   
   void Reset()
   {
      if (instance != nullptr) 
      {
         std::cout << "Opl3_SoftCommandReset()" << std::endl;
         instance->Opl3_SoftCommandReset();
      }
         
   }
   
   void WriteMidiData(DWORD dwData)
   {
      if (instance != nullptr)
      {
         //std::cout << "WriteMidiData(" << dwData << ")" << std::endl;
         instance->WriteMidiData(dwData);
      }
         
   }
   
   void GetSample(short *samplem, int len)
   {
      if (instance != nullptr)
      {
         //std::cout << "GetSample( len = " << len << ")" << std::endl;
         instance->GetSample(samplem, len);
      }
      /*
      //debug
      //std::cout << "-START SAMPLES" << std::endl;
      for (int i = 0; i < len; ++i)
      {
         EM_ASM_({
            document.getElementById('bufNum').innerHTML = $0;
         }, samplem[i]);
         
         //std::cout << samplem[i] << std::endl;
      }
      //std::cout << "-END SAMPLES" << std::endl;
      */
   }
   
   void GetSampleTwo(short *samplem, int len, float* fsamparrL, float* fsamparrR)
   {
      GetSample(samplem, len);
      
      for (int i = 0, j=0; j < len/2; i+=2, j++)
      {
         fsamparrL[j] = samplem[i] / 32768.f;
         fsamparrL[j] = (fsamparrL[j]<-1)?-1:(fsamparrL[j]>1)?1:fsamparrL[j];
      }
         
      
      for (int i = 1, j=0; j < len/2; i+=2, j++)
      {
         fsamparrR[j] = samplem[i] / 32768.f;
         fsamparrR[j] = (fsamparrR[j]<-1)?-1:(fsamparrR[j]>1)?1:fsamparrR[j];
      }
         
      
      /*
      for (int i = 0; i < len; ++i)
      {
         fsamparr[i] = (float)samplem[i] / 32768.f;
         fsamparr[i] = (fsamparr[i] < -1) ? -1 : (fsamparr[i] > 1) ? 1 : fsamparr[i];
         //EM_ASM_({
         //   document.getElementById('bufNum').innerHTML = $0 + '<br />';
         //}, fsamparr[i]);
      }
      */
   }
   
   void PlaySysex(Bit8u *bufpos, DWORD len)
   {
      if (instance != nullptr)
      {
         std::cout << "PlaySysex( len = " << len << ")" << std::endl;
         instance->PlaySysex(bufpos, len);
      }
   }
  
   void CloseInstance()
   {
      if (instance != nullptr)
      {
         std::cout << "CloseInstance()" << std::endl;
         instance->close();
         delete instance;
         instance = nullptr;
      }
   }
}