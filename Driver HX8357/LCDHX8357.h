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
#if			!defined __LCD_HX8357_H__
#define		__LCD_HX8357_H__
/*============================================================================*/
#include "LCDInternals.h"
/*============================================================================*/
extern const lcd_t	LCDHX8357;
/*============================================================================*/
const lcd_t	*LCDHX8357Probe		( void );
void		LCDHX8357Init		( void );
/*============================================================================*/
#endif	/*	__LCD_HX8357_H__ */
/*============================================================================*/
#if 0
/*============================================================================*/
#if			!defined __LCD_HX8357_H__
#define __LCD_HX8357_H__
/*============================================================================*/
#include "Font.h"
/*============================================================================*/
unsigned long	LCD_GetWidth			( void );
unsigned long	LCD_GetHeight			( void );
unsigned long	LCD_SetWindow			( int xStart, int yStart, int xEnd, int yEnd );
void			LCD_DrawPixel			( int x, int y, unsigned short color );
void			LCD_DrawFilledRectangle	( int xStart, int yStart, int xEnd, int yEnd, unsigned short color );
void			LCD_Clear				( unsigned short color );
void			LCD_DrawVLine			( int x, int y1, int y2, unsigned short color );
void			LCD_DrawHLine			( int x1, int x2, int y, unsigned short color );
void			LCD_DrawLine			( int x1, int y1, int x2, int y2, unsigned short color );
void			LCD_DrawRectangle		( int x1, int y1, int x2, int y2, unsigned short color );
void			LCD_DrawCircle			( int xCenter, int yCenter, unsigned int radius, unsigned short color );
void			LCD_DrawFilledCircle	( int xCenter, int yCenter, unsigned int radius, unsigned short color );
void			LCD_DrawEllipse			( int xCenter, int yCenter, unsigned int XRadius, unsigned int YRadius, unsigned short color );
void			LCD_DrawFilledEllipse	( int xCenter, int yCenter, unsigned int XRadius, unsigned int YRadius, unsigned short color );
void			LCDInit					( void );

void			clrscr					( void );
void			LCD_SetCurrentFont		( font_t *Font );
void			LCD_SetCurrentBGColor	( unsigned short Color );
void			LCD_SetCurrentFGColor	( unsigned short Color );
/*============================================================================*/
#endif	/*	!defined __LCD_HX8357_H__ */
/*============================================================================*/
#endif