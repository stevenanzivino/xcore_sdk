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
//#include "file_utils.cc" //Added due to inability to find filp and pad functions
#include "vision_utils.hpp"

//FreeRtos Headers
#include "FreeRTOS.h"
//#include <platform.h>
#include "rtos/osal/api/rtos_osal.h"
//#include "header_file.h"


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

//persondetect
#define portDISABLE_INTERRUPTS() rtos_interrupt_mask_all();
#define portENABLE_INTERRUPTS() rtos_interrupt_unmask_all();

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
        rtos_printf("\n--------------------");
        rtos_printf("\nReading from file...");

        if (imageFile != NULL) // file was opened successfully
        {
            rtos_printf("\nFile %s opened!\n", fileName.c_str());
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
            rtos_printf("\nBefore channel correction:\n");
            rtos_printf("data offset: %d\n", dataOffset);
            rtos_printf("image columns: %d\n", image.cols());
            rtos_printf("image rows: %d\n", image.rows());
            rtos_printf("image row stride: %d\n", image.row_stride());
            rtos_printf("image channels: %d\n", image.chans());
            rtos_printf("bits per pixel: %d\n", bitsPerPixel);
            rtos_printf("bytes per pixel: %d\n", bytesPerPixel);

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
            rtos_printf("\nAfter channel correction:\n");
            rtos_printf("data offset: %d\n", dataOffset);
            rtos_printf("image columns: %d\n", image.cols());
            rtos_printf("image rows: %d\n", image.rows());
            rtos_printf("image row stride: %d\n", image.row_stride());
            rtos_printf("image channels: %d\n", image.chans());
            rtos_printf("bits per pixel: %d\n", bitsPerPixel);
            rtos_printf("bytes per pixel: %d\n", bytesPerPixel);
            rtos_printf("unpadded row size [bytes]: %d\n", unpaddedRowSize);
            rtos_printf("padded row size [bytes]: %d\n", paddedRowSize);
            rtos_printf("total size of (padded) data [bytes]: %d\n", imageSize);

            // Close file
            fclose(imageFile);
            rtos_printf("File %s closed!\n", fileName.c_str());
            rtos_printf("--------------------\n");
        }
        else // file couldn't be opened
        {
            // Close file
            rtos_printf("File %s CANNOT be opened!\n\n", fileName.c_str());
            rtos_printf("--------------------\n");
        }
    }

    // ****Inputs***
    // fileName: The name of the file to open
    // pixels: A pointer to a byte array. This will contain the .bmp pixel data
    void write_bitmap(const std::string fileName, Image &image, const int debug_mode)
    {
        #define SETSR(c) asm volatile("setsr %0" : : "n"(c));
        #define CLRSR(c) asm volatile("clrsr %0" : : "n"(c));
        if (image.chans() > 4)
        {
            rtos_printf("\nMore than 4 channels detected!\n");
            return;
        }
        // Open the file for reading in binary mode
        rtos_printf("\n--------------------+++++++++++");
        //char* buffer = (char*) malloc (10000);
        //buffer[9999] = 'a';
        portDISABLE_INTERRUPTS();
        CLRSR(XS1_SR_KEDI_MASK);
        //int state = rtos_osal_critical_enter();
        FILE *imageFile = fopen(fileName.c_str(), "wb");

        rtos_printf("\n--------------------");
        rtos_printf("\nWriting to file...");
        //*
        if (imageFile != NULL) // file was opened successfully
        {
            // TODO figure out how to make an 8-bit bitmap!
            if (image.chans() == 1)
            {
                image.resize(image.rows(), image.cols(), 3);
                vision::uncollated_replication(*image.image_data, 3, false);
            }

            rtos_printf("\nFile %s opened!\n", fileName.c_str());

            // ** Bitmap file header (mandatory) ** //
            // Write signature ("BM")
            const char *signature = "BM";
            fwrite(&signature[0], 1, 1, imageFile);
            rtos_printf("Print after first Fwrite\n");
            fwrite(&signature[1], 1, 1, imageFile);

            // Write file size
            uint32 bytesPerPixel = image.chans();
            rtos_printf("\nbytesPerPixel: %d", bytesPerPixel);
            uint32 dataSize = image.row_stride() * image.rows();
            rtos_printf("\ndataSize: %d", dataSize);
            uint32 fileSize = dataSize + HEADER_SIZE + DIB_HEADER_SIZE;

            fwrite(&fileSize, 4, 1, imageFile);

            // Write reserved bits
            uint32 reserved = RESERVED;
            fwrite(&reserved, 4, 1, imageFile);

            // Write data offset
            uint32 dataOffset = HEADER_SIZE + DIB_HEADER_SIZE;
            fwrite(&dataOffset, 4, 1, imageFile);
            rtos_printf("Print after first Fwrite\n");
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
            rtos_printf("Print after first Fwrite\n");
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
            rtos_printf("\nimageSize: %d\n", imageSize);
            fwrite(&imageSize, 4, 1, imageFile);

            // Write resolution (pixels/meter)
            uint32 resolutionX = 11811; // 300 dpi
            uint32 resolutionY = 11811; // 300 dpi
            fwrite(&resolutionX, 4, 1, imageFile);
            fwrite(&resolutionY, 4, 1, imageFile);
            rtos_printf("Print after first Fwrite\n");
            rtos_printf("Print after first Fwrite 2\n");
            // Write colours in colour table
            uint32 colourTable = MAX_NUMBER_OF_COLORS;
            fwrite(&colourTable, 4, 1, imageFile);
            rtos_printf("Print after first Fwrite 2\n");
            // Write important colour count
            uint32 importantColours = ALL_COLORS_REQUIRED;
            fwrite(&importantColours, 4, 1, imageFile);
            rtos_printf("Print after first Fwrite 2\n");
            // Make unsigned
            for (auto &s : *image.image_data)
            {
                s = (sample_t)sample_to_index(s);
            }
            rtos_printf("Print after first Fwrite 2\n");
            // ** Pixel array (mandatory) ** //
            // write data from image->pix starting from the end (last row)
            rtos_printf("Print after first Fwrite 3\n");
            flip(image, ROW);
            rtos_printf("Print after first Fwrite 3\n");
            //horizontal_pad(image, 0, paddedRowSize - image.row_stride(), 0);
            rtos_printf("Print after first Fwrite 3\n");
            fwrite(image.image_data->data(), 1, image.image_data->size(), imageFile);
            rtos_printf("Print after first Fwrite\n");
            rtos_printf("Print after first Fwrite 2\n");
            // Image info
            rtos_printf("data offset: %d\n", dataOffset);
            rtos_printf("image columns: %d\n", image.cols());
            rtos_printf("image rows: %d\n", image.rows());
            rtos_printf("image row stride: %d\n", image.row_stride());
            rtos_printf("image channels: %d\n", image.chans());
            rtos_printf("bits per pixel: %d\n", bitsPerPixel);
            rtos_printf("bytes per pixel: %d\n", bytesPerPixel);
            rtos_printf("unpadded row size [bytes]: %d\n", unpaddedRowSize);
            rtos_printf("padded row size [bytes]: %d\n", paddedRowSize);
            rtos_printf("total size of (padded) data [bytes]: %d\n", imageSize);

            // Close file
            fclose(imageFile);
            rtos_printf("File %s closed!\n", fileName.c_str());
            rtos_printf("--------------------\n");
        }
        else // file couldn't be opened
        {
            rtos_printf("File %s CANNOT be opened!\n", fileName.c_str());
            rtos_printf("--------------------\n");
        }//*/
        //free(buffer);
        //rtos_osal_critical_exit(state);
        SETSR(XS1_SR_KEDI_MASK);
        portENABLE_INTERRUPTS()
    }

} // end namespace
