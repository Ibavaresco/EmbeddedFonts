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
#include <stdio.h>
#include <delay.h>
/*============================================================================*/
#include "LCDInternals.h"
#include "LCD.h"
#include "BusHX8357.h"
#include "LCDHX8357.h"
#include "Font.h"
#include "Font-TimesNewRoman-20.h"
#include "Font-TimesNewRoman-48.h"
/*============================================================================*/
#define	LCD_WIDTH	480
#define	LCD_HEIGHT	320
/*============================================================================*/
static int				delayedscroll			= 0;
static font_t			*LCDCurrentFont			= &FontTimesNewRoman20;
static unsigned short	LCDCurrentXPos			= 0;
static unsigned short	LCDCurrentYPos			= 0;
static unsigned short	LCDCurrentLineHeight	= 8;
static unsigned short	LCDCurrentBaseLine		= 0;
static unsigned short	LCDCurrentBGColor		= 0x001f;
static unsigned short	LCDCurrentFGColor		= 0xffff;
/*============================================================================*/
static unsigned long HX8357SetWindow( int xStart, int yStart, int xEnd, int yEnd )
	{
	if( xStart > xEnd )
		{
		int Aux	= xStart;
		xStart	= xEnd;
		xEnd	= Aux;
		}
	if( yStart > yEnd )
		{
		int Aux	= yStart;
		yStart	= yEnd;
		yEnd	= Aux;
		}

	if( xStart > LCD_WIDTH - 1 )
		return 0;
	else if( xStart < 0 )
		xStart	= 0;

	if( xEnd < 0 )
		return 0;
	else if( xEnd > LCD_WIDTH - 1 )
		xEnd	= LCD_WIDTH - 1;

	if( yStart > LCD_HEIGHT - 1 )
		return 0;
	else if( yStart < 0 )
		yStart	= 0;

	if( yEnd < 0 )
		return 0;
	else if( yEnd > LCD_HEIGHT - 1 )
		yEnd	= LCD_HEIGHT - 1;

	BusHX8357WriteLCDCmd( 0x2A );
	BusHX8357WriteLCDData( yStart >> 8 );
	BusHX8357WriteLCDData( yStart >> 0 );		
	BusHX8357WriteLCDData( yEnd   >> 8 );
	BusHX8357WriteLCDData( yEnd   >> 0 );

	BusHX8357WriteLCDCmd( 0x2B );
	BusHX8357WriteLCDData( xStart >> 8 );
	BusHX8357WriteLCDData( xStart >> 0 );		
	BusHX8357WriteLCDData( xEnd   >> 8 );
	BusHX8357WriteLCDData( xEnd   >> 0 );

	return ( xEnd - xStart + 1 ) * ( yEnd - yStart + 1 );
	}
/*============================================================================*/
static int HX8357PutPixel( int x, int y, int color )
	{
	if( HX8357SetWindow( x, y, x, y ))
		{
		BusHX8357WriteLCDCmd( 0x2C );
		BusHX8357WriteLCDData( color >> 8 );
		BusHX8357WriteLCDData( color >> 0 );
		return 1;
		}
	return 0;
	} 	 
/*============================================================================*/
static void HX8357_DrawFilledRectangle( int xStart, int yStart, int xEnd, int yEnd, unsigned short color )
	{          
	long	i;

	if(( i = HX8357SetWindow( xStart, yStart, xEnd, yEnd )) > 0 )
		{
		BusHX8357WriteLCDCmd( 0x2C );
		
		for( ; i > 0; i-- )
			{
			BusHX8357WriteLCDData( color >> 8 );
			BusHX8357WriteLCDData( color >> 0 );
			}
		}
	}
/*============================================================================*/
static void HX8357_Clear( unsigned short color )
	{
	HX8357_DrawFilledRectangle( 0, 0, LCD_WIDTH, LCD_HEIGHT, color );
	}
/*============================================================================*/
static int HX8357DrawVLine( int x, int y1, int y2, int color )
	{
	long	i;
	int		Aux;

	if( x < 0 || x > LCD_WIDTH - 1 || ( y1 < 0 && y2 < 0 ) || ( y1 > LCD_HEIGHT - 1 && y2 > LCD_HEIGHT - 1 ))
		return 0;

	if( y1 > y2 )
		{
		Aux		= y1;
		y1		= y2;
		y2		= Aux;
		}

	if( y1 < 0 )
		y1	= 0;
	if( y2 > LCD_HEIGHT - 1 )
		y2	= LCD_HEIGHT - 1;

	if(( i = HX8357SetWindow( x, y1, x, y2 )) > 0 )
		{
		BusHX8357WriteLCDCmd( 0x2C );
		
		for( ; i > 0; i-- )
			{
			BusHX8357WriteLCDData( color >> 8 );
			BusHX8357WriteLCDData( color >> 0 );
			}
		}

	return 1;
	}
/*============================================================================*/
static int HX8357DrawHLine( int x1, int x2, int y, int color )
	{
	long	i;
	int		Aux;

	if( y < 0 || y > LCD_HEIGHT - 1 || ( x1 < 0 && x2 < 0 ) || ( x1 > LCD_WIDTH - 1 && x2 > LCD_WIDTH - 1 ))
		return 0;

	if( x1 > x2 )
		{
		Aux		= x1;
		x1		= x2;
		x2		= Aux;
		}

	if( x1 < 0 )
		x1	= 0;
	if( x2 > LCD_WIDTH - 1 )
		x2	= LCD_WIDTH - 1;

	if(( i = HX8357SetWindow( x1, y, x2, y )) > 0 )
		{
		BusHX8357WriteLCDCmd( 0x2C );
		
		for( ; i > 0; i-- )
			{
			BusHX8357WriteLCDData( color >> 8 );
			BusHX8357WriteLCDData( color >> 0 );
			}
		}

	return 1;
	}
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/*
 Copyright (c) 2005-2017, Isaac Marino Bavaresco
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Neither the name of the author nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

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
*/
/*============================================================================*/
#if	0
/*
Lines below this length threshold are not optimized to draw only the on-screen
pixels because the optimization calculations use a lot of floating-point math
and the time spent would negate the benefits.
*/
#define	LENGTH_THRESHOLD	100
/*============================================================================*/
void LCD_DrawLine( int x1, int y1, int x2, int y2, unsigned short color )
	{
	/* The line is horizontal... */
	if( y1 == y2 )
		/* ... call a specialized, more efficient function. */
		HX8357DrawHLine( x1, x2, y1, color );
	/* The line is vertical... */
	else if( x1 == x2 )
		/* ... call a specialized, more efficient function. */
		HX8357DrawVLine( x1, y1, y2, color );
	/* The line is long... */
	else if( abs( x2 - x1 ) > LENGTH_THRESHOLD || abs( y2 - y1 ) > LENGTH_THRESHOLD )
		{
		/* ... let's optimize and calculate the endpoints that are inside the screen
		   borders so we don't need to run the Bresenham algorithm for unnecessary
		   points.
		*/
		/*--------------------------------------------------------------------*/
		/* The line isn't totally inside the screen borders... */
		if( x1 < 0 || x1 > LCD_WIDTH - 1 || y1 < 0 || y1 > LCD_HEIGHT - 1 || x2 < 0 || x2 > LCD_WIDTH - 1 || y2 < 0 || y2 > LCD_HEIGHT - 1 )
			{
			/* Both endpoints are on the same side of the screen... */
			if(( x1 < 0 && x2 < 0 ) || ( x1 > LCD_WIDTH - 1 && x2 > LCD_WIDTH - 1 ) || ( y1 < 0 && y2 < 0 ) || ( y1 > LCD_HEIGHT - 1 && y2 > LCD_HEIGHT - 1 ))
				/* ... there is nothing to be drawn. */
				return;

			/* Equations of the line:						*/
			/*				a = ( y2 - y1 ) / ( x2 - x1 )	*/
			/*				b = y - a * x					*/
			/*	y = f(x):	y = a * x + b					*/
			/*	x = f(y):	x = ( y - b ) / a				*/

			/* Calculate the parameters of the line. */
			double	a	= (double)( y2 - y1 ) / (double)( x2 - x1 );
			double	b	= y1 - a * x1;

			/* Note: here the concepts of 'top' and 'bottom' are reversed.
			   'bottom' has y == 0, but appears on the top of the visual screen.
			*/

			/* y( 0 ) = a * 0 + b */
			int	yleft	= (int)b;
			/* y( width - 1 ) = a * ( width - 1 ) + b */
			int	yright	= (int)( a * ( LCD_WIDTH - 1 ) + b );
			/* x( 0 ) = ( 0 - b ) / a */
			int	xbottom	= (int)( -b / a );
			/* x( height - 1 ) = ( height - 1 - b ) / a */
			int	xtop	= (int)(( LCD_HEIGHT - 1 - b ) / a );

			/* Point 1 is outside of the screen... */
			if( x1 < 0 || x1 > LCD_WIDTH - 1 || y1 < 0 || y1 > LCD_HEIGHT - 1 )
				{
				/* The line crosses the left screen border... */
				if( x1 < 0 && yleft >= 0 && yleft <= LCD_HEIGHT - 1 )
					{
					x1	= 0;
					y1	= yleft;
					}
				/* The line crosses the right screen border... */
				else if( x1 > LCD_WIDTH - 1 && yright >= 0 && yright <= LCD_HEIGHT - 1 )
					{
					x1	= LCD_WIDTH - 1;
					y1	= yright;
					}
				/* The line crosses the bottom screen border... */
				else if( y1 < 0 && xbottom >= 0 && xbottom <= LCD_WIDTH - 1 )
					{
					y1	= 0;
					x1	= xbottom;
					}
				/* The line crosses the top screen border... */
				else if( y1 > LCD_HEIGHT - 1 && xtop >= 0 && xtop <= LCD_WIDTH - 1 )
					{
					y1	= LCD_HEIGHT - 1;
					x1	= xtop;
					}
				/* The line doesn't intersect the screen... */
				else
					/* ... there is nothing to be drawn. */
					return;
				}

			/* Point 2 is outside of the screen... */
			if( x2 < 0 || x2 > LCD_WIDTH - 1 || y2 < 0 || y2 > LCD_HEIGHT - 1 )
				{
				/* The line crosses the left screen border... */
				if( x2 < 0 && yleft >= 0 && yleft <= LCD_HEIGHT - 1 )
					{
					x2	= 0;
					y2	= yleft;
					}
				/* The line crosses the right screen border... */
				else if( x2 > LCD_WIDTH - 1 && yright >= 0 && yright <= LCD_HEIGHT - 1 )
					{
					x2	= LCD_WIDTH - 1;
					y2	= yright;
					}
				/* The line crosses the bottom screen border... */
				else if( y2 < 0 && xbottom >= 0 && xbottom <= LCD_WIDTH - 1 )
					{
					y2	= 0;
					x2	= xbottom;
					}
				/* The line crosses the top screen border... */
				else if( y2 > LCD_HEIGHT - 1 && xtop >= 0 && xtop <= LCD_WIDTH - 1 )
					{
					y2	= LCD_HEIGHT - 1;
					x2	= xtop;
					}
				/* The line doesn't intersect the screen... */
				else
					/* ... there is nothing to be drawn. */
					return;
				}

			}
		/*--------------------------------------------------------------------*/
		/* Draw the line using the Bresenham algorithm. */
		/*--------------------------------------------------------------------*/

		int dx, dy;
		int StepX, StepY;
		int Fraction;

		dx	= x2 - x1;
		dy	= y2 - y1;

		if( dx < 0 )
			{
			dx		= -dx;
			StepX	= -1;
			}
		else
			StepX	= 1;

		if( dy < 0 )
			{
			dy		= -dy;
			StepY	= -1;
			}
		else
			StepY	= 1;

		dy	<<= 1; 									// dy is now 2*dy
		dx	<<= 1; 									// dx is now 2*dx
		PutPixel( x1, y1, color );

		if( dx > dy )
			{
			Fraction	= dy - ( dx >> 1 ); 		// same as 2*dy - dx
			while( x1 != x2 )
				{
				if( Fraction >= 0 )
					{
					y1			+= StepY;
					Fraction	-= dx; 				// same as fraction -= 2*dx
					}
				x1			+= StepX;
				Fraction	+= dy; 					// same as fraction -= 2*dy
				PutPixel( x1, y1, color );
				}
			}
		else
			{
			Fraction	= dx - ( dy >> 1 );
			while( y1 != y2 )
				{
				if( Fraction >= 0 )
					{
					x1			+= StepX;
					Fraction	-= dy;
					}
				y1			+= StepY;
				Fraction	+= dx;
				PutPixel( x1, y1, color );
				}
			}
		/*--------------------------------------------------------------------*/
		}
	}
/*============================================================================*/
static void HX8357_DrawRectangle( int x1, int y1, int x2, int y2, unsigned short color )
	{
	HX8357DrawHLine( x1, x2, y1, color );
	HX8357DrawHLine( x1, x2, y2, color );
	HX8357DrawVLine( x1, y1, y2, color );
	HX8357DrawVLine( x2, y2, y2, color );
	}
/*============================================================================*/
static void HX8357_DrawCircle( int xCenter, int yCenter, unsigned r, unsigned short color )
	{
	int	a, b;
	int	di;
	
	a	= 0;
	b	= r;	  
	di	= 3 - ( r << 1 );

	while( a <= b )
		{
		HX8357_DrawPixel( xCenter - b, yCenter - a, color );
		HX8357_DrawPixel( xCenter + b, yCenter - a, color );
		HX8357_DrawPixel( xCenter - b, yCenter + a, color );
		HX8357_DrawPixel( xCenter + b, yCenter + a, color );
		HX8357_DrawPixel( xCenter - a, yCenter - b, color );
		HX8357_DrawPixel( xCenter + a, yCenter - b, color );
		HX8357_DrawPixel( xCenter - a, yCenter + b, color );
		HX8357_DrawPixel( xCenter + a, yCenter + b, color );

		a++;

		if( di < 0 )
			di	+= 4 * a + 6;	  
		else
			{
			di	+= 10 + 4 * ( a - b );   
			b--;
			} 
		HX8357_DrawPixel( xCenter + a, yCenter + b, color );
		}
	} 
/*============================================================================*/
static void HX8357_DrawFilledCircle( int xCenter, int yCenter, unsigned r, unsigned short color )
	{
	int	a, b;
	int	di;
	
	a	= 0;
	b	= r;	  
	di	= 3 - ( r << 1 );

	while( a <= b )
		{
		HX8357DrawHLine( xCenter - b, xCenter + b, yCenter - a, color );
		HX8357DrawHLine( xCenter - b, xCenter + b, yCenter + a, color );
		HX8357DrawHLine( xCenter - a, xCenter + a, yCenter - b, color );
		HX8357DrawHLine( xCenter - a, xCenter + a, yCenter + b, color );

		a++;

		if( di < 0 )
			di	+= 4 * a + 6;	  
		else
			{
			di	+= 10 + 4 * ( a - b );   
			b--;
			} 
		}
	} 
/*============================================================================*/
static void HX8357_DrawEllipse( int xCenter, int yCenter, unsigned int xRadius, unsigned int yRadius, unsigned short color )
	{
	long	X, Y;
	long	XChange, YChange;
	long	EllipseError;
	long	ASquare, BSquare, TwoASquare, TwoBSquare;
	long	StoppingX, StoppingY;

	ASquare			= xRadius * xRadius;
	TwoASquare		= 2 * ASquare;
	BSquare			= yRadius * yRadius;
	TwoBSquare		= 2 * BSquare;

	X				= 0;
	Y				= yRadius;
	XChange 		= BSquare;
	YChange 		= ASquare * ( 1 - 2 * yRadius );
	EllipseError	= 0;
	StoppingX		= 0;
	StoppingY		= TwoASquare * yRadius;

	while( StoppingX <= StoppingY ) // 2nd set of points, y'< -1
		{
		HX8357_DrawPixel( xCenter + X, yCenter + Y, color );
		HX8357_DrawPixel( xCenter + X, yCenter - Y, color );
		HX8357_DrawPixel( xCenter - X, yCenter + Y, color );
		HX8357_DrawPixel( xCenter - X, yCenter - Y, color );
		++X;
		StoppingX		+= TwoBSquare;
		EllipseError	+= XChange;
		XChange			+= TwoBSquare;
		if(( 2 * EllipseError + YChange ) > 0 )
			{
			--Y;
			StoppingY		-= TwoASquare;
			EllipseError	+= YChange;
			YChange			+= TwoASquare;
			}
		}

	// 1st point set is done; start the 2nd set of points

	X				= xRadius;
	Y				= 0;
	XChange			= BSquare * ( 1 - 2 * xRadius );
	YChange			= ASquare;
	EllipseError	= 0;
	StoppingX		= TwoBSquare * xRadius;
	StoppingY		= 0;

	while( StoppingX >= StoppingY ) // 1st set of points, y' > -1
		{
		HX8357_DrawPixel( xCenter + X, yCenter + Y, color );
		HX8357_DrawPixel( xCenter + X, yCenter - Y, color );
		HX8357_DrawPixel( xCenter - X, yCenter + Y, color );
		HX8357_DrawPixel( xCenter - X, yCenter - Y, color );
		++Y;
		StoppingY		+= TwoASquare;
		EllipseError	+= YChange;
		YChange			+= TwoASquare;
		if(( 2 * EllipseError + XChange ) > 0 )
			{
			--X;
			StoppingX		-= TwoBSquare;
			EllipseError	+= XChange;
			XChange			+= TwoBSquare;
			}
		};
	}
/*============================================================================*/
static void LCD_DrawFilledEllipse( int xCenter, int yCenter, unsigned int xRadius, unsigned int yRadius, unsigned short color )
	{
	long	X, Y;
	long	XChange, YChange;
	long	EllipseError;
	long	ASquare, BSquare, TwoASquare, TwoBSquare;
	long	StoppingX, StoppingY;

	ASquare			= xRadius * xRadius;
	TwoASquare		= 2 * ASquare;
	BSquare			= yRadius * yRadius;
	TwoBSquare		= 2 * BSquare;

	X				= 0;
	Y				= yRadius;
	XChange 		= BSquare;
	YChange 		= ASquare * ( 1 - 2 * yRadius );
	EllipseError	= 0;
	StoppingX		= 0;
	StoppingY		= TwoASquare * yRadius;

	while( StoppingX <= StoppingY ) // 2nd set of points, y'< -1
		{
		HX8357DrawHLine( xCenter - X, xCenter + X, yCenter + Y, color );
		HX8357DrawHLine( xCenter - X, xCenter + X, yCenter - Y, color );
		++X;
		StoppingX		+= TwoBSquare;
		EllipseError	+= XChange;
		XChange			+= TwoBSquare;
		if(( 2 * EllipseError + YChange ) > 0 )
			{
			--Y;
			StoppingY		-= TwoASquare;
			EllipseError	+= YChange;
			YChange			+= TwoASquare;
			}
		}

	// 1st point set is done; start the 2nd set of points

	X				= xRadius;
	Y				= 0;
	XChange			= BSquare * ( 1 - 2 * xRadius );
	YChange			= ASquare;
	EllipseError	= 0;
	StoppingX		= TwoBSquare * xRadius;
	StoppingY		= 0;

	while( StoppingX >= StoppingY ) // 1st set of points, y' > -1
		{
		HX8357DrawHLine( xCenter - X, xCenter + X, yCenter + Y, color );
		HX8357DrawHLine( xCenter - X, xCenter + X, yCenter - Y, color );
		++Y;
		StoppingY		+= TwoASquare;
		EllipseError	+= YChange;
		YChange			+= TwoASquare;
		if(( 2 * EllipseError + XChange ) > 0 )
			{
			--X;
			StoppingX		-= TwoBSquare;
			EllipseError	+= XChange;
			XChange			+= TwoBSquare;
			}
		};
	}
#endif
/*============================================================================*/
static const unsigned short LCDInitTable[]	=
	{
	0x00EE, 0x0102, 0x0101, 0x0102, 0x0101,
	0x00ED, 0x0100, 0x0100, 0x019A, 0x019A, 0x019B, 0x019B, 0x0100, 0x0100, 0x0100, 0x0100, 0x01AE, 0x01AE, 0x0101, 0x01A2,
	0x0100,
	0x00B4, 0x0100,
	0x00C0, 0x0100, 0x013B, 0x0100, 0x0102, 0x0111,
	0x00C1, 0x0110,
	0x00C8, 0x0100, 0x0146, 0x0112, 0x0120, 0x010c, 0x0100, 0x0156, 0x0112, 0x0167, 0x0102, 0x0100, 0x010c,
	0x00D0, 0x0144, 0x0142, 0x0106,
	0x00D1, 0x0143, 0x0116,
	0x00D2, 0x0104, 0x0122,
	0x00D3, 0x0104, 0x0112,
	0x00D4, 0x0107, 0x0112,
	0x00E9, 0x0100,
	0x00C5, 0x0108,
	0x0036, 0x014a,	// MADCTL
	0x003A, 0x0155,
	0x002A, 0x0100, 0x0100, 0x0101, 0x013F,
	0x002B, 0x0100, 0x0100, 0x0101, 0x01E0,

	0x8014,	/* delay_ms( 20 )*/

	0x0011,

	0x8078,	/* delay_ms( 120 )*/

	0x0035, 0x0100,
	0x0021,
	0x0029,

	0x8005,	/* delay_ms( 5 )*/
	};
/*============================================================================*/
int putch( void *fildes, int c );

void LCDHX8357Init( void )
	{
	unsigned int	i;

	BusHX8357Init();

	for( i = 0; i < sizeof LCDInitTable / sizeof LCDInitTable[0]; i++ )
		{
		if( LCDInitTable[i] & 0x8000 )
			delay_ms( LCDInitTable[i] & 0x7fff );
		else if( LCDInitTable[i] & 0x0100 )
			BusHX8357WriteLCDData( LCDInitTable[i] & 0x00ff );
		else
			BusHX8357WriteLCDCmd( LCDInitTable[i] & 0x00ff );
		}

	HX8357_Clear( 0xffff );
	}
/*============================================================================*/
static unsigned char	ScrollBuffer[2*480];

static void HX8357Scroll( unsigned short ScrollLines )
	{
	unsigned short	x, y;
	unsigned char	*p;

	for( y = ScrollLines; y < 320; y++ )
		{
		LCDSetWindow( 0, y, 480 - 1, y );
		BusHX8357WriteLCDCmd( 0x2E );
		BusHX8357ReadLCDData();
		for( p = ScrollBuffer, x = 0; x < 480; x++ )
			{
			*p++	= BusHX8357ReadLCDData();
			*p++	= BusHX8357ReadLCDData();
			}
		LCDSetWindow( 0, y - ScrollLines, 480 - 1, y - ScrollLines );
		BusHX8357WriteLCDCmd( 0x2C );
		for( p = ScrollBuffer, x = 0; x < 480; x++ )
			{
			BusHX8357WriteLCDData( *p++ );
			BusHX8357WriteLCDData( *p++ );
			}
		}
	HX8357_DrawFilledRectangle( 0, LCD_HEIGHT - ScrollLines - 1, LCD_WIDTH - 1, LCD_HEIGHT - 1, LCDCurrentBGColor );
	}
/*============================================================================*/
static int HX8357ScrollOneLine( void )
	{
	HX8357Scroll( 1 );

	return 1;
	}
/*============================================================================*/
static int HX8357ShowChar( int X, int Y, unsigned char Char )
	{
	point_t	CharSize;
	int		CharBaseLine	= LCD_GetCharacterBaseLine( LCDCurrentFont, Char );

	LCD_GetCharacterSize( LCDCurrentFont, Char, &CharSize );

	/* O caractere não cabe no espaço restante da linha... */
	if( LCDCurrentXPos + CharSize.x > LCD_WIDTH )
		{
		/* ... apaga o restante da linha */
		HX8357_DrawFilledRectangle( LCDCurrentXPos, LCDCurrentYPos, LCD_WIDTH - 1, LCDCurrentYPos + CharSize.y - 1, LCDCurrentBGColor );
		/* move o cursor gráfico para o início da linha */
		LCDCurrentXPos			= 0;
		/* move o cursor gráfico para baixo da linha atual */
		LCDCurrentYPos		   += LCDCurrentLineHeight;
		LCDCurrentLineHeight	= 0;
		LCDCurrentBaseLine		= 0;
		}

	if( CharBaseLine > LCDCurrentBaseLine )
		LCDCurrentBaseLine	= CharBaseLine;
	if( LCDCurrentBaseLine + CharSize.y - CharBaseLine > LCDCurrentLineHeight )
		LCDCurrentLineHeight	= LCDCurrentBaseLine + CharSize.y - CharBaseLine;

	if( LCDCurrentYPos + LCDCurrentLineHeight > LCD_HEIGHT )
		{
		HX8357Scroll( LCDCurrentYPos + LCDCurrentLineHeight - LCD_HEIGHT );
		LCDCurrentYPos = LCD_HEIGHT - LCDCurrentLineHeight;
		}
	
	LCD_ShowCharacter( LCDCurrentXPos, LCDCurrentYPos + LCDCurrentBaseLine - CharBaseLine, LCDCurrentFont, Char, LCDCurrentBGColor, LCDCurrentFGColor, LCD_CHARACTER_MODE_OPAQUE, NULL );

	LCDCurrentXPos	+= CharSize.x;

	if( LCDCurrentXPos >= LCD_WIDTH && !delayedscroll )
		{
		LCDCurrentXPos	= 0;
		LCDCurrentYPos += CharSize.y;
		}

	if( LCDCurrentYPos + LCDCurrentLineHeight > LCD_HEIGHT )
		{
		HX8357Scroll( LCDCurrentYPos + LCDCurrentLineHeight - LCD_HEIGHT );
		LCDCurrentYPos = LCD_HEIGHT - LCDCurrentLineHeight;
		}
	return Char;
	}
/*============================================================================*/
static void LCD_SetCurrentFont( font_t *Font )
	{
	LCDCurrentFont	= Font;
	}
/*============================================================================*/
static void LCD_SetCurrentBGColor( unsigned short Color )
	{
	LCDCurrentBGColor	= Color;
	}
/*============================================================================*/
static void LCD_SetCurrentFGColor( unsigned short Color )
	{
	LCDCurrentFGColor	= Color;
	}
/*============================================================================*/
static int HX8357ClrScr( void )
	{
	HX8357_Clear( LCDCurrentBGColor );
	LCDCurrentYPos			= 0;
	LCDCurrentXPos			= 0;
	LCDCurrentLineHeight	= 0;
	LCDCurrentBaseLine		= 0;

	return 1;
	}
/*============================================================================*/
#if 0
static int xxputch( void *fildes, int c )
	{
	point_t			CharSize;

	LCD_GetCharacterSize( LCDCurrentFont, c, &CharSize );

	// There is a pending scroll and the current character is not '\a', '\b' or '\f', ...
	if( LCDCurrentXPos + CharSize.x >= LCD_WIDTH && c != (char)'\a' && c != (char)'\b' && c != (char)'\f' )
		{
		// ... return the cursor to the beginning of the next line...
		LCDCurrentXPos	= 0;
		// ... and increment the line.
		LCDCurrentYPos	+= LCDCurrentLineHeight;
		// The cursor is beyond the last line, ...
		if( LCDCurrentYPos >= LCD_HEIGHT )
			{
			HX8357Scroll( LCDCurrentYPos - LCD_HEIGHT );
			// ... position the cursor on the last line...
			LCDCurrentYPos = LCD_HEIGHT - 1;
			}
		}

	switch( c )
		{
		case '\0':
			return c;

		case '\b':
		if( LCDCurrentXPos )
			LCDCurrentXPos--;
		else if( LCDCurrentYPos )
			{
			LCDCurrentYPos--;
			LCDCurrentXPos	= 20;
			}
		return c;

		case '\f':
			//@@@@clrscr();
			return c;

		case '\n':
			LCDCurrentXPos			= 0;
			LCDCurrentYPos		   += LCDCurrentLineHeight;
			LCDCurrentLineHeight	= 0;
			LCDCurrentBaseLine		= 0;

			if( LCDCurrentYPos >= LCD_HEIGHT )
				{
				HX8357Scroll( LCDCurrentYPos - LCD_HEIGHT );
				// ... position the cursor on the last line...
				LCDCurrentYPos = LCD_HEIGHT - 1;
				}
			return c;

		case '\r':
			LCDCurrentXPos	= 0;
			return c;

		case '\t':
#if 0
			c = ( LCDCurrentXPos & 0x1c ) + 4;
			if( c > LCD_WIDTH )
				c = LCD_WIDTH;

			Bit = LCDCurrentXPos * 6;
			for( i = ( c - LCDCurrentXPos ) * 6; i; i-- )
				WriteLCDData( LCDCurrentYPos, Bit + i, 0 );

			LCDCurrentXPos = ( LCDCurrentXPos & 0x1c ) + 4;
			if( LCDCurrentXPos >= LCD_WIDTH && ( LCDCurrentYPos + 1 < LCD_HEIGHT || !delayedscroll ))
				{
				LCDCurrentXPos	= 0;
				if( LCDCurrentYPos < 7 )
					LCDCurrentYPos++;
				else
					HX8357ScrollOneLine();
				}
#endif
			return c;
		}

//@@@@	LCD_putch( NULL, c );

	return c;
	}
#endif
/*============================================================================*/
/*
int ShowBitmap( unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char *p )
	{
	int	i, j;

	if( w == 0 || h == 0 || x + w > 128 || y + h > 64 )
	return -1;

	y	= y >> 3;
	h	= ( h + 7 ) >> 3;

	for( i = 0; i < h; i++ )
		{
		for( j = 0; j < w; j++, p++ )
		WriteLCDData( Linha, x + j, *p );
		}

	return 0;
	}
*/
/*============================================================================*/
const lcd_t	LCDHX8357	=
	{
	.IsColor		= 1,
	.TextWidth		= LCD_WIDTH / 6,
	.TextHeight		= LCD_HEIGHT / 8,
	.GraphicWidth	= LCD_WIDTH,
	.GraphicHeigth	= LCD_HEIGHT,
	.PutPixel		= HX8357PutPixel,
	.ReadPixel		= NULL,
	.GotoXY			= NULL,
	.ShowChar		= HX8357ShowChar,
	.GetChar		= NULL,
	.DrawVLine		= HX8357DrawVLine,
	.DrawHLine		= HX8357DrawHLine,
	.Scroll			= HX8357ScrollOneLine,
	.ClrScr			= HX8357ClrScr,
	.BusReadKBD		= NULL,
	.SetSize		= NULL,
	.Init			= LCDHX8357Init,
	.SetWindow		= HX8357SetWindow
	};
/*============================================================================*/
void BusOperateBackLight( int );
void BusAdjustBackLight( int );

const lcd_t *LCDHX8357Probe( void )
	{
	unsigned char Result[3];

	//BusAdjustBackLight( 0xffff );
	//BusOperateBackLight( 1 );

	BusHX8357WriteLCDCmd( 0x04 );
	BusHX8357ReadLCDData();
	Result[0]	= BusHX8357ReadLCDData();
	Result[1]	= BusHX8357ReadLCDData();
	Result[2]	= BusHX8357ReadLCDData();

	if( Result[0] != 0x00 || Result[1] != 0x00 || Result[2] != 0x00 )
		return NULL;

	return &LCDHX8357;
	}
/*============================================================================*/
