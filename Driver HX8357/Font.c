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
//#include "BusLCD.h"
#include <stdlib.h>
#include "LCD.h"
#include "LCDHX8357.h"
#include "BUSHX8357.h"
#include "Font.h"
/*============================================================================*/
int LCD_GetCharacterWidth( font_t *Font, char c )
	{
	if( Font->CharacterDataOffset[(unsigned)c] == 0xffff )
		return 0;
	return Font->SpaceBefore + (int)Font->Data[Font->CharacterDataOffset[(unsigned)c]] + Font->SpaceAfter;
	}
/*============================================================================*/
int LCD_GetCharacterHeight( font_t *Font, char c )
	{
	if( Font->CharacterDataOffset[(unsigned)c] == 0xffff )
		return 0;
	return (int)Font->Height + Font->VerticalSpacing;
	}
/*============================================================================*/
int LCD_GetCharacterBaseLine( font_t *Font, char c )
	{
	return Font->BaseLine;
	}
/*============================================================================*/
int LCD_GetCharacterSize( font_t *Font, char c, point_t *p )
	{
	if( Font->CharacterDataOffset[(unsigned)c] == 0xffff )
		{
		*p	= (point_t){ 0, 0 };
		return 0;
		}
	*p	= (point_t){ Font->SpaceBefore + (int)Font->Data[Font->CharacterDataOffset[(unsigned)c]] + Font->SpaceAfter, Font->Height + Font->VerticalSpacing };
	return 1;
	}
/*============================================================================*/
int LCD_ShowCharacter( int X, int Y, font_t *Font, unsigned char Ch, unsigned short BGColor, unsigned short FGColor, int Mode, point_t *Size )
	{
	volatile unsigned char	*p, Aux;
	character_t	*Character;
	int			i, j;

	if( Font->CharacterDataOffset[Ch] == 0xffff )
		{
		if( Size != NULL )
			*Size	= (point_t){ 0, 0 };
		return 0;
		}

	/* Get a pointer to the character data */
	Character	= (character_t*)&Font->Data[Font->CharacterDataOffset[Ch]];

	if( Mode == LCD_CHARACTER_MODE_OPAQUE )
		{
		/*--------------------------------------------------------------------*/
		/* Opaque mode */
		/*--------------------------------------------------------------------*/

		/* Set the window that contains all the pixels that will be changed by the character */
		LCDSetWindow( X, Y, X + Character->Width + Font->SpaceBefore + Font->SpaceAfter - 1, Y + Font->Height + Font->VerticalSpacing - 1 );
		BusHX8357WriteLCDCmd( 0x2C );

		/* Clear the space to the left of the character */
		for( i = ( Font->Height + Font->VerticalSpacing ) * Font->SpaceBefore; i > 0; i-- )
			{
			BusHX8357WriteLCDData( BGColor >> 8 );
			BusHX8357WriteLCDData( BGColor >> 0 );
			}

		/* Show the character image, one column of pixels at a time */
		for( p = &Character->Data[0], j = 0; j < Character->Width; j++ )
			{
			/* Show one column of pixels of the stored image */
			for( i = 0; i < Font->Height; i++, Aux <<= 1 )
				{
				if(( i & 7 ) == 0 )
					Aux	= *p++;
				BusHX8357WriteLCDData((( Aux & 0x80 ) ? FGColor : BGColor ) >> 8 );
				BusHX8357WriteLCDData((( Aux & 0x80 ) ? FGColor : BGColor ) >> 0 );
				}
			/* Clear the space below the character for the current column */
			for( i = Font->VerticalSpacing; i > 0; i-- )
				{
				BusHX8357WriteLCDData( BGColor >> 8 );
				BusHX8357WriteLCDData( BGColor >> 0 );
				}
			};

		/* Clear the space to the right of the character */
		for( i = ( Font->Height + Font->VerticalSpacing ) * Font->SpaceAfter; i > 0; i-- )
			{
			BusHX8357WriteLCDData( BGColor >> 8 );
			BusHX8357WriteLCDData( BGColor >> 0 );
			}
		/*--------------------------------------------------------------------*/
		}
	else
		{
		/*--------------------------------------------------------------------*/
		/* Transparent mode */
		/*--------------------------------------------------------------------*/
		int	Skipped				= 0;
		int	Moved				= 0;
		unsigned short	xStart	= X + Font->SpaceBefore;
		unsigned short	xEnd	= X + Font->SpaceBefore + Character->Width - 1;
		unsigned short	yStart	= Y;
		unsigned short	yEnd	= Y + Font->Height - 1;

		/* Set the window that contains all the pixels that will be changed by the character */
		LCDSetWindow( xStart, yStart, xEnd, yEnd );
		BusHX8357WriteLCDCmd( 0x2C );

		for( p = &Character->Data[0], j = 0; j < Character->Width; j++ )
			{
			if( Moved )
				{
				BusHX8357WriteLCDCmd( 0x2B );
				BusHX8357WriteLCDData(( xStart + j ) >> 8 );
				BusHX8357WriteLCDData(( xStart + j ) >> 0 );		
				BusHX8357WriteLCDData( xEnd   >> 8 );
				BusHX8357WriteLCDData( xEnd   >> 0 );
				Moved	= 0;
				}
			for( i = 0; i < Font->Height; i++, Aux <<= 1 )
				{
				if(( i & 7 ) == 0 )
					Aux	= *p++;
				/* This pixel is to be painted with the foreground color... */
				if( Aux & 0x80 )
					{
					if( Skipped )
						{
						BusHX8357WriteLCDCmd( 0x2A );
						BusHX8357WriteLCDData(( yStart + i ) >> 8 );
						BusHX8357WriteLCDData(( yStart + i ) >> 0 );		
						BusHX8357WriteLCDData( yEnd   >> 8 );
						BusHX8357WriteLCDData( yEnd   >> 0 );
						BusHX8357WriteLCDCmd( 0x2C );
						Skipped	= 0;
						Moved	= 1;
						}
					BusHX8357WriteLCDData( FGColor >> 8 );
					BusHX8357WriteLCDData( FGColor >> 0 );
					}
				/* This pixel is to be left with the current color... */
				else
					Skipped	= 1;
				}
			};
		/*--------------------------------------------------------------------*/
		}

	if( Size != NULL )
		*Size	= (point_t){ Font->SpaceBefore + Character->Width + Font->SpaceAfter, Font->Height + Font->VerticalSpacing };
	return 1;
	}
/*============================================================================*/
