//This is a c++ file representing an api to C code.

//It is my assumption that this file includes the C++ Headers for the
// rest of the library

//extern "C" <FunctionReturnType> <FunctionSignature>;

/* System headers */
#include <platform.h>


//#if ON_TILE(0)

//#include <iostream>
#include <string>
#include <limits>
#include "Image.h"
#include "vision.h"

extern "C" void ArrayToFile(uint8_t*, int, int, int, char*);

void ArrayToFile(uint8_t* DataPtr, int width, int height, int channels, char* filepath){

    vision::Image my_image(width,height,channels);
    
    if(std::numeric_limits<vision::sample_t>::max() == 127){
        //Data comes in unsigned and has to end up signed.
        for(int i = 0; i < width*height*channels; i++){
            int ResignedData = DataPtr[i];
            ResignedData -= std::numeric_limits<vision::sample_t>::max();
            my_image.image_data->at(i) = ResignedData;
        }
    }
    else{
        for(int i = 0; i < width*height*channels; i++){
            my_image.image_data->at(i) = DataPtr[i];
        }
    }

    my_image.print();
    
    std::string stringpath = filepath;

    write_bitmap(stringpath, my_image, 0);

}

//#endif //On_Tile(0)