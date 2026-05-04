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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
/*============================================================================*/
typedef struct
	{
	unsigned char	*Buffer;
	unsigned long	Width;
	unsigned long	Height;
	unsigned long	LineLength;
	} image_t;
/*============================================================================*/
static int QueryPixel( const image_t * const Image, int X, int Y )
    {
	if( Image == NULL || X < 0 || X >= Image->Width || Y < 0 || Y >= Image->Height )
		return -1;

    if(( Image->Buffer[( Image->Height - Y - 1 ) * Image->LineLength + X / 8] & ( 0x80 >> ( X & 0x07 ))) == 0 )
        return 1;

    return 0;
    }
/*============================================================================*/
static int CheckColumn( const image_t * const Image, int X )
    {
    unsigned long   Y;

    if( Image == NULL || X < 0 || X >= Image->Height )
		return -1;

    for( Y = 0; Y < Image->Height; Y++ )
        if( QueryPixel( Image, X, Y ))
            return 1;
    return 0;
    }
/*============================================================================*/
static int CheckColumnSegment( const image_t * const Image, int Y1, int Y2, int X )
    {
    int	Y;

    if( Image == NULL || X < 0 || X >= Image->Width )
		return -1;

    if( Y1 < -(int)Image->Height )
		Y1	= 0;
	else if( Y1 < 0 )
		Y1 += Image->Height;
	else if( Y1 >= Image->Height )
		Y1	= Image->Height - 1;

    if( Y2 < -(int)Image->Height )
		Y2	= 0;
	else if( Y2 < 0 )
		Y2 += Image->Height;
	else if( Y1 >= Image->Height )
		Y2	= Image->Height - 1;

	if( Y2 < Y1 )
		{
		Y2 ^= Y1;
		Y1 ^= Y2;
		Y2 ^= Y1;
		}

    for( Y = Y1; Y <= Y2; Y++ )
        if( QueryPixel( Image, X, Y ))
            return Y;

    return -1;
    }
/*============================================================================*/
static int CheckLine( const image_t * const Image, int Y )
    {
    unsigned long   x;

    if( Image == NULL || Y < 0 || Y >= Image->Height )
		return -1;

    for( x = 0; x < Image->Width; x++ )
        if( QueryPixel( Image, x, Y ))
            return x;

    return -1;
    }
/*============================================================================*/
static int CheckLineSegment( const image_t * const Image, int X1, int X2, int Y )
    {
    int	X;

    if( Image == NULL || Y < 0 || Y >= Image->Height )
		return -1;

    if( X1 < -(int)Image->Width )
		X1	= 0;
	else if( X1 < 0 )
		X1 += Image->Width;
	else if( X1 >= Image->Width )
		X1	= Image->Width - 1;

    if( X2 < -(int)Image->Width )
		X2	= 0;
	else if( X2 < 0 )
		X2 += Image->Width;
	else if( X1 >= Image->Width )
		X2	= Image->Width - 1;

	if( X2 < X1 )
		{
		X2 ^= X1;
		X1 ^= X2;
		X2 ^= X1;
		}

    for( X = X1; X <= X2; X++ )
        if( QueryPixel( Image, X, Y ))
            return X;

    return -1;
    }
/*============================================================================*/
unsigned char   Bitmaps[65536];
unsigned short  CurrentOffset   = 0;
unsigned short  Offsets[256];
unsigned short	BaseLines[256];
/*============================================================================*/
static int ExtractHoriz( const image_t * const Image, int LineHeight, int BaseLine, int XStart, int XEnd, int YTop, int YBottom, int c, int Reverse )
    {
    unsigned long   x, BitCount, Value;
    long int        y;

    Offsets[c]  				= CurrentOffset;
    BaseLines[c]				= BaseLine - YTop;
    Bitmaps[CurrentOffset++]    = XEnd - XStart + 1;

    for( Reverse ? ( x = XEnd ) : ( x = XStart ); Reverse ? ( x >= XStart ) : ( x <= XEnd ); x += ( Reverse ? -1 : +1 ))
        {
        for( BitCount = 0, Value = 0, y = YTop; y < YTop + LineHeight; y++ )
            {
            Value   = ( Value << 1 ) | QueryPixel( Image, x, y );
            if( ++BitCount >= 8 )
                {
                Bitmaps[CurrentOffset++]    = Value;
                BitCount    = 0;
                Value       = 0;
                }
            }
        if( BitCount != 0 )
            {
            Value <<= 8 - BitCount;
            Bitmaps[CurrentOffset++]    = Value;
            }
        }

    return 0;
    }
/*============================================================================*/
static int ExtractVert( const image_t * const Image, int LineHeight, int BaseLine, int XStart, int XEnd, int YTop, int YBottom, int c )
    {
    unsigned long   x, BitCount, Value;
    long int        y;

    Offsets[c]  = CurrentOffset;
    BaseLines[c]= BaseLine;

    Bitmaps[CurrentOffset++]    = XEnd - XStart + 1;
    for( x = XStart; x <= XEnd; x++ )
        {
        for( BitCount = 0, Value = 0, y = YTop; y <= (long int)YBottom; y++ )
            {
            Value   = ( Value << 1 ) | QueryPixel( Image, x, y );
            if( ++BitCount >= 8 )
                {
                Bitmaps[CurrentOffset++]    = Value;
                BitCount    = 0;
                Value       = 0;
                }
            }
        if( BitCount != 0 )
            {
            Value <<= 8 - BitCount;
            Bitmaps[CurrentOffset++]    = Value;
            }
        }
    return 0;
    }
/*============================================================================*/
static int Process( const image_t * const Image, int Reverse )
    {
    int		YTop, YBottom;
    int		i, j, x, y, XStart;
    int		c;

    unsigned long	LineHeight, GapWidth, SpaceWidth;

    memset( Offsets, 0xff, sizeof Offsets );
    memset( Bitmaps, 0x00, sizeof Bitmaps );
    memset( BaseLines, 0x00, sizeof BaseLines );
    CurrentOffset   = 0;

    /* Find the top of the first underscore. */
    for( y = 0; y < Image->Height && CheckLine( Image, y ) < 0; y++ )
        {}
    if( y >= Image->Height )
        return 0;

    YTop = y;

    /* Find the bottom of the first underscore. */
    for( ; y < Image->Height && CheckLine( Image, y ) >= 0; y++ )
        {}
    if( y >= Image->Height )
        return 0;

    /* Find the top of the second underscore. */
    for( ; y < Image->Height && CheckLine( Image, y ) < 0; y++ )
        {}
    if( y >= Image->Height )
        return 0;

    LineHeight	= y - YTop;

    /* Find the bottom of the second underscore. */
    for( ; y < Image->Height && CheckLine( Image, y ) >= 0; y++ )
        {}
    if( y >= Image->Height )
        return 0;

    /* Find the top of the first vertical bar. */
    for( ; y < Image->Height && ( XStart = CheckLine( Image, y )) < 0; y++ )
        {}
    if( y >= Image->Height )
        return 0;

    YTop	= y;

    /* Find the bottom of the first vertical bar. */
    for( ; y < Image->Height && CheckLine( Image, y ) >= 0; y++ )
        {}
    if( y >= Image->Height )
        return 0;

    YBottom	= y - 1;

    /* Find the end of the first vertical bar. */
    for( x = XStart; x < Image->Width && CheckColumnSegment( Image, YTop, YBottom, x ) >= 0; x++ )
		{}
    if( x >= Image->Width )
        return 0;

    /* Find the start of the second vertical bar. */
    for( ; x < Image->Width && CheckColumnSegment( Image, YTop, YBottom, x ) < 0; x++ )
		{}
    if( x >= Image->Width )
        return 0;

    GapWidth	= x - XStart - 1;

    /* Find the top of the third vertical bar. */
    for( ; y < Image->Height && ( XStart = CheckLine( Image, y )) < 0; y++ )
        {}
    if( y >= Image->Height )
        return 0;

    YTop	= y;

    /* Find the bottom of the third vertical bar. */
    for( ; y < Image->Height && CheckLine( Image, y ) >= 0; y++ )
        {}
    if( y >= Image->Height )
        return 0;

    YBottom	= y - 1;

    /* Find the end of the third vertical bar. */
    for( x = XStart; x < Image->Width && CheckColumnSegment( Image, YTop, YBottom, x ) >= 0; x++ )
		{}
    if( x >= Image->Width )
        return 0;

    /* Find the start of the fourth vertical bar. */
    for( ; x < Image->Width && CheckColumnSegment( Image, YTop, YBottom, x ) < 0; x++ )
		{}
    if( x >= Image->Width )
        return 0;

    SpaceWidth	= x - XStart - GapWidth - 1;

    Offsets[32]  				= CurrentOffset;
    BaseLines[32]				= 0;
    Bitmaps[CurrentOffset++]    = SpaceWidth;
    memset( &Bitmaps[CurrentOffset], 0x00, (( LineHeight + 7 ) / 8 ) * SpaceWidth );
    CurrentOffset  += (( LineHeight + 7 ) / 8 ) * SpaceWidth;

	/* Find the top of the first row. */
	for( ; y < Image->Height && CheckLine( Image, y ) < 0; y++ )
		{}
	if( y >= Image->Height )
		return 0;

    for( c = '!', i = 0; i < 14; i++ )
		{
		int	BaseLine;

		YTop	= y;

		do
			{
			/* Find the bottom of the row. */
			for( ; y < Image->Height && CheckLine( Image, y ) >= 0; y++ )
				{}
			if( y >= Image->Height )
				return 0;

			YBottom	= y - 1;

			/* Find the top of the next row. */
			for( ; y < Image->Height && CheckLine( Image, y ) < 0; y++ )
				{}
			if( y >= Image->Height )
				break;

			}
		while( y - YBottom < LineHeight );

		/* Find the start of the guard character. */
		for( x = 0; x < Image->Width && CheckColumnSegment( Image, YTop, YBottom, x ) < 0; x++ )
			{}
		if( x >= Image->Width )
			return 0;
		XStart	= x;

		/* Find the end of the guard character. */
		for( ; x < Image->Width && CheckColumnSegment( Image, YTop, YBottom, x ) >= 0; x++ )
			{}
		if( x >= Image->Width )
			return 0;

		/* Find the baseline of the guard character. */
		for( j = YBottom; j >= YTop && CheckLineSegment( Image, XStart, x - 1, j ) < 0; j-- )
			{}
		BaseLine	= j;

		/* Find the start of the first character. */
		for( ; x < Image->Width && CheckColumnSegment( Image, YTop, YBottom, x ) < 0; x++ )
			{}
		if( x >= Image->Width )
			break;

		do
			{
			int	X1, XEnd;
			XStart	= x;
			do
				{
				/* Find the end of the segment. */
				for( ; x < Image->Width && CheckColumnSegment( Image, YTop, YBottom, x ) >= 0; x++ )
					{}
				if( x >= Image->Width )
					break;
				XEnd	= x - 1;
				X1		= x;

				/* Find the start of the next segment. */
				for( ; x < Image->Width && CheckColumnSegment( Image, YTop, YBottom, x ) < 0; x++ )
					{}
				if( x >= Image->Width )
					break;
				}
			while( x - X1 < SpaceWidth );

			ExtractHoriz( Image, LineHeight, BaseLine, XStart, XEnd, YTop, YBottom, c++, Reverse );

			/* Skip the non-printable characters. */
			if( c == 0x7f || c == 0x81 || c == 0x8d || c == 0x98 || c == 0x9d || c == 0xa0 || c == 0xad )
				c++;
			else if( c == 0x8f )
				c += 2;
			}
		while( x < Image->Width );
		}

	for( i = 0, j = 0; i < sizeof BaseLines /sizeof BaseLines[0]; i++ )
		if( j < BaseLines[i] )
			j	= BaseLines[i];

	BaseLines[0]	= j;

	for( i = 32; i < 256; i++ )
		{
		if( Offsets[i] != 0xffff && BaseLines[i] < j )
			{
			unsigned char	*p		= &Bitmaps[Offsets[i]];
			int				Columns	= *p++;
			for( x = 0; x < Columns; x++ )
				{
				unsigned char	Carry	= 0;
				for( y = 0; y < ( LineHeight + 7 ) / 8; y++ )
					{
					unsigned char	t = *p;
					*p		= Carry | ( t >> ( j - BaseLines[i] ));
					Carry	= t << ( 8 - ( j - BaseLines[i] ));
					p++;
					}
				}
			}
		}

	printf( "#include \"Font.h\"\n" );
    printf( "const font_t\tMyFont\t=\n\t{\n" );

    printf(
	"\t.Height\t\t\t\t\t= %3u,\n"
	"\t.Width\t\t\t\t\t= %3u,\n"
	"\t.SpaceBefore\t\t\t=   0,\n"
	"\t.SpaceAfter\t\t\t\t= %3u,\n"
	"\t.VerticalSpacing\t\t=   1,\n"
	"\t.Flags\t\t\t\t\t=   0,\n"
	"\t.BaseLine\t\t\t\t= %3u,\n"
	"\t.CharacterDataOffset\t=\n\t\t{", (unsigned)LineHeight, (unsigned)SpaceWidth, (unsigned)GapWidth, (unsigned)BaseLines[0] );


    for( c = 0; c < 256; c++ )
        {
        if(( c & 7 ) == 0 )
            printf( "\n\t\t" );
        printf( "0x%04X,", Offsets[c] );
        }
    printf( "\n\t\t},\n\t.Data\t=\n\t\t{" );

    for( c = 0; c < 256; c++ )
        {
        if( Offsets[c] != 0xffff )
            {
            unsigned char   *p  = &Bitmaps[Offsets[c]];
            int             w   = *p++;
            printf( "\n\n\t\t// \'%c\' = 0x%02X\n\t\t%u,", c >= ' ' && c < 127 ? c : '?', c, w );
#if 0
//            w   = ( w + 7 ) / 8;
            for( y = 0; y < Height; y++ )
                {
                printf( "\n\t" );
    #if 0
                for( x = 0; x < w; x++ )
                    printf( "0x%02X, ", *p++ );
    #else
                char    t;
                for( x = 0; x < w; x++, t <<= 1 )
                    {
                    if(( x & 7 ) == 0 )
                        t = *p++;
                    printf( "%c", ( t & 0x80 ) ? 'X' : '.' );
                    }
    #endif // 0

#else

            for( x = 0; x < w; x++ )
                {
                unsigned char   *q = p;
                char    t;
                printf( "\n\t\t" );
                for( y = 0; y < ( LineHeight + 7 ) / 8; y++ )
                    printf( "0x%02X, ", *p++ );
                printf( "\t\t// " );
                for( y = 0; y < LineHeight; y++, t <<= 1 )
                    {
                    if(( y & 7 ) == 0 )
                        t = *q++;
                    printf( "%c", ( t & 0x80 ) ? 'X' : '.' );
                    }
#endif
                }
            }
        }
	printf( "\n\t\t}\n\t};\n" );

    return 1;
    }
/*============================================================================*/
#if 0
void Process( const image_t * const Image )
    {
    unsigned long   YTop, YBottom;
    unsigned long   x, y, XStart, XEnd;
    unsigned int   c;

    memset( Offsets, 0xff, sizeof Offsets );
    memset( Bitmaps, 0x00, sizeof Bitmaps );
    CurrentOffset   = 0;

    for( y = 0; y < Image->Height && CheckLine( Image, y ) == 0; y++ )
        {}
    if( y == Image->Height )
        {
        printf( "\nThe image is empty.\n" );
        return;
        }
    YBottom = y;
    for( y = Image->Height - 1; y > YBottom && CheckLine( Image, y ) == 0; y-- )
        {}
    YTop    = y;

    for( c = '!', x = 0; x < Image->Width; c++ )
        {
        if( c == 0x81 || c == 0x8d || c == 0x8f || c == 0x90 || c == 0x9d || c == 0xa0 || c == 0xad )
            {
            XStart  = x;
            XEnd    = x;
            }
        else
            {
            for( ; x < Image->Width && CheckColumn( Image, x ) == 0; x++ )
                {}
            if( x >= Image->Width )
                break;
            XStart  = x;
            do
                {
                for( ; x < Image->Width && CheckColumn( Image, x ) != 0; x++ )
                    {}
                XEnd    = x - 1;
                for( ; x < Image->Width && CheckColumn( Image, x ) == 0; x++ )
                    {}
                }
            while( x < Image->Width && x - XEnd < 16 /*8*/ );

#if 0
            if( x >= Width )
                break;

            XStart  = x;

            if( c == '\"' || c == 0x84 || c == 0x85 || c == 0x89 || c == 0x93 || c == 0x94 || c == 0x99 || c == 0xa8 ) // || c == 0xcf )
                {
                for( ; x < Width && CheckColumn( Image, Width, Height, x ) != 0; x++ )
                    {}
                for( ; x < Width && CheckColumn( Image, Width, Height, x ) == 0; x++ )
                    {}
                if( c == 0x85 )
                    {
                    for( ; x < Width && CheckColumn( Image, Width, Height, x ) != 0; x++ )
                        {}
                    for( ; x < Width && CheckColumn( Image, Width, Height, x ) == 0; x++ )
                        {}
                    }
                }

            for( ; x < Width && CheckColumn( Image, Width, Height, x ) != 0; x++ )
                {}

            XEnd    = x - 1;
#endif
            ExtractVert( Image, YTop, YBottom, XStart, XEnd, c );
            }

        }

    printf( "\n\t{" );
    for( c = 0; c < 256; c++ )
        {
        if(( c & 7 ) == 0 )
            printf( "\n\t" );
        printf( "0x%04X,", Offsets[c] );
        }
    printf( "\n\t}\n" );

    for( c = 0; c < 256; c++ )
        {
        if( Offsets[c] != 0xffff )
            {
            unsigned char   *p  = &Bitmaps[Offsets[c]];
            int             w   = *p++;
            printf( "\n\n\t// \'%c\' = 0x%02X\n\t%u,", c >= ' ' && c < 127 ? c : '?', c, w );
#if 0
//            w   = ( w + 7 ) / 8;
            for( y = 0; y < Height; y++ )
                {
                printf( "\n\t" );
    #if 0
                for( x = 0; x < w; x++ )
                    printf( "0x%02X, ", *p++ );
    #else
                char    t;
                for( x = 0; x < w; x++, t <<= 1 )
                    {
                    if(( x & 7 ) == 0 )
                        t = *p++;
                    printf( "%c", ( t & 0x80 ) ? 'X' : '.' );
                    }
    #endif // 0

#else

            for( x = 0; x < w; x++ )
                {
                unsigned char   *q = p;
                char    t;
                printf( "\n\t" );
                for( y = 0; y < ( YTop - YBottom + 1 + 7 ) >> 3; y++ )
                    printf( "0x%02X, ", *p++ );
                printf( "\t\t// " );
                for( y = 0; y < YTop - YBottom + 1; y++, t <<= 1 )
                    {
                    if(( y & 7 ) == 0 )
                        t = *q++;
                    printf( "%c", ( t & 0x80 ) ? 'X' : '.' );
                    }
#endif
                }
            }
        }
    }
#endif
/*============================================================================*/
#pragma pack(1)

typedef struct
    {
    unsigned short  Type;
    unsigned long   FileSize;
    unsigned short  Reserved1;
    unsigned short  Reserved2;
    unsigned long   DataOffset;
    } fileheader_t;

typedef struct
    {
    unsigned long   HeaderSize;
    unsigned long   ImageWidth;
    unsigned long   ImageHeight;
    unsigned short  Planes;
    unsigned short  BitsPerPixel;
    unsigned long   Compression;
    unsigned long   ImageSize;
    unsigned long   XPixelsPerMeter;
    unsigned long   YPixelsPerMeter;
    unsigned long   ColorsUsed;
    unsigned long   SignificantColors;
    } imageheader_t;

#pragma pack()
/*============================================================================*/
static int ReadBitmap( int File, image_t *Image )
	{
    fileheader_t    FileHeader;
    imageheader_t	ImageHeader;
	size_t			n;

	if( Image == NULL )
		return -1;

    if( read( File, &FileHeader, sizeof FileHeader ) != sizeof FileHeader )
        return -1;

    if( FileHeader.Type != 0x4d42 )
        return -1;

    if( read( File, &ImageHeader, sizeof ImageHeader ) != sizeof ImageHeader )
        return -1;

    if( lseek( File, 0, SEEK_END ) != FileHeader.FileSize )
        return -1;

    if( ImageHeader.Planes != 1 || ImageHeader.BitsPerPixel != 1 || ImageHeader.Compression != 0
        || ImageHeader.ColorsUsed != 0 || ImageHeader.SignificantColors != 0 )
        return -1;

    if(( Image->Buffer = malloc( ImageHeader.ImageSize )) == NULL )
        return -1;

    lseek( File, FileHeader.DataOffset, SEEK_SET );
    if(( n = read( File, Image->Buffer, ImageHeader.ImageSize )) != ImageHeader.ImageSize )
		{
		free( Image->Buffer );
		Image->Buffer	= NULL;
        return -1;
		}

	Image->Width		= ImageHeader.ImageWidth;
	Image->Height		= ImageHeader.ImageHeight;
	//Image->LineLength	= (( Image->Width + 31 ) * 32 ) / 32;
	Image->LineLength	= ImageHeader.ImageSize / ImageHeader.ImageHeight;

    return 1;
    }
/*============================================================================*/
static int PrintUsage( int Result )
	{
	fprintf( stderr,
		"\nUsage: fontgen <--printseed> | <[--reverse] <inputfilename>"  /*" [<outputfilename>]"*/ ">\n\n"
		"\t--printseed\t Prints the template to generate the source bitmap. Save\n"
		"\t\t\t it to a text file (\"fontgen --printseed > file.txt\") and\n"
		"\t\t\t use Paint.exe to create a monochromatic bitmap file (\".BMP\"\n"
		"\t\t\t extension) with a text with the exact representation of the\n"
		"\t\t\t template, and with the font, size and effects that you want\n"
		"\t\t\t for your font.\n\n"
		"\t\t\t or\n\n"
		"\t--reverse\t reverses the order of the columns of the bitmap.\n"
		"\t<inputfilename>\t the name of a bitmap file with the image to extract the font.\n"
/*		"\t<outputfilename> (optional) the name of the output file. If it is omitted,\n"
		"\t\t\t the result will be output to 'stdout'.\n" */
		 );

	return Result;
	}
/*============================================================================*/
static int PrintSeed( void )
	{
	printf( "%s",
#if 0
		"_\n"
		"_\n"
		"\n"
		"||\n"
		"\n"
		"| |\n"
		"\n"
		"A  !  \"  #  $  %  &  '  (  )  *  +  ,  -  .  /\n"
		"\n"
		"A  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?\n"
		"\n"
		"A  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O\n"
		"\n"
		"A  P  Q  R  S  T  U  V  W  X  Y  Z  [  \\  ]  ^  _\n"
		"\n"
		"A  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o\n"
		"\n"
		"A  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~\n"
		"\n"
		"A  €  ‚  ƒ  „  …  †  ‡  ˆ  ‰  Š  ‹  Œ  Ž  ‘  ’  “  ”\n"
		"\n"
		"A  •  –  —  ~  ™  š  ›  œ  ž  Ÿ  ¡  ¢  £  ¤  ¥  ¦  §\n"
		"\n"
		"A  ¨  ©  ª  «  ¬  ­  ®  ¯  °  ±  ²  ³  ´  µ  ¶  ·  ¸\n"
		"\n"
		"A  ¹  º  »  ¼  ½  ¾  ¿  À  Á  Â  Ã  Ä  Å  Æ  Ç  È  É  Ê  Ë\n"
		"\n"
		"A  Ì  Í  Î  Ï  Ð  Ñ  Ò  Ó  Ô  Õ  Ö  ×  Ø  Ù  Ú  Û  Ü\n"
		"\n"
		"A  Ý  Þ  ß  à  á  â  ã  ä  å  æ  ç  è  é  ê  ë  ì  í  î  ï\n"
		"\n"
		"A  ð  ñ  ò  ó  ô  õ  ö  ÷  ø  ù  ú  û  ü  ý  þ  ÿ"
#else
		"_\n"
		"_\n"
		"\n"
		"||\n"
		"\n"
		"| |\n"
		"\n"
		"A     !  \"  #  $  %  &  '  (  )  *  +  ,  -  .  / \n"
		"\n"
		"A  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ? \n"
		"\n"
		"A  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O \n"
		"\n"
		"A  P  Q  R  S  T  U  V  W  X  Y  Z  [  \\  ]  ^  _ \n"
		"\n"
		"A  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o \n"
		"\n"
		"A  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~    \n"
		"\n"
		"A  €     ‚  ƒ  „  …  †  ‡  ˆ  ‰  Š  ‹  Œ     Ž    \n"
		"\n"
		"A     ‘  ’  “  ”  •  –  —     ™  š  ›  œ     ž  Ÿ \n"
		"\n"
		"A     ¡  ¢  £  ¤  ¥  ¦  §  ¨  ©  ª  «  ¬     ®  ¯ \n"
		"\n"
		"A  °  ±  ²  ³  ´  µ  ¶  ·  ¸  ¹  º  »  ¼  ½  ¾  ¿ \n"
		"\n"
		"A  À  Á  Â  Ã  Ä  Å  Æ  Ç  È  É  Ê  Ë  Ì  Í  Î  Ï \n"
		"\n"
		"A  Ð  Ñ  Ò  Ó  Ô  Õ  Ö  ×  Ø  Ù  Ú  Û  Ü  Ý  Þ  ß \n"
		"\n"
		"A  à  á  â  ã  ä  å  æ  ç  è  é  ê  ë  ì  í  î  ï \n"
		"\n"
		"A  ð  ñ  ò  ó  ô  õ  ö  ÷  ø  ù  ú  û  ü  ý  þ  ÿ \n"
#endif
		);

	return 0;
	}
/*============================================================================*/
int main( int ArgC, char *ArgV[] )
    {
    int             Handle, Reverse = 0, FirstArg = 1;
    image_t			Image;

#if 0
    for( Handle = 32; Handle < 256; Handle++ )
		printf( " %c ", Handle );
	return 0;
#endif

    fprintf( stderr, "\nFontGen 1.0 - Copyright (c) 2010-2026, Isaac Marino Bavaresco\n" );

    if( ArgC < 2 || ArgC > 4 )
		return PrintUsage( 0 );

	if( stricmp( ArgV[1], "--printseed" ) == 0 )
		{
		if( ArgC != 2 )
			{
			fprintf( stderr, "\nError: Too many arguments.\n" );
			return PrintUsage( 1 );
			}
		return PrintSeed();
		}

	if( stricmp( ArgV[1], "--reverse" ) == 0 )
		{
		if( ArgC < 3 )
			{
			fprintf( stderr, "\nError: Too few arguments.\n" );
			return PrintUsage( 1 );
			}
		Reverse		= 1;
		FirstArg	= 2;
		}

    if(( Handle = open( ArgV[FirstArg], O_RDONLY | O_BINARY )) == -1 )
        {
        printf( "\nError opening file.\n" );
        return -1;
        }


	if( ReadBitmap( Handle, &Image ) < 0 )
		{
		close( Handle );
        printf( "\nError reading file.\n" );
		return -1;
		}

    close( Handle );

    Process( &Image, Reverse );

    free( Image.Buffer );

    return 0;
    }

/*============================================================================*/
