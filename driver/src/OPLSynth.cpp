// ==============================================================================
//
// Copyright (c) 1996-2000 Microsoft Corporation.  All rights reserved.
//
// Extensions (C) 2013 James Alan Nguyen
// http://www.codingchords.com
//
// ==============================================================================

#include "OPLSynth.h"

// TODO - Determine way to read configuration for existing bank file before playback
//#include "patch.h"
//#include "mauipatch.h"
//#include "fmsynthpatch.h"
#include "2x2patchtest.h"
//#include "ctmidipatch.h"

void
   OPLSynth::
   WriteMidiData(DWORD dwData)
{
   BYTE    bMsgType,bChannel, bVelocity, bNote;
   DWORD   dwTemp;

   bMsgType = (BYTE) dwData & (BYTE)0xf0;
   bChannel = (BYTE) dwData & (BYTE)0x0f;
   bNote = (BYTE) ((WORD) dwData >> 8) & (BYTE)0x7f;
   bVelocity = (BYTE) (dwData >> 16) & (BYTE)0x7f;

   switch (bMsgType)
   {
   case 0x90:      /* turn key on, or key off if VELOCITY == 0 */
      if (bVelocity)
      {
         //if (bChannel == DRUMCHANNEL) // TODO: change to dynamically assignable drum channels
         if ((m_wDrumMode & (1<<bChannel)) || bChannel == DRUMCHANNEL)
         {
            //if(bNote>=35 && bNote<88)
            {
               //Opl3_NoteOn((BYTE)(gbPercMap[bNote - 35][0]+35+128),gbPercMap[bNote - 35][1],bChannel,bVelocity,m_iBend[bChannel]);
               Opl3_NoteOn((BYTE)(gbPercMap[bNote][0]+128),gbPercMap[bNote][1],bChannel,bVelocity,m_iBend[bChannel]);
            }
         }
         else
         {
            Opl3_NoteOn((BYTE)m_bPatch[bChannel],bNote,bChannel,bVelocity,m_iBend[bChannel]);
         }
         break;
      } // if bVelocity.
      //NOTE: no break specified here. On an else case we want to continue through and turn key off

   case 0x80:
      /* turn key off */
      //  we don't care what the velocity is on note off
      //if (bChannel == DRUMCHANNEL)
      if ((m_wDrumMode & (1<<bChannel)) || bChannel == DRUMCHANNEL)
      {
         //if(bNote>=35 && bNote<88)
         {
            //Opl3_NoteOff((BYTE)(gbPercMap[bNote - 35][0]+35+128),gbPercMap[bNote - 35][1], bChannel, 0);
            Opl3_NoteOff((BYTE)(gbPercMap[bNote][0]+128),gbPercMap[bNote][1], bChannel, 0);
         }
      }
      else
      {
         Opl3_NoteOff ((BYTE) m_bPatch[bChannel],bNote, bChannel, m_bSustain[ bChannel ]);
      }
      break;

   case 0xb0:
      /* change control */
      switch (bNote)
      {

      case 0: // Bank Select MSB
         //only care for setting percussion for now
         // TODO: dynamic selection for switchable patches

         m_wDrumMode = (bVelocity == 0x7F) ?
            m_wDrumMode | (1<<bChannel) :
            m_wDrumMode &~(1<<bChannel) ;

         break;
     
      case 1: // Modulation Wheel
         m_bModWheel[bChannel] = bVelocity;
         break;

      case 5: // Portamento Time
         Opl3_SetPortamento(bChannel, bVelocity);
         break;

      case 6: // Data Entry (CC98-101 dependent)
         Opl3_ProcessDataEntry(bVelocity, bChannel);
         break;

      case 7:
         /* change channel volume */
         //Opl3_ChannelVolume(bChannel,gbVelocityAtten[bVelocity >> 1]);
         m_curVol[bChannel] = bVelocity;
         Opl3_ChannelVolume(bChannel,gbVelocityAtten[(BYTE)((bVelocity >> 1) * ((float)m_iExpThres[bChannel]/0x7F))]);
         break;

      case 8:
         break;

      case 10:
         /* change the pan level */
         Opl3_SetPan(bChannel, bVelocity);
         break;
         
      case 11: 
         m_iExpThres[bChannel] = bVelocity;
         /* set expression threshold. should influence bChannel.gbVelocityAtten[curVol>>1] range */
         Opl3_ChannelVolume(bChannel,gbVelocityAtten[(BYTE)((m_curVol[bChannel] >> 1) * ((float)m_iExpThres[bChannel]/0x7F))]);
         break;

      case 64:  // Sustain Pedal
         /* Change the sustain level */
         Opl3_SetSustain(bChannel, bVelocity);
         break;

      case 65:  // Portamento Pedal
         m_wPortaMode = (bVelocity > 0x3F) ?
            m_wPortaMode | (1<<bChannel) :
            m_wPortaMode & ~(1<<bChannel) ;

         break;

      case 72:  // Release
         // TODO: read adjustment depending on algorithm
         m_bRelease[bChannel] = bVelocity;
         for (int i = 0; i < NUM2VOICES; ++i)
         {
            if (m_Voice[i].bChannel == bChannel)
            {
               char bOffset = (char)lin_intp(m_bRelease[bChannel], 0, 127, (-8), 8);
               
               if (!bOffset) 
                  continue;

               for (int j = 0; j < 2; ++j)
               {
                  WORD wOffset = gw2OpOffset[ i ][ j ] ;
                  BYTE bInst = glpPatch[m_Voice[i].bPatch].note.op[j].bAt80;
                  char bTemp = bInst & 0xF;
                  
                  if (glpPatch[m_Voice[i].bPatch].note.bAtC0[0] & 0x01)
                     continue;

                  bInst &= ~0xF;
                  bTemp -= bOffset;
                  bInst |= (bTemp > 0xF) ? 0xF : (bTemp >= 0) ? bTemp : 0;
                  Opl3_ChipWrite( 0x80 + wOffset, bInst);
               }
            }
         }
         break;
      
      case 73:  // Attack
         // TODO: read adjustment depending on algorithm
         m_bAttack[bChannel] = bVelocity;
         for (int i = 0; i < NUM2VOICES; ++i)
         {
            if (m_Voice[i].bChannel == bChannel)
            {
               char bOffset = (char)lin_intp(m_bAttack[bChannel], 0, 127, (-8), 8);
               WORD wOffset;
               BYTE bInst = glpPatch[m_Voice[i].bPatch].note.op[1].bAt60;
               char bTemp = ((bInst & 0xF0)>>4);
               
               if (!bOffset) 
                  continue;

               for (int j = 0; j < 2; ++j)
               {
                  if (j == 0 && !((bInst & 0xF0)>>4))
                     continue;

                  wOffset = gw2OpOffset[ i ][ j ] ;
                  bInst = glpPatch[m_Voice[i].bPatch].note.op[j].bAt60;
                  bTemp = ((bInst & 0xF0)>>4);
               
                  bInst &= ~0xF0;
                  bTemp -= bOffset;
                  bInst |= (bTemp > 0xF) ? 0xF : (bTemp >= 0) ? (bTemp<<4) : 0;
                  Opl3_ChipWrite( 0x60 + wOffset, bInst) ;
               }
            }
         }
         break;
      
      case 74:  // "brightness"
         // TODO: read adjustment depending on algorithm (?)
         m_bBrightness[bChannel] = bVelocity;
         for (int i = 0; i < NUM2VOICES; ++i)
         {
            if (m_Voice[i].bChannel == bChannel)
            {
               WORD wOffset = gw2OpOffset[ i ][ 0 ] ;
               BYTE bInst = glpPatch[m_Voice[i].bPatch].note.op[0].bAt40;
               char bTemp = bInst & 0x3F;
               char bOffset = (char)lin_intp(m_bBrightness[bChannel], 0, 127, (-32), 32);
               bInst &= ~0x3F;
               bTemp -= bOffset;
               bInst |= (bTemp > 0x3F) ? 0x3F : (bTemp >= 0) ? bTemp : 0;
               Opl3_ChipWrite( 0x40 + wOffset, bInst) ;
            }
         }
         //Opl3_UpdateBrightness(bChannel, bVelocity);
         break;

      case 98:  // NRPN LSB
      case 99:  // NRPN MSB
         break;

      case 100: // RPN LSB
         m_RPN[bChannel][0] = bVelocity;
         break;
      case 101: // RPN MSB
         m_RPN[bChannel][1] = bVelocity;
         break;

      case 126: // Mono mode on
         Opl3_ChannelNotesOff(bChannel);
         m_wMonoMode |= (1<<bChannel);
         m_bLastVoiceUsed[bChannel] = bChannel; // Assign to midi-voice channel 1:1; last two channels only used if overflow for poly mode.
         break;

      case 127: // Poly mode on
         Opl3_ChannelNotesOff(bChannel);
         m_wMonoMode &= ~(1<<bChannel);
         break;

      default:
         if (bNote >= 120)        /* Channel mode messages */
         {
            Opl3_ChannelNotesOff(bChannel);
         }
         //  else unknown controller
      };
      break;

   case 0xc0: // Program change
      //if (bChannel != DRUMCHANNEL)
      if (!(m_wDrumMode & (1<<bChannel)))
      {
         m_bPatch[ bChannel ] = bNote ;
      }
      break;

   case 0xe0:  // pitch bend
      dwTemp = ((WORD)bNote << 0) | ((WORD)bVelocity << 7);
		dwTemp -= 0x2000;
      //dwTemp += 0x8000;
      dwTemp *= (m_iBendRange[bChannel]);
      //dwTemp *= (12<<1);
      m_iBend[bChannel] = (long) (dwTemp);
      
      //wTemp = ((WORD) bVelocity << 9) | ((WORD) bNote << 2);
      ////m_iBend[bChannel] = (short) (WORD) (wTemp + 0x7fff);
      //m_iBend[bChannel] = (long) (DWORD) (wTemp + 0x8000);
      Opl3_PitchBend(bChannel, m_iBend[bChannel]);
      break;
   };
   return;
}

// ========================= opl3 specific methods ============================

// ==========================================================================
// Opl3_AllNotesOff - turn off all notes
// ==========================================================================
void
   OPLSynth::
   Opl3_AllNotesOff()
{
   BYTE i;

   for (i = 0; i < NUM2VOICES; i++)
   {
      if (m_Voice[i].bSusHeld)
         Opl3_CutVoice(i, FALSE);
      else
         Opl3_NoteOff(m_Voice[i].bPatch, m_Voice[i].bNote, m_Voice[i].bChannel, 0);
  } 
}

// ==========================================================================
//  void Opl3_NoteOff
//
//  Description:
//     This turns off a note, including drums with a patch
//     # of the drum note + 128, but the first drum instrument is at MIDI note _35_.
//
//  Parameters:
//     BYTE bPatch
//        MIDI patch
//
//     BYTE bNote
//        MIDI note
//
//     BYTE bChannel
//        MIDI channel
//
//  Return Value:
//     Nothing.
//
//
// ==========================================================================
void
   OPLSynth::
   Opl3_NoteOff(BYTE bPatch, BYTE bNote, BYTE bChannel, BYTE bSustain)
{
   patchStruct *lpPS = glpPatch + (BYTE)bPatch; ;
   WORD         wTemp, wTemp2 ;
   BYTE         b4Op = (BYTE)(lpPS->note.bOp != PATCH_1_2OP);

   // Find the note slot
   wTemp = Opl3_FindFullSlot( bNote, bChannel ) ;
   
   // Remove note instance
   for (std::vector<BYTE>::iterator it = m_noteHistory[bChannel].begin();
         it != m_noteHistory[bChannel].end();
         ++it)
   {
      if ((*it) == bNote)
      {
         m_noteHistory[bChannel].erase(it);
         break;
      }
   }

   if (wTemp != 0xffff)
   {
      if (bSustain)
      {
         // This channel is sustained, don't really turn the note off,
         // just flag it.
         m_Voice[wTemp].bSusHeld = 1;
         if (b4Op)
         {
            wTemp2 = (lpPS->note.bOp == PATCH_2_2OP) ?
               Opl3_FindSecondVoice((BYTE)wTemp, m_Voice[wTemp].bVoiceID) :
               wTemp + 3;
            if (wTemp2 != (WORD)~0) 
               m_Voice[wTemp2].bSusHeld = 1;
         }
         return;
      }

      // if 2x2op / 4-op patch
      switch(lpPS->note.bOp)
      {
         case PATCH_1_4OP: // note cut on second voice is not necessary but need to be verified.
            wTemp2 = /*(lpPS->note.bAtC0[1] & 0x1) ? Opl3_FindSecondVoice((BYTE)wTemp, m_Voice[wTemp].bVoiceID) :*/ (wTemp+3);

            Opl3_CutVoice((BYTE)wTemp, FALSE);

            /*if (wTemp2 != (WORD)~0 && (lpPS->note.bAtC0[1] & 0x1))
               Opl3_CutVoice((BYTE)wTemp2, FALSE);
            else*/
            {
               m_Voice[wTemp+3].bOn = FALSE;
               m_Voice[wTemp+3].bSusHeld = FALSE;
               m_Voice[wTemp+3].dwTime = ++m_dwCurTime;
            }
            break;

         case PATCH_2_2OP:
            // obtain voice with identical binding ID
            wTemp2 = Opl3_FindSecondVoice((BYTE)wTemp, m_Voice[wTemp].bVoiceID);
            
            Opl3_CutVoice((BYTE)wTemp, FALSE);

            if (wTemp2 != (WORD)~0)
            {
               Opl3_CutVoice((BYTE)wTemp2, FALSE);
               break;
            }

         // fall through

         case PATCH_1_2OP:
            // shut off the note portion
            // we have the note slot, turn it off.
            Opl3_CutVoice((BYTE)wTemp, FALSE);
            break;
      }

      
      
   }
}

// ==========================================================================
//  WORD Opl3_FindSecondVoice
//
//  Description:
//     This finds a slot which shares the same voice ID.
//     Needed particularly for 4op patches.
//
//  Parameters:
//     BYTE bFirstVoice
//        Voice number of the first pair
//
//     BYTE bVoiceID
//        Voice ID to find
//
//  Return Value:
//     WORD
//        Second Voice number, or ~0 if not found.
//
//
// ==========================================================================
WORD 
   OPLSynth::
   Opl3_FindSecondVoice(BYTE bFirstVoice, BYTE bVoiceID)
{
   for (WORD i = 0; i < NUM2VOICES; i++)
   {
      if (i == bFirstVoice) continue;

      if (bVoiceID == m_Voice[ i ].bVoiceID)
         return i;

      // couldn't find it
   }
   return (WORD)~0;
}


// ==========================================================================
//  WORD Opl3_FindFullSlot
//
//  Description:
//     This finds a slot with a specific note, and channel.
//     If it is not found then 0xFFFF is returned.
//
//  Parameters:
//     BYTE bNote
//        MIDI note number
//
//     BYTE bChannel
//        MIDI channel #
//
//  Return Value:
//     WORD
//        note slot #, or 0xFFFF if can't find it
//
//
// ==========================================================================
WORD
   OPLSynth::
   Opl3_FindFullSlot
   (
   BYTE            bNote,
   BYTE            bChannel
   )
{
   WORD  i ;

   for (i = 0; i < NUM2VOICES; i++)
   {
      if ((bChannel == m_Voice[ i ].bChannel)
         && (bNote == m_Voice[ i ].bNote)
         && (m_Voice[ i ].bOn)
         && !(m_Voice[ i ].bSusHeld))
      {
         return ( i ) ;
      }
      // couldn't find it
   }
   return ( 0xFFFF ) ;
}

//------------------------------------------------------------------------
//  void Opl3_FMNote
//
//  Description:
//     Turns on an FM-synthesizer note.
//
//  Parameters:
//     WORD wNote
//        the note number from 0 to NUMVOICES
//
//     noteStruct *lpSN
//        structure containing information about what
//        is to be played.
//
//     BYTE bChannel
//        Current MIDI channel of note (needed for mono legato mode)
//
//     WORD wNote2
//        the note number from 0 to NUMVOICES (2nd pair)
//        (ignored if NS->bOp == PATCH_1_2OP)
//
//  Return Value:
//     Nothing.
//------------------------------------------------------------------------
void
   OPLSynth::
   Opl3_FMNote(WORD wNote, noteStruct * lpSN, BYTE bChannel, WORD wNote2)
{
   WORD            i ;
   WORD            wOffset ;
   operStruct      *lpOS ;
   BYTE            b4Op = (BYTE)(lpSN->bOp != PATCH_1_2OP);

   // Do not send note off to allow for mono mode legato
   if ( !(m_wMonoMode & (1<<bChannel)) )
   {
      // write out a note off, just to make sure...
      wOffset = wNote;
      if (wNote >= (NUM2VOICES / 2))
         wOffset += (0x100 - (NUM2VOICES / 2));

      Opl3_ChipWrite(AD_BLOCK + wOffset, 0 ) ;

      // needed for 2x2op patches
      if (b4Op && lpSN->bOp == PATCH_2_2OP)
      {
         wOffset = wNote2;
         if (wNote2 >= (NUM2VOICES / 2))
            wOffset += (0x100 - (NUM2VOICES / 2));

         Opl3_ChipWrite(AD_BLOCK + wOffset, 0 ) ;
      }
   }

   // TODO: Switch between 4-op mode enables.

   // writing the operator information

   //for (i = 0; i < (WORD)((wNote < NUM4VOICES) ? NUMOPS : 2); i++)
   for (i = 0; i < NUMOPS; i++)
   {
      if (i == 2 && !b4Op)
         break;

      lpOS = &lpSN -> op[ i ] ;
      wOffset = 
         (i < 2) ? gw2OpOffset[ wNote ][ i ] :
         gw2OpOffset[ wNote2 ][ (i-2) ] ;

      Opl3_ChipWrite( 0x20 + wOffset, lpOS -> bAt20) ;
      Opl3_ChipWrite( 0x40 + wOffset, lpOS -> bAt40) ;
      Opl3_ChipWrite( 0x60 + wOffset, lpOS -> bAt60) ;
      Opl3_ChipWrite( 0x80 + wOffset, lpOS -> bAt80) ;
      Opl3_ChipWrite( 0xE0 + wOffset, lpOS -> bAtE0) ;

   }

   // write out the voice information
   wOffset = (wNote < 9) ? wNote : (wNote + 0x100 - 9) ;
   Opl3_ChipWrite(0xa0 + wOffset, lpSN -> bAtA0[ 0 ] ) ;
   Opl3_ChipWrite(0xc0 + wOffset, lpSN -> bAtC0[ 0 ] ) ;

   // Note on...
   Opl3_ChipWrite(0xb0 + wOffset,
      (BYTE)(lpSN -> bAtB0[ 0 ] | 0x20) ) ;

   if (b4Op && (lpSN->bOp == PATCH_2_2OP /*|| (lpSN->bOp == PATCH_1_4OP && (lpSN->bAtC0[1] & 0x1))*/))
   {
      wOffset = (wNote2 < 9) ? wNote2 : (wNote2 + 0x100 - 9) ;
      Opl3_ChipWrite(0xa0 + wOffset, lpSN -> bAtA0[ 1 ] ) ;
      Opl3_ChipWrite(0xc0 + wOffset, lpSN -> bAtC0[ 1 ] ) ;

      Opl3_ChipWrite(0xb0 + wOffset,
         (BYTE)(lpSN -> bAtB0[ 1 ] | 0x20) ) ;
   }

} // end of Opl3_FMNote()

//=======================================================================
//  WORD Opl3_NoteOn
//
//  Description:
//     This turns on a note, including drums with a patch # of the
//     drum note + 0x80.  The first GM drum instrument is mapped to note 35 instead of zero, though, so
//     we expect 0 as the first drum patch (acoustic kick) if note 35 comes in.
//
//  Parameters:
//     BYTE bPatch
//        MIDI patch
//
//     BYTE bNote
//        MIDI note
//
//     BYTE bChannel
//        MIDI channel
//
//     BYTE bVelocity
//        velocity value
//
//     short iBend
//        current pitch bend from -32768 to 32767
//
//  Return Value:
//     WORD
//        note slot #, or 0xFFFF if it is inaudible
//=======================================================================
void
   OPLSynth::
   Opl3_NoteOn(BYTE bPatch, BYTE bNote, BYTE bChannel, BYTE bVelocity, long iBend)
{
   WORD             wTemp, i, j, wTemp2 = ~0 ;
   BYTE             b4Op, /*bTemp, */bMode, bStereo ;
   patchStruct      *lpPS ;
   DWORD            /*dwBasicPitch, */dwPitch[ 2 ] ;
   noteStruct       NS;
  
   // Increment voice allocation ID (needed for pairing operator pairs for 2x2op patches)
   static BYTE      bVoiceID = 0;

   // Get a pointer to the patch
   lpPS = glpPatch + bPatch ;
   // Find out the basic pitch according to our
   // note value.  This may be adjusted because of
   // pitch bends or special qualities for the note.

   /*dwBasicPitch = gdwPitch[ bNote % 12 ] ;
   bTemp = bNote / (BYTE) 12 ;
   if (bTemp > (BYTE) (60 / 12))
      dwBasicPitch = AsLSHL( dwBasicPitch, (BYTE)(bTemp - (BYTE)(60/12)) ) ;
   else if (bTemp < (BYTE) (60/12))
      dwBasicPitch = AsULSHR( dwBasicPitch, (BYTE)((BYTE) (60/12) - bTemp) ) ;*/

   // if blank patch, ignore completely
   if (Opl3_IsPatchEmpty(bPatch))
      return;

   // Copy the note information over and modify
   // the total level and pitch according to
   // the velocity, midi volume, and tuning.

   RtlCopyMemory( (LPSTR) &NS, (LPSTR) &lpPS -> note, sizeof( noteStruct ) ) ;
   
   // TODO: 4op patch mode
   b4Op = (BYTE)(NS.bOp != PATCH_1_2OP) ;

   for (j = 0; j < 2; j++)
   {
      // modify pitch
      /*dwPitch[ j ] = dwBasicPitch ;*/
      /*bTemp = (BYTE)((NS.bAtB0[ j ] >> 2) & 0x07) ;
      if (bTemp > 4)
         dwPitch[ j ] = AsLSHL( dwPitch[ j ], (BYTE)(bTemp - (BYTE)4) ) ;
      else if (bTemp < 4)
         dwPitch[ j ] = AsULSHR( dwPitch[ j ], (BYTE)((BYTE)4 - bTemp) ) ;*/

      //wTemp = Opl3_CalcFAndB( Opl3_CalcBend( dwPitch[ j ], iBend ) ) ;
      wTemp = Opl3_MIDINote2FNum(bNote, bChannel, 0);
      NS.bAtA0[ j ] = (BYTE) wTemp ;
      NS.bAtB0[ j ] = (BYTE) 0x20 | (BYTE) (wTemp >> 8) ;
   }
   // Modify level for each operator, but only
   // if they are carrier waves

   bMode = (BYTE) ((NS.bAtC0[ 0 ] & 0x01) * 2 + 4) ;

   for (i = 0; i < NUMOPS; i++)
   {
      if (!b4Op && i >= 2)
         break;

      wTemp = (BYTE)
         Opl3_CalcVolume(  (BYTE)(NS.op[ i ].bAt40 & (BYTE) 0x3f),
         bChannel,
         bVelocity,
         (BYTE) i,
         bMode ) ;
      NS.op[ i ].bAt40 = (NS.op[ i ].bAt40 & (BYTE)0xc0) | (BYTE) wTemp ;
   }

   // Do stereo panning, but cutting off a left or
   // right channel if necessary...

   bStereo = Opl3_CalcStereoMask( bChannel ) ;
   NS.bAtC0[ 0 ] &= bStereo ;

   // Check if mono mode is set.
   wTemp = 0;
   if ((m_wMonoMode & (1<<bChannel)) > 0)
   {
      // Is last voice used if it indeed is used by this channel
      // else find a new slot.
      // TODO: special case needed for PATCH_1_4OP

      //wTemp = m_Voice[m_bLastVoiceUsed[bChannel]].bChannel;
      wTemp = m_bLastVoiceUsed[bChannel];
      
      // safety measure for pure 4-op patches
      if (NS.bOp == PATCH_1_4OP && ((wTemp > 2 && wTemp < 9) || wTemp > 11))
      {
         bool is4OpVoice = false;

         for (i = 0; i < NUM4VOICES; ++i)
         {
            if (gb4OpVoices[i] == wTemp)
            {
               is4OpVoice = true;
               break;
            }
         }

         if (!is4OpVoice)
         {
            wTemp = Opl3_FindEmptySlot4Op(bPatch);
         }
      }

      // Check if last channel used else find one.
      else if (NS.bOp != PATCH_1_4OP)
      {
         wTemp = (wTemp == bChannel) ? wTemp : 
                 //(NS.bOp == PATCH_1_4OP) ? Opl3_FindEmptySlot4Op(bPatch) :
                 Opl3_FindEmptySlot(bPatch);
         m_Voice[wTemp].bPrevNote = m_Voice[wTemp].bNote;
      }
      
      m_Voice[wTemp].bPortaSampTime = m_bPortaTime[bChannel];
      m_Voice[wTemp].bPortaSampCnt = m_bPortaTime[bChannel];
      m_Voice[wTemp].bPrevNote = m_Voice[wTemp].bNote;
      if (bNote == m_Voice[wTemp].bNote)
         m_Voice[wTemp].bPortaSampCnt = 0;

      /*if (b4Op)
      {
         wTemp2 = Opl3_FindSecondVoice((BYTE)wTemp, m_Voice[m_bLastVoiceUsed[bChannel]].bVoiceID);
         wTemp2 = (wTemp2 != (WORD)~0) ? wTemp2 : Opl3_FindEmptySlot( bPatch );
      }*/
      // TODO: configuration flag for locking to 1:1 channel mapping.
   } 
   
   else
   {
      // Find an empty slot, and use it...
      wTemp = (NS.bOp == PATCH_1_4OP /*&& (NS.bAtC0[1] & 0x1) == 0*/) ?
         Opl3_FindEmptySlot4Op(bPatch) : Opl3_FindEmptySlot( bPatch );
   }

   m_Voice[ wTemp ].bNote = bNote ;
   m_Voice[ wTemp ].bChannel = bChannel ;
   m_Voice[ wTemp ].bPatch = bPatch ;
   m_Voice[ wTemp ].bVelocity = bVelocity ;
   m_Voice[ wTemp ].bOn = TRUE ;
   m_Voice[ wTemp ].dwOrigPitch[0] = dwPitch[ 0 ] ;  // not including bend
   m_Voice[ wTemp ].dwOrigPitch[1] = dwPitch[ 1 ] ;  // not including bend
   m_Voice[ wTemp ].bBlock[0] = NS.bAtB0[ 0 ] ;
   m_Voice[ wTemp ].bBlock[1] = NS.bAtB0[ 1 ] ;
   m_Voice[ wTemp ].bSusHeld = 0;
   m_Voice[ wTemp ].dwTime = ++m_dwCurTime ;
   m_Voice[ wTemp ].dwStartTime = m_dwCurSample;
   m_Voice[ wTemp ].dwLFOVal = 0;
   m_Voice[ wTemp ].dwDetuneEG = 0;

   if (b4Op)
   {
      switch(lpPS->note.bOp)
      {
         case PATCH_2_2OP:
            wTemp2 = ((m_wMonoMode & (1<<bChannel)) > 0) ? // Get corresponding voice last used
               Opl3_FindSecondVoice((BYTE)wTemp, m_Voice[m_bLastVoiceUsed[bChannel]].bVoiceID) : 
               ~0;
            wTemp2 = (wTemp2 != (WORD)~0) ? wTemp2 : Opl3_FindEmptySlot( bPatch );
            Opl3_Set4OpFlag((BYTE)wTemp, false, PATCH_2_2OP);
            Opl3_Set4OpFlag((BYTE)wTemp2, false, PATCH_2_2OP);

            if ((m_wMonoMode & (1<<bChannel)) > 0)
            {
               m_Voice[wTemp2].bPrevNote = m_Voice[wTemp2].bNote;
               m_Voice[wTemp2].bPortaSampTime = m_bPortaTime[bChannel];
               m_Voice[wTemp2].bPortaSampCnt = m_bPortaTime[bChannel];
               if (m_Voice[wTemp2].bPrevNote == m_Voice[wTemp2].bNote)
                  m_Voice[wTemp2].bPortaSampCnt = 0;
               
            }
            break;

         case PATCH_1_4OP:
            /*if ((NS.bAtC0[1] & 0x1) == 0)*/
            {
               wTemp2 = wTemp + 3; // TODO check if correct voice distance

               // set 4-op mode for this channel
               Opl3_Set4OpFlag((BYTE)wTemp, true, PATCH_1_4OP);

               break;
            }
            /*else
            {
               wTemp2 = ((m_wMonoMode & (1<<bChannel)) > 0) ? // Get corresponding voice last used
               Opl3_FindSecondVoice((BYTE)wTemp, m_Voice[m_bLastVoiceUsed[bChannel]].bVoiceID) : 
               ~0;
               wTemp2 = (wTemp2 != (WORD)~0) ? wTemp2 : Opl3_FindEmptySlot( bPatch );
               
               Opl3_Set4OpFlag((BYTE)wTemp, FALSE);
               break;
            }*/
      }

      m_Voice[ wTemp2 ].bNote = bNote ;
      m_Voice[ wTemp2 ].bChannel = bChannel ;
      m_Voice[ wTemp2 ].bPatch = bPatch ;
      m_Voice[ wTemp2 ].bVelocity = bVelocity ;
      m_Voice[ wTemp2 ].bOn = TRUE ;
      m_Voice[ wTemp2 ].dwOrigPitch[0] = dwPitch[ 0 ] ;  // not including bend
      m_Voice[ wTemp2 ].dwOrigPitch[1] = dwPitch[ 1 ] ;  // not including bend
      m_Voice[ wTemp2 ].bBlock[0] = NS.bAtB0[ 0 ] ;
      m_Voice[ wTemp2 ].bBlock[1] = NS.bAtB0[ 1 ] ;
      m_Voice[ wTemp2 ].bSusHeld = 0;
      m_Voice[ wTemp2 ].dwTime = ++m_dwCurTime ;
      m_Voice[ wTemp2 ].dwStartTime = m_dwCurSample;
      m_Voice[ wTemp2 ].dwLFOVal = 0;
      m_Voice[ wTemp2 ].dwDetuneEG = 0;
   }
   else
      Opl3_Set4OpFlag((BYTE)wTemp, false, PATCH_1_2OP);
   
   // Send data
   Opl3_CalcPatchModifiers(&NS, bChannel);
   Opl3_FMNote(wTemp, &NS, bChannel, wTemp2 ) ; // TODO refactor functionality to insert second operator

   m_bLastVoiceUsed[bChannel] = (BYTE)wTemp; // save voice ref
   m_Voice[ wTemp ].bVoiceID = ++bVoiceID;
   if (b4Op)
   {
      m_Voice[ wTemp2 ].bVoiceID = bVoiceID;
   }

   // Add to note history as most recent note
   m_noteHistory[bChannel].push_back(bNote);

} // end of Opl3_NoteOn()

void
   OPLSynth::
   Opl3_Set4OpFlag(BYTE bVoice, bool bSetFlag, BYTE bOp)
{
   BYTE cur4OpVoiceSet = b4OpVoiceSet;

   for (int i = 0; i < NUM4VOICES; ++i)
   {
      if (gb4OpVoices[i] == (BYTE)bVoice)
      {
         // Unset voice if not 4op
         if (bOp != PATCH_1_4OP && (cur4OpVoiceSet & (1<<i)) > 0)
            Opl3_CutVoice(gb4OpVoices[i], TRUE);

         // Unset relevant voices if 4op
         /*else if (bOp == PATCH_1_4OP && (cur4OpVoiceSet & (i<<1)) > 0)
         {
            //if (m_Voice[bVoice].bOn)
            Opl3_CutVoice(gb4OpVoices[i], TRUE);

            //if (m_Voice[bVoice+3].bOn)
            Opl3_CutVoice(gb4OpVoices[i]+3, TRUE);
         }*/

         // Update local register
         b4OpVoiceSet = (bSetFlag) ?
            (b4OpVoiceSet | (1<<i)) :
            (b4OpVoiceSet & ~(1<<i)) ;

         // Write 4op enable/disable register to chip
         if (cur4OpVoiceSet != b4OpVoiceSet)
            Opl3_ChipWrite(AD_CONNECTION, (BYTE)(b4OpVoiceSet)) ;
         
         break;
      }

      // this will always be to unset since FindEmptySlot4Op only looks for first pair
      else if (gb4OpVoices[i]+3 == (BYTE)bVoice/* && !bSetFlag*/)
      {
         // Cut base voice to be safe (?)
         if (bOp != PATCH_1_4OP && (cur4OpVoiceSet & (1<<i)) > 0)
            Opl3_CutVoice(gb4OpVoices[i], TRUE);

         // Update local register
         b4OpVoiceSet &= ~(1<<i);

         // Write 4op enable/disable register to chip
         if (cur4OpVoiceSet != b4OpVoiceSet)
            Opl3_ChipWrite(AD_CONNECTION, (BYTE)(b4OpVoiceSet)) ;
         break;
      }
   }
}

bool 
   OPLSynth::
   Opl3_IsPatchEmpty(BYTE bPatch)
{
   noteStruct *lpPS = &((glpPatch + bPatch)->note);
   DWORD isEmpty = 0;

   for (BYTE i = 0; i < NUMOPS; ++i)
   {
       isEmpty += (lpPS->op[i].bAt20 + lpPS->op[i].bAt40 
                 + lpPS->op[i].bAt60 + lpPS->op[i].bAt80
                 + lpPS->op[i].bAtE0);
   }

   //isEmpty += (lpPS->bAtA0[0] + lpPS->bAtB0[0] + lpPS->bAtC0[0]);
   //isEmpty += (lpPS->bAtA0[1] + lpPS->bAtB0[1] + lpPS->bAtC0[1]);

   return (isEmpty == 0);
}

void
   OPLSynth::
   Opl3_CalcPatchModifiers(noteStruct *lpSN, BYTE bChannel)
{
   char bTemp;
   char bOffset;

   //Attack
   bOffset = (char)lin_intp(m_bAttack[bChannel], 0, 127, (-8), 8);
   if (bOffset)
   {
      for (int i = 0; i < NUMOPS; i+=2)
      {
         if (i == 2 && lpSN->bOp == PATCH_1_2OP)
            break;

         bTemp = ((lpSN->op[1+i].bAt60 & 0xF0)>>4);
         lpSN->op[1+i].bAt60 &= ~0xF0;
         bTemp = (char)bTemp - bOffset;
         lpSN->op[1+i].bAt60 |= (bTemp > 0xF) ? 0xF0 : (bTemp > 0) ? (bTemp<<4) : 0;

         // If AM mode (assuming 2op)
         if (((lpSN->bAtC0[0+i]) & 0x1) == 1)
         {
            bTemp = ((lpSN->op[0].bAt60 & 0xF0)>>4);
            lpSN->op[0+i].bAt60 &= ~0xF0;
            bTemp = (char)bTemp - bOffset;
            lpSN->op[0+i].bAt60 |= (bTemp > 0xF) ? 0xF0 : (bTemp > 0) ? (bTemp<<4) : 0;
         }
      }
   }

   //Release
   bOffset = (char)lin_intp(m_bRelease[bChannel], 0, 127, (-8), 8);
   if (bOffset)
   {
      for (int i = 0; i < NUMOPS; ++i)
      {
         if (i == 2 && lpSN->bOp == PATCH_1_2OP)
            break;

         bTemp = lpSN->op[i].bAt80 & 0xF;
         lpSN->op[i].bAt80 &= ~0xF;
         bTemp = (char)bTemp - bOffset;
         lpSN->op[i].bAt80 |= (bTemp > 0xF) ? 0xF : (bTemp > 0) ? bTemp : 0;
      }
   }

   //Brightness
   bOffset = (char)lin_intp(m_bBrightness[bChannel], 0, 127, (-32), 32);
   if (bOffset)
   {
      for (int i = 0; i < NUMOPS; i+=2)
      {
         if (i == 2 && lpSN->bOp == PATCH_1_2OP)
            break;

         bTemp = lpSN->op[0+i].bAt40 & 0x3F;  // retain output level (0 = loud, 0x3f = quiet)
         lpSN->op[0+i].bAt40 &= ~0x3F;        // clear old output level
         bTemp = (char)bTemp - bOffset;
         lpSN->op[0+i].bAt40 |= (bTemp > 0x3F) ? 0x3F : (bTemp > 0) ? bTemp : 0;
      }
   }
}

void
   OPLSynth::
   Opl3_ProcessDataEntry(BYTE val, BYTE bChannel)
{
   WORD rpn = (WORD)(m_RPN[bChannel][0])|(m_RPN[bChannel][1] << 8) & (WORD)(0xFF);
   DWORD dwTemp;

   // Pitch Bend Range extension
   if (rpn == (WORD)0x0000)
   {
      // Calculate base bend value then apply

      // TODO determine what's wrong - how is it becoming 0 at all?
      m_iBendRange[bChannel] = (!m_iBendRange[bChannel]) ? 2 : m_iBendRange[bChannel];

      dwTemp = ((long)m_iBend[bChannel] / m_iBendRange[bChannel]);
      m_iBendRange[bChannel] = val & 0x7f;
      dwTemp *= m_iBendRange[bChannel];
      m_iBend[bChannel] = (long) (dwTemp);

      Opl3_PitchBend(bChannel, m_iBend[bChannel]);
   }
         
}

//=======================================================================
//Opl3_CalcFAndB - Calculates the FNumber and Block given a frequency.
//
//inputs
//       DWORD   dwPitch - pitch
//returns
//        WORD - High byte contains the 0xb0 section of the
//                        block and fNum, and the low byte contains the
//                        0xa0 section of the fNumber
//=======================================================================
//WORD
//   OPLSynth::
//   Opl3_CalcFAndB(DWORD dwPitch)
//{
//   BYTE    bBlock;
//
//   /* bBlock is like an exponential to dwPitch (or FNumber) */
//   for (bBlock = 1; dwPitch >= 0x400; dwPitch >>= 1, bBlock++)
//      ;
//
//   if (bBlock > 0x07)
//      bBlock = 0x07;  /* we cant do anything about this */
//
//   /* put in high two bits of F-num into bBlock */
//   return ((WORD) bBlock << 10) | (WORD) dwPitch;
//}

//=======================================================================
//Opl3_CalcBend - This calculates the effects of pitch bend
//        on an original value.
//
//inputs
//        DWORD   dwOrig - original frequency
//        short   iBend - from -32768 to 32768, -2 half steps to +2
//returns
//        DWORD - new frequency
//=======================================================================
//DWORD
//   OPLSynth::
//   Opl3_CalcBend (DWORD dwOrig, long iBend)
//{
//   /* do different things depending upon positive or
//   negative bend */
//   DWORD dw;
//
//   if (iBend > 0)
//   {
//      dw = (DWORD)((iBend * (LONG)(256.0 * (EQUAL * EQUAL - 1.0))) >> 8);
//      dwOrig += (DWORD)(AsULMUL(dw, dwOrig) >> 15);
//   }
//   else if (iBend < 0)
//   {
//      dw = (DWORD)(((-iBend) * (LONG)(256.0 * (1.0 - 1.0 / EQUAL / EQUAL))) >> 8);
//      dwOrig -= (DWORD)(AsULMUL(dw, dwOrig) >> 15);
//   }
//
//   return dwOrig;
//}

//=======================================================================
// Opl3_CalcVolume - This calculates the attenuation for an operator.
//
//inputs
//        BYTE    bOrigAtten - original attenuation in 0.75 dB units
//        BYTE    bChannel - MIDI channel
//        BYTE    bVelocity - velocity of the note
//        BYTE    bOper - operator number (from 0 to 3)
//        BYTE    bMode - voice mode (from 0 through 7 for
//                                modulator/carrier selection)
//returns
//        BYTE - new attenuation in 0.75 dB units, maxing out at 0x3f.
//=======================================================================
BYTE
   OPLSynth::
   Opl3_CalcVolume(BYTE bOrigAtten,BYTE bChannel,BYTE bVelocity,BYTE bOper,BYTE bMode)
{
   BYTE        bVolume;
   WORD        wTemp;
   WORD        wMin;

   switch (bMode) {
   case 0:
      bVolume = (BYTE)(bOper == 3);
      break;
   case 1:
      bVolume = (BYTE)((bOper == 1) || (bOper == 3));
      break;
   case 2:
      bVolume = (BYTE)((bOper == 0) || (bOper == 3));
      break;
   case 3:
      bVolume = (BYTE)(bOper != 1);
      break;
   case 4:
      bVolume = (BYTE)((bOper == 1) || (bOper == 3));
      break;
   case 5:
      bVolume = (BYTE)(bOper >= 1);
      break;
   case 6:
      bVolume = (BYTE)(bOper <= 2);
      break;
   case 7:
      bVolume = TRUE;
      break;
   default:
      bVolume = FALSE;
      break;
   };
   if (!bVolume)
      return bOrigAtten; /* this is a modulator wave */

   wMin =(m_wSynthAttenL < m_wSynthAttenR) ? m_wSynthAttenL : m_wSynthAttenR;
   wTemp = bOrigAtten +
      ((wMin << 1) +
      m_bChanAtten[bChannel] +
      gbVelocityAtten[bVelocity >> 1]);
   return (wTemp > 0x3f) ? (BYTE) 0x3f : (BYTE) wTemp;
}

// ===========================================================================
// Opl3_ChannelNotesOff - turn off all notes on a channel
// ===========================================================================
void
   OPLSynth::
   Opl3_ChannelNotesOff(BYTE bChannel)
{
   int i;

   for (i = 0; i < NUM2VOICES; i++)
   {
      if ((m_Voice[ i ].bOn) && (m_Voice[ i ].bChannel == bChannel))
      {
         //Opl3_NoteOff(m_Voice[i].bPatch, m_Voice[i].bNote,m_Voice[i].bChannel, 0) ;
         Opl3_CutVoice(i, TRUE);
      }
   }
}

// ===========================================================================
/* Opl3_ChannelVolume - set the volume level for an individual channel.
*
* inputs
*      BYTE    bChannel - channel number to change
*      WORD    wAtten  - attenuation in 1.5 db units
*
* returns
*      none
*/
// ===========================================================================
void
   OPLSynth::
   Opl3_ChannelVolume(BYTE bChannel, WORD wAtten)
{
   m_bChanAtten[bChannel] = (BYTE)wAtten;

   Opl3_SetVolume(bChannel);
}

// ===========================================================================
//  void Opl3_SetVolume
//
//  Description:
//     This should be called if a volume level has changed.
//     This will adjust the levels of all the playing voices.
//
//  Parameters:
//     BYTE bChannel
//        channel # of 0xFF for all channels
//
//  Return Value:
//     Nothing.
//
// ===========================================================================
void
   OPLSynth::
   Opl3_SetVolume
   (
   BYTE   bChannel
   )
{
   WORD            i, j, wTemp, wOffset ;
   noteStruct      *lpPS;
   operStruct      opSt;
   BYTE            bMode, bStereo, bOffset ;
   char            bTemp;

   // Make sure that we are actually open...
   if (!glpPatch)
      return ;

   // Loop through all the notes looking for the right
   // channel.  Anything with the right channel gets
   // its pitch bent.
   for (i = 0; i < NUM2VOICES; i++)
   {
      if ((m_Voice[ i ].bChannel == bChannel) || (bChannel == 0xff))
      {
         // Get a pointer to the patch
         lpPS = &(glpPatch + m_Voice[ i ].bPatch) -> note ;

         // Modify level for each operator, IF they are carrier waves...
         bMode = (BYTE) ( (lpPS->bAtC0[0] & 0x01) * 2 + 4);

         for (j = 0; j < 2; ++j)
         {
            // Do not attempt to modify second pair for 2x2op; this will be picked up later.
            //if (j == 2 && lpPS->bOp == PATCH_1_2OP)
            //   break;

            // Copy to buffer
            RtlCopyMemory( (LPSTR) &opSt, (LPSTR) &(lpPS->op[j]), sizeof( operStruct ) ) ;

            wTemp = (BYTE) Opl3_CalcVolume(
               //(BYTE) (lpPS -> op[j].bAt40 & (BYTE) 0x3f),
               (BYTE) (opSt.bAt40 & (BYTE) 0x3f),
               m_Voice[i].bChannel, m_Voice[i].bVelocity,
               (BYTE) j,            bMode ) ;

            // Brightness update  TODO: refactor
            bOffset = (char)lin_intp(m_bBrightness[bChannel], 0, 127, (-32), 32);
            if (bOffset != 0 && j==0)
            {
               bTemp = lpPS->op[j].bAt40 & 0x3F;  // retain output level (0 = loud, 0x3f = quiet)
               opSt.bAt40 &= ~0x3F;        // clear old output level
               bTemp = (char)wTemp - bOffset;
               wTemp = (bTemp > (BYTE)0x3F) ? (WORD)0x3F : (bTemp > (BYTE)0) ? (WORD)bTemp : (WORD)0;
            }

            opSt.bAt40 = (opSt.bAt40 & (BYTE)0xc0) | (BYTE) wTemp;

            // Write new value.
            wOffset = gw2OpOffset[ i ][ j ] ;
            Opl3_ChipWrite(
               0x40 + wOffset,
               //(BYTE) ((lpPS -> op[j].bAt40 & (BYTE)0xc0) | (BYTE) wTemp) ) ;
               (BYTE) (opSt.bAt40) ) ;
         }

         // Do stereo pan, but cut left or right channel if needed.
         bStereo = Opl3_CalcStereoMask( m_Voice[ i ].bChannel ) ;
         wOffset = i;
         if (i >= (NUM2VOICES / 2))
            wOffset += (0x100 - (NUM2VOICES / 2));
         Opl3_ChipWrite(0xc0 + wOffset, (BYTE)(lpPS -> bAtC0[ 0 ] & bStereo) ) ;
      }
   }
} // end of Opl3_SetVolume

// ===========================================================================
// Opl3_SetPan - set the left-right pan position.
//
// inputs
//      BYTE    bChannel - channel number to alter
//      BYTE    bPan     - 0-47 for left, 81-127 for right, or somewhere in the middle.
//
// returns - none
//
//  As a side note, I think it's odd that (since 64 = CENTER, 127 = RIGHT and 0 = LEFT)
//  there are 63 intermediate gradations for the left side, but 62 for the right.
// ===========================================================================
void
   OPLSynth::
   Opl3_SetPan(BYTE bChannel, BYTE bPan)
{
   /* change the pan level */
   if (bPan > (64 + 16))
      m_bStereoMask[bChannel] = 0xef;      /* let only right channel through */
   else if (bPan < (64 - 16))
      m_bStereoMask[bChannel] = 0xdf;      /* let only left channel through */
   else
      m_bStereoMask[bChannel] = 0xff;      /* let both channels */

   /* change any curently playing patches */
   Opl3_SetVolume(bChannel);
}


// ===========================================================================
//  void Opl3_PitchBend
//
//  Description:
//     This pitch bends a channel.
//
//  Parameters:
//     BYTE bChannel
//        channel
//
//     short iBend
//        values from -32768 to 32767, being -2 to +2 half steps
//
//  Return Value:
//     Nothing.
// ===========================================================================
void
   OPLSynth::
   Opl3_PitchBend(BYTE bChannel, long iBend)
{
   WORD   i, wTemp[ 2 ], wOffset, j ;
//   DWORD  dwNew ;

   // Remember the current bend..
   m_iBend[ bChannel ] = iBend;

   // Loop through all the notes looking for
   // the correct channel.  Anything with the
   // correct channel gets its pitch bent...
   for (i = 0; i < NUM2VOICES; i++)
      if (m_Voice[ i ].bChannel == bChannel/* && (m_Voice [ i ].bOn || m_bSustain[bChannel])*/)
      {
         j = 0 ;
         //wTemp[ j ] = Opl3_CalcFAndB( Opl3_CalcBend( m_Voice[ i ].dwOrigPitch[ j ], iBend ) ) ;
         wTemp[ j ] = Opl3_MIDINote2FNum(m_Voice[ i ].bNote, bChannel, (m_Voice[ i ].dwLFOVal + m_Voice[ i ].dwDetuneEG));
         m_Voice[ i ].bBlock[ j ] =
            (m_Voice[ i ].bBlock[ j ] & (BYTE) 0xe0) |
            (BYTE) (wTemp[ j ] >> 8) ;

         wOffset = i;
         if (i >= (NUM2VOICES / 2))
            wOffset += (0x100 - (NUM2VOICES / 2));

         Opl3_ChipWrite(AD_BLOCK + wOffset, m_Voice[ i ].bBlock[ 0 ] ) ;
         Opl3_ChipWrite(AD_FNUMBER + wOffset, (BYTE) wTemp[ 0 ] ) ;
      }
} // end of Opl3_PitchBend

// ===========================================================================
//  void Opl3_MIDINote2FNum
//
//  Description:
//     Obtains FNumber from MIDI note + current bend values
//     Special thanks to ValleyBell for MidiPlay sources for adaption
//
//  Parameters:
//     double note   - MIDI note number (promoted from BYTE)
//     BYTE bChannel - channel
//     long dwLFOVal - current magnitude of the modulation waveform
//
//  Return Value:
//     ((WORD) BlockVal << 10) | (WORD) keyVal;
// ===========================================================================
WORD
   OPLSynth::
   Opl3_MIDINote2FNum(double note, BYTE bChannel, long dwLFOVal)
{
	double freqVal, curNote;
	signed char BlockVal;
	WORD keyVal;
//	signed long CurPitch;
	
   /*TODO: keep for later, may add other features */
	//CurPitch = //MMstTuning + TempMid->TunePb + TempMid->Pitch + TempMid->ModPb;
	
	curNote = (double)(note + (m_iBend[bChannel] + dwLFOVal) / 8192.0); //Note + CurPitch / 8192.0;
	freqVal = 440.0 * pow(2.0, (curNote - 69) / 12.0);
	//BlockVal = ((signed short int)CurNote / 12) - 1;
	BlockVal = ((signed short int)(curNote + 6) / 12) - 2;
	if (BlockVal < 0x00)
		BlockVal = 0x00;
	else if (BlockVal > 0x07)
		BlockVal = 0x07;
	//KeyVal = (unsigned short int)(FreqVal * pow(2, 20 - BlockVal) / CHIP_RATE + 0.5);
	keyVal = (WORD)(freqVal * (1 << (20 - BlockVal)) / FSAMP + 0.5);
	if (keyVal > 0x03FF)
		keyVal = 0x03FF;
	
	return (BlockVal << 10) | keyVal;	// << (8+2)
}


// ===========================================================================
//  Opl3_CalcStereoMask - This calculates the stereo mask.
//
//  inputs
//            BYTE  bChannel - MIDI channel
//  returns
//            BYTE  mask (for register 0xc0-c8) for eliminating the
//                  left/right/both channels
// ===========================================================================
BYTE
   OPLSynth::
   Opl3_CalcStereoMask(BYTE bChannel)
{
   WORD        wLeft, wRight;

   /* figure out the basic levels of the 2 channels */
   wLeft = (m_wSynthAttenL << 1) + m_bChanAtten[bChannel];
   wRight = (m_wSynthAttenR << 1) + m_bChanAtten[bChannel];

   /* if both are too quiet then mask to nothing */
   if ((wLeft > 0x3f) && (wRight > 0x3f))
      return 0xcf;

   /* if one channel is significantly quieter than the other than
   eliminate it */
   if ((wLeft + 8) < wRight)
      return (BYTE)(0xef & m_bStereoMask[bChannel]);   /* right is too quiet so eliminate */
   else if ((wRight + 8) < wLeft)
      return (BYTE)(0xdf & m_bStereoMask[bChannel]);   /* left too quiet so eliminate */
   else
      return (BYTE)(m_bStereoMask[bChannel]);  /* use both channels */
}

//------------------------------------------------------------------------
//  WORD Opl3_FindEmptySlot
//
//  Description:
//     This finds an empty note-slot for a MIDI voice.
//     If there are no empty slots then this looks for the oldest
//     off note.  If this doesn't work then it looks for the oldest
//     on-note of the same patch.  If all notes are still on then
//     this finds the oldests turned-on-note.
//
//  Parameters:
//     BYTE bPatch
//        MIDI patch that will replace it.
//
//  Return Value:
//     WORD
//        note slot #
//
//
//------------------------------------------------------------------------
WORD
   OPLSynth::
   Opl3_FindEmptySlot(BYTE bPatch)
{
   WORD   i, found ;
   DWORD  dwOldest ;

   // First, look for a slot with a time == 0
   for (i = 0;  i < NUM2VOICES; i++)
      if (!m_Voice[ i ].dwTime)
         return ( i ) ;

   // Now, look for a slot of the oldest off-note
   dwOldest = 0xffffffff ;
   found = 0xffff ;

   for (i = 0; i < NUM2VOICES; i++)
      if ((!m_Voice[ i ].bOn/* || !m_Voice[ i ].bSusHeld*/)
       && (m_Voice[ i ].dwTime < dwOldest))
      {
         dwOldest = m_Voice[ i ].dwTime ;
         found = i ;
      }
   if (found != 0xffff)
      return ( found ) ;

   // Now, look for a slot of the oldest note with
   // the same patch
   dwOldest = 0xffffffff ;
   found = 0xffff ;
   for (i = 0; i < NUM2VOICES; i++)
      if ((m_Voice[ i ].bPatch == bPatch) && (m_Voice[ i ].dwTime < dwOldest))
      {
         dwOldest = m_Voice[ i ].dwTime ;
         found = i ;
      }
   if (found != 0xffff)
      return ( found ) ;

   // Now, just look for the oldest voice
   found = 0 ;
   dwOldest = m_Voice[ found ].dwTime ;
   for (i = (found + 1); i < NUM2VOICES; i++)
      if (m_Voice[ i ].dwTime < dwOldest)
      {
         dwOldest = m_Voice[ i ].dwTime ;
         found = i ;
      }

    return ( found ) ;

} // end of Opl3_FindEmptySlot()

//------------------------------------------------------------------------
//  WORD Opl3_FindEmptySlot4Op
//
//  Description:
//     This finds an empty note-slot for a 4-op MIDI voice.
//     If there are no empty slots then this looks for the oldest
//     off note.  If this doesn't work then it looks for the oldest
//     on-note of the same patch.  If all notes are still on then
//     this finds the oldests turned-on-note.
//
//  Parameters:
//     BYTE bPatch
//        MIDI patch that will replace it.
//
//  Return Value:
//     WORD
//        note slot #
//
//
//------------------------------------------------------------------------
WORD
   OPLSynth::
   Opl3_FindEmptySlot4Op(BYTE bPatch)
{
   WORD   i, found ;
   DWORD  dwOldest ;

   // First, look for a slot with a time == 0
   for (i = 0;  i < NUM4VOICES; i++)
      if (!m_Voice[ gb4OpVoices[ i ] ].dwTime)
         return ( gb4OpVoices[ i ] ) ;

   // Now, look for a slot of the oldest off-note
   dwOldest = 0xffffffff ;
   found = 0xffff ;

   for (i = 0; i < NUM4VOICES; i++)
      if ((!m_Voice[ gb4OpVoices[ i ] ].bOn/* || !m_Voice[ gb4OpVoices[ i ] ].bSusHeld*/)
       && (m_Voice[ gb4OpVoices[ i ] ].dwTime < dwOldest))
      {
         dwOldest = m_Voice[ gb4OpVoices[ i ] ].dwTime ;
         found = gb4OpVoices[ i ] ;
      }

   if (found != 0xffff)
      return ( found ) ;

   // Now, look for a slot of the oldest note with
   // the same patch
   dwOldest = 0xffffffff ;
   found = 0xffff ;
   for (i = 0; i < NUM4VOICES; i++)
      if ((m_Voice[ gb4OpVoices[ i ] ].bPatch == bPatch) && (m_Voice[ gb4OpVoices[ i ] ].dwTime < dwOldest))
      {
         dwOldest = m_Voice[ gb4OpVoices[ i ] ].dwTime ;
         found = gb4OpVoices[ i ] ;
      }
   if (found != 0xffff)
      return ( found ) ;

   // Now, just look for the oldest voice
   found = 0 ;
   dwOldest = m_Voice[ gb4OpVoices[ found ] ].dwTime ;
   for (i = (found + 1); i < NUM4VOICES; i++)
      if (m_Voice[ gb4OpVoices[ i ] ].dwTime < dwOldest)
      {
         dwOldest = m_Voice[ gb4OpVoices[ i ] ].dwTime ;
         found = gb4OpVoices[ i ] ;
      }

   return ( found ) ;

} // end of Opl3_FindEmptySlot4Op()

//------------------------------------------------------------------------
//  WORD Opl3_SetSustain
//
//  Description:
//     Set the sustain controller on the current channel.
//
//  Parameters:
//     BYTE bSusLevel
//        The new sustain level
//
//
//------------------------------------------------------------------------
void
   OPLSynth::
   Opl3_SetSustain(BYTE bChannel,BYTE bSusLevel)
{
   BYTE i;

   if (m_bSustain[ bChannel ] && !bSusLevel)
   {
      // Sustain has just been turned off for this channel
      // Go through and turn off all notes that are being held for sustain
      //
      for (i = 0; i < NUM2VOICES; i++)
      {
         if ((bChannel == m_Voice[ i ].bChannel) &&
            m_Voice[ i ].bSusHeld)
         {
            // this is not guaranteed to cut repeated sustained notes
            //Opl3_NoteOff(m_Voice[i].bPatch, m_Voice[i].bNote, m_Voice[i].bChannel, 0);

            Opl3_CutVoice(i, FALSE);
         }
      }
   }
   m_bSustain[ bChannel ] = bSusLevel;
}

void
   OPLSynth::
   Opl3_SetPortamento(BYTE bChannel, BYTE bPortaTime)
{
   if ((m_wPortaMode & (1<<bChannel)) > 0)
   {
      m_bPortaTime[bChannel] = bPortaTime;

      // Set current pitch etc.

   }
}

void
   OPLSynth::
   Opl3_BoardReset()
{
   BYTE i;

   /* ---- silence the chip -------- */

   /* tell the FM chip to use 4-operator mode, and
   fill in any other random variables */
   Opl3_ChipWrite(AD_NEW, 0x01);
   Opl3_ChipWrite(AD_MASK, 0x60);
   Opl3_ChipWrite(AD_CONNECTION, 0x00);
   Opl3_ChipWrite(AD_NTS, 0x00);

   /* turn off the drums, and use high vibrato/modulation */
   Opl3_ChipWrite(AD_DRUM, 0xc0);

   /* turn off all the oscillators */
   for (i = 0; i <= 0x15; i++)
   {
      if ((i & 0x07) <= 0x05)
      {
         Opl3_ChipWrite(AD_LEVEL + i, 0x3f);
         Opl3_ChipWrite(AD_LEVEL2 + i, 0x3f);
      }
   };

   /* turn off all the voices */
   for (i = 0; i <= 0x08; i++)
   {
      Opl3_ChipWrite(AD_BLOCK + i, 0x00);
      Opl3_ChipWrite(AD_BLOCK2 + i, 0x00);
   };
}

//------------------------------------------------------------------------
//  void Opl3_CutNote
//
//  Description:
//     Routine to note off or note cut the FM voice channel
//
//  Parameters:
//     BYTE bVoice
//        The selected FM voice
//     
//     BYTE bIsInstantCut
//        Flag to indicate whether the note-off should be instant.
//
//------------------------------------------------------------------------
void 
   OPLSynth::
   Opl3_CutVoice(BYTE bVoice, BYTE bIsInstantCut)
{
   WORD wOffset = bVoice, wOpOffset;
   noteStruct *lpNS = &((glpPatch + m_Voice[bVoice].bPatch)->note);
   BYTE bOp = lpNS->bOp;

   if (bVoice >= (NUM2VOICES / 2))
      wOffset += (0x100 - (NUM2VOICES / 2));

   // set op1 sustain to high
   if (bIsInstantCut)
   {
      if (bOp == PATCH_1_4OP)
      {
         BYTE alg = (lpNS->bAtC0[0] & 0x01) | ((lpNS->bAtC0[1] & 0x01)<<1);

         //Check carriers for 4op patch
         switch(alg)
         {
            case 3:
               wOpOffset = gw2OpOffset[ bVoice ][ 0 ] ;
               Opl3_ChipWrite( 0x80 + wOpOffset, 0xFF) ; // set SR to high
               wOpOffset = gw2OpOffset[ bVoice +3 ][ 0 ] ;
               Opl3_ChipWrite( 0x80 + wOpOffset, 0xFF) ; // set SR to high
               wOpOffset = gw2OpOffset[ bVoice +3 ][ 1 ] ;
               Opl3_ChipWrite( 0x80 + wOpOffset, 0xFF) ; // set SR to high
               break;

            case 2:
               wOpOffset = gw2OpOffset[ bVoice ][ 1 ] ;
               Opl3_ChipWrite( 0x80 + wOpOffset, 0xFF) ; // set SR to high
               wOpOffset = gw2OpOffset[ bVoice +3 ][ 1 ] ;
               Opl3_ChipWrite( 0x80 + wOpOffset, 0xFF) ; // set SR to high
               break;

            case 1:
               wOpOffset = gw2OpOffset[ bVoice ][ 0 ] ;
               Opl3_ChipWrite( 0x80 + wOpOffset, 0xFF) ; // set SR to high
               //fall through

            case 0:
               wOpOffset = gw2OpOffset[ bVoice +3 ][ 1 ] ;
               Opl3_ChipWrite( 0x80 + wOpOffset, 0xFF) ; // set SR to high
               break;
         }

         
      }
      else
      {
         wOpOffset = gw2OpOffset[ bVoice ][ 1 ] ; // assuming 2op
         Opl3_ChipWrite( 0x80 + wOpOffset, 0xFF) ; // set SR to high

         // if AM mode
         if (lpNS->bAtC0[0] & 0x01)
         {
            wOpOffset = gw2OpOffset[ bVoice ][ 0 ] ; // assuming 2op
            Opl3_ChipWrite( 0x80 + wOpOffset, 0xFF) ; // set SR to high
         }
      }
   }

   Opl3_ChipWrite(AD_BLOCK + wOffset,
      (BYTE)(m_Voice[ bVoice ].bBlock[ 0 ] & 0x1f) ) ;

   // Note this...
   m_Voice[ bVoice ].bOn = FALSE ;
   m_Voice[ bVoice ].bBlock[ 0 ] &= 0x1f ;
   m_Voice[ bVoice ].bBlock[ 1 ] &= 0x1f ;
   m_Voice[ bVoice ].dwTime = ++m_dwCurTime ;
   m_Voice[ bVoice ].bSusHeld = FALSE ;
}

bool
   OPLSynth::
   Init()
{
   int i;

   // init some members
   m_dwCurTime = 1;    /* for note on/off * /
   /* volume */
   m_wSynthAttenL = 0;        /* in 1.5dB steps */
   m_wSynthAttenR = 0;        /* in 1.5dB steps */

   m_MinVolValue  = 0xFFD0C000;    //  minimum -47.25(dB) * 0x10000
   m_MaxVolValue  = 0x00000000;    //  maximum  0    (dB) * 0x10000
   m_VolStepDelta = 0x0000C000;    //  steps of 0.75 (dB) * 0x10000

   m_wMonoMode = 0;  // Set all channels to polyphonic mode
   m_wDrumMode = (1<<9);  // Set ch10 to drum by default
   m_wPortaMode = 0;

   b4OpVoiceSet = 0; // disable 4op mode by default

   /* start attenuations at -3 dB, which is 90 MIDI level */
   for (i = 0; i < NUMMIDICHN; i++)
   {
      m_bSustain[i] = 0;
      m_bChanAtten[i] = 4;      // default attenuation value
      m_bStereoMask[i] = 0xff;  // center
      m_iBendRange[i] = 2;      // -/+ 2 semitones
      memset(m_RPN[i], -1, sizeof(WORD));  
      m_iExpThres[i] = 0x7F;
      m_curVol[i] = 100;
      m_bAttack[i] = 64;
      m_bRelease[i] = 64;
      m_bBrightness[i] = 64;
      m_bModWheel[i] = 0;
      m_noteHistory[i].clear();
      m_iBend[i] = 0;
      m_bPortaTime[i] = 0;
   };

   for (i = 0; i < NUM2VOICES; ++i)
   {
      memset(&m_Voice[i], 0, sizeof(voiceStruct));
      m_Voice[i].bChannel = ~0;
   }

//#ifdef DISABLE_HW_SUPPORT
   //m_Miniport.adlib_init();
   m_Miniport = opl_init();
//#else
   OPL_Hardware_Detection();
   OPL_HW_Init(); // start hardware
//#endif /*DISABLE_HW_SUPPORT*/
   VGMLog_Init();
   Opl3_BoardReset();
   return true;
}

void
   OPLSynth::
   Opl3_LFOUpdate(BYTE bVoice)
{
   WORD  wTemp, wOffset;
   long  newLFOVal = m_Voice[bVoice].dwLFOVal, 
         newDetuneEG = m_Voice[bVoice].dwDetuneEG;
   DWORD timeDiff = m_dwCurSample-m_Voice[bVoice].dwStartTime;
   double timeLapse = 0.00025*3.14159265358979323846*(double)(timeDiff),
          noteDiff = 0;

   // Portamento update first
   if (m_Voice[bVoice].bPortaSampCnt > 0)
   {
      --m_Voice[bVoice].bPortaSampCnt;
      noteDiff = (double)lin_intp(
         (m_Voice[bVoice].bPortaSampTime)-m_Voice[bVoice].bPortaSampCnt,
         0,
         m_Voice[bVoice].bPortaSampTime,
         m_Voice[bVoice].bPrevNote, 
         m_Voice[bVoice].bNote
      );
         
      /*noteDiff = m_Voice[bVoice].dwNoteFactor*(m_Voice[bVoice].bPortaSampTime 
         - m_Voice[bVoice].bPortaSampCnt) - 1;*/
   }

   // 100Hz sine wave with half semitone magnitude by default
   // (mod*32)sin((1/100)*FSAMP * curSample)
   if (m_bModWheel[m_Voice[bVoice].bChannel] > 0)
      newLFOVal = (DWORD)floor(0.5 + (m_bModWheel[m_Voice[bVoice].bChannel]*32)*sin(timeLapse));

   // Linear envelope generator hack  (TODO: improve)
   wTemp = gbPercMap[m_Voice[bVoice].bPatch-128][2] & 0xFF;
   if ((m_wDrumMode & (1<<m_Voice[bVoice].bChannel)) > 0 && wTemp > 0
      && (m_Voice[bVoice].bOn || m_Voice[bVoice].bSusHeld))   // only continue it if the note is held
   {
      newDetuneEG = (long)((signed char)wTemp * 0.25 * (timeDiff));
   }

   if (newLFOVal == m_Voice[bVoice].dwLFOVal && newDetuneEG == m_Voice[bVoice].dwDetuneEG 
      && m_Voice[bVoice].bPortaSampCnt == 0 && noteDiff == 0)
      return;

   m_Voice[bVoice].dwLFOVal = newLFOVal;
   m_Voice[bVoice].dwDetuneEG = newDetuneEG;

   newLFOVal += newDetuneEG;

   wTemp = Opl3_MIDINote2FNum((noteDiff)?noteDiff:m_Voice[ bVoice ].bNote,
      m_Voice[bVoice].bChannel, newLFOVal);
   m_Voice[ bVoice ].bBlock[ 0 ] =
      (m_Voice[ bVoice ].bBlock[ 0 ] & (BYTE) 0xe0) |
      (BYTE) (wTemp >> 8) ;

   wOffset = bVoice;
   if (bVoice >= (NUM2VOICES / 2))
      wOffset += (0x100 - (NUM2VOICES / 2));

   Opl3_ChipWrite(AD_BLOCK + wOffset, m_Voice[ bVoice ].bBlock[ 0 ] ) ;
   Opl3_ChipWrite(AD_FNUMBER + wOffset, (BYTE) wTemp ) ;
   
} // end of Opl3_PitchBend

void 
   OPLSynth::
   GetSample(short *sample, int len)
{
   BYTE i;

   // Perform stepping of pitch LFO
   //for (i = 0; i < len; ++i)
   //{
   //   if (i%(FSAMP/100)==0)
   //   {
   // Step through each channel
   for (i = 0; i < NUM2VOICES; i++)
   {
      // Only execute once initialized
      if (m_Voice[i].bChannel != (BYTE)~0)
         Opl3_LFOUpdate(i);
   }
      //}
   //}

   m_dwCurSample += len;

//#ifdef DISABLE_HW_SUPPORT
   //m_Miniport.adlib_getsample(sample,len);
   opl_getoutput(m_Miniport, sample, len);
//#endif /*DISABLE_HW_SUPPORT*/

   // Increment logger sample history
   VGMLog_IncrementSamples(len);
}


void
   OPLSynth::
   PlaySysex(Bit8u *bufpos, DWORD len)
{
   bool IsResetSysex = false;
   std::string SysExVal((char*)bufpos);

   // For reference: http://homepage2.nifty.com/mkmk/midi_lab/exref.htm
   //            and http://www.bandtrax.com.au/sysex.htm
   //GM          F0 7E 7F 09 01 F7 
   //GM-2        F0 7E 7F 09 02 F7
   //GS          F0 41 10 42 12 40 00 7F 00 41 F7 
   //SC88 Mode 1 F0 41 10 42 12 00 00 7F 00 01 F7
   //SC88 Mode 2 F0 41 10 42 12 00 00 7F 01 00 F7
   //XG          F0 43 10 4C 00 00 7E 00 F7 
   //TG300B      (SAME AS GS RESET)



   /*
    * General MIDI System Exclusive
    *  - Do not allow bank switching at all (melodic or drum)
    *  - CC71-74 disabled
    */

   /*
    * General MIDI Level 2 System Exclusive
    *  - Allow bank switching
    *  - CC71-74 enabled
    *  - Drum bank select 127 disabled - must use NRPN
    */

   /*
    * Rolands GS System Exclusive
    *  - Allow bank switching
    *  - Drum bank select 127 disabled - must use NRPN
    *  - CC71-74 enabled (TG300b-mode compatibility)
    *  - 
    */

   /*
    * Yamaha XG System Exclusive
    *  - Allow bank switching
    *  - Allow bank-switchable MIDI channels for drum bank
    *  - CC71-74 enabled
    *  - Drum bank select via NRPN enabled
    */
   
   //TODO: common reset routines to all channels
   if (IsResetSysex)
   {
      for (int i = 0; i < NUM2VOICES; ++i)
      {
         Opl3_CutVoice(i, TRUE);
         m_Voice[i].bPatch = 0;
         m_Voice[i].dwTime = 0;
      }

      for (int i = 0; i < NUMMIDICHN; ++i)
      {
         m_iBend[i] = 0;
         m_iBendRange[i] = 2;
      }
   }
}

inline void
   OPLSynth::
   Opl3_ChipWrite(WORD idx, BYTE val)
{
#ifdef DISABLE_HW_SUPPORT
   // Write to software chip
   //m_Miniport.adlib_write(idx,val);
   opl_writereg(m_Miniport,idx,val);

#else

   // Write to hardware
   OPL_HW_WriteReg(idx, val);

#endif /*DISABLE_HW_SUPPORT*/

   VGMLog_CmdWrite((0x5E | (idx>>8)), (BYTE)idx, val);
}

void 
   OPLSynth::
   close()
{
   // Reset board
   Opl3_BoardReset();
   VGMLog_Close();

#ifndef DISABLE_HW_SUPPORT
   OPL_HW_Close();
#endif /*DISABLE_HW_SUPPORT*/
}

// Some stupid f***ing reason using this breaks playback, with or without anything in it
//OPLSynth::~OPLSynth()
//{
//   //OPL_HW_Close();
//}
