/* System headers */
#include <platform.h>

/* C Headers */
#include <string>
#include <limits>

/* Lib Vision Headers */
#include "Image.h"
#include "vision.h"

//This code, and all that it calls, is allocated to tile 0 because it is currently called from tile 0

extern "C" void ArrayToFile(uint8_t*, int, int, int, char*);

void ArrayToFile(uint8_t* DataPtr, int width, int height, int channels, char* filepath){

    vision::Image my_image(width,height,channels);

    //The camera data and lib vision maintain differnt definitions for what '0' is.
    for(int i = 0; i < width*height*channels; i++){
        int ResignedData = DataPtr[i];
        ResignedData += 128;
        my_image.image_data->at(i) = ResignedData;
    }
    
    std::string stringpath = filepath;
    //my_image.print();
    
    /*Put all functions you want applied to the image here*/
    dither(my_image, 3);
    //write_bitmap(stringpath, my_image, 0);
    write_image_vec2bin(stringpath, my_image);
}