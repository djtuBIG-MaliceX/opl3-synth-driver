// ==============================================================================
//
// Copyright (c) 1996-2000 Microsoft Corporation.  All rights reserved.
//
// Extensions (C) 2013 James Alan Nguyen
// http://www.codingchords.com
//
// ==============================================================================

#include "OPLSynth.h"
#include "patch.h"

BYTE gbVelocityAtten[64] = 
{
   40, 37, 35, 33, 31, 29, 27, 25, 24, 22, 21, 20, 19, 18, 17, 16,
   16, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10, 9,  9,  8,  8,
   7,  7,  6,  6,  6,  5,  5,  5,  4,  4,  4,  4,  3,  3,  3,  3,
   2,  2,  2,  2,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0 
};

BYTE offsetSlot[] =
{
   0, 1, 2, 3, 4, 5,
   8, 9, 10, 11, 12, 13,
   16, 17, 18, 19, 20, 21
};
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
   case 0x90:      /* turn key on, or key off if volume == 0 */
      if (bVelocity)
      {
         if (bChannel == DRUMCHANNEL)
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
      if (bChannel == DRUMCHANNEL)
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
      case 7:
         /* change channel volume */
         Opl3_ChannelVolume(bChannel,gbVelocityAtten[bVelocity >> 1]);
         break;

      case 6: // Data Entry (CC98-101 dependent)
         /* TODO: Write stuff based on preceeding N/RPN commands received */
         {
            WORD rpn = (WORD)(m_RPN[bChannel][0])|(m_RPN[bChannel][1] << 8) & (WORD)(0xFF);

            //if (m_RPN[bChannel] == (WORD)0x0000)
            {
               m_iBendRange[bChannel] = bVelocity;
            }
         }
         break;

      case 8:
      case 10:
         /* change the pan level */
         Opl3_SetPan(bChannel, bVelocity);
         break;
         
      case 11: 
         /* TODO: set expression threshold. should influence bChannel.gbVelocityAtten[bVelocity>>1] range */
         break;

      case 64:
         /* Change the sustain level */
         Opl3_SetSustain(bChannel, bVelocity);
         break;

      case 98:  // NRPN LSB
      case 99:  // NRPN MSB
      case 100: // RPN LSB
         m_RPN[bChannel][0] = bVelocity;
         break;
      case 101: // RPN MSB
         m_RPN[bChannel][1] = bVelocity;
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
      if (bChannel != DRUMCHANNEL)
      {
         m_bPatch[ bChannel ] = bNote ;
      }
      break;

   case 0xe0:  // pitch bend
      dwTemp = ((WORD)bNote << 0) | ((WORD)bVelocity << 7);
		dwTemp -= 0x2000;
      //dwTemp += 0x8000;
      dwTemp *= (m_iBendRange[bChannel] << 1);
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
   Opl3_NoteOff
   (
   BYTE            bPatch,
   BYTE            bNote,
   BYTE            bChannel,
   BYTE            bSustain
   )
{

   patchStruct      *lpPS ;
   WORD             wOffset, wTemp ;

   // Find the note slot
   wTemp = Opl3_FindFullSlot( bNote, bChannel ) ;

   if (wTemp != 0xffff)
   {
      if (bSustain)
      {
         // This channel is sustained, don't really turn the note off,
         // just flag it.
         //
         m_Voice[ wTemp ].bSusHeld = 1;

         return;
      }

      // get a pointer to the patch
      lpPS = glpPatch + (BYTE) m_Voice[ wTemp ].bPatch ;

      // shut off the note portion
      // we have the note slot, turn it off.
      wOffset = wTemp;
      if (wTemp >= (NUM2VOICES / 2))
         wOffset += (0x100 - (NUM2VOICES / 2));

      m_Miniport.adlib_write(AD_BLOCK + wOffset,
         (BYTE)(m_Voice[ wTemp ].bBlock[ 0 ] & 0x1f) ) ;

      // Note this...
      m_Voice[ wTemp ].bOn = FALSE ;
      m_Voice[ wTemp ].bBlock[ 0 ] &= 0x1f ;
      m_Voice[ wTemp ].bBlock[ 1 ] &= 0x1f ;
      m_Voice[ wTemp ].dwTime = m_dwCurTime ;
   }
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
         && (m_Voice[ i ].bOn))
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
//  Return Value:
//     Nothing.
//------------------------------------------------------------------------
void
   OPLSynth::
   Opl3_FMNote(WORD wNote, noteStruct * lpSN)
{
   WORD            i ;
   WORD            wOffset ;
   operStruct      *lpOS ;

   // write out a note off, just to make sure...

   wOffset = wNote;
   if (wNote >= (NUM2VOICES / 2))
      wOffset += (0x100 - (NUM2VOICES / 2));

   m_Miniport.adlib_write(AD_BLOCK + wOffset, 0 ) ;

   // writing the operator information

   //for (i = 0; i < (WORD)((wNote < NUM4VOICES) ? NUMOPS : 2); i++)
   for (i = 0; i < 2; i++)
   {
      lpOS = &lpSN -> op[ i ] ;
      wOffset = gw2OpOffset[ wNote ][ i ] ;
      m_Miniport.adlib_write( 0x20 + wOffset, lpOS -> bAt20) ;
      m_Miniport.adlib_write( 0x40 + wOffset, lpOS -> bAt40) ;
      m_Miniport.adlib_write( 0x60 + wOffset, lpOS -> bAt60) ;
      m_Miniport.adlib_write( 0x80 + wOffset, lpOS -> bAt80) ;
      m_Miniport.adlib_write( 0xE0 + wOffset, lpOS -> bAtE0) ;

   }

   // write out the voice information
   wOffset = (wNote < 9) ? wNote : (wNote + 0x100 - 9) ;
   m_Miniport.adlib_write(0xa0 + wOffset, lpSN -> bAtA0[ 0 ] ) ;
   m_Miniport.adlib_write(0xc0 + wOffset, lpSN -> bAtC0[ 0 ] ) ;

   // Note on...
   m_Miniport.adlib_write(0xb0 + wOffset,
      (BYTE)(lpSN -> bAtB0[ 0 ] | 0x20) ) ;

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
   WORD             wTemp, i, j ;
   BYTE             b4Op, bTemp, bMode, bStereo ;
   patchStruct      *lpPS ;
   DWORD            dwBasicPitch, dwPitch[ 2 ] ;
   noteStruct       NS;

   // Get a pointer to the patch
   lpPS = glpPatch + bPatch ;
   // Find out the basic pitch according to our
   // note value.  This may be adjusted because of
   // pitch bends or special qualities for the note.

   dwBasicPitch = gdwPitch[ bNote % 12 ] ;
   bTemp = bNote / (BYTE) 12 ;
   if (bTemp > (BYTE) (60 / 12))
      dwBasicPitch = AsLSHL( dwBasicPitch, (BYTE)(bTemp - (BYTE)(60/12)) ) ;
   else if (bTemp < (BYTE) (60/12))
      dwBasicPitch = AsULSHR( dwBasicPitch, (BYTE)((BYTE) (60/12) - bTemp) ) ;

   // Copy the note information over and modify
   // the total level and pitch according to
   // the velocity, midi volume, and tuning.

   RtlCopyMemory( (LPSTR) &NS, (LPSTR) &lpPS -> note, sizeof( noteStruct ) ) ;
   b4Op = (BYTE)(NS.bOp != PATCH_1_2OP) ;

   for (j = 0; j < 2; j++)
   {
      // modify pitch
      dwPitch[ j ] = dwBasicPitch ;
      bTemp = (BYTE)((NS.bAtB0[ j ] >> 2) & 0x07) ;
      if (bTemp > 4)
         dwPitch[ j ] = AsLSHL( dwPitch[ j ], (BYTE)(bTemp - (BYTE)4) ) ;
      else if (bTemp < 4)
         dwPitch[ j ] = AsULSHR( dwPitch[ j ], (BYTE)((BYTE)4 - bTemp) ) ;

      wTemp = Opl3_CalcFAndB( Opl3_CalcBend( dwPitch[ j ], iBend ) ) ;
      NS.bAtA0[ j ] = (BYTE) wTemp ;
      NS.bAtB0[ j ] = (BYTE) 0x20 | (BYTE) (wTemp >> 8) ;
   }
   // Modify level for each operator, but only
   // if they are carrier waves

   bMode = (BYTE) ((NS.bAtC0[ 0 ] & 0x01) * 2 + 4) ;

   for (i = 0; i < 2; i++)
   {
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

   // Find an empty slot, and use it...
   wTemp = Opl3_FindEmptySlot( bPatch ) ;

   Opl3_FMNote(wTemp, &NS ) ;
   m_Voice[ wTemp ].bNote = bNote ;
   m_Voice[ wTemp ].bChannel = bChannel ;
   m_Voice[ wTemp ].bPatch = bPatch ;
   m_Voice[ wTemp ].bVelocity = bVelocity ;
   m_Voice[ wTemp ].bOn = TRUE ;
   m_Voice[ wTemp ].dwTime = m_dwCurTime++ ;
   m_Voice[ wTemp ].dwOrigPitch[0] = dwPitch[ 0 ] ;  // not including bend
   m_Voice[ wTemp ].dwOrigPitch[1] = dwPitch[ 1 ] ;  // not including bend
   m_Voice[ wTemp ].bBlock[0] = NS.bAtB0[ 0 ] ;
   m_Voice[ wTemp ].bBlock[1] = NS.bAtB0[ 1 ] ;
   m_Voice[ wTemp ].bSusHeld = 0;


} // end of Opl3_NoteOn()

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
WORD
   OPLSynth::
   Opl3_CalcFAndB(DWORD dwPitch)
{
   BYTE    bBlock;

   /* bBlock is like an exponential to dwPitch (or FNumber) */
   for (bBlock = 1; dwPitch >= 0x400; dwPitch >>= 1, bBlock++)
      ;

   if (bBlock > 0x07)
      bBlock = 0x07;  /* we cant do anything about this */

   /* put in high two bits of F-num into bBlock */
   return ((WORD) bBlock << 10) | (WORD) dwPitch;
}

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
DWORD
   OPLSynth::
   Opl3_CalcBend (DWORD dwOrig, long iBend)
{
   /* do different things depending upon positive or
   negative bend */
   DWORD dw;
   if (iBend > 0)
   {
      dw = (DWORD)((iBend * (LONG)(256.0 * (EQUAL * EQUAL - 1.0))) >> 8);
      dwOrig += (DWORD)(AsULMUL(dw, dwOrig) >> 15);
   }
   else if (iBend < 0)
   {
      dw = (DWORD)(((-iBend) * (LONG)(256.0 * (1.0 - 1.0 / EQUAL / EQUAL))) >> 8);
      dwOrig -= (DWORD)(AsULMUL(dw, dwOrig) >> 15);
   }

   return dwOrig;
}

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
         Opl3_NoteOff(m_Voice[i].bPatch, m_Voice[i].bNote,m_Voice[i].bChannel, 0) ;
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
   noteStruct      *lpPS ;
   BYTE            bMode, bStereo ;

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

         for (j = 0; j < 2; j++)
         {
            wTemp = (BYTE) Opl3_CalcVolume(
               (BYTE) (lpPS -> op[j].bAt40 & (BYTE) 0x3f),
               m_Voice[i].bChannel, m_Voice[i].bVelocity,
               (BYTE) j,            bMode ) ;

            // Write new value.
            wOffset = gw2OpOffset[ i ][ j ] ;
            m_Miniport.adlib_write(
               0x40 + wOffset,
               (BYTE) ((lpPS -> op[j].bAt40 & (BYTE)0xc0) | (BYTE) wTemp) ) ;
         }

         // Do stereo pan, but cut left or right channel if needed.
         bStereo = Opl3_CalcStereoMask( m_Voice[ i ].bChannel ) ;
         wOffset = i;
         if (i >= (NUM2VOICES / 2))
            wOffset += (0x100 - (NUM2VOICES / 2));
         m_Miniport.adlib_write(0xc0 + wOffset, (BYTE)(lpPS -> bAtC0[ 0 ] & bStereo) ) ;
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
   DWORD  dwNew ;

   // Remember the current bend..
   m_iBend[ bChannel ] = iBend ;

   // Loop through all the notes looking for
   // the correct channel.  Anything with the
   // correct channel gets its pitch bent...
   for (i = 0; i < NUM2VOICES; i++)
      if (m_Voice[ i ].bChannel == bChannel)
      {
         j = 0 ;
         dwNew = Opl3_CalcBend( m_Voice[ i ].dwOrigPitch[ j ], iBend ) ;
         wTemp[ j ] = Opl3_CalcFAndB( dwNew ) ;
         m_Voice[ i ].bBlock[ j ] =
            (m_Voice[ i ].bBlock[ j ] & (BYTE) 0xe0) |
            (BYTE) (wTemp[ j ] >> 8) ;

         wOffset = i;
         if (i >= (NUM2VOICES / 2))
            wOffset += (0x100 - (NUM2VOICES / 2));

         m_Miniport.adlib_write(AD_BLOCK + wOffset, m_Voice[ i ].bBlock[ 0 ] ) ;
         m_Miniport.adlib_write(AD_FNUMBER + wOffset, (BYTE) wTemp[ 0 ] ) ;
      }
} // end of Opl3_PitchBend


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
      if (!m_Voice[ i ].bOn && (m_Voice[ i ].dwTime < dwOldest))
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
   WORD            i;

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
            Opl3_NoteOff(m_Voice[i].bPatch, m_Voice[i].bNote, m_Voice[i].bChannel, 0);
         }
      }
   }
   m_bSustain[ bChannel ] = bSusLevel;
}

void
   OPLSynth::
   Opl3_BoardReset()
{
   BYTE i;

   /* ---- silence the chip -------- */

   /* tell the FM chip to use 4-operator mode, and
   fill in any other random variables */
   m_Miniport.adlib_write(AD_NEW, 0x01);
   m_Miniport.adlib_write(AD_MASK, 0x60);
   m_Miniport.adlib_write(AD_CONNECTION, 0x00);
   m_Miniport.adlib_write(AD_NTS, 0x00);

   /* turn off the drums, and use high vibrato/modulation */
   m_Miniport.adlib_write(AD_DRUM, 0xc0);

   /* turn off all the oscillators */
   for (i = 0; i <= 0x15; i++)
   {
      if ((i & 0x07) <= 0x05)
      {
         m_Miniport.adlib_write(AD_LEVEL + i, 0x3f);
         m_Miniport.adlib_write(AD_LEVEL2 + i, 0x3f);
      }
   };

   /* turn off all the voices */
   for (i = 0; i <= 0x08; i++)
   {
      m_Miniport.adlib_write(AD_BLOCK + i, 0x00);
      m_Miniport.adlib_write(AD_BLOCK2 + i, 0x00);
   };
}
bool
   OPLSynth::
   Init()
{
   int i;

   // init some members
   m_dwCurTime = 1;    /* for note on/off */
   /* volume */
   m_wSynthAttenL = 0;        /* in 1.5dB steps */
   m_wSynthAttenR = 0;        /* in 1.5dB steps */

   m_MinVolValue  = 0xFFD0C000;    //  minimum -47.25(dB) * 0x10000
   m_MaxVolValue  = 0x00000000;    //  maximum  0    (dB) * 0x10000
   m_VolStepDelta = 0x0000C000;    //  steps of 0.75 (dB) * 0x10000

   /* start attenuations at -3 dB, which is 90 MIDI level */
   for (i = 0; i < 16; i++)
   {
      m_bChanAtten[i] = 4;      // default attenuation value
      m_bStereoMask[i] = 0xff;  // center
      m_iBendRange[i] = 2;      // -/+ 2 semitones
      memset(m_RPN[i], -1, sizeof(WORD));  
   };
   m_Miniport.adlib_init();
   Opl3_BoardReset();
   return true;
}

void 
   OPLSynth::
   GetSample(short *sample, int len)
{
   m_Miniport.adlib_getsample(sample,len);
}
