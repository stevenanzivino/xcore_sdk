#include <cmath>    // std::ceil
#include <iostream> // std::cin, std::cout, std:endl
#include <cstring>  // std::memcpy

extern "C"
{
#include <sys/time.h>
}

#ifdef XCORE
// #include <xcore_all.h> // TODO get hwtimer for profiling
#endif

#include "Image.h"
#include "Kernel.h"
#include "vision.h"
#include "vision_utils.hpp"

#define DATA_OFFSET_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define RESERVED 0x0000
#define HEADER_SIZE 14
#define DIB_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0

namespace vision
{
    typedef unsigned int uint32;
    typedef signed short int int16;

    // ****Inputs***
    // fileName: The name of the file to open
    // ****Outputs***
    // pixels: A pointer to a byte array. This will contain the .bmp pixel data
    void read_bitmap(const std::string fileName, Image &image, const int debug_mode)
    {
        // Open the file for reading in binary mode
        FILE *imageFile;
        imageFile = fopen(fileName.c_str(), "rb");
        debug_printf("\n--------------------");
        debug_printf("\nReading from file...");

        if (imageFile != NULL) // file was opened successfully
        {
            debug_printf("\nFile %s opened!\n", fileName.c_str());
            // Read data offset - starting address of the byte where the .bmp image (pixel array) can be found (size = 4 bytes)
            uint32 dataOffset;
            fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
            fread(&dataOffset, 4, 1, imageFile);

            // Read width (size = 4 bytes)
            uint32_t cols = 0;
            fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
            fread(&cols, 4, 1, imageFile);

            // Read height (size = 4 bytes)
            uint32_t rows = 0;
            fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
            fread(&rows, 4, 1, imageFile);

            // Read bits per pixel = colour depth of image (size = 2 bytes)
            int16 bitsPerPixel;
            fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
            fread(&bitsPerPixel, 2, 1, imageFile);

            // Compute bytes per pixel = number of channels
            uint32 bytesPerPixel = 0;
            bytesPerPixel = ((uint32)bitsPerPixel) / 8;

            // Image info
            debug_printf("\nBefore channel correction:\n");
            debug_printf("data offset: %d\n", dataOffset);
            debug_printf("image columns: %d\n", image.cols());
            debug_printf("image rows: %d\n", image.rows());
            debug_printf("image row stride: %d\n", image.row_stride());
            debug_printf("image channels: %d\n", image.chans());
            debug_printf("bits per pixel: %d\n", bitsPerPixel);
            debug_printf("bytes per pixel: %d\n", bytesPerPixel);

            // Resize image matrix and set image params., i.e., image.rows, image.cols etc.)
            image.resize(rows, cols, bytesPerPixel);

            // Calculate the unpadded row size in bytes
            int unpaddedRowSize = (image.cols()) * (bytesPerPixel);

            // Calculate the padded row size in bytes
            int paddedRowSize = image.row_stride();
            if (image.row_stride() % 4)
                paddedRowSize += 4 - (image.row_stride() % 4);

            // Calculate the total size of the pixel data in bytes
            int imageSize = paddedRowSize * (image.rows());

            // Read the pixel data Row by Row.
            // Data is padded and stored bottom-up, so fill up image.pix from the end (last row)
            std::vector<sample_t> temp(image.row_stride());
            for (int i = 0; i < image.rows(); i++)
            {
                // Start filling starting from the last row of our pixel array (unpadded)
                // Put file cursor in the next row from top to bottom
                fseek(imageFile, dataOffset + (i * paddedRowSize), SEEK_SET);

                // Read only unpaddedRowSize bytes (ignore the padding bytes)
                fread(temp.data(), 1, unpaddedRowSize, imageFile);

                // Make signed
                for (auto &s : temp)
                    s = index_to_sample(s);

                // Update image
                std::memmove(&image.image_data->front() + image.row_stride() * (image.rows() - i - 1), temp.data(), image.row_stride());
            }

            // Image info
            debug_printf("\nAfter channel correction:\n");
            debug_printf("data offset: %d\n", dataOffset);
            debug_printf("image columns: %d\n", image.cols());
            debug_printf("image rows: %d\n", image.rows());
            debug_printf("image row stride: %d\n", image.row_stride());
            debug_printf("image channels: %d\n", image.chans());
            debug_printf("bits per pixel: %d\n", bitsPerPixel);
            debug_printf("bytes per pixel: %d\n", bytesPerPixel);
            debug_printf("unpadded row size [bytes]: %d\n", unpaddedRowSize);
            debug_printf("padded row size [bytes]: %d\n", paddedRowSize);
            debug_printf("total size of (padded) data [bytes]: %d\n", imageSize);

            // Close file
            fclose(imageFile);
            debug_printf("File %s closed!\n", fileName.c_str());
            debug_printf("--------------------\n");
        }
        else // file couldn't be opened
        {
            // Close file
            debug_printf("File %s CANNOT be opened!\n\n", fileName.c_str());
            debug_printf("--------------------\n");
        }
    }

    // ****Inputs***
    // fileName: The name of the file to open
    // pixels: A pointer to a byte array. This will contain the .bmp pixel data
    void write_bitmap(const std::string fileName, Image &image, const int debug_mode)
    {
        if (image.chans() > 4)
        {
            debug_printf("\nMore than 4 channels detected!\n");
            return;
        }
        // Open the file for reading in binary mode
        FILE *imageFile = fopen(fileName.c_str(), "wb");

        debug_printf("\n--------------------");
        debug_printf("\nWriting to file...");

        if (imageFile != NULL) // file was opened successfully
        {
            // TODO figure out how to make an 8-bit bitmap!
            if (image.chans() == 1)
            {
                image.resize(image.rows(), image.cols(), 3);
                vision::uncollated_replication(*image.image_data, 3, false);
            }

            debug_printf("\nFile %s opened!\n", fileName.c_str());

            // ** Bitmap file header (mandatory) ** //
            // Write signature ("BM")
            const char *signature = "BM";
            fwrite(&signature[0], 1, 1, imageFile);
            fwrite(&signature[1], 1, 1, imageFile);

            // Write file size
            uint32 bytesPerPixel = image.chans();
            debug_printf("\nbytesPerPixel: %d", bytesPerPixel);
            uint32 dataSize = image.row_stride() * image.rows();
            debug_printf("\ndataSize: %d", dataSize);
            uint32 fileSize = dataSize + HEADER_SIZE + DIB_HEADER_SIZE;

            fwrite(&fileSize, 4, 1, imageFile);

            // Write reserved bits
            uint32 reserved = RESERVED;
            fwrite(&reserved, 4, 1, imageFile);

            // Write data offset
            uint32 dataOffset = HEADER_SIZE + DIB_HEADER_SIZE;
            fwrite(&dataOffset, 4, 1, imageFile);

            // ** DIB header (mandatory) ** //
            // Write DIB header size
            uint32 DIBHeaderSize = DIB_HEADER_SIZE;
            fwrite(&DIBHeaderSize, 4, 1, imageFile);

            // Write width
            uint32_t rows = image.cols();
            fwrite(&rows, 4, 1, imageFile);

            // Write height
            uint32_t cols = image.rows();
            fwrite(&cols, 4, 1, imageFile);

            // Write planes
            uint32 planes = 1; // always 1
            fwrite(&planes, 2, 1, imageFile);

            // Write bits per pixel
            uint32 bitsPerPixel = bytesPerPixel * 8;
            fwrite(&bitsPerPixel, 2, 1, imageFile);

            // Write compression
            uint32 compression = NO_COMPRESION;
            fwrite(&compression, 4, 1, imageFile);

            // Calculate the unpadded row size in bytes
            int unpaddedRowSize = (image.cols()) * (bytesPerPixel);

            // Calculate the padded row size in bytes
            int paddedRowSize = image.row_stride();
            if (image.row_stride() % 4)
                paddedRowSize += 4 - (image.row_stride() % 4);

            // Write image size (in bytes)
            uint32 imageSize = paddedRowSize * image.cols() * bytesPerPixel;
            debug_printf("\nimageSize: %d\n", imageSize);
            fwrite(&imageSize, 4, 1, imageFile);

            // Write resolution (pixels/meter)
            uint32 resolutionX = 11811; // 300 dpi
            uint32 resolutionY = 11811; // 300 dpi
            fwrite(&resolutionX, 4, 1, imageFile);
            fwrite(&resolutionY, 4, 1, imageFile);

            // Write colours in colour table
            uint32 colourTable = MAX_NUMBER_OF_COLORS;
            fwrite(&colourTable, 4, 1, imageFile);

            // Write important colour count
            uint32 importantColours = ALL_COLORS_REQUIRED;
            fwrite(&importantColours, 4, 1, imageFile);

            // Make unsigned
            for (auto &s : *image.image_data)
            {
                s = (sample_t)sample_to_index(s);
            }

            // ** Pixel array (mandatory) ** //
            // write data from image->pix starting from the end (last row)
            flip(image, ROW);
            horizontal_pad(image, 0, paddedRowSize - image.row_stride(), 0);
            fwrite(image.image_data->data(), 1, image.image_data->size(), imageFile);

            // Image info
            debug_printf("data offset: %d\n", dataOffset);
            debug_printf("image columns: %d\n", image.cols());
            debug_printf("image rows: %d\n", image.rows());
            debug_printf("image row stride: %d\n", image.row_stride());
            debug_printf("image channels: %d\n", image.chans());
            debug_printf("bits per pixel: %d\n", bitsPerPixel);
            debug_printf("bytes per pixel: %d\n", bytesPerPixel);
            debug_printf("unpadded row size [bytes]: %d\n", unpaddedRowSize);
            debug_printf("padded row size [bytes]: %d\n", paddedRowSize);
            debug_printf("total size of (padded) data [bytes]: %d\n", imageSize);

            // Close file
            fclose(imageFile);
            debug_printf("File %s closed!\n", fileName.c_str());
            debug_printf("--------------------\n");
        }
        else // file couldn't be opened
        {
            debug_printf("File %s CANNOT be opened!\n", fileName.c_str());
            debug_printf("--------------------\n");
        }
    }

} // end namespace
