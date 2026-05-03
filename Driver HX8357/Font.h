/*==============================================================================
 Copyright (c) 2010-2026, Isaac Marino Bavaresco
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
#if			!defined __FONT_H__
#define __FONT_H__
/*============================================================================*/

#define LCD_CHARACTER_MODE_TRANSPARENT	0
#define LCD_CHARACTER_MODE_OPAQUE		1

/*============================================================================*/

#pragma pack( 1 )

typedef struct
	{
	unsigned short	x;
	unsigned short	y;	
	} point_t;

/*============================================================================*/

typedef struct
	{
	unsigned char	Width;
//	unsigned char	SpaceBefore;
//	unsigned char	SpaceAfter;
	unsigned char	Data[];
	} character_t;
/*============================================================================*/
typedef struct
	{
	unsigned short	Height;
	unsigned short	Width;
	unsigned char	SpaceBefore;
	unsigned char	SpaceAfter;
	unsigned short	VerticalSpacing;
	unsigned short	Flags;
	unsigned short	BaseLine;
	unsigned short	CharacterDataOffset[256];
	unsigned char	Data[];
	} font_t;

#pragma pack()

/*============================================================================*/
int		LCD_GetCharacterWidth	( font_t *Font, char c );
int		LCD_GetCharacterHeight	( font_t *Font, char c );
int		LCD_GetCharacterBaseLine( font_t *Font, char c );
int		LCD_GetCharacterSize	( font_t *Font, char c, point_t *p );
int		LCD_ShowCharacter		( int X, int Y, font_t *Font, unsigned char Ch, unsigned short BGColor, unsigned short FGColor, int Mode, point_t *p );
/*============================================================================*/
#endif	/*	!defined __FONT_H__ */
/*============================================================================*/
