#include <cmath>         // std::ceil
#include <iostream>      // std::cin, std::cout, std:endl, std:: std
#include <fstream>       // std::ofstream
#include <cstring>       // std::memcpy
#include <stdexcept>     // std::invalid argument
#include <unordered_map> // std::unordered_map
#include <string>        // std::string

//FreeRTOS Headers
#include "FreeRTOS.h"
#include "rtos/osal/api/rtos_osal.h"

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
#define INFO_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0

//person Detect
#define portDISABLE_INTERRUPTS() rtos_interrupt_mask_all();
#define portENABLE_INTERRUPTS() rtos_interrupt_unmask_all();

namespace vision
{
    typedef unsigned int uint32;
    typedef signed short int int16;

    /// Map colorspace types to strings
    std::unordered_map<Colorspace, std::string> valColorspace{
        {GRAY, "GRAY"},
        {BGR, "BGR"},
        {BGRA, "BGRA"},
        {RGB, "RGB"},
        {YUV, "YUV"}};

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
        debug_printf("\nReading image from .BMP file...");

        if (imageFile != NULL) // file was opened successfully
        {
            debug_printf("\nFile %s opened!\n", fileName.c_str());

            // Read data offset - starting address of the byte where the .bmp image (pixel array) can be found (size = 4 bytes)
            uint32 dataOffset;
            fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
            fread(&dataOffset, 4, 1, imageFile);

            // Read width (size = 4 bytes)
            uint32_t width;
            fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
            fread(&width, 4, 1, imageFile);

            // Read height (size = 4 bytes)
            uint32_t height;
            fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
            fread(&height, 4, 1, imageFile);

            // Read bits per pixel = colour depth of image (size = 2 bytes)
            int16 bitsPerPixel;
            fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
            fread(&bitsPerPixel, 2, 1, imageFile);

            // Compute bytes per pixel = number of channels
            uint32 bytesPerPixel;
            bytesPerPixel = ((uint32)bitsPerPixel) / 8;

            // Resize image matrix and set image params., i.e., image.rows, image.cols etc.)
            image.resize(height, width, bytesPerPixel);

            // Set image colorspace
            // chans=1  -> GRAY
            // chans=3  and chan1=chan2=chan3 -> GRAY -> TODO: add check
            // chans=3 -> BGR)
            if (image.chans() == 1)
            {
                image.set_colorspace(GRAY);
            }
            else if (image.chans() == 3)
            {
                image.set_colorspace(BGR);
            }
            else
            {
#ifndef NDEBUG
                //throw std::invalid_argument("Number of channels can only be 1 or 3. Unavailable colorspace option for other number of channels.");
#endif
                return;
            }

            // Calculate the unpadded row size in bytes
            int unpaddedRowSize = image.cols() * bytesPerPixel;

            // Calculate the padded row size in bytes
            int paddedRowSize = image.row_stride();
            if (image.row_stride() % 4)
                paddedRowSize += 4 - (image.row_stride() % 4);
            // int paddedRowSize = (int)(4 * ceil((float)image.cols() / 4.0f)) * bytesPerPixel;

            // Calculate the total size of the pixel data in bytes
            int imageSize = unpaddedRowSize * image.rows();

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
            debug_printf("data offset: %d\n", dataOffset);
            debug_printf("image columns: %d\n", image.cols());
            debug_printf("image rows: %d\n", image.rows());
            debug_printf("image row stride: %d\n", image.row_stride());
            debug_printf("image channels: %d\n", image.chans());
            debug_printf("image colorspace: %s\n", (valColorspace[image.get_colorspace()]).c_str());
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

        #define SETSR(c) asm volatile("setsr %0" : : "n"(c));
        #define CLRSR(c) asm volatile("clrsr %0" : : "n"(c));
        portDISABLE_INTERRUPTS()
        CLRSR(XS1_SR_KEDI_MASK);

        // Open the file for reading in binary mode
        FILE *imageFile = fopen(fileName.c_str(), "wb");

        debug_printf("\n--------------------");
        debug_printf("\nWriting image to .BMP file...");

        if (imageFile != NULL) // file was opened successfully
        {
            debug_printf("\nFile %s opened!\n", fileName.c_str());

            //////////////////////// Data checks & modifications /////////////////////////////
            // TODO figure out how to make an 8-bit bitmap!
            if (image.chans() == 1)
            {
                image.resize(image.rows(), image.cols(), 3);
                vision::uncollated_replication(*image.image_data, 3, false);
            }

            // Calculate the unpadded/padded row size [bytes] (required for modifying data)
            uint32 bytesPerPixel = image.chans();
            int unpaddedRowSize = image.cols() * bytesPerPixel;

            // Calculate the padded row size in bytes
            int paddedRowSize = image.row_stride();
            if (image.row_stride() % 4)
                paddedRowSize += 4 - (image.row_stride() % 4);
            // int paddedRowSize = (int)(4 * ceil((float)image.cols() / 4.0f)) * bytesPerPixel;

            //////////////////////// Write header + data to file //////////////////////////////
            // ** Bitmap file header ** //
            // Write signature ("BM")
            const char *signature = "BM";
            fwrite(&signature[0], 1, 1, imageFile);
            fwrite(&signature[1], 1, 1, imageFile);

            // Write file size
            uint32 headerSize = HEADER_SIZE + INFO_HEADER_SIZE;
            uint32 fileSize = paddedRowSize * image.rows() + headerSize;
            fwrite(&fileSize, 4, 1, imageFile);

            // Write reserved bits
            uint32 reserved = RESERVED;
            fwrite(&reserved, 4, 1, imageFile);

            // Write data offset
            uint32 dataOffset = HEADER_SIZE + INFO_HEADER_SIZE;
            fwrite(&dataOffset, 4, 1, imageFile);

            // Write info header size
            uint32 infoHeaderSize = INFO_HEADER_SIZE;
            fwrite(&infoHeaderSize, 4, 1, imageFile);

            // Write width
            uint32_t width = image.cols();
            fwrite(&width, 4, 1, imageFile);

            // Write height
            uint32_t height = image.rows();
            fwrite(&height, 4, 1, imageFile);

            // Write planes
            uint32 planes = 1; // always 1
            fwrite(&planes, 2, 1, imageFile);

            // Write bits per pixel
            uint32 bitsPerPixel = bytesPerPixel * 8;
            fwrite(&bitsPerPixel, 2, 1, imageFile);

            // Write compression
            uint32 compression = NO_COMPRESION;
            fwrite(&compression, 4, 1, imageFile);

            // Write image size [bytes]
            uint32 imageSize = unpaddedRowSize * height;
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

            // Image info
            debug_printf("data offset: %d\n", dataOffset);
            debug_printf("image columns: %d\n", image.cols());
            debug_printf("image rows: %d\n", image.rows());
            debug_printf("image row stride: %d\n", image.row_stride());
            debug_printf("image channels: %d\n", image.chans());
            debug_printf("image colorspace: %s\n", (valColorspace[image.get_colorspace()]).c_str());
            debug_printf("bits per pixel: %d\n", bitsPerPixel);
            debug_printf("bytes per pixel: %d\n", bytesPerPixel);
            debug_printf("unpadded row size [bytes]: %d\n", unpaddedRowSize);
            debug_printf("padded row size [bytes]: %d\n", paddedRowSize);
            debug_printf("total size of (padded) data [bytes]: %d\n", imageSize);

            // ** Bitmap image data ** //
            // Write data to file
            // Flip image upside down (data is written from the end -> make begin = end)
            // TODO Jira issue MAYA-54
            flip(image, ROW);

            // Make data unsigned before saving to file
            for (auto &s : *image.image_data) // make data unsigned before saving to file
            {
                s = (sample_t)sample_to_index(s);
            }

            // Width of image has to be a multiple of 4 -> byte-pad end of rows with extra zeros
            horizontal_byte_pad(image, 0, paddedRowSize - unpaddedRowSize, 0);

            // Write data to file
            for (int i = 0; i < height; i++)
            {
                int pixelOffset = i * paddedRowSize;
                fwrite(&image.image_data->at(pixelOffset), 1, paddedRowSize, imageFile);
            }

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

        SETSR(XS1_SR_KEDI_MASK);
        portENABLE_INTERRUPTS()
    }

    // fileName: The name of the binary file to open
    // vec: A vector of data.
    template <typename T>
    void write_vec2bin_tmp(const std::string fileName, const std::vector<T> vec)
    {
        std::ofstream out_file(fileName, std::ios::binary | std::ios::out);
        if (out_file.is_open())
        {
            for (auto i : vec)
            {
                out_file.write(reinterpret_cast<const char *>(&i), sizeof(i));
            }
            out_file.close();
        }
        else
        {
            std::cout << "Could not create file " + fileName + "\n";
        }
    }

    // ****Inputs***
    // fileName: The name of the binary file to open
    // vec: A vector of uint8_t data.
    void write_vec2bin(const std::string fileName, const std::vector<uint8_t> vec)
    {
        write_vec2bin_tmp<uint8_t>(fileName, vec);
    }

    // fileName: The name of the binary file to open
    // vec: A vector of unisgned data.
    void write_vec2bin(const std::string fileName, const std::vector<unsigned> vec)
    {
        write_vec2bin_tmp<unsigned>(fileName, vec);
    }

    // ****Inputs***
    // fileName: The name of the binary file to open
    // image: Image data.
    void write_image_vec2bin(const std::string fileName, const Image &image)
    {
        uint8_t pixel_value;
        std::ofstream out_file(fileName, std::ios::binary | std::ios::out);

        debug_printf("\n--------------------");
        debug_printf("\nWriting image vector to binary file...");

        if (out_file.is_open())
        {
            for (int i = 0; i < image.image_data->size(); i++)
            {
                pixel_value = sample_to_index(image.image_data->operator[](i));
                out_file.write(reinterpret_cast<const char *>(&pixel_value), sizeof(pixel_value));
            }
            out_file.close();
            debug_printf("\n--------------------\n\n");
        }
        else
        {
            std::cout << "Could not open file" + fileName;
            debug_printf("\n--------------------\n\n");
        }
    }

    // ****Inputs***
    // fileName: The name of the binary file to open
    // box_array: Box array data = [box.top_left.row, box.top_left.col, box.extents.row, box.extents.col]
    void write_box_array2bin(const std::string fileName, const std::array<int, 4> box_array)
    {
        std::ofstream out_file(fileName, std::ios::binary | std::ios::out);

        debug_printf("\n--------------------");
        debug_printf("\nWriting box array to binary file...");

        if (out_file.is_open())
        {
            for (int i = 0; i < 4; i++)
            {
                out_file.write(reinterpret_cast<const char *>(&box_array[i]), sizeof(box_array[i]));
            }
            out_file.close();
            debug_printf("\n--------------------\n\n");
        }
        else
        {
            std::cout << "Could not open file" + fileName;
            debug_printf("\n--------------------\n\n");
        }
    }

    // This is a function to turn RGB bitmap like images into RGGB bayered images
    // simply by deleting elements
    void CheapBayerSim(Image &output, const Image &input)
    {
        output.resize(input.rows(), input.cols() * 2, 1);
        output.clear();
        for (int i = 0; i < input.rows(); i++)
        {
            for (int j = 0; j < input.cols(); j++)
            {
                output.image_data->at(output.get_sample_ind(i, 2 * j)) = input.get_sample(i, j, i % 2);
                output.image_data->at(output.get_sample_ind(i, 2 * j + 1)) = input.get_sample(i, j, 1 + i % 2);
            }
        }
    }

} // end namespace
