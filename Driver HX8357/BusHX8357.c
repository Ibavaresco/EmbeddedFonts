/*==============================================================================
 Copyright (c) 2005-2026, Isaac Marino Bavaresco
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Neither the name of the author nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.;;

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================*/

#include "asf.h"
#include "pwm.h"
#include "BusHX8357.h"
#include <delay.h>

#include "SimpleRTOS/PortSAM3Internals.h"
//==============================================================================
#if 0
static void __attribute__((noinline,section(".ramfunc"))) delay_50ns( unsigned long n )
	{
	UNUSED(n);

	asm volatile(
		"Loop:	dmb					\n"
		"		dmb					\n"
		"		subs	R0, R0, #1  \n"
		"		bne.n	Loop		\n"
		);
	}
#endif
//==============================================================================
#define Delay_tWRL()	do{asm volatile( "dmb" );}while(0)
	
#define Delay_tRDLRAM()	do{asm volatile( "dmb" );}while(0)
#define Delay_tRDHRAM()	do{asm volatile( "dmb" );}while(0)
	
#define Delay_tRDLREG()	do{asm volatile( "dmb" );}while(0)
#define Delay_tRDHREG()	do{asm volatile( "dmb" );}while(0)
	
#define Delay_tWRH()	do{asm volatile( "dmb" );}while(0)
//==============================================================================
// Definições de pinos do processador (todos no porto B)

#define	LCD_DATA_PORT		PIOA
#define	LCD_DATA_FIRST_BIT	 0

#define	LCD_CS_PORT			PIOA
#define	LCD_CS_BIT			29

#define	LCD_RD_PORT			PIOA
#define	LCD_RD_BIT			30

#define	LCD_nWR_PORT		PIOC
#define	LCD_nWR_BIT			 8

#define	LCD_DC_PORT			PIOC
#define	LCD_DC_BIT			10

#define	LCD_nRST_PORT		PIOC
#define	LCD_nRST_BIT		 9
//==============================================================================

#define LCD_DATA_MASK		(0x000000ff<<LCD_DATA_FIRST_BIT)
#define	LCD_DATA_PDSR		LCD_DATA_PORT->PIO_PDSR
#define	LCD_DATA_ODR		LCD_DATA_PORT->PIO_ODR
#define	LCD_DATA_OER		LCD_DATA_PORT->PIO_OER
#define	LCD_DATA_OWDR		LCD_DATA_PORT->PIO_OWDR
#define	LCD_DATA_OWER		LCD_DATA_PORT->PIO_OWER
#define	LCD_DATA_ODSR		LCD_DATA_PORT->PIO_ODSR
#define	LCD_DATA_PUDR		LCD_DATA_PORT->PIO_PUDR
#define	LCD_DATA_PUER		LCD_DATA_PORT->PIO_PUER
#define	LCD_DATA_CODR		LCD_DATA_PORT->PIO_CODR
#define	LCD_DATA_SODR		LCD_DATA_PORT->PIO_SODR
#define	LCD_DATA_MDDR		LCD_DATA_PORT->PIO_MDDR
#define	LCD_DATA_PER		LCD_DATA_PORT->PIO_PER

#define LCD_CS_MASK			(1<<LCD_CS_BIT)
#define	LCD_CS_CODR			LCD_CS_PORT->PIO_CODR
#define	LCD_CS_SODR			LCD_CS_PORT->PIO_SODR
#define	LCD_CS_MDDR			LCD_CS_PORT->PIO_MDDR
#define	LCD_CS_PER			LCD_CS_PORT->PIO_PER
#define	LCD_CS_OER			LCD_CS_PORT->PIO_OER
#define	LCD_CS_PUDR			LCD_CS_PORT->PIO_PUDR

#define LCD_RD_MASK			(1<<LCD_RD_BIT)
#define	LCD_RD_CODR			LCD_RD_PORT->PIO_CODR
#define	LCD_RD_SODR			LCD_RD_PORT->PIO_SODR
#define	LCD_RD_MDDR			LCD_RD_PORT->PIO_MDDR
#define	LCD_RD_PER			LCD_RD_PORT->PIO_PER
#define	LCD_RD_OER			LCD_RD_PORT->PIO_OER
#define	LCD_RD_PUDR			LCD_RD_PORT->PIO_PUDR

#define LCD_nWR_MASK		(1<<LCD_nWR_BIT)
#define	LCD_nWR_CODR		LCD_nWR_PORT->PIO_CODR
#define	LCD_nWR_SODR		LCD_nWR_PORT->PIO_SODR
#define	LCD_nWR_MDDR		LCD_nWR_PORT->PIO_MDDR
#define	LCD_nWR_PER			LCD_nWR_PORT->PIO_PER
#define	LCD_nWR_OER			LCD_nWR_PORT->PIO_OER
#define	LCD_nWR_PUDR		LCD_nWR_PORT->PIO_PUDR

#define LCD_DC_MASK			(1<<LCD_DC_BIT)
#define	LCD_DC_CODR			LCD_DC_PORT->PIO_CODR
#define	LCD_DC_SODR			LCD_DC_PORT->PIO_SODR
#define	LCD_DC_MDDR			LCD_DC_PORT->PIO_MDDR
#define	LCD_DC_PER			LCD_DC_PORT->PIO_PER
#define	LCD_DC_OER			LCD_DC_PORT->PIO_OER
#define	LCD_DC_PUDR			LCD_DC_PORT->PIO_PUDR

#define LCD_nRST_MASK		(1<<LCD_nRST_BIT)
#define	LCD_nRST_CODR		LCD_nRST_PORT->PIO_CODR
#define	LCD_nRST_SODR		LCD_nRST_PORT->PIO_SODR
#define	LCD_nRST_MDDR		LCD_nRST_PORT->PIO_MDDR
#define	LCD_nRST_PER		LCD_nRST_PORT->PIO_PER
#define	LCD_nRST_OER		LCD_nRST_PORT->PIO_OER
#define	LCD_nRST_PUDR		LCD_nRST_PORT->PIO_PUDR

//==============================================================================
#define	LCD_DataBusInit()			(LCD_DATA_PUER=LCD_DATA_MASK,LCD_DATA_ODR=LCD_DATA_MASK,LCD_DATA_MDDR=LCD_DATA_MASK,LCD_DATA_CODR=LCD_DATA_MASK,LCD_DATA_PER=LCD_DATA_MASK,LCD_DATA_OWDR=~LCD_DATA_MASK,LCD_DATA_OWER=LCD_DATA_MASK)
#define	LCD_DataBusSetAsInputs()	(LCD_DATA_ODR = LCD_DATA_MASK)
#define	LCD_DataBusSetAsOutputs()	(LCD_DATA_OER = LCD_DATA_MASK)
#define	LCD_DataBusReadValue()		((LCD_DATA_PDSR>>LCD_DATA_FIRST_BIT)&0xff)
#define	LCD_DataBusSetValue(v)		(LCD_DATA_ODSR = ( LCD_DATA_ODSR & ~( 0xff << LCD_DATA_FIRST_BIT )) | (( v & 0xff ) << LCD_DATA_FIRST_BIT ))

#define	LCD_DCSetAsData()			(LCD_DC_SODR=LCD_DC_MASK)
#define	LCD_DCSetAsCommand()		(LCD_DC_CODR=LCD_DC_MASK)
#define	LCD_DCSetAsOutput()			(LCD_DC_MDDR= LCD_DC_MDDR,LCD_DC_PER=LCD_DC_MASK,LCD_DC_OER=LCD_DC_MASK,LCD_DC_PUDR=LCD_DC_PUDR)

#define	LCD_CSEnable()				(LCD_CS_SODR=LCD_CS_MASK)
#define	LCD_CSDisable()				(LCD_CS_CODR=LCD_CS_MASK)
#define	LCD_CSSetAsOutput()			(LCD_CS_MDDR= LCD_CS_MDDR,LCD_CS_PER=LCD_CS_MASK,LCD_CS_OER=LCD_CS_MASK,LCD_CS_PUDR=LCD_CS_PUDR)

#define	LCD_RDDisable()				(LCD_RD_CODR=LCD_RD_MASK)
#define	LCD_RDEnable()				(LCD_RD_SODR=LCD_RD_MASK)
#define	LCD_RDSetAsOutput()			(LCD_RD_MDDR= LCD_RD_MDDR,LCD_RD_PER=LCD_RD_MASK,LCD_RD_OER=LCD_RD_MASK,LCD_RD_PUDR=LCD_RD_PUDR)

#define	LCD_nWRDisable()			(LCD_nWR_SODR=LCD_nWR_MASK)
#define	LCD_nWREnable()				(LCD_nWR_CODR=LCD_nWR_MASK)
#define	LCD_nWRSetAsOutput()		(LCD_nWR_MDDR= LCD_nWR_MDDR,LCD_nWR_PER=LCD_nWR_MASK,LCD_nWR_OER=LCD_nWR_MASK,LCD_nWR_PUDR=LCD_nWR_PUDR)

#define	LCD_nRSTEnable()			(LCD_nRST_CODR=LCD_nRST_MASK)
#define	LCD_nRSTDisable()			(LCD_nRST_SODR=LCD_nRST_MASK)
#define	LCD_nRSTSetAsOutput()		(LCD_nRST_MDDR= LCD_nRST_MDDR,LCD_nRST_PER=LCD_nRST_MASK,LCD_nRST_OER=LCD_nRST_MASK,LCD_nRST_PUDR=LCD_nRST_PUDR)
//==============================================================================
#if 1
void __attribute__((section(".ramfunc"))) BusHX8357WriteLCDCmd( unsigned char Value )
	{
	intsave_t	s	= SaveAndDisableInterrupts();

	LCD_DCSetAsCommand();			// DC	= 0 (Command)
	LCD_nWREnable();				// nWR	= 0 (Enable)
	LCD_CSEnable();					// CS	= 1 (Enable)
	LCD_DataBusSetValue( Value );
	LCD_DataBusSetAsOutputs();
	Delay_tWRL();
	LCD_nWRDisable();				// nWR	= 1 (Disable)
	LCD_CSDisable();				// CS	= 1 (Enable)

	RestoreInterrupts( s );
	}
#else
void __attribute__((naked,section(".ramfunc"))) BusHX8357WriteLCDCmd( unsigned char Value )
	{
	asm volatile(
		"push	{r4}				\n"

		//intsave_t	s	= SaveAndDisableInterrupts();
		"mov.w	r2, #128			\n"	// 0x80
		"mrs	r4, BASEPRI			\n"
		"msr	BASEPRI, r2			\n"

		"ldr	r3, =0x400e1000		\n" // (8ba00 <BusWriteLCD+0xb4>)

		//if( DataInstruction )
		"mov.w	r2, #8388608		\n"	// 0x800000
		"str	r2, [r3, #52]		\n"	// 0x30

		//LCD_CS_SODR		= LCD_CS_MASK;
		"mov.w	r2, #16384			\n"	// 0x4000
		"str	r2, [r3, #48]		\n"	// 0x30

		//LCD_nWR_CODR	= LCD_nWR_MASK;
		"mov.w	r2, #16777216		\n"	// 0x1000000
		"str	r2, [r3, #52]		\n"	// 0x34

		//ldr	r1, [pc, #84]	; (8ba04 <BusWriteLCD+0xb8>)
		"ldr	r1, =0x400e1200		\n"	// (8ba04 <BusWriteLCD+0xb8>)

		//LCD_DATA_ODSR	= Value << LCD_DATA_FIRST_BIT;
		"lsls	r0, r0, #21			\n"
		"str	r0, [r1, #56]		\n"	// 0x38

		//LCD_DATA_OER	= LCD_DATA_MASK;
		//		"mov.w	r2, #534773760		\n"	// 0x1fe00000
		"str	r2, [r1, #16]		\n"

		//LCD_nWR_SODR	= LCD_nWR_MASK;
		"mov.w	r2, #16777216		\n"	// 0x1000000
		"str	r2, [r3, #48]		\n"	// 0x30

		//LCD_CS_CODR	= LCD_CS_MASK;
		"mov.w	r2, #16384			\n"	// 0x4000
		"str	r2, [r3, #52]		\n"	// 0x34

		//RestoreInterrupts( s );
		"msr	BASEPRI, r4			\n"

		"pop	{r4}				\n"
		"bx		lr					\n"
		//"nop						\n"
		//:: "r"(DataInstruction), "r"(Value) : "r0", "r1", "r2", "r3" );
		::: "r0", "r1", "r2", "r3" );
	}
#endif

//==============================================================================
#if 1
void __attribute__((section(".ramfunc"))) BusHX8357WriteLCDData( unsigned char Value )
	{
	intsave_t	s	= SaveAndDisableInterrupts();

	LCD_DCSetAsData();				// DC	= 0 (Command)
	LCD_nWREnable();				// nWR	= 0 (Enable)
	LCD_CSEnable();					// CS	= 1 (Enable)
	LCD_DataBusSetValue( Value );
	LCD_DataBusSetAsOutputs();
	Delay_tWRL();
	LCD_nWRDisable();				// nWR	= 1 (Disable)
	LCD_CSDisable();				// CS	= 1 (Enable)
	Delay_tWRH();

	RestoreInterrupts( s );
	}
#else
void __attribute__((naked,section(".ramfunc"))) BusHX8357WriteLCDData( unsigned char Value )
	{
	asm volatile(
		"push	{r4}				\n"

		//intsave_t	s	= SaveAndDisableInterrupts();
		"mov.w	r2, #128			\n"	// 0x80
		"mrs	r4, BASEPRI			\n"
		"msr	BASEPRI, r2			\n"

		"ldr	r3, =0x400e1000		\n" // (8ba00 <BusWriteLCD+0xb4>)

		//if( DataInstruction )
		"mov.w	r2, #8388608		\n"	// 0x800000
		"str	r2, [r3, #48]		\n"	// 0x30

		//LCD_CS_SODR		= LCD_CS_MASK;
		"mov.w	r2, #16384			\n"	// 0x4000
		"str	r2, [r3, #48]		\n"	// 0x30

		//LCD_nWR_CODR	= LCD_nWR_MASK;
		"mov.w	r2, #16777216		\n"	// 0x1000000
		"str	r2, [r3, #52]		\n"	// 0x34

		//ldr	r1, [pc, #84]	; (8ba04 <BusWriteLCD+0xb8>)
		"ldr	r1, =0x400e1200		\n"	// (8ba04 <BusWriteLCD+0xb8>)

		//LCD_DATA_ODSR	= Value << LCD_DATA_FIRST_BIT;
		"lsls	r0, r0, #21			\n"
		"str	r0, [r1, #56]		\n"	// 0x38

		//LCD_DATA_OER	= LCD_DATA_MASK;
		//		"mov.w	r2, #534773760		\n"	// 0x1fe00000
		"str	r2, [r1, #16]		\n"

		//LCD_nWR_SODR	= LCD_nWR_MASK;
		"mov.w	r2, #16777216		\n"	// 0x1000000
		"str	r2, [r3, #48]		\n"	// 0x30

		//LCD_CS_CODR	= LCD_CS_MASK;
		"mov.w	r2, #16384			\n"	// 0x4000
		"str	r2, [r3, #52]		\n"	// 0x34

		//RestoreInterrupts( s );
		"msr	BASEPRI, r4			\n"

		"pop	{r4}				\n"
		"bx		lr					\n"
		//"nop						\n"
		//:: "r"(DataInstruction), "r"(Value) : "r0", "r1", "r2", "r3" );
		::: "r0", "r1", "r2", "r3" );
	}
#endif
//==============================================================================
#if	1
unsigned char __attribute__((section(".ramfunc"))) BusHX8357ReadLCDData( void )
	{
	unsigned char	Aux;

	LCD_CSEnable();
	LCD_DCSetAsData();
	LCD_DataBusSetAsInputs();
	LCD_RDEnable();
	Delay_tRDLRAM();
	Aux	= LCD_DataBusReadValue();
	LCD_RDDisable();
	LCD_CSDisable();
	Delay_tRDHRAM();

	return Aux;
	}
#else
unsigned char __attribute__((naked,section(".ramfunc"))) BusHX8357ReadLCDData( void )
	{
	asm volatile(
		//intsave_t	s	= SaveAndDisableInterrupts();
		"mov.w	r2, #128			\n"	// 0x80
		"mrs	r1, BASEPRI			\n"
		"msr	BASEPRI, r2			\n"

		"ldr	r3, =0x400e1000		\n" // (8ba00 <BusWriteLCD+0xb4>)

		//LCD_RS_SODR		= LCD_RS_MASK;
		"mov.w	r2, #0x00800000		\n"
		"str	r2, [r3, #0x030]	\n"

		//LCD_DATA_ODR	= LCD_DATA_MASK;
		"mov.w	r2,#0x1FE00000		\n"
		"str	r2, [r3, #0x214]	\n"

		//LCD_CS_SODR		= LCD_CS_MASK;
		"mov.w	r2, #0x00004000		\n"
		"str	r2, [r3, #0x030]	\n"

		//LCD_RD_SODR		= LCD_RD_MASK;
		"mov.w	r2, #0x00400000		\n"
		"str	r2, [r3, #0x030]	\n"

		//LCD_DATA_ODSR	= Value << LCD_DATA_FIRST_BIT;
		"ldr	r0, [r3, #0x238]	\n"
		"lsrs	r0, r0, #21			\n"

		//LCD_RD_CODR		= LCD_RD_MASK;
		"str	r2, [r3, #0x034]	\n"

		//LCD_CS_CODR	= LCD_CS_MASK;
		"mov.w	r2, #0x00004000		\n"
		"str	r2, [r3, #0x034]	\n"

		//RestoreInterrupts( s );
		"msr	BASEPRI, r1			\n"

		"bx		lr					\n"

		::: "r0", "r1", "r2", "r3" );
	}
#endif
//==============================================================================
#if 0
void __attribute__((naked,section(".ramfunc"))) BusHX8357WriteLCD( unsigned int DataInstruction, unsigned char Value )
	{
	asm volatile(
		"push	{r4}				\n"
	
		//intsave_t	s	= SaveAndDisableInterrupts();
		"mov.w	r2, #128			\n"	// 0x80
		"mrs	r4, BASEPRI			\n"
		"msr	BASEPRI, r2			\n"

		//ldr	r3, [pc, #144]	; (8ba00 <BusWriteLCD+0xb4>)
		"ldr	r3, =0x400e1000		\n" // (8ba00 <BusWriteLCD+0xb4>)

		//LCD_CS_CODR		= LCD_CS_MASK;
//		"mov.w	r2, #16384			\n"	// 0x4000
//		"str	r2, [r3, #52]		\n" // 0x34

		//;LCD_RD_CODR		= LCD_RD_MASK;
//		"mov.w	r2, #4194304		\n"	// 0x400000
//		"str	r2, [r3, #52]		\n"	// 0x34

		//LCD_nWR_SODR	= LCD_nWR_MASK;
//		"mov.w	r2, #16777216		\n"	// 0x1000000
//		"str	r2, [r3, #48]		\n" // 0x30
		
		//if( DataInstruction )
		"cmp	r0, #0				\n"
		"ite	eq					\n"
		"moveq.w	r0,#52			\n"
		"movne.w	r0,#48			\n"

		"mov.w	r2, #8388608		\n"	// 0x800000
		"str	r2, [r3, r0]		\n"	// 0x30

		//LCD_CS_SODR		= LCD_CS_MASK;
		"mov.w	r2, #16384			\n"	// 0x4000
		"str	r2, [r3, #48]		\n"	// 0x30

		//LCD_nWR_CODR	= LCD_nWR_MASK;
		"mov.w	r2, #16777216		\n"	// 0x1000000
		"str	r2, [r3, #52]		\n"	// 0x34

		//ldr	r0, [pc, #84]	; (8ba04 <BusWriteLCD+0xb8>)
		"ldr	r0, =0x400e1200		\n"	// (8ba04 <BusWriteLCD+0xb8>)

		//LCD_DATA_OWDR	= ~LCD_DATA_MASK;
		"mvn.w	r2, #534773760		\n"	// 0x1fe00000
		"str.w	r2, [r0, #164]		\n"	// 0xa4

		//LCD_DATA_OWER	=  LCD_DATA_MASK;
		"mov.w	r2, #534773760		\n"	// 0x1fe00000
		"str.w	r2, [r0, #160]		\n"	// 0xa0

		//LCD_DATA_ODSR	= Value << LCD_DATA_FIRST_BIT;
		"lsls	r1, r1, #21			\n"
		"str	r1, [r0, #56]		\n"	// 0x38

		//LCD_DATA_OER	= LCD_DATA_MASK;
//		"mov.w	r2, #534773760		\n"	// 0x1fe00000
		"str	r2, [r0, #16]		\n"

		//LCD_DATA_OWDR	= 0xffffffff;
//		"mov.w	r2, #4294967295		\n"
//		"str.w	r2, [r0, #164]		\n"	// 0xa4

		//LCD_nWR_SODR	= LCD_nWR_MASK;
		"mov.w	r2, #16777216		\n"	// 0x1000000
		"str	r2, [r3, #48]		\n"	// 0x30

		//LCD_CS_CODR	= LCD_CS_MASK;
		"mov.w	r2, #16384			\n"	// 0x4000
		"str	r2, [r3, #52]		\n"	// 0x34

		//RestoreInterrupts( s );
		"msr	BASEPRI, r4			\n"

		"pop	{r4}				\n"
		"bx		lr					\n"
		//"nop						\n"
		//:: "r"(DataInstruction), "r"(Value) : "r0", "r1", "r2", "r3" );
		::: "r0", "r1", "r2", "r3" );
	}
#endif
//==============================================================================
#if 0
void BusHX8357WriteLCD( unsigned int DataInstruction, unsigned char Value )
	{
#if 1

	intsave_t	s	= SaveAndDisableInterrupts();

#if 1
	// Faz o pino LCD_CS = 0 (Disable).
	LCD_CSDisable();
	// Faz o pino LCD_RD = 0 (Disable).
	LCD_RDDisable();
	// Faz o pino LCD_nWR = 1 (Disable).
	LCD_nWRDisable();
#endif

	if( DataInstruction )
		LCD_DCSetAsData();
	else
		LCD_DCSetAsCommand();

	// Faz o pino LCD_CS = 1 (Enable).
	LCD_CSEnable();

	// Faz o pino LCD_nWR = 0 (Enable).
	LCD_nWREnable();

	// Envia o dado de saída para o barramento de dados.
	LCD_DATA_OWDR	= ~LCD_DATA_MASK;
	LCD_DATA_OWER	=  LCD_DATA_MASK;
	LCD_DATA_ODSR	= Value << LCD_DATA_FIRST_BIT;
	// Faz o barramento de dados como saídas.
	LCD_DataBusSetValue( Value );
	LCD_DataBusSetAsOutputs();
	LCD_DATA_OWDR	= 0xffffffff;

	// Espera o tempo tWRL.
	Delay_tWRL();

	// Faz o pino LCD_nWR = 1 (Disable).
	LCD_nWRDisable();

	// Espera o tempo tWRH.
	Delay_tWRH();

	// Faz o pino LCD_CS = 0 (Disable).
	LCD_CSDisable();

	RestoreInterrupts( s );

#endif	
	}
#endif
//==============================================================================
#if 0
int BusHX8357ReadLCD( unsigned int DataInstruction )
	{
#if 1
	int	Value;

	intsave_t	s	= SaveAndDisableInterrupts();

	// Faz o pino LCD_CS = 0 (Disable).
	LCD_CSDisable();
	// Faz o pino LCD_RD = 1 (Disable).
	LCD_RDDisable();
	// Faz o pino LCD_nWR = 1 (Disable).
	LCD_nWRDisable();

	if( DataInstruction )
		LCD_DCSetAsData();
	else
		LCD_DCSetAsCommand();

	// Faz o barramento de dados como entradas.
	LCD_DataBusSetAsInputs();

	// Faz o pino LCD_CS = 1 (Enable).
	LCD_CSEnable();

	// Faz o pino LCD_RD = 1 (Enable).
	LCD_RDEnable();

	// Espera o tempo tRDL.
	Delay_tRDLRAM();

	Value			= LCD_DataBusReadValue();

	// Faz o pino LCD_nWR = 1 (Disable).
	LCD_nWRDisable();

	// Espera o tempo tRDH.
	Delay_tRDHRAM();

	// Faz o pino LCD_CS = 0 (Disable).
	LCD_CSDisable();

	RestoreInterrupts( s );

	return Value;
#else
	return 0;
#endif
	}
#endif
//==============================================================================
void BusHX8357Init( void )
	{
	PMC->PMC_WPMR	= 0x504D4300;			/* Disable write protect            */
	PMC->PMC_PCER0	= ( 1UL << ID_PIOC );	/* enable PIOC clock                */
	PMC->PMC_WPMR	= 0x504D4301;			/* Enable write protect             */
//	/* Enable PWM peripheral clock */
//	pmc_enable_periph_clk( ID_PIOC );

	LCD_nRSTEnable();
	LCD_nRSTSetAsOutput();

	/* Barramento de dados */
	LCD_DataBusInit();

	LCD_CSDisable();
	LCD_CSSetAsOutput();

	LCD_RDDisable();
	LCD_RDSetAsOutput();

	LCD_nWRDisable();
	LCD_nWRSetAsOutput();

	LCD_DCSetAsCommand();
	LCD_DCSetAsOutput();

	delay_ms( 1 );

	LCD_nRSTDisable();
	}
//==============================================================================
