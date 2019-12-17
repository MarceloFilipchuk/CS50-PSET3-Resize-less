// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        printf("Usage: ./resize n infile outfile\n");
        return 1;
    }

    // remembers 'n' and filenames
    const int n = atoi(argv[1]);
    char *infile = argv[2];
    char *outfile = argv[3];

    if (n < 1 || n > 100)
    {
        printf("Usage: ./resize n infile outfile\n");
        return 1;
    }

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        printf("Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        printf("Unsupported file format.\n");
        return 4;
    }
    // define infile padding
    int infile_padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    // save the data from infile's height and width for later iterations in order to safely modify the outfile's headers
    int infile_width = bi.biWidth;
    int infile_height = bi.biHeight;
    // modifys BITMAPINFOHEADER's width and height by 'n' times
    bi.biHeight = bi.biHeight * n;
    bi.biWidth = bi.biWidth * n;
    // determine outfile's padding for scanlines using the new height and width values
    int outfile_padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    // define the new BITMAPFILEHEADER's bfSizeImage according to the new biHeight and biWidth
    bi.biSizeImage = ((sizeof(RGBTRIPLE)*bi.biWidth) + outfile_padding) * abs(bi.biHeight);
    // define the new BITMAPFILEHEADER's biSize according to the new dimensions
    bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);
    // write outfile's BITMAPINFOHEADER multiplying the image's height and width 'n' times
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);


     // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(infile_height); i < biHeight; i++)
    {
        // iterates n times to create the vertical copy
        for (int m = 0; m < (n - 1); m++)
        {
            // iterates over each pixel
            for (int j = 0; j < infile_width; j++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                // write RGB triple to outfile 'n' times
                for (int l = 0; l < n; l++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }

            // adds padding (if necesary) to outfile
            if (outfile_padding != 0)
                for (int k = 0; k < outfile_padding; k++)
                {
                    fputc(0x00, outptr);
                }

            // skip over padding in infile, if any
            fseek(inptr, infile_padding, SEEK_CUR);

            // returns to the begining of the scanline
            fseek(inptr, -(sizeof(RGBTRIPLE) * infile_width + infile_padding), SEEK_CUR);
        }
        // adds the final scanline
        for (int j = 0; j < infile_width; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            // write RGB triple to outfile 'n' times
            for (int l = 0; l < n; l++)
            {
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
            }
        }

        // adds padding (if necesary) to outfile
        if (outfile_padding != 0)
            for (int k = 0; k < outfile_padding; k++)
            {
                fputc(0x00, outptr);
            }

        // skip over padding in infile, if any
        fseek(inptr, infile_padding, SEEK_CUR);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}


