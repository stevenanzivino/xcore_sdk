//This is a c++ file representing an api to C code.

//It is my assumption that this file includes the C++ Headers for the
// rest of the library

//extern "C" <FunctionReturnType> <FunctionSignature>;

#include "Image.h"
#include "vision.h"

extern "C" void* CreateImage(int*, int,int,int);

void* CreateImage(int* DataPtr,int width,int height,int channels){
    vision::Image my_image(width,height,channels);

    return(&my_image);

}