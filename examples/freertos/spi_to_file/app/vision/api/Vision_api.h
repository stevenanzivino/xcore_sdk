//This is Steven's attempt to get the lib vision library to work with the Person_Detect SDK example.
//It should be noted that steven does not know what he is doing.
//https://isocpp.org/wiki/faq/mixing-c-and-cpp#cpp-higher-than-c

#ifndef Vision_Api_H
#define Vision_Api_H

//System Headers
#include <platform.h>

#if ON_TILE(0)
//void* CreateImage(uint8_t*, int,int,int);
//void WriteToDirectory(void*,char*);
void ArrayToFile(uint8_t*, int, int, int, char*);

/*
void* GetImagePointer(uint8_t* DataPtr, int width, int height, int channels){
    return(CreateImage(DataPtr, width, height, channels));
}

void printImage(void* imagePTR){
    WriteToDirectory(imagePTR, "/FILEPATH/HERE.txt");
}
*/
writeImage(uint8_t* DataPtr, int width, int height, int channels, char* writingpath){
    bool firstcall = true;

    
    while(1){
        if(firstcall){
            ArrayToFile(DataPtr,width,height,channels,writingpath);
            firstcall = false;
        }
    }
    
}
//Should probably have some Destroy image function, since another one creates it.

#endif //On_Tile(0)

#endif  // Vision_Api_H