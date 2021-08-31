//This is a c++ file representing an api to C code.

//It is my assumption that this file includes the C++ Headers for the
// rest of the library

//extern "C" <FunctionReturnType> <FunctionSignature>;

//#if ON_TILE(0)

#include <iostream>
#include <cstring>
#include <limits>
#include "Image.h"
#include "vision.h"

//extern "C" void* CreateImage(uint8_t*, int,int,int);
//extern "C" void WriteToDirectory(void*,char*);
extern "C" void ArrayToFile(uint8_t*, int, int, int, char*);

/*
void* CreateImage(uint8_t* DataPtr,int width,int height,int channels){
    //vision::Image my_image(width,height,channels);
    std::cout << "This has reached CreateImage Function" << std::endl;
    vision::Image* my_image;
    my_image = new vision::Image(width,height,channels);
    //Fill my_image vector with data from dataptr


    return(my_image);

}

void WriteToDirectory(void*,char*){
    /*
    Realize the void* as an image.
    Realize the Char* as a filepath
    write_bitmap(filepath,image);
    //*/
/*/
}
//*/
void ArrayToFile(uint8_t* DataPtr, int width, int height, int channels, char* filepath){
    std::cout << "This has reached ArrayToFile Function" << std::endl;

    vision::Image my_image(width,height,channels);
    
    //Note image_data is signed.
    for(int i = 0; i < sizeof(DataPtr); i++){
        int ResignedData = DataPtr[i] - std::numeric_limits<vision::sample_t>::max();
        my_image.image_data->at(i) = ResignedData;
    }

    std::string stringpath(filepath);
    std::cout << "Recieved Filepath: " << stringpath << std::endl;
    
    write_bitmap(stringpath, my_image, 1);
}

//#endif //On_Tile(0)