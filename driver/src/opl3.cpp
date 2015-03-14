/*
 *  Copyright (C) 2013-2014 Alexey Khokholov
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 * 
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* Changelog:
	v2:
		Vibrato's sign fix
	v3:
		Operator key fix
		Corrected 4-operator mode
		Corrected rhythm mode
		Some small fixes
	v3.1:
		Small envelope generator fix
		Removed EX_Get function(not used)
*/

#include <cstdlib>
#include <cstring>
#include "opl3.h"

// Channel types

#define CH_4OP2  	0x00

#define CH_2OP		0x01

#define CH_4OP		0x02

#define CH_DRUM		0x03

// Envelope generator states

#define EG_OFF		0x00

#define EG_ATTACK	0x01

#define EG_DECAY	0x02

#define EG_SUSTAIN	0x03

#define EG_RELEASE	0x04

// Envelope generator states

#define EG_KEY_NORM	0x01

#define EG_KEY_DRUM	0x02

static int tables = 0;

static int exptab[6226];
//
// logsin table
//
static int logsinrom[256] = 
{	
	0x859, 0x6c3, 0x607, 0x58b, 0x52e, 0x4e4, 0x4a6, 0x471, 0x443, 0x41a, 0x3f5, 0x3d3, 0x3b5, 0x398, 0x37e, 0x365,
	0x34e, 0x339, 0x324, 0x311, 0x2ff, 0x2ed, 0x2dc, 0x2cd, 0x2bd, 0x2af, 0x2a0, 0x293, 0x286, 0x279, 0x26d, 0x261,
	0x256, 0x24b, 0x240, 0x236, 0x22c, 0x222, 0x218, 0x20f, 0x206, 0x1fd, 0x1f5, 0x1ec, 0x1e4, 0x1dc, 0x1d4, 0x1cd,
	0x1c5, 0x1be, 0x1b7, 0x1b0, 0x1a9, 0x1a2, 0x19b, 0x195, 0x18f, 0x188, 0x182, 0x17c, 0x177, 0x171, 0x16b, 0x166,
	0x160, 0x15b, 0x155, 0x150, 0x14b, 0x146, 0x141, 0x13c, 0x137, 0x133, 0x12e, 0x129, 0x125, 0x121, 0x11c, 0x118,
	0x114, 0x10f, 0x10b, 0x107, 0x103, 0x0ff, 0x0fb, 0x0f8, 0x0f4, 0x0f0, 0x0ec, 0x0e9, 0x0e5, 0x0e2, 0x0de, 0x0db,
	0x0d7, 0x0d4, 0x0d1, 0x0cd, 0x0ca, 0x0c7, 0x0c4, 0x0c1, 0x0be, 0x0bb, 0x0b8, 0x0b5, 0x0b2, 0x0af, 0x0ac, 0x0a9,
	0x0a7, 0x0a4, 0x0a1, 0x09f, 0x09c, 0x099, 0x097, 0x094, 0x092, 0x08f, 0x08d, 0x08a, 0x088, 0x086, 0x083, 0x081,
	0x07f, 0x07d, 0x07a, 0x078, 0x076, 0x074, 0x072, 0x070, 0x06e, 0x06c, 0x06a, 0x068, 0x066, 0x064, 0x062, 0x060,
	0x05e, 0x05c, 0x05b, 0x059, 0x057, 0x055, 0x053, 0x052, 0x050, 0x04e, 0x04d, 0x04b, 0x04a, 0x048, 0x046, 0x045,
	0x043, 0x042, 0x040, 0x03f, 0x03e, 0x03c, 0x03b, 0x039, 0x038, 0x037, 0x035, 0x034, 0x033, 0x031, 0x030, 0x02f,
	0x02e, 0x02d, 0x02b, 0x02a, 0x029, 0x028, 0x027, 0x026, 0x025, 0x024, 0x023, 0x022, 0x021, 0x020, 0x01f, 0x01e,
	0x01d, 0x01c, 0x01b, 0x01a, 0x019, 0x018, 0x017, 0x017, 0x016, 0x015, 0x014, 0x014, 0x013, 0x012, 0x011, 0x011,
	0x010, 0x00f, 0x00f, 0x00e, 0x00d, 0x00d, 0x00c, 0x00c, 0x00b, 0x00a, 0x00a, 0x009, 0x009, 0x008, 0x008, 0x007,
	0x007, 0x007, 0x006, 0x006, 0x005, 0x005, 0x005, 0x004, 0x004, 0x004, 0x003, 0x003, 0x003, 0x002, 0x002, 0x002,
	0x002, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

static int sin_table[8][1024];
static int negsin_table[8][1024];

//
// exp table
//

static int exprom[256] =
{
	0x000, 0x003, 0x006, 0x008, 0x00b, 0x00e, 0x011, 0x014, 0x016, 0x019, 0x01c, 0x01f, 0x022, 0x025, 0x028, 0x02a,
	0x02d, 0x030, 0x033, 0x036, 0x039, 0x03c, 0x03f, 0x042, 0x045, 0x048, 0x04b, 0x04e, 0x051, 0x054, 0x057, 0x05a,
	0x05d, 0x060, 0x063, 0x066, 0x069, 0x06c, 0x06f, 0x072, 0x075, 0x078, 0x07b, 0x07e, 0x082, 0x085, 0x088, 0x08b,
	0x08e, 0x091, 0x094, 0x098, 0x09b, 0x09e, 0x0a1, 0x0a4, 0x0a8, 0x0ab, 0x0ae, 0x0b1, 0x0b5, 0x0b8, 0x0bb, 0x0be,
	0x0c2, 0x0c5, 0x0c8, 0x0cc, 0x0cf, 0x0d2, 0x0d6, 0x0d9, 0x0dc, 0x0e0, 0x0e3, 0x0e7, 0x0ea, 0x0ed, 0x0f1, 0x0f4,
	0x0f8, 0x0fb, 0x0ff, 0x102, 0x106, 0x109, 0x10c, 0x110, 0x114, 0x117, 0x11b, 0x11e, 0x122, 0x125, 0x129, 0x12c,
	0x130, 0x134, 0x137, 0x13b, 0x13e, 0x142, 0x146, 0x149, 0x14d, 0x151, 0x154, 0x158, 0x15c, 0x160, 0x163, 0x167,
	0x16b, 0x16f, 0x172, 0x176, 0x17a, 0x17e, 0x181, 0x185, 0x189, 0x18d, 0x191, 0x195, 0x199, 0x19c, 0x1a0, 0x1a4,
	0x1a8, 0x1ac, 0x1b0, 0x1b4, 0x1b8, 0x1bc, 0x1c0, 0x1c4, 0x1c8, 0x1cc, 0x1d0, 0x1d4, 0x1d8, 0x1dc, 0x1e0, 0x1e4,
	0x1e8, 0x1ec, 0x1f0, 0x1f5, 0x1f9, 0x1fd, 0x201, 0x205, 0x209, 0x20e, 0x212, 0x216, 0x21a, 0x21e, 0x223, 0x227,
	0x22b, 0x230, 0x234, 0x238, 0x23c, 0x241, 0x245, 0x249, 0x24e, 0x252, 0x257, 0x25b, 0x25f, 0x264, 0x268, 0x26d,
	0x271, 0x276, 0x27a, 0x27f, 0x283, 0x288, 0x28c, 0x291, 0x295, 0x29a, 0x29e, 0x2a3, 0x2a8, 0x2ac, 0x2b1, 0x2b5,
	0x2ba, 0x2bf, 0x2c4, 0x2c8, 0x2cd, 0x2d2, 0x2d6, 0x2db, 0x2e0, 0x2e5, 0x2e9, 0x2ee, 0x2f3, 0x2f8, 0x2fd, 0x302,
	0x306, 0x30b, 0x310, 0x315, 0x31a, 0x31f, 0x324, 0x329, 0x32e, 0x333, 0x338, 0x33d, 0x342, 0x347, 0x34c, 0x351,
	0x356, 0x35b, 0x360, 0x365, 0x36a, 0x370, 0x375, 0x37a, 0x37f, 0x384, 0x38a, 0x38f, 0x394, 0x399, 0x39f, 0x3a4,
	0x3a9, 0x3ae, 0x3b4, 0x3b9, 0x3bf, 0x3c4, 0x3c9, 0x3cf, 0x3d4, 0x3da, 0x3df, 0x3e4, 0x3ea, 0x3ef, 0x3f5, 0x3fa
};
// 
// freq mult table multiplied by 2
//
// 1/2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15
//

static int mt[16] = { 1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30 };

//
// ksl table
//

static int ksl_table[16][8];

static int kslrom[16] = { 0, 24, 32, 37, 40, 43, 45, 47, 48, 50, 51, 52, 53, 54, 55, 56 };

static int ksl[4] = { 31, 1, 2, 0};

//
// LFO vibrato
//

static int vib_table[8] = { 0, 1, 3, 1, 0, 1, 3, 1 };
static int negvib_table[8] = { 1, 1, 1, -1, -1, -1, -1, 1 };

//
// LFO tremolo
//

static int trem_table[210];

//
// envelope generator constants
//

static int eg_inctab[4][4] = {
	{ 0, 0, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 1, 0, 1 },
	{ 0, 1, 1, 1 }
};

static int eg_inctab2[4][4] = {
	{ 1, 1, 1, 1 },
	{ 1, 1, 1, 2 },
	{ 1, 2, 1, 2 },
	{ 1, 2, 2, 2 }
};

static int eg_sltable[16];


//
// address decoding
//

static int ad_slot[0x20] = { 0, 2, 4, 1, 3, 5, -1, -1, 6, 8, 10, 7, 9, 11, -1, -1, 12, 14, 16, 13, 15, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  

static int op_offset[18] = { 0x00, 0x03, 0x01, 0x04, 0x02, 0x05, 0x08, 0x0b, 0x09, 0x0c, 0x0a, 0x0d, 0x10, 0x13, 0x11, 0x14, 0x12, 0x15 };

typedef struct _channel {
	int op1id, op2id;
	int con;
	int chtype;
	int alg;
	int offset;
	int feedback;
	unsigned int cha,chb,chc,chd;
	int out;
	int f_number;
	int block;
	int ksv;
} channel;

typedef struct _slot {
	int PG_pos;
	int PG_inc;
	int PG_vibpos;
	int PG_vibinc;
	int EG_out;
	int EG_mout;
	int EG_trempos;
	int EG_ksl;
	int EG_sll;
	int EG_vol;
	int EG_ar;
	int EG_dr;
	int EG_sl;
	int EG_rr;
	int EG_state;
	int EG_type;
	int out;
	int prevout[2];
	int offset;
	int mult;
	int vibrato;
	int tremolo;
	int ksr;
	int EG_tl;
	int ksl;
	int key;
	int waveform;
} slot;


typedef struct OPL3Emu
{
	int opl_memory[0x200];
	int opl3mode;
	int nts;
	int rhythm;
	int dvb;
	int dam;
	int noise;
	int vib_pos;
	int timer;
	int trem_inc;
	int trem_pos;
	channel Channels[18];
	slot OPs[36];
} OPL3;

//
// Phase generator 
//

void PG_UpdateFreq(OPL3 *opl, int op)
{
	opl->OPs[op].PG_inc = ((opl->Channels[op/2].f_number<<opl->Channels[op/2].block) *  mt[opl->OPs[op].mult])>>1;
}

void PG_Generate(OPL3 *opl, int op)
{
	if(opl->OPs[op].vibrato)
	{
		int fnum_high = opl->Channels[op/2].f_number>>(7+vib_table[(opl->vib_pos>>10)&0x7]+(1-opl->dvb));
		int fnum = opl->Channels[op/2].f_number + fnum_high*negvib_table[(opl->vib_pos>>10)&0x7];
		opl->OPs[op].PG_pos += (((fnum<<opl->Channels[op/2].block) *  mt[opl->OPs[op].mult])>>1);
	}
	else
	{
		opl->OPs[op].PG_pos += opl->OPs[op].PG_inc;
	}
}

void PG_Reset(OPL3 *opl, int op)
{
	opl->OPs[op].PG_pos = 0;
}

//
// Envelope generator
//

int EG_GetRate(OPL3 *opl, int op, int rate)
{
	int rof = opl->OPs[op].ksr ? opl->Channels[op/2].ksv : (opl->Channels[op/2].ksv>>2);
	int rat = (rate<<2) + rof;
	if(rat>60)
		rat = 60;
	return rat;
}

void EG_UpdateKSL(OPL3 *opl, int op)
{
	int fnum_high = (opl->Channels[op/2].f_number>>6)&0x0F;
	opl->OPs[op].EG_ksl = (ksl_table[fnum_high][opl->Channels[op/2].block]<<2) >> ksl[opl->OPs[op].ksl];
}

void EG_UpdateVol(OPL3 *opl, int op)
{
	opl->OPs[op].EG_vol = opl->OPs[op].EG_tl<<2;
}

void EG_Generate(OPL3 *opl, int op)
{
	int rate;
	switch(opl->OPs[op].EG_state)
	{
	case EG_ATTACK:
		if(opl->OPs[op].EG_ar==15)
		{
			opl->OPs[op].EG_out = 0;
			opl->OPs[op].EG_state = EG_DECAY;
			break;
		}
		if(!opl->OPs[op].EG_ar)
		{
			break;
		}
		rate = EG_GetRate(opl, op, opl->OPs[op].EG_ar);
		if( !(opl->timer & (0xFFF >> (rate>>2))))
		{
			if((rate>>2)>12)
			{
				int inc = opl->timer&0x3;
				opl->OPs[op].EG_out += (~opl->OPs[op].EG_out*(1<<((rate>>2)+eg_inctab2[rate&0x3][inc])))>>3;
			}
			else
			{
				int inc = (opl->timer >>(12-opl->OPs[op].EG_ar))&0x7;
				if( inc&0x1 || eg_inctab[rate&0x3][(inc>>1)])
				{
					opl->OPs[op].EG_out += (~opl->OPs[op].EG_out)>>3;
				}
			}
		}
		if(opl->OPs[op].EG_out<=0)
		{
			opl->OPs[op].EG_out = 0;
			opl->OPs[op].EG_state = EG_DECAY;
		}
		break;
	case EG_DECAY:
		if(!opl->OPs[op].EG_dr)
		{
			break;
		}
		rate = EG_GetRate(opl, op, opl->OPs[op].EG_dr);
		if( !(opl->timer & (0xFFF >> (rate>>2))))
		{
			if((rate>>2)>12)
			{
				int inc = opl->timer&0x3;
				opl->OPs[op].EG_out += 1<<((rate>>2)+eg_inctab2[rate&0x3][inc]);
			}
			else
			{
				int inc = (opl->timer >>(12-(rate>>2)))&0x7;
				if( inc&0x1 || eg_inctab[rate&0x3][(inc>>1)])
				{
					opl->OPs[op].EG_out++;
				}
			}
		}
		if(opl->OPs[op].EG_out>=eg_sltable[opl->OPs[op].EG_sl])
		{
			opl->OPs[op].EG_out = eg_sltable[opl->OPs[op].EG_sl];
			opl->OPs[op].EG_state = EG_SUSTAIN;
		}
		break;
	case EG_SUSTAIN:
		if(opl->OPs[op].EG_type)
		{
			break;
		}
	case EG_RELEASE:
		if(!opl->OPs[op].EG_rr)
		{
			break;
		}
		rate = EG_GetRate(opl, op, opl->OPs[op].EG_rr);
		if( !(opl->timer & (0xFFF >> (rate>>2))))
		{
			if((rate>>2)>12)
			{
				int inc = opl->timer&0x3;
				opl->OPs[op].EG_out += 1<<((rate>>2)+eg_inctab2[rate&0x3][inc]);
			}
			else
			{
				int inc = (opl->timer >>(12-(rate>>2)))&0x7;
				if( inc&0x1 || eg_inctab[rate&0x3][(inc>>1)])
				{
					opl->OPs[op].EG_out++;
				}
			}
		}
		if(opl->OPs[op].EG_out>=511)
		{
			opl->OPs[op].EG_out = 511;
			opl->OPs[op].EG_state = EG_OFF;
		}
		break;
	}
	opl->OPs[op].EG_mout = opl->OPs[op].EG_out + opl->OPs[op].EG_ksl + opl->OPs[op].EG_vol;
	if(opl->OPs[op].tremolo)
	{
		opl->OPs[op].EG_mout+=trem_table[opl->trem_pos]>>((1-opl->dam)*2);
	}
	if(opl->OPs[op].EG_mout>511)
	{
		opl->OPs[op].EG_mout = 511;
	}
}

void EG_KeyOn(OPL3 *opl, int op, int type)
{
	if(opl->OPs[op].key == 0x00)
	{
		opl->OPs[op].EG_state = EG_ATTACK;
		PG_Reset(opl,op);
	}
	opl->OPs[op].key |= type;
}

void EG_KeyOff(OPL3 *opl, int op, int type)
{
	if(opl->OPs[op].key != 0x00)
	{
		opl->OPs[op].key &= (~type);
		if(opl->OPs[op].key == 0x00)
		{
			opl->OPs[op].EG_state = EG_RELEASE;
		}
	}
}

void EG_Reset(OPL3 *opl, int op)
{
	opl->OPs[op].EG_state = EG_OFF;
	opl->OPs[op].EG_out = 511;
	opl->OPs[op].EG_mout = 511;
}

//
// Noise Generator
//

void N_Generate(OPL3 *opl)
{
	int nois = ((opl->noise) ^ (opl->noise>>14) ^ (opl->noise>>15) ^ (opl->noise>>22))&0x01;
	opl->noise = (nois<<22) | (opl->noise>>1);
}

//
// Operator(Slot)
//

void OP_Update20(OPL3 *opl, int op)
{
	opl->OPs[op].tremolo = (opl->opl_memory[0x20+opl->OPs[op].offset]>>7);
	opl->OPs[op].vibrato = (opl->opl_memory[0x20+opl->OPs[op].offset]>>6)&0x01;
	opl->OPs[op].EG_type = (opl->opl_memory[0x20+opl->OPs[op].offset]>>5)&0x01;
	opl->OPs[op].ksr = (opl->opl_memory[0x20+opl->OPs[op].offset]>>4)&0x01;
	opl->OPs[op].mult = (opl->opl_memory[0x20+opl->OPs[op].offset])&0x0F;
	PG_UpdateFreq(opl,op);
}

void OP_Update40(OPL3 *opl, int op)
{
	opl->OPs[op].EG_tl = (opl->opl_memory[0x40+opl->OPs[op].offset])&0x3F;
	opl->OPs[op].ksl = (opl->opl_memory[0x40+opl->OPs[op].offset]>>6)&0x03;
	EG_UpdateVol(opl,op);
	EG_UpdateKSL(opl,op);
}

void OP_Update60(OPL3 *opl, int op)
{
	opl->OPs[op].EG_dr = (opl->opl_memory[0x60+opl->OPs[op].offset])&0x0F;
	opl->OPs[op].EG_ar = (opl->opl_memory[0x60+opl->OPs[op].offset]>>4)&0x0F;
}

void OP_Update80(OPL3 *opl, int op)
{
	opl->OPs[op].EG_rr = (opl->opl_memory[0x80+opl->OPs[op].offset])&0x0F;
	opl->OPs[op].EG_sl = (opl->opl_memory[0x80+opl->OPs[op].offset]>>4)&0x0F;
}

void OP_UpdateE0(OPL3 *opl, int op)
{
	opl->OPs[op].waveform = opl->opl_memory[0xE0+opl->OPs[op].offset]&0x07;
	if(!opl->opl3mode)
	{
		opl->OPs[op].waveform&=0x03;
	}
}

void OP_GeneratePhase(OPL3 *opl, int op, int phase)
{
	int EG = opl->OPs[op].EG_mout<<3;
	EG += sin_table[opl->OPs[op].waveform][phase];
	if(EG>6225)
	{
		EG = 6225;
	}
	opl->OPs[op].out = exptab[EG];
	if(negsin_table[opl->OPs[op].waveform][phase])
	{
		opl->OPs[op].out = ~opl->OPs[op].out;
	}
}

void OP_Generate(OPL3 *opl, int op, int modulator)
{
	int EG = opl->OPs[op].EG_mout<<3;
	int wp = (opl->OPs[op].PG_pos>>10) + modulator;
	wp&=0x3FF;

	EG += sin_table[opl->OPs[op].waveform][wp];
	if(EG>6225)
	{
		EG = 6225;
	}
	opl->OPs[op].out = exptab[EG];
	if(negsin_table[opl->OPs[op].waveform][wp])
	{
		opl->OPs[op].out = ~opl->OPs[op].out;
	}
}

//
// Channel
//

void CH_UpdateRhythm(OPL3 *opl)
{
	opl->rhythm = opl->opl_memory[0xBD]&0x3F;
	if(opl->rhythm&0x20)
	{
		for(int i = 6; i < 9; i++)
		{
			opl->Channels[i].chtype = CH_DRUM;
		}
		//HH
		if(opl->rhythm&0x01)
		{
			EG_KeyOn(opl,14,EG_KEY_DRUM);
		}
		else
		{
			EG_KeyOff(opl,14,EG_KEY_DRUM);
		}
		//TC
		if(opl->rhythm&0x02)
		{
			EG_KeyOn(opl,17,EG_KEY_DRUM);
		}
		else
		{
			EG_KeyOff(opl,17,EG_KEY_DRUM);
		}
		//TOM
		if(opl->rhythm&0x04)
		{
			EG_KeyOn(opl,16,EG_KEY_DRUM);
		}
		else
		{
			EG_KeyOff(opl,16,EG_KEY_DRUM);
		}
		//SD
		if(opl->rhythm&0x08)
		{
			EG_KeyOn(opl,15,EG_KEY_DRUM);
		}
		else
		{
			EG_KeyOff(opl,15,EG_KEY_DRUM);
		}
		//BD
		if(opl->rhythm&0x10)
		{
			EG_KeyOn(opl,12,EG_KEY_DRUM);
			EG_KeyOn(opl,13,EG_KEY_DRUM);
		}
		else
		{
			EG_KeyOff(opl,12,EG_KEY_DRUM);
			EG_KeyOff(opl,13,EG_KEY_DRUM);
		}
	}
	else
	{
		for(int i = 6; i < 9; i++)
		{
			opl->Channels[i].chtype = CH_2OP;
		}
	}
}

void CH_UpdateAB0(OPL3 *opl, int ch)
{
	if(opl->opl3mode && opl->Channels[ch].chtype==CH_4OP2)
		return;
	int f_number = (opl->opl_memory[0xA0+opl->Channels[ch].offset]) | (((opl->opl_memory[0xB0+opl->Channels[ch].offset])&0x03)<<8);
	int block = ((opl->opl_memory[0xB0+opl->Channels[ch].offset])>>2)&0x07;
	int ksv = block*2 | ((f_number>>(9-opl->nts))&0x01);
	opl->Channels[ch].f_number = f_number;
	opl->Channels[ch].block = block;
	opl->Channels[ch].ksv = ksv;
	PG_UpdateFreq(opl,opl->Channels[ch].op1id);
	PG_UpdateFreq(opl,opl->Channels[ch].op2id);
	EG_UpdateKSL(opl,opl->Channels[ch].op1id);
	EG_UpdateKSL(opl,opl->Channels[ch].op2id);
	OP_Update60(opl,opl->Channels[ch].op1id);
	OP_Update60(opl,opl->Channels[ch].op2id);
	OP_Update80(opl,opl->Channels[ch].op1id);
	OP_Update80(opl,opl->Channels[ch].op2id);
	if(opl->opl3mode && opl->Channels[ch].chtype == CH_4OP)
	{
		opl->Channels[ch+3].f_number = f_number;
		opl->Channels[ch+3].block = block;
		opl->Channels[ch+3].ksv = ksv;
		PG_UpdateFreq(opl,opl->Channels[ch+3].op1id);
		PG_UpdateFreq(opl,opl->Channels[ch+3].op2id);
		EG_UpdateKSL(opl,opl->Channels[ch+3].op1id);
		EG_UpdateKSL(opl,opl->Channels[ch+3].op2id);
		OP_Update60(opl,opl->Channels[ch+3].op1id);
		OP_Update60(opl,opl->Channels[ch+3].op2id);
		OP_Update80(opl,opl->Channels[ch+3].op1id);
		OP_Update80(opl,opl->Channels[ch+3].op2id);
	}
}

void CH_UpdateC0(OPL3 *opl, int ch)
{
	int fb = (opl->opl_memory[0xC0+opl->Channels[ch].offset]&0x0E)>>1;
	opl->Channels[ch].feedback = fb?(9-fb):0;
	opl->Channels[ch].con = opl->opl_memory[0xC0+opl->Channels[ch].offset]&0x01;
	opl->Channels[ch].alg = opl->Channels[ch].con;
	if(opl->opl3mode)
	{
		if(opl->Channels[ch].chtype == CH_4OP)
		{
			opl->Channels[ch+3].alg = 0x04 | (opl->Channels[ch].con<<1) | (opl->Channels[ch+3].con);
			opl->Channels[ch].alg = 0x08;
		}
		else if(opl->Channels[ch].chtype == CH_4OP2)
		{
			opl->Channels[ch].alg = 0x04 | (opl->Channels[ch-3].con<<1) | (opl->Channels[ch].con);
			opl->Channels[ch-3].alg = 0x08;
		}
	}
	if(opl->opl3mode)
	{
		opl->Channels[ch].cha = ((opl->opl_memory[0xC0+opl->Channels[ch].offset]>>4)&0x01) ? ~0 : 0;
		opl->Channels[ch].chb = ((opl->opl_memory[0xC0+opl->Channels[ch].offset]>>5)&0x01) ? ~0 : 0;
		opl->Channels[ch].chc = ((opl->opl_memory[0xC0+opl->Channels[ch].offset]>>6)&0x01) ? ~0 : 0;
		opl->Channels[ch].chd = ((opl->opl_memory[0xC0+opl->Channels[ch].offset]>>7)&0x01) ? ~0 : 0;
	}
	else
	{
		opl->Channels[ch].cha = opl->Channels[ch].chb = opl->Channels[ch].chc = opl->Channels[ch].chd = ~0;
	}
}

void CH_Set2OP(OPL3 *opl)
{
	for(int i = 0; i < 9; i++)
	{
		opl->Channels[i].chtype = CH_2OP;
		opl->Channels[i+9].chtype = CH_2OP; 
	}
}

void CH_Set4OP(OPL3 *opl)
{
	for(int i = 0; i < 3; i++)
	{
		if((opl->opl_memory[0x104]>>i)&0x01)
		{
			opl->Channels[i].chtype = CH_4OP;
			opl->Channels[i+3].chtype = CH_4OP2; 
		}
		if((opl->opl_memory[0x104]>>(i+3))&0x01)
		{
			opl->Channels[i+9].chtype = CH_4OP;
			opl->Channels[i+3+9].chtype = CH_4OP2; 
		}
	}
}

void CH_GenerateRhythm(OPL3 *opl)
{
	if(opl->rhythm&0x20)
	{
		//BD
		int fbb = (opl->OPs[12].prevout[0]+opl->OPs[12].prevout[1])>>opl->Channels[6].feedback;
		if(opl->Channels[6].feedback==0)
			fbb = 0;
		if(!opl->Channels[6].con)
		{
			OP_Generate(opl,12,fbb);
			OP_Generate(opl,13,opl->OPs[12].out);
			opl->Channels[6].out = opl->OPs[opl->Channels[6].op2id].out*2;
		}
		else
		{
			OP_Generate(opl,12,fbb);
			OP_Generate(opl,13,0);
			opl->Channels[6].out = opl->OPs[opl->Channels[6].op2id].out*2;
		}
		int P14 = (opl->OPs[14].PG_pos>>10)&0x3FF;
		int P17 = (opl->OPs[17].PG_pos>>10)&0x3FF;
		int phase = 0;
		// HH TC Phase bit
		int PB = ((P14 & 0x08) | (((P14>>5) ^ P14)&0x04) | (((P17>>2) ^ P17)&0x08)) ? 0x01 : 0x00; 
		//HH
		phase = (PB<<9) | ( 0x34 << ((PB ^ (opl->noise & 0x01)<<1)));
		OP_GeneratePhase(opl,14,phase);
		//SD
		phase = (0x100<<((P14>>8)&0x01)) ^ ((opl->noise & 0x01)<<8);
		OP_GeneratePhase(opl,15,phase);
		//TT
		fbb = (opl->OPs[8].prevout[0]+opl->OPs[8].prevout[1])>>opl->Channels[8].feedback;
		if(opl->Channels[8].feedback==0)
			fbb = 0;
		OP_Generate(opl,16,fbb);
		//TC
		phase = 0x100 | (PB<<9);
		OP_GeneratePhase(opl,17,phase);
		opl->Channels[7].out = (opl->OPs[14].out + opl->OPs[15].out) * 2;
		opl->Channels[8].out = (opl->OPs[16].out + opl->OPs[17].out) * 2;
		opl->OPs[12].prevout[1] = opl->OPs[12].prevout[0];
		opl->OPs[12].prevout[0] = opl->OPs[12].out;
		opl->OPs[14].prevout[1] = opl->OPs[14].prevout[0];
		opl->OPs[14].prevout[0] = opl->OPs[14].out;
		opl->OPs[16].prevout[1] = opl->OPs[16].prevout[0];
		opl->OPs[16].prevout[0] = opl->OPs[16].out;
	}
}

void CH_Generate(OPL3 *opl, int ch)
{
	if(opl->Channels[ch].chtype==CH_DRUM)
	{
		return;
	}
	int fbb = (opl->OPs[opl->Channels[ch].op1id].prevout[0]+opl->OPs[opl->Channels[ch].op1id].prevout[1])>>opl->Channels[ch].feedback;
	if(opl->Channels[ch].feedback == 0)
		fbb = 0;
	if(opl->Channels[ch].alg&0x08)
	{
		opl->Channels[ch].out = 0;
		opl->OPs[opl->Channels[ch].op1id].prevout[1] = opl->OPs[opl->Channels[ch].op1id].prevout[0];
		opl->OPs[opl->Channels[ch].op1id].prevout[0] = 0;
		return;
	}
	else if(opl->Channels[ch].alg&0x04)
	{
		switch(opl->Channels[ch].alg&0x03)
		{
		case 0x00:
			OP_Generate(opl,opl->Channels[ch-3].op1id,fbb);
			OP_Generate(opl,opl->Channels[ch-3].op2id,opl->OPs[opl->Channels[ch-3].op1id].out);
			OP_Generate(opl,opl->Channels[ch].op1id,opl->OPs[opl->Channels[ch-3].op2id].out);
			OP_Generate(opl,opl->Channels[ch].op2id,opl->OPs[opl->Channels[ch].op1id].out);
			opl->Channels[ch].out = opl->OPs[opl->Channels[ch].op2id].out;
			break;
		case 0x01:
			OP_Generate(opl,opl->Channels[ch-3].op1id,fbb);
			OP_Generate(opl,opl->Channels[ch-3].op2id,opl->OPs[opl->Channels[ch-3].op1id].out);
			OP_Generate(opl,opl->Channels[ch].op1id,0);
			OP_Generate(opl,opl->Channels[ch].op2id,opl->OPs[opl->Channels[ch].op1id].out);
			opl->Channels[ch].out = opl->OPs[opl->Channels[ch-3].op2id].out+opl->OPs[opl->Channels[ch].op2id].out;
			break;
		case 0x02:
			OP_Generate(opl,opl->Channels[ch-3].op1id,fbb);
			OP_Generate(opl,opl->Channels[ch-3].op2id,0);
			OP_Generate(opl,opl->Channels[ch].op1id,opl->OPs[opl->Channels[ch-3].op2id].out);
			OP_Generate(opl,opl->Channels[ch].op2id,opl->OPs[opl->Channels[ch].op1id].out);
			opl->Channels[ch].out = opl->OPs[opl->Channels[ch-3].op1id].out+opl->OPs[opl->Channels[ch].op2id].out;
			break;
		case 0x03:
			OP_Generate(opl,opl->Channels[ch-3].op1id,fbb);
			OP_Generate(opl,opl->Channels[ch-3].op2id,0);
			OP_Generate(opl,opl->Channels[ch].op1id,opl->OPs[opl->Channels[ch-3].op2id].out);
			OP_Generate(opl,opl->Channels[ch].op2id,0);
			opl->Channels[ch].out = opl->OPs[opl->Channels[ch-3].op1id].out+opl->OPs[opl->Channels[ch].op1id].out+opl->OPs[opl->Channels[ch].op2id].out;
			break;
		}
	}
	else
	{
		if(!(opl->Channels[ch].alg&0x01))
		{
			OP_Generate(opl,opl->Channels[ch].op1id,fbb);
			OP_Generate(opl,opl->Channels[ch].op2id,opl->OPs[opl->Channels[ch].op1id].out);
			opl->Channels[ch].out = opl->OPs[opl->Channels[ch].op2id].out;
		}
		else
		{
			OP_Generate(opl,opl->Channels[ch].op1id,fbb);
			OP_Generate(opl,opl->Channels[ch].op2id,0);
			opl->Channels[ch].out = opl->OPs[opl->Channels[ch].op1id].out + opl->OPs[opl->Channels[ch].op2id].out;
		}
	}
	opl->OPs[opl->Channels[ch].op1id].prevout[1] = opl->OPs[opl->Channels[ch].op1id].prevout[0];
	opl->OPs[opl->Channels[ch].op1id].prevout[0] = opl->OPs[opl->Channels[ch].op1id].out;
}

void CH_Enable(OPL3 *opl, int ch)
{
	if(opl->opl3mode)
	{
		if(opl->Channels[ch].chtype == CH_4OP)
		{
			EG_KeyOn(opl,opl->Channels[ch].op1id,EG_KEY_NORM);
			EG_KeyOn(opl,opl->Channels[ch].op2id,EG_KEY_NORM);
			EG_KeyOn(opl,opl->Channels[ch+3].op1id,EG_KEY_NORM);
			EG_KeyOn(opl,opl->Channels[ch+3].op2id,EG_KEY_NORM);
		}
		else if(opl->Channels[ch].chtype == CH_2OP)
		{
			EG_KeyOn(opl,opl->Channels[ch].op1id,EG_KEY_NORM);
			EG_KeyOn(opl,opl->Channels[ch].op2id,EG_KEY_NORM);
		}
	}
	else
	{
		EG_KeyOn(opl,opl->Channels[ch].op1id,EG_KEY_NORM);
		EG_KeyOn(opl,opl->Channels[ch].op2id,EG_KEY_NORM);
	}
}

void CH_Disable(OPL3 *opl, int ch)
{
	if(opl->opl3mode)
	{
		if(opl->Channels[ch].chtype == CH_4OP)
		{
			EG_KeyOff(opl,opl->Channels[ch].op1id,EG_KEY_NORM);
			EG_KeyOff(opl,opl->Channels[ch].op2id,EG_KEY_NORM);
			EG_KeyOff(opl,opl->Channels[ch+3].op1id,EG_KEY_NORM);
			EG_KeyOff(opl,opl->Channels[ch+3].op2id,EG_KEY_NORM);
		}
		else if(opl->Channels[ch].chtype == CH_2OP)
		{
			EG_KeyOff(opl,opl->Channels[ch].op1id,EG_KEY_NORM);
			EG_KeyOff(opl,opl->Channels[ch].op2id,EG_KEY_NORM);
		}
	}
	else
	{
		EG_KeyOff(opl,opl->Channels[ch].op1id,EG_KEY_NORM);
		EG_KeyOff(opl,opl->Channels[ch].op2id,EG_KEY_NORM);
	}
}

void opl_inittables()
{
	trem_table[0] = 0;
	trem_table[1] = 0;
	trem_table[2] = 0;
	trem_table[107] = 26;
	trem_table[108] = 26;
	trem_table[109] = 26;
	for(int i = 3,x = 0; i < 107; i+=4, x++)
	{
		trem_table[i]=x;
		trem_table[i+1]=x;
		trem_table[i+2]=x;
		trem_table[i+3]=x;
	}
	for(int i = 110,x = 25; i < 210; i+=4, x--)
	{
		trem_table[i]=x;
		trem_table[i+1]=x;
		trem_table[i+2]=x;
		trem_table[i+3]=x;
	}
	for(int i = 0; i < 15; i++)
	{
		eg_sltable[i]=i*16;
	}
	eg_sltable[15] = 496;
	for(int i = 0; i < 16; i++)
	{
		for(int j = 7,x=kslrom[i]; j>0 && x>=0; j--, x-=8)
		{
			ksl_table[i][j]=x;
		}
	}
	for(int i = 0; i < 6226; i++)
	{
		exptab[i] = (((exprom[(i&0xff)^0xff]*2+2048)>>(i/256)));
	}
	for(int i = 0; i < 256; i++)
	{
		sin_table[0][i] = logsinrom[i];
		negsin_table[0][i] = 0;
		sin_table[0][i+256] = logsinrom[255-i];
		negsin_table[0][i+256] = 0;
		sin_table[0][i+512] = logsinrom[i];
		negsin_table[0][i+512] = 1;
		sin_table[0][i+768] = logsinrom[255-i];
		negsin_table[0][i+768] = 1;

		sin_table[1][i] = logsinrom[i];
		negsin_table[1][i] = 0;
		sin_table[1][i+256] = logsinrom[255-i];
		negsin_table[1][i+256] = 0;
		sin_table[1][i+512] = logsinrom[0];
		negsin_table[1][i+512] = 1;
		sin_table[1][i+768] = logsinrom[0];
		negsin_table[1][i+768] = 1;
		
		sin_table[2][i] = logsinrom[i];
		negsin_table[2][i] = 0;
		sin_table[2][i+256] = logsinrom[255-i];
		negsin_table[2][i+256] = 0;
		sin_table[2][i+512] = logsinrom[i];
		negsin_table[2][i+512] = 0;
		sin_table[2][i+768] = logsinrom[255-i];
		negsin_table[2][i+768] = 0;
		
		sin_table[3][i] = logsinrom[i];
		negsin_table[3][i] = 0;
		sin_table[3][i+256] = logsinrom[0];
		negsin_table[3][i+256] = 1;
		sin_table[3][i+512] = logsinrom[i];
		negsin_table[3][i+512] = 0;
		sin_table[3][i+768] = logsinrom[0];
		negsin_table[3][i+768] = 1;

	}
	for(int i = 0; i < 512; i++)
	{
		sin_table[7][i] = i*8;
		negsin_table[7][i] = 0;
		sin_table[7][1023-i] = i*8;
		negsin_table[7][1023-i] = 1;
		sin_table[6][i] = 0;
		negsin_table[6][i] = 0;
		sin_table[6][512+i] = 0;
		negsin_table[6][512+i] = 1;
	}
	for(int i = 0; i < 128; i++)
	{
		sin_table[4][i] = logsinrom[i*2];
		negsin_table[4][i] = 0;
		sin_table[4][i+128] = logsinrom[255 - i*2];
		negsin_table[4][i+128] = 0;
		sin_table[4][i+256] = logsinrom[i*2];
		negsin_table[4][i+256] = 1;
		sin_table[4][i+384] = logsinrom[255 - i*2];
		negsin_table[4][i+384] = 1;
		sin_table[4][i+512] = logsinrom[0];
		negsin_table[4][i+512] = 1;
		sin_table[4][i+128+512] = logsinrom[0];
		negsin_table[4][i+128+512] = 1;
		sin_table[4][i+256+512] = logsinrom[0];
		negsin_table[4][i+256+512] = 1;
		sin_table[4][i+384+512] = logsinrom[0];
		negsin_table[4][i+384+512] = 1;

		sin_table[5][i] = logsinrom[i*2];
		negsin_table[5][i] = 0;
		sin_table[5][i+128] = logsinrom[255 - i*2];
		negsin_table[5][i+128] = 0;
		sin_table[5][i+256] = logsinrom[i*2];
		negsin_table[5][i+256] = 0;
		sin_table[5][i+384] = logsinrom[255 - i*2];
		negsin_table[5][i+384] = 0;
		sin_table[5][i+512] = logsinrom[0];
		negsin_table[5][i+512] = 1;
		sin_table[5][i+128+512] = logsinrom[0];
		negsin_table[5][i+128+512] = 1;
		sin_table[5][i+256+512] = logsinrom[0];
		negsin_table[5][i+256+512] = 1;
		sin_table[5][i+384+512] = logsinrom[0];
		negsin_table[5][i+384+512] = 1;
	}
}

void* opl_init()
{
	OPL3 *opl;
	opl = (OPL3 *)malloc(sizeof(OPL3));	
	if(opl == NULL)
	{
		return NULL;
	}
   memset(opl,0,sizeof(OPL3));
	if(!tables)
	{
		opl_inittables();
		tables = 1;
	}
	opl->noise = 1;
	for(int i = 0; i < 36; i++)
	{
		opl->OPs[i].out = 6225;
		opl->OPs[i].offset = op_offset[i%18]+((i>17)<<8);
		opl->OPs[i].prevout[0] = 0;
		opl->OPs[i].prevout[1] = 0; 
		EG_Reset(opl,i);
	}
	for(int i = 0; i < 9; i++)
	{
		opl->Channels[i].op1id = i*2;
		opl->Channels[i+9].op1id = i*2+18;
		opl->Channels[i].op2id = i*2+1;
		opl->Channels[i+9].op2id = i*2+1+18;
		opl->Channels[i].con = 0;
		opl->Channels[i+9].con = 0;
		opl->Channels[i].chtype = CH_2OP;
		opl->Channels[i+9].chtype = CH_2OP;
		opl->Channels[i].alg = 0;
		opl->Channels[i+9].alg = 0;
		opl->Channels[i].offset = i;
		opl->Channels[i+9].offset = 0x100 + i;
		opl->Channels[i].out = 0;
		opl->Channels[i+9].out = 0;
		opl->Channels[i].cha = ~0;
		opl->Channels[i+9].cha = ~0;
		opl->Channels[i].chb = ~0;
		opl->Channels[i+9].chb = ~0;
		opl->Channels[i].chc = ~0;
		opl->Channels[i+9].chc = ~0;
		opl->Channels[i].chd = ~0;
		opl->Channels[i+9].chd = ~0;
	}
	opl->timer = 0;
	opl->trem_inc = 0;
	opl->trem_pos = 0;
	opl->vib_pos = 0;
	return opl;
}
void opl_writereg(void *opl3, int regg, int data)
{
	OPL3 *opl = (OPL3*)opl3;
	data&=0xFF;
	regg&=0x1FF;
	int highbank = (regg>>8)&0x01;
	int reg=regg&0xFF;
	opl->opl_memory[regg&0x1FF] = data;
	switch(reg&0xF0)
	{
	case 0x00:
		if(highbank)
		{
			switch(reg&0x0F)
			{
			case 0x04:
				//OPL3's 4op mode
				CH_Set2OP(opl);
				CH_Set4OP(opl);
				break;
			case 0x05:
				//OPL3 mode
				opl->opl3mode = data&0x01;
				break;
			}
		}
		else
		{
			switch(reg&0x0F)
			{
			case 0x08:
				opl->nts = (data>>6)&0x01;
				break;
			}
		}
		break;
	case 0x20:
	case 0x30:
		if(ad_slot[reg&0x1F]>=0)
			OP_Update20(opl,18*highbank+ad_slot[reg&0x1F]);
		break;
	case 0x40:
	case 0x50:
		if(ad_slot[reg&0x1F]>=0)
			OP_Update40(opl,18*highbank+ad_slot[reg&0x1F]);
		break;
	case 0x60:
	case 0x70:
		if(ad_slot[reg&0x1F]>=0)
			OP_Update60(opl,18*highbank+ad_slot[reg&0x1F]);
		break;
	case 0x80:
	case 0x90:
		if(ad_slot[reg&0x1F]>=0)
			OP_Update80(opl,18*highbank+ad_slot[reg&0x1F]);
		break;
	case 0xE0:
	case 0xF0:
		if(ad_slot[reg&0x1F]>=0)
			OP_UpdateE0(opl,18*highbank+ad_slot[reg&0x1F]);
		break;
	case 0xA0:
		if((reg&0xf)<9)
		{
			CH_UpdateAB0(opl,9*highbank+(reg&0x0f));
		}
		break;
	case 0xB0:
		if(reg == 0xBD && !highbank)
		{
			opl->dam = data>>7;
			opl->dvb = (data>>6)&0x01;
			CH_UpdateRhythm(opl);
		}
		else
		if((reg&0xf)<9)
		{
			CH_UpdateAB0(opl,9*highbank+(reg&0x0f));
			if((data&0x20) == 0x20)
				CH_Enable(opl,9*highbank+(reg&0x0f));
			else
				CH_Disable(opl,9*highbank+(reg&0x0f));
		}
		break;
	case 0xC0:
		if((reg&0xf)<9)
		{
			CH_UpdateC0(opl,9*highbank+(reg&0x0f));
		}
		break;
	}
}

int16_t limshort(int a)
{
	if(a>32767)
	{
		a = 32767;
	}
	else if(a<-32768)
	{
		a = -32768;
	}
	return (int16_t)(a);
}

void opl_getoutput(void *opl3, int16_t *buffer, int len)
{
	OPL3 *opl = (OPL3*)opl3;
	int16_t diga,digb;
	for(int i = 0; i < len; i++)
	{
		int outa = 0,outb = 0;
		opl->trem_inc++;
		if((opl->trem_inc>>6))
		{
			opl->trem_pos++;
			if(opl->trem_pos==210)
			{
				opl->trem_pos = 0;
			}
			opl->trem_inc&=0x3F;
		}
		opl->vib_pos++;
		CH_GenerateRhythm(opl);
		for(int ii = 0; ii < 18; ii++)
		{
			CH_Generate(opl,ii);
			outa+=opl->Channels[ii].out&opl->Channels[ii].cha;
			outb+=opl->Channels[ii].out&opl->Channels[ii].chb;
		}
		N_Generate(opl);
		opl->timer++;
		for(int ii = 0; ii < 36; ii++)
		{
			EG_Generate(opl,ii);
			PG_Generate(opl,ii);
		}
		diga = limshort(outa);
		digb = limshort(outb);
		*buffer++ = diga;
		*buffer++ = digb;
	}
}