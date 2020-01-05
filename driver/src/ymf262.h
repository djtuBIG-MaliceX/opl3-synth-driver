#ifndef MAME_YMF262_H
#define MAME_YMF262_H

#include "stdafx.h"

/* select number of output bits: 8 or 16 */
#define OPL3_SAMPLE_BITS 16

typedef int32_t OPL3SAMPLE;

typedef void (*OPL3_TIMERHANDLER)(void *param,int timer,int period);
typedef void (*OPL3_IRQHANDLER)(void *param,int irq);
typedef void (*OPL3_UPDATEHANDLER)(void *param/*,int min_interval_us*/);



typedef struct {
   uint32_t	ar;			/* attack rate: AR<<2           */
   uint32_t	dr;			/* decay rate:  DR<<2           */
   uint32_t	rr;			/* release rate:RR<<2           */
   uint8_t	KSR;		/* key scale rate               */
   uint8_t	ksl;		/* keyscale level               */
   uint8_t	ksr;		/* key scale rate: kcode>>KSR   */
   uint8_t	mul;		/* multiple: mul_tab[ML]        */

   /* Phase Generator */
   uint32_t	Cnt;		/* frequency counter            */
   uint32_t	Incr;		/* frequency counter step       */
   uint8_t   FB;			/* feedback shift value         */
   int32_t* connect;	/* slot output pointer          */
   int32_t   op1_out[2];	/* slot1 output for feedback    */
   uint8_t   CON;		/* connection (algorithm) type  */

   /* Envelope Generator */
   uint8_t	eg_type;	/* percussive/non-percussive mode */
   uint8_t	state;		/* phase type                   */
   uint32_t	TL;			/* total level: TL << 2         */
   int32_t	TLL;		/* adjusted now TL              */
   int32_t	volume;		/* envelope counter             */
   uint32_t	sl;			/* sustain level: sl_tab[SL]    */

   uint32_t	eg_m_ar;	/* (attack state)               */
   uint8_t	eg_sh_ar;	/* (attack state)               */
   uint8_t	eg_sel_ar;	/* (attack state)               */
   uint32_t	eg_m_dr;	/* (decay state)                */
   uint8_t	eg_sh_dr;	/* (decay state)                */
   uint8_t	eg_sel_dr;	/* (decay state)                */
   uint32_t	eg_m_rr;	/* (release state)              */
   uint8_t	eg_sh_rr;	/* (release state)              */
   uint8_t	eg_sel_rr;	/* (release state)              */

   uint32_t	key;		/* 0 = KEY OFF, >0 = KEY ON     */

   /* LFO */
   uint32_t	AMmask;		/* LFO Amplitude Modulation enable mask */
   uint8_t	vib;		/* LFO Phase Modulation enable flag (active high)*/

   /* waveform select */
   uint8_t	waveform_number;
   unsigned int wavetable;

   //unsigned char reserved[128-84];//speedup: pump up the struct size to power of 2
   unsigned char reserved[128 - 100];//speedup: pump up the struct size to power of 2

} OPL3_SLOT;

typedef struct {
   OPL3_SLOT SLOT[2];

   uint32_t	block_fnum;	/* block+fnum                   */
   uint32_t	fc;			/* Freq. Increment base         */
   uint32_t	ksl_base;	/* KeyScaleLevel Base step      */
   uint8_t	kcode;		/* key code (for key scaling)   */

   /*
  there are 12 2-operator channels which can be combined in pairs
  to form six 4-operator channel, they are:
   0 and 3,
   1 and 4,
   2 and 5,
   9 and 12,
   10 and 13,
   11 and 14
*/
   uint8_t	extended;	/* set to 1 if this channel forms up a 4op channel with another channel(only used by first of pair of channels, ie 0,1,2 and 9,10,11) */
   uint8_t	Muted;

   unsigned char reserved[512 - 272];//speedup:pump up the struct size to power of 2

} OPL3_CH;

/* OPL3 state */
typedef struct {
   OPL3_CH	P_CH[18];				/* OPL3 chips have 18 channels  */

   uint32_t	pan[18 * 4];				/* channels output masks (0xffffffff = enable); 4 masks per one channel */
   uint32_t	pan_ctrl_value[18];		/* output control values 1 per one channel (1 value contains 4 masks) */
   uint8_t	MuteSpc[5];				/* for the 5 Rhythm Channels */

   signed int chanout[18];			/* 18 channels */
   signed int phase_modulation;	/* phase modulation input (SLOT 2) */
   signed int phase_modulation2;	/* phase modulation input (SLOT 3 in 4 operator channels) */

   uint32_t	eg_cnt;					/* global envelope generator counter    */
   uint32_t	eg_timer;				/* global envelope generator counter works at frequency = chipclock/288 (288=8*36) */
   uint32_t	eg_timer_add;			/* step of eg_timer                     */
   uint32_t	eg_timer_overflow;		/* envelope generator timer overlfows every 1 sample (on real chip) */

   uint32_t	fn_tab[1024];			/* fnumber->increment counter   */

   /* LFO */
   uint32_t	LFO_AM;
   int32_t	LFO_PM;
   uint8_t	lfo_am_depth;
   uint8_t	lfo_pm_depth_range;
   uint32_t	lfo_am_cnt;
   uint32_t	lfo_am_inc;
   uint32_t	lfo_pm_cnt;
   uint32_t	lfo_pm_inc;

   uint32_t	noise_rng;				/* 23 bit noise shift register  */
   uint32_t	noise_p;				/* current noise 'phase'        */
   uint32_t	noise_f;				/* current noise period         */

   uint8_t	OPL3_mode;				/* OPL3 extension enable flag   */

   uint8_t	rhythm;					/* Rhythm mode                  */

   int		T[2];					/* timer counters               */
   uint8_t	st[2];					/* timer enable                 */

   uint32_t	address;				/* address register             */
   uint8_t	status;					/* status flag                  */
   uint8_t	statusmask;				/* status mask                  */

   uint8_t	nts;					/* NTS (note select)            */

   /* external event callback handlers */
   OPL3_TIMERHANDLER  timer_handler;/* TIMER handler                */
   void* TimerParam;					/* TIMER parameter              */
   OPL3_IRQHANDLER    IRQHandler;	/* IRQ handler                  */
   void* IRQParam;					/* IRQ parameter                */
   OPL3_UPDATEHANDLER UpdateHandler;/* stream update handler       */
   void* UpdateParam;				/* stream update parameter      */

   uint8_t type;						/* chip type                    */
   int clock;						/* master clock  (Hz)           */
   int rate;						/* sampling rate (Hz)           */
   double freqbase;				/* frequency base               */
   //attotime TimerBase;			/* Timer base time (==sampling time)*/
} OPL3;

void *ymf262_init(int clock, int rate);
void ymf262_shutdown(void *chip);
void ymf262_reset_chip(void *chip);
int  ymf262_write(void *chip, int a, int v);
unsigned char ymf262_read(void *chip, int a);
int  ymf262_timer_over(void *chip, int c);
void ymf262_update_one(void *chip, OPL3SAMPLE **buffers, int length);

void ymf262_set_timer_handler(void *chip, OPL3_TIMERHANDLER TimerHandler, void *param);
void ymf262_set_irq_handler(void *chip, OPL3_IRQHANDLER IRQHandler, void *param);
void ymf262_set_update_handler(void *chip, OPL3_UPDATEHANDLER UpdateHandler, void *param);

void ymf262_set_emu_core(uint8_t Emulator);
void ymf262_set_mutemask(void *chip, uint32_t MuteMask);

#endif