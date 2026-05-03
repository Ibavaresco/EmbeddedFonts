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

unsigned long   LineWidth;

int QueryPixel( unsigned char *Image, unsigned long Width, unsigned long Height, unsigned long X, unsigned long Y )
    {
//    unsigned long   LineWidth   = ((( Width + 7 ) / 8 ) + 3 ) & ~3;
    if(( Image[Y * LineWidth + ( X / 8 )] & ( 0x80 >> ( X & 0x07 ))) == 0 )
        return 1;
    else
        return 0;
    }

/*============================================================================*/

int CheckColumn( unsigned char *Image, unsigned long Width, unsigned long Height, unsigned long X )
    {
    unsigned long   Y;

    for( Y = 0; Y < Height; Y++ )
        if( QueryPixel( Image, Width, Height, X, Y ))
            return 1;
    return 0;
    }

/*============================================================================*/

int CheckLine( unsigned char *Image, unsigned long Width, unsigned long Height, unsigned long Y )
    {
    unsigned long   x;

    for( x = 0; x < Width; x++ )
        if( QueryPixel( Image, Width, Height, x, Y ))
            return 1;
    return 0;
    }

/*============================================================================*/

unsigned char   Bitmaps[65536];
unsigned short  CurrentOffset   = 0;
unsigned short  Offsets[256];

/*============================================================================*/

int ExtractHoriz( unsigned char *Image, unsigned long Width, unsigned long Height, unsigned long XStart, unsigned long XEnd, unsigned char c )
    {
    unsigned long   x, BitCount, Value;
    long int        y;

    Offsets[c]  = CurrentOffset;

    Bitmaps[CurrentOffset++]    = XEnd - XStart + 1;
    for( y = Height - 1; y >= 0 ; y-- )
        {
        for( BitCount = 0, Value = 0, x = XStart; x <= XEnd; x++ )
            {
            Value   = ( Value << 1 ) | QueryPixel( Image, Width, Height, x, y );
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

int ExtractVert( unsigned char *Image, unsigned long Width, unsigned long Height, unsigned long YTop, unsigned long YBottom, unsigned long XStart, unsigned long XEnd, unsigned char c )
    {
    unsigned long   x, BitCount, Value;
    long int        y;

    Offsets[c]  = CurrentOffset;

    Bitmaps[CurrentOffset++]    = XEnd - XStart + 1;
    for( x = XStart; x <= XEnd; x++ )
        {
        for( BitCount = 0, Value = 0, y = YTop; y >= (long int)YBottom; y-- )
            {
            Value   = ( Value << 1 ) | QueryPixel( Image, Width, Height, x, y );
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

void Process( unsigned char *Image, unsigned long Width, unsigned long Height )
    {
    unsigned long   YTop, YBottom;
    unsigned long   x, y, XStart, XEnd;
    unsigned int   c;

    memset( Offsets, 0xff, sizeof Offsets );
    memset( Bitmaps, 0x00, sizeof Bitmaps );
    CurrentOffset   = 0;

    for( y = 0; y < Height && CheckLine( Image, Width, Height, y ) == 0; y++ )
        {}
    if( y == Height )
        {
        printf( "\nThe image is empty.\n" );
        return;
        }
    YBottom = y;
    for( y = Height - 1; y > YBottom && CheckLine( Image, Width, Height, y ) == 0; y-- )
        {}
    YTop    = y;

    for( c = '!', x = 0; x < Width; c++ )
        {
        if( c == 0x81 || c == 0x8d || c == 0x8f || c == 0x90 || c == 0x9d || c == 0xa0 || c == 0xad )
            {
            XStart  = x;
            XEnd    = x;
            }
        else
            {
            for( ; x < Width && CheckColumn( Image, Width, Height, x ) == 0; x++ )
                {}
            if( x >= Width )
                break;
            XStart  = x;
            do
                {
                for( ; x < Width && CheckColumn( Image, Width, Height, x ) != 0; x++ )
                    {}
                XEnd    = x - 1;
                for( ; x < Width && CheckColumn( Image, Width, Height, x ) == 0; x++ )
                    {}
                }
            while( x < Width && x - XEnd < 16 /*8*/ );

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
            ExtractVert( Image, Width, Height, YTop, YBottom, XStart, XEnd, c );
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
            printf( "\n\n\t// \'%c\' = 0x%02X\n\t%u,", c, c, w );
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

/*============================================================================*/

int main( int ArgC, char *ArgV[] )
    {
    int             Handle, n;
    fileheader_t    FileHeader;
    imageheader_t   ImageHeader;
    unsigned char   *ImageData;

#if 0
    for( Handle = 33; Handle < 256; Handle++ )
        printf( "%c ", Handle );
    return 0;
#endif

    if( ArgC != 2 )
        {
        printf( "\nMissing argument.\n" );
        return -1;
        }
    if(( Handle = open( ArgV[1], O_RDONLY | O_BINARY )) == -1 )
        {
        printf( "\nErro opening file.\n" );
        return -1;
        }

    if( read( Handle, &FileHeader, sizeof FileHeader ) != sizeof FileHeader )
        {
        printf( "\nError reading file.\n" );
        close( Handle );
        return -1;
        }

    if( read( Handle, &ImageHeader, sizeof ImageHeader ) != sizeof ImageHeader )
        {
        printf( "\nError reading file.\n" );
        close( Handle );
        return -1;
        }

    if(( ImageData = malloc( ImageHeader.ImageSize )) == NULL )
        {
        printf( "\nError allocating memory.\n" );
        close( Handle );
        return -1;
        }

    LineWidth   = ImageHeader.ImageSize / ImageHeader.ImageHeight;

    n = lseek( Handle, FileHeader.DataOffset, SEEK_SET );
    if(( n = read( Handle, ImageData, ImageHeader.ImageSize )) != ImageHeader.ImageSize )
        {
        printf( "\nError reading file.\n" );
        close( Handle );
        return -1;
        }

    Process( ImageData, ImageHeader.ImageWidth, ImageHeader.ImageHeight );

    free( ImageData );
    close( Handle );

    return 0;
    }

/*============================================================================*/
