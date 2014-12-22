
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#pragma pack(1)


typedef short                WORD;
typedef unsigned int        DWORD;
typedef int                    LONG;
typedef unsigned char    BYTE;


typedef struct tagBITMAPFILEHEADER {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBTRIPLE {
  BYTE rgbtBlue;
  BYTE rgbtGreen;
  BYTE rgbtRed;
} RGBTRIPLE;

int ReadBitmap(char* filename, unsigned char** bitmap, int* width, int* height, int* colordepth)
{
    FILE*                    l_file;
    BITMAPFILEHEADER    fileheader;
    BITMAPINFOHEADER    infoheader;
    RGBTRIPLE            rgb;
    unsigned char*        thebitmap;
    int                    row,column;

    if ((l_file = fopen(filename, "rb"))==NULL)
    {
        printf("Could not open %s\n", filename);
        return -1;
    }


    fseek(l_file, 0, SEEK_SET);
    fread(&fileheader.bfType, sizeof(WORD), 1, l_file);
    fread(&fileheader.bfSize, sizeof(DWORD), 1, l_file);
    fread(&fileheader.bfReserved1, sizeof(WORD), 1, l_file);
    fread(&fileheader.bfReserved2, sizeof(WORD), 1, l_file);
    fread(&fileheader.bfOffBits, sizeof(DWORD), 1, l_file);


    fseek(l_file, 18, SEEK_SET);
    fread(&infoheader.biWidth, sizeof(int), 1, l_file);
    fread(&infoheader.biHeight, sizeof(int), 1, l_file);
    fread(&infoheader.biPlanes, sizeof(short int), 1, l_file);

    *width = infoheader.biWidth;
    *height = infoheader.biHeight;

    if (infoheader.biPlanes != 1)
    {
        printf("Planes from %s is not 1: %u\n", filename, infoheader.biPlanes);
        return -1;
    }

    fread(&infoheader.biBitCount, sizeof(unsigned short int), 1, l_file);
    if (infoheader.biBitCount != 24)
    {
        printf("Bpp from %s is not 24:\n", filename);
        return -1;
    }

    fseek(l_file, 54, SEEK_SET);

    thebitmap = (BYTE *)calloc((infoheader.biWidth * infoheader.biHeight * 4), 1);

    for (row = infoheader.biHeight-1; row >= 0 ; row--)
    {
        for (column = 0; column < infoheader.biWidth; column++)
        {
            fread(&rgb, sizeof(rgb), 1, l_file);
            thebitmap[ ((row * infoheader.biWidth) + column) * 4 + 0 ] = rgb.rgbtRed;
            thebitmap[ ((row * infoheader.biWidth) + column) * 4 + 1 ] = rgb.rgbtGreen;
            thebitmap[ ((row * infoheader.biWidth) + column) * 4 + 2 ] = rgb.rgbtBlue;
            thebitmap[ ((row * infoheader.biWidth) + column) * 4 + 3 ] = 255; /* Alpha value */
        }

        // PADDING
        unsigned char padbyte;
        int i;
        for (i=0 ; i < ((infoheader.biWidth*3) % 4); i++ )
        {
            fread(&padbyte, 1, 1, l_file);
        }

    }

    *bitmap = thebitmap;
    *colordepth = 32;

    fclose(l_file);

    return 0;
}

void FreeBitmap(unsigned char* bitmap)
{
    if (bitmap) free(bitmap);
}


int WriteBitmap(char* filename, unsigned char* bitmap, int width, int height, int colordepth)
{
    FILE*               l_file;
    BITMAPFILEHEADER    fileheader;
    BITMAPINFOHEADER    infoheader;
    RGBTRIPLE           rgb;
    int                 row,column;

    if( (l_file = fopen(filename, "wb+"))==NULL)
    {
        printf("Could not create %s\n", filename);
        return -1;
    }

    fileheader.bfType = 0x4D42;
    fileheader.bfSize =  (( (width*3) + (width*3) % 4)  * height) + 54;
    fileheader.bfReserved1 = 0x0;
    fileheader.bfReserved2 = 0x0;
    fileheader.bfOffBits = 54;

    fwrite(&fileheader, sizeof(fileheader), 1, l_file);

    infoheader.biSize               = 40;
    infoheader.biWidth              = width;
    infoheader.biHeight             = height;
    infoheader.biPlanes             = 1;
    infoheader.biBitCount           = 24;
    infoheader.biCompression        = 0x0;
    infoheader.biSizeImage          = (( (width*3) + (width*3) % 4)  * height) + 54;
    infoheader.biXPelsPerMeter      = 0;
    infoheader.biYPelsPerMeter      = 0;
    infoheader.biClrUsed            = 0x0;
    infoheader.biClrImportant       = 0x0;

    fwrite(&infoheader, sizeof(infoheader), 1, l_file);

    for (row = infoheader.biHeight - 1; row >= 0 ; row--)
    {
        for (column = 0; column < infoheader.biWidth; column++)
        {
            rgb.rgbtBlue  = bitmap[ ((row * infoheader.biWidth) + column) * 4 + 0 ];
            rgb.rgbtGreen = bitmap[ ((row * infoheader.biWidth) + column) * 4 + 1 ];
            rgb.rgbtRed   = bitmap[ ((row * infoheader.biWidth) + column) * 4 + 2 ];

            fwrite(&rgb, sizeof(rgb), 1, l_file);
        }

        // PADDING
        char nullbyte = 0;
        int i;

        for (i=0 ; i < ((width*3) % 4); i++ )
        {
            fwrite(&nullbyte, 1, 1, l_file);
        }

    }

    fclose(l_file);

    return 0;
}

int ReadBitmapInfo(char* filename)
{
    FILE*                    l_file;
    BITMAPFILEHEADER    fileheader;
    BITMAPINFOHEADER    infoheader;

    if( (l_file = fopen(filename, "rb"))==NULL)
    {
         printf("Could not open %s\n", filename);
        return -1;
    }

    fread(&fileheader, sizeof(fileheader), 1, l_file);

    fread(&infoheader, sizeof(infoheader), 1, l_file);

    fclose(l_file);

    printf("fileheader.bfType             = 0x%0x\n", fileheader.bfType);
    printf("fileheader.bfSize             = %d\n", fileheader.bfSize);
    printf("fileheader.bfReserved1        = 0x%0x\n", fileheader.bfReserved1);
    printf("fileheader.bfReserved2        = 0x%0x\n", fileheader.bfReserved2);
    printf("fileheader.bfOffBits          = %d\n", fileheader.bfOffBits);

    printf("infoheader.biSize             = %d\n", infoheader.biSize);
    printf("infoheader.biWidth            = %d\n", infoheader.biWidth);
    printf("infoheader.biHeight           = %d\n", infoheader.biHeight);
    printf("infoheader.biPlanes           = %d\n", infoheader.biPlanes);
    printf("infoheader.biBitCount         = %d\n", infoheader.biBitCount);
    printf("infoheader.biCompression      = 0x%0x\n", infoheader.biCompression);
    printf("infoheader.biSizeImage        = %d\n", infoheader.biSizeImage);
    printf("infoheader.biXPelsPerMeter    = %d\n", infoheader.biXPelsPerMeter);
    printf("infoheader.biYPelsPerMeter    = %d\n", infoheader.biYPelsPerMeter);
    printf("infoheader.biClrUsed          = 0x%0x\n", infoheader.biClrUsed);
    printf("infoheader.biClrImportant     = 0x%0x\n", infoheader.biClrImportant);

    return 0;
}
